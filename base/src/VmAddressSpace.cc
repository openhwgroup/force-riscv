//
// Copyright (C) [2020] Futurewei Technologies, Inc.
//
// FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
// FIT FOR A PARTICULAR PURPOSE.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <VmAddressSpace.h> //Force_VmAddressSpace_H

//Force Modules
#include <AddressReuseMode.h>
#include <Choices.h>
#include <Constraint.h>
#include <ConstraintUtils.h>
#include <Defines.h>
#include <GenException.h>
#include <GenRequest.h>
#include <Generator.h>
#include <Log.h>
#include <Memory.h>
#include <MemoryAttributes.h>
#include <MemoryConstraint.h>
#include <MemoryConstraintUpdate.h>
#include <MemoryManager.h>
#include <ObjectRegistry.h>
#include <Page.h>
#include <PageInfoRecord.h>
#include <PageRequestRegulator.h>
#include <PageTableConstraint.h>
#include <PageTableManager.h>
#include <PagingChoicesAdapter.h>
#include <PagingInfo.h>
#include <PhysicalPageManager.h>
#include <PteAttribute.h>
#include <PteStructure.h>
#include <Random.h>
#include <UtilityFunctions.h>
#include <VmConstraintValidator.h>
#include <VmFactory.h>
#include <VmPaMapper.h>
#include <VmUtils.h>
#include <VmasControlBlock.h>

//Library Modules
#include <algorithm>
#include <iomanip>
#include <memory>

using namespace std;

/*!
  \file VmAddressSpace.cc
  \brief Code supporting address space behaviors.
*/

namespace Force {


  VmAddressSpace::VmAddressSpace(const VmFactory* pFactory, VmasControlBlock* pVmasCtlrBlock)
    : VmMapper(pFactory), Object(), mpControlBlock(pVmasCtlrBlock), mpLookUpPage(nullptr), mpDefaultPageRequest(nullptr),  mpVirtualUsable(nullptr), mPages(), mPhysicalRegions(), mNoTablePages(), mVmConstraints(), mPageTableConstraints(), mFlatMapped(false)
  {

  }

  VmAddressSpace::VmAddressSpace()
    : VmMapper(), Object(), mpControlBlock(nullptr), mpLookUpPage(nullptr), mpDefaultPageRequest(nullptr),  mpVirtualUsable(nullptr), mPages(), mPhysicalRegions(), mNoTablePages(), mVmConstraints(), mPageTableConstraints(), mFlatMapped(false)
  {
  }

  VmAddressSpace::VmAddressSpace(const VmAddressSpace& rOther)
    : VmMapper(rOther), Object(rOther), mpControlBlock(nullptr), mpLookUpPage(nullptr), mpDefaultPageRequest(nullptr),  mpVirtualUsable(nullptr), mPages(), mPhysicalRegions(), mNoTablePages(), mVmConstraints(), mPageTableConstraints(), mFlatMapped(false)
  {
    if (nullptr != rOther.mpControlBlock) {
      mpControlBlock = dynamic_cast<VmasControlBlock* > (rOther.mpControlBlock->Clone());
    }
  }

  VmAddressSpace::~VmAddressSpace()
  {
    delete mpControlBlock;

    // delete Pages and page tables.
    delete mpLookUpPage;
    delete mpDefaultPageRequest;
    delete mpVirtualUsable;

    if (mPhysicalRegions.size()) {
      LOG(fail) << "{VmAddressSpace::~VmAddressSpace} physical regions not all mapped." << endl;
      FAIL("physical-regions-not-all-mapped");
    }

    for (auto no_table_page : mNoTablePages) {
      delete no_table_page;
    }

    for (auto vm_constr : mVmConstraints) {
      delete vm_constr;
    }
  }

  void VmAddressSpace::Setup(Generator* gen)
  {
    VmMapper::Setup(gen);

    if (mpVirtualUsable == nullptr) {
      mpVirtualUsable = new SingleThreadMemoryConstraint(mpGenerator->ThreadId());
    }

    if (nullptr != mpControlBlock)
    {
      mpControlBlock->Setup(gen);
    }

    mpLookUpPage = PageInstance();
    mpVmFactory->SetupVmConstraints(mVmConstraints);

    if (mpControlBlock->GetChoicesAdapter()->GetPlainPagingChoice("Page Allocation Scheme") == 1)
    {
      mFlatMapped = true;
      LOG(notice) << "{VmAddressSpace::Setup} flat mapped allocation scheme enabled" << endl;
    }
    else
    {
      mFlatMapped = false;
      LOG(notice) << "{VmAddressSpace::Setup} random allocation scheme enabled" << endl;
    }

    SetupControlBlock();
    SetupPageTableConstraints();
    AddAddressErrorPages();
    MapPhysicalRegions();
  }

  void VmAddressSpace::SetupPageTableConstraints()
  {
    mpVmFactory->CreatePageTableConstraints(mPageTableConstraints);
    for (auto constr : mPageTableConstraints)
    {
      constr->Setup(mpGenerator, mpControlBlock);
    }
  }

  void VmAddressSpace::SetupControlBlock()
  {
    mpControlBlock->Initialize();
    //mpControlBlock->InitializeRootPageTable(this);

    auto mem_manager   = mpGenerator->GetMemoryManager();
    auto ptable_manager = mem_manager->GetPageTableManager(DefaultMemoryBank());

    bool alloc = ptable_manager->AllocateRootPageTable(this);

    if (!alloc)
    {
      LOG(fail) << "{VmAddressSpace::SetupControlBlock} unable to alias existing or create a new root page table." << endl;
      FAIL("root_page_table_alloc_fail");
    }

    ptable_manager->CommitRootPageTable(mpControlBlock->GetRootPageTable());

    unique_ptr<ConstraintSet> init_virtual_usable(mpControlBlock->InitialVirtualConstraint());

    // Exclude flat mapped page table region from the initial virtual constraint
    for (PageTableConstraint* page_table_constr : mPageTableConstraints) {
      init_virtual_usable->SubConstraintSet(*(page_table_constr->BlockAllocated()));
    }

    mpVirtualUsable->Initialize(*init_virtual_usable);
  }

  bool VmAddressSpace::InitializeRootPageTable(RootPageTable* pRootTable)
  {
    return mpControlBlock->InitializeRootPageTable(this, pRootTable);
  }

  bool VmAddressSpace::CompatiblePageTableContext(const VmAddressSpace* pOtherVmas) const
  {
    //use control block of other vmas and call comparison against this Vmas control block, return true if the two contexts can share all mappings
    //return mpControlBlock->CompatiblePageTableContext(pOtherVmas->GetControlBlock());
    return true;
  }

  void VmAddressSpace::AddAddressErrorPages()
  {
    // Populate address error address ranges.
    vector<TranslationRange> addr_error_ranges;
    mpControlBlock->GetAddressErrorRanges(addr_error_ranges);
    auto last_iter = mPages.end();
    auto addr_error_constr = mVmConstraints[uint32(EVmConstraintType::AddressError)];

    for (const auto & addr_error_range : addr_error_ranges) {
      auto addr_err_page = new AddressErrorPage();
      addr_err_page->SetBoundary(addr_error_range.Lower(), addr_error_range.Upper());
      addr_err_page->SetPhysicalBoundary(addr_error_range.PhysicalLower(), addr_error_range.PhysicalUpper());
      addr_err_page->SetMemoryBank(addr_error_range.MemoryBank());
      addr_error_constr->AddRange(addr_error_range.Lower(), addr_error_range.Upper());

      LOG(info) << "{VmAddressSpace::AddAddressErrorPages} adding page " << addr_err_page->VaRangeString() << endl;

      auto find_iter = lower_bound(mPages.begin(), last_iter, addr_err_page, compare_pages);
      if (find_iter == last_iter) { // no match found.
        mPages.insert(last_iter, addr_err_page);
      }
      else if ((*find_iter)->Overlaps(*addr_err_page)) {
        LOG(fail) << "{VmAddressSpace::AddAddressErrorPages} adding page " << addr_err_page->VaRangeString() << " overlaps with " << (*find_iter)->VaRangeString() << endl;
        FAIL("add-page-overlaps-existing");
      }
      else { // not overlapping
        mPages.insert(find_iter, addr_err_page);
      }
      mNoTablePages.push_back(addr_err_page);
    }
  }

  void VmAddressSpace::PopulateVirtualUsableConstraint()
  {
    unique_ptr<ConstraintSet> init_virtual_usable(mpControlBlock->InitialVirtualConstraint());

    // Exclude flat mapped page table region from the initial virtual constraint
    for (PageTableConstraint* page_table_constr : mPageTableConstraints) {
      init_virtual_usable->SubConstraintSet(*(page_table_constr->BlockAllocated()));
    }

    if (mFlatMapped)
    {
      auto usable_constr = mpControlBlock->GetPhysicalUsableConstraint();

      if (usable_constr != nullptr)
      {
        init_virtual_usable->ApplyConstraintSet(*usable_constr);
      }
      else {
        LOG(fail) << "{VmAddressSpace::PopulateVirtualUsableConstraint} physical usable not initialized" << endl;
        FAIL("v-constr-setup-phys-usable-not-initialized");
      }

      delete usable_constr;
    }

    mpVirtualUsable->Initialize(*init_virtual_usable);

    for (auto page : mPages)
    {
      UpdateVirtualUsableByPage(page);
    }

    auto cset_s_init_vir_usable = ConstraintSetSerializer(*init_virtual_usable, FORCE_CSET_DEFAULT_PERLINE);
    LOG(trace) << "{VmAddressSpace::PopulateVirtualUsableConstraint} final vir_usable=" << cset_s_init_vir_usable.ToDebugString() << endl;
  }

  void VmAddressSpace::UpdateVirtualUsableByPage(const Page* pPage)
  {
    if (!mpVirtualUsable->IsInitialized())
    {
      LOG(fail) << "{VmAddressSpace::UpdateVirtualUsableByPage} calling with empty virtual constraints" << endl;
      FAIL("update-vir-usable-by-page-empty-constr");
    }

    LOG(debug) << "{VmAddressSpace::UpdateVirtualUsableByPage} page v_lower=0x" << hex << pPage->Lower() << " v_upper=0x"
               << pPage->Upper() << " p_lower=0x" << pPage->PhysicalLower() << " p_upper=0x" << pPage->PhysicalUpper()
               << " p_bank=" << EMemBankType_to_string(pPage->MemoryBank()) << endl;

    auto mem_manager   = mpGenerator->GetMemoryManager();
    auto mem_bank      = mem_manager->GetMemoryBank(uint32(pPage->MemoryBank()));
    auto usable_constr = mem_bank->Usable();

    //auto cset_s_phys_usable = ConstraintSetSerializer(*usable_constr, FORCE_CSET_DEFAULT_PERLINE);
    //auto cset_s_vir_usable = ConstraintSetSerializer(*mpVirtualUsable->Usable(), FORCE_CSET_DEFAULT_PERLINE);

    // << "UVUBP phys_usable_constr: " << cset_s_phys_usable.ToDebugString() << endl;
    // << "UVUBP vir_usable_constr: " << cset_s_vir_usable.ToDebugString() << endl;

    //we only need to modify the existing constraints if the page is partially usable
    if (not usable_constr->ContainsRange(pPage->PhysicalLower(), pPage->PhysicalUpper())) {
      //get any physical usable space in the page's physical address range
      ConstraintSet phys_page_usable;
      bool any_page_usable = usable_constr->CopyInRange(pPage->PhysicalLower(), pPage->PhysicalUpper(), phys_page_usable);

      LOG(debug) << "{VmAddressSpace::UpdateVirtualUsableByPage} any_page_usable=" << any_page_usable << endl;
      //add any usable physical space back into virtual usable, translating physical to virtual before adding back in
      if (any_page_usable) {
        //auto cset_s_phys_page_usable = ConstraintSetSerializer(phys_page_usable, FORCE_CSET_DEFAULT_PERLINE);
        //LOG(debug) << "{VmAddressSpace::UpdateVirtualUsableByPage} phys_page_usable: " << cset_s_phys_page_usable.ToDebugString() << endl;
        uint64 mask  = (pPage->PageSize() - 0x1ull);
        uint64 frame = pPage->Lower() & (~mask);
        //LOG(debug) << "{VmAddressSpace::UpdateVirtualUsableByPage} mask=0x" << mask << " frame=0x" << frame << endl;
        phys_page_usable.Translate(mask, frame);
        //LOG(debug) << "{VmAddressSpace::UpdateVirtualUsableByPage} post translate phys_page_usable:" << cset_s_phys_page_usable.ToDebugString() << endl;

        mpVirtualUsable->ReplaceUsableInRange(pPage->Lower(), pPage->Upper(), phys_page_usable);
      }
      else {
        //sub whole page range from virtual constraints
        mpVirtualUsable->MarkUsed(pPage->Lower(), pPage->Upper());
      }
    }

    UpdateVirtualSharedByPage(pPage);

    //auto cset_s_vir_usable = ConstraintSetSerializer(*mpVirtualUsable->Usable(), FORCE_CSET_DEFAULT_PERLINE);
    //LOG(debug) << "{VmAddressSpace::UpdateVirtualUsableByPage} end vir_usable: " << cset_s_vir_usable.ToDebugString() << endl;
  }

  Page* VmAddressSpace::PageInstance() const
  {
    return new Page();
  }

  uint32 VmAddressSpace::Asid() const
  {
    return mpControlBlock->Asid();
  }

  const ConstraintSet* VmAddressSpace::VirtualUsableConstraintSet(bool isInstr) const
  {
    // << " virtual usable called " << mpControlBlock->Type() << " EL: " <<  EPrivilegeLevelType_to_string(mpControlBlock->ExceptionLevel()) << " : flat map? " << mFlatMapped << endl;
    if (!mpVirtualUsable->IsInitialized())
    {
      LOG(fail) << "{VmAddressSpace::VirtualUsableConstraintSet} virtual memory constraint uninitialized" << endl;
      FAIL("vir-mem-constr-uninit");
    }

    return mpVirtualUsable->Usable();
  }

  ConstraintSet* VmAddressSpace::VirtualUsableConstraintSetClone(bool isInstr)
  {
    // << " virtual usable clone called " << mpControlBlock->Type() << " EL: " <<  EPrivilegeLevelType_to_string(mpControlBlock->ExceptionLevel()) << " : flat map? " << mFlatMapped << endl;
    if (!mpVirtualUsable->IsInitialized())
    {
      LOG(fail) << "{VmAddressSpace::VirtualUsableConstraintSetClone} virtual memory constraint uninitialized" << endl;
      FAIL("vir-mem-constr-uninit");
    }

    return mpVirtualUsable->Usable()->Clone();
  }

  const ConstraintSet* VmAddressSpace::VirtualSharedConstraintSet() const
  {
    if (!mpVirtualUsable->IsInitialized())
    {
      LOG(fail) << "{VmAddressSpace::VirtualSharedConstraintSet} virtual memory constraint uninitialized" << endl;
      FAIL("vir-mem-constr-uninit");
    }

    return mpVirtualUsable->Shared();
  }

  void VmAddressSpace::ApplyVirtualUsableConstraint(const EMemDataType memDataType, const EMemAccessType memAccessType, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const
  {
    if (!mpVirtualUsable->IsInitialized())
    {
      LOG(fail) << "{VmAddressSpace::ApplyVirtualUsableConstraint} virtual memory constraint uninitialized" << endl;
      FAIL("vir-mem-constr-uninit");
    }

    mpVirtualUsable->ApplyToConstraintSet(memDataType, memAccessType, mpGenerator->ThreadId(), rAddrReuseMode, constrSet);
  }

  void VmAddressSpace::MapEssentialPhysicalRegions()
  {
    auto mem_manager = mpGenerator->GetMemoryManager();
    auto region_vec = mpControlBlock->FilterEssentialPhysicalRegions(mem_manager->GetPhysicalRegions());

    for (auto phys_region : region_vec)
    {
      bool region_compatible = MapPhysicalRegion(phys_region);
      if (not region_compatible) {
        LOG(fail) << "{VmAddressSpace::MapEssentialPhysicalRegions} incompatible region: " << phys_region->ToString() << endl;
        FAIL("mapping-incompatible-physical-region");
      }
    }
  }

  void VmAddressSpace::Activate()
  {
    if (!IsInitialized())
    {
      Initialize();
    }

    if (!IsActive())
    {
      //PopulateVirtualUsableConstraint();
      //MapEssentialPhysicalRegions();
      mState = EVmStateType::Active;
    }
  }

  void VmAddressSpace::Initialize()
  {
    if (!IsInitialized())
    {
      PopulateVirtualUsableConstraint();
      MapEssentialPhysicalRegions();
      mState = EVmStateType::Initialized;
    }
  }

  void VmAddressSpace::Deactivate()
  {
    if (mState != EVmStateType::Uninitialized)
    {
      mState = EVmStateType::Uninitialized;
      mpVirtualUsable->Uninitialize();
    }
  }

  bool VmAddressSpace::MapAddressRange(uint64 VA, uint64 size, bool isInstr, const GenPageRequest* pPageReq)
  {
    if (size == 0) {
      LOG(fail) << "{VmAddressSpace::MapAddressRange} address range size cannot be 0." << endl;
      FAIL("mapping-range-with-size-0");
    }

    bool any_new_page_alloc = false;
    uint64 map_va = VA;
    uint64 size_remaining = size;

    do {
      LOG(info) << "{VmAddressSpace::MapAddressRange} mapping address 0x" << hex << map_va << " remaining_size 0x" << size_remaining << endl;

      bool new_page_alloc = false;
      const Page* map_page = nullptr;

      try
      {
        map_page = MapAddress(map_va, size_remaining, isInstr, pPageReq, new_page_alloc);
      }
      catch (const PagingError& err)
      {
        LOG(fail) << "{VmAddressSpace::MapAddressRange} Unable to map address range VA=0x" << hex << VA << " size=0x" << size << " Paging Error: " << err.what() << endl;
        FAIL("paging-error-in-map-address-range");
      }

      any_new_page_alloc |= new_page_alloc;

      uint64 map_size = (map_page->Upper() - map_va + 1);
      if (map_size >= size_remaining)
      {
        break;
      }
      size_remaining -= map_size;
      map_va += map_size;
    }
    while (size_remaining);

    MapPhysicalRegions();

    return any_new_page_alloc;
  }

  uint64 VmAddressSpace::MapAddressRangeForPA(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, const GenPageRequest* pPageReq)
  {
    if (size == 0)
    {
      LOG(fail) << "{VmAddressSpace::MapAddressRangeForPA} address range size cannot be 0." << endl;
      FAIL("mapping-range-with-size-0");
    }

    uint64 map_pa = PA;
    uint64 size_remaining = size;
    uint64 start_va = 0;
    bool start_va_set = false;

    LOG(trace) << "{VmAddressSpace::MapAddressRangeForPA} pa=0x" << PA << " size=0x" << size_remaining;

    do {
      LOG(info) << "{VmAddressSpace::MapAddressRangeForPA} mapping address 0x" << hex << map_pa << "[" << uint32(bank) << "]" << " remaining size 0x" << size_remaining << endl;

      const Page* map_page = nullptr;

      try
      {
        map_page = MapAddressForPA(map_pa, bank, size_remaining, isInstr, pPageReq);
      }
      catch (const PagingError& err)
      {
        LOG(fail) << "{VmAddressSpace::MapAddressRangeForPA} Unable to map address range PA=0x" << hex << PA << " size=0x" << size << " mem_bank=" << EMemBankType_to_string(bank) << " Paging Error: " << err.what() << endl;
        FAIL("paging-error-in-map-address-range-for-pa");
      }

      if (!start_va_set)
      {
        map_page->TranslatePaToVa(PA, bank, start_va);
        start_va_set = true;
      }

      //uint64 map_size = (map_page->Upper() - start_va + 1);
      uint64 map_size = (map_page->PhysicalUpper() - map_pa + 1);

      if (map_size >= size_remaining) break;
      size_remaining -= map_size;
      map_pa += map_size;
    }
    while (size_remaining);

    MapPhysicalRegions();

    return start_va;
  }

  void VmAddressSpace::MapPhysicalRegions()
  {
    vector<PhysicalRegion* > phys_regions;
    phys_regions.swap(mPhysicalRegions);

    for (auto phys_region : phys_regions) {
      bool region_compatible = MapPhysicalRegion(phys_region);

      if (not region_compatible) {
        LOG(fail) << "{VmAddressSpace::MapPhysicalRegions} not expecting incompatible regions: " << phys_region->ToString() << endl;
        FAIL("unexpected-incompatible-physical-region");
      }

      delete phys_region;
    }
  }

  bool VmAddressSpace::MapPhysicalRegion(const PhysicalRegion* pPhysRegion)
  {
    bool region_compatible = false;
    unique_ptr<GenPageRequest> page_req_storage(mpControlBlock->PhysicalRegionPageRequest(pPhysRegion, region_compatible));

    auto regulator = mpGenerator->GetPageRequestRegulator();
    regulator->RegulatePageRequest(this, page_req_storage.get());

    LOG(info) << "{VmAddressSpace::MapPhysicalRegion} mapping region " << pPhysRegion->ToString() << ", compatible=" << region_compatible << " PrivilegeLevel=" << EPrivilegeLevelType_to_string(mpControlBlock->PrivilegeLevel()) << endl;

    if (region_compatible) {
      bool is_instr = (pPhysRegion->DataType() == EMemDataType::Instruction);
      MapAddressRange(pPhysRegion->Lower(), pPhysRegion->Size(), is_instr, page_req_storage.get());
      if (not mpControlBlock->PhysicalRegionAttributeCompatibility(pPhysRegion, this))
      {
        LOG(fail) << "{VmAddressSpace::MapPhysicalRegion} mapping region " << pPhysRegion->ToString() << ", attribute are incompatible." << endl;
        FAIL("physical-region-attribute-incompatible");
      }
    }

    return region_compatible;
  }

  ETranslationResultType VmAddressSpace::TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const
  {
    ETranslationResultType trans_result = ETranslationResultType::NotMapped;
    const Page* trans_page = GetPage(VA);
    if (nullptr != trans_page) {
      trans_result = trans_page->TranslateVaToPa(VA, PA, bank);
    }
    return trans_result;
  }

  ETranslationResultType VmAddressSpace::TranslatePaToVa(uint64 PA, EMemBankType bank, uint64& VA) const
  {
    ETranslationResultType trans_result = ETranslationResultType::NotMapped;
    auto mem_manager       = mpGenerator->GetMemoryManager();

    const PhysicalPageManager* phys_page_manager = mem_manager->GetPhysicalPageManager(bank);
    const Page* trans_page = phys_page_manager->GetVirtualPage(PA, this);

    if (nullptr != trans_page)
    {
      trans_page->TranslatePaToVa(PA, bank, VA);
      if (nullptr != GetPage(VA))
      {
        trans_result = ETranslationResultType::Mapped;
      }
    }

    return trans_result;
  }

  const Page* VmAddressSpace::GetPage(uint64 VA) const
  {
    mpLookUpPage->SetBoundary(VA, VA); // same value for lower and upper boundaries.

    auto last_iter = mPages.end();
    auto find_iter = lower_bound(mPages.begin(), last_iter, mpLookUpPage, compare_pages);
    if (find_iter == last_iter) {
      // no match found.
      return nullptr;
    }

    if ((*find_iter)->Lower() <= VA) {
      //LOG(debug) << "{VmAddressSpace::GetPage} found page " << (*find_iter)->ToString() << " for 0x" << hex << VA << endl;
      return (*find_iter);
    }

    return nullptr;
  }

  const Page* VmAddressSpace::GetPageWithAssert(uint64 VA) const
  {
    mpLookUpPage->SetBoundary(VA, VA); // same value for lower and upper boundaries.

    auto last_iter = mPages.end();
    auto find_iter = lower_bound(mPages.begin(), last_iter, mpLookUpPage, compare_pages);
    if (find_iter != last_iter) {
      // match found
      if ((*find_iter)->Lower() <= VA) {
        //LOG(debug) << "{VmAddressSpace::GetPageWithAssert} found page " << (*find_iter)->ToString() << " for 0x" << hex << VA << endl;
        return (*find_iter);
      }
    }

    // no match found, report an error
    LOG(fail) << "{VmAddressSpace::GetPageWithAssert} expecting virtual address 0x" << hex << VA << " to be mapped at this point, please investigate." << endl;
    FAIL("va-shoulda-been-but-not-mapped");

    return nullptr;
  }

  bool VmAddressSpace::GetTranslationRange(uint64 VA, TranslationRange& rTransRange) const
  {
    bool valid = false;
    const Page* trans_page = GetPage(VA);
    if (nullptr != trans_page) {
      trans_page->GetTranslationRange(rTransRange);
      valid = true;
    }
    return valid;
  }

  uint64 VmAddressSpace::MaxVirtualAddress() const
  {
    return mpControlBlock->MaxVirtualAddress(mFlatMapped);
  }

  uint64 VmAddressSpace::MaxPhysicalAddress() const
  {
    return mpControlBlock->MaxPhysicalAddress();
  }

  const Page* VmAddressSpace::MapAddress(uint64 VA, uint64 size, bool isInstr, const GenPageRequest* pPageReq, bool& newAlloc)
  {
    const Page* map_page = GetPage(VA);
    if (nullptr != map_page)
    {
      return map_page;
    }

    if (nullptr == pPageReq)
    {
      pPageReq = DefaultPageRequest(isInstr);
    }

    bool is_sys_page = false;
    const ConstraintSet* sys_page_choice = pPageReq->PteAttributeConstraint(EPteAttributeType::SystemPage);
    if (sys_page_choice != nullptr)
    {
      is_sys_page = (sys_page_choice->ChooseValue());
    }

    GenPageRequest* updated_page_req = pPageReq->Clone();
    std::unique_ptr<GenPageRequest> page_req_storage(updated_page_req); // delete object when going out of scope.

    const Page* page_obj = SetupPageMapping(VA, size, is_sys_page, updated_page_req);
    CommitPage(page_obj, size);

    newAlloc = true; //set flag for new page allocation on successful commit
    return page_obj;
  }

  const Page* VmAddressSpace::MapAddressForPA(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, const GenPageRequest* pPageReq)
  {
    uint64 VA = 0;
    //see if PA to VA mapping exists for this VM address space
    bool mapping_exists = (ETranslationResultType::Mapped == TranslatePaToVa(PA, bank, VA));

    if (nullptr == pPageReq)
    {
      pPageReq = DefaultPageRequest(isInstr);
    }

    GenPageRequest* local_page_req = pPageReq->Clone();
    std::unique_ptr<GenPageRequest> page_req_storage(local_page_req);

    // Update the PA for mappings that span multiple pages
    local_page_req->SetAttributeValue(EPageRequestAttributeType::PA, PA);

    //Override flat mapped attribute setting if global enable is on
    if (mFlatMapped) {
      local_page_req->SetGenBoolAttribute(EPageGenBoolAttrType::FlatMap, true);
    }

    bool is_flat_map = false;
    pPageReq->GetGenBoolAttribute(EPageGenBoolAttrType::FlatMap, is_flat_map);

    bool force_new_addr = false;
    pPageReq->GetGenBoolAttribute(EPageGenBoolAttrType::ForceNewAddr, force_new_addr);

    //if it does exist, can return page it is mapped to.
    //if it exists, but force new addr, see if we can alias. (!flatmap case)
    if (!force_new_addr)
    {
      if (mapping_exists) return GetPage(VA);
    }
    else
    {
      if (mapping_exists && is_flat_map && (PA == VA))
      {
        LOG(fail) << "{VmAddressSpace::MapAddressForPA} new addr forced, and flat mapping to PA already exists. PA=0x" << hex << PA;
        FAIL("new-addr-forced-flat-mapping-already-exists");
      }
      else if (mapping_exists && !is_flat_map)
      {
        //set forced alias flag, if can't alias gen should fail
        local_page_req->SetGenBoolAttribute(EPageGenBoolAttrType::ForceAlias, true);
      }
    }

    //if it doesn't exist, attempt to find a VA mapping for the PA during SetupPageMappingForPA.
    const Page* page_obj = SetupPageMappingForPA(PA, bank, size, isInstr, local_page_req);
    CommitPage(page_obj, size);

    return page_obj;
  }

  const string VmAddressSpace::ControlBlockInfo() const
  {
    return mpControlBlock->ToString();
  }

  const Page* VmAddressSpace::SetupPageMapping(uint64 VA, uint64 size, bool isSysPage, GenPageRequest* pPageReq)
  {
    const string& granule_suffix = mpControlBlock->PageGranuleSuffix(VA);
    ChoiceTree* choices_tree = nullptr;
    if (isSysPage)
      choices_tree = mpControlBlock->GetChoicesAdapter()->GetSystemPageSizeChoiceTree(granule_suffix);
    else
      choices_tree = mpControlBlock->GetChoicesAdapter()->GetPageSizeChoiceTree(granule_suffix);

    std::unique_ptr<ChoiceTree> choices_tree_storage(choices_tree);

    const Page* ret_page = nullptr;
    PageSizeInfo psize_info;

    psize_info.UpdateMaxPhysical(mpControlBlock->MaxPhysicalAddress());

    try
    {
      while (true)
      {
        auto  chosen_ptr       = choices_tree->ChooseMutable();
        const string& size_str = chosen_ptr->Name();
        PageSizeInfo::StringToPageSizeInfo(size_str, psize_info);

        string err_msg;
        ret_page = CreatePage(VA, size, pPageReq, psize_info, err_msg);
        if (nullptr == ret_page) {
          LOG(info) << "{VmPaMapper::SetupPageMapping} failed to create page for VA=0x" << hex << VA << " size=" << dec << size << ", due to: " << err_msg << endl;
          chosen_ptr->SetWeight(0);
          continue;
        }

        break;
      }
    }
    catch (const ChoicesError& choices_error)
    {
      stringstream err_stream;
      err_stream << "Paging selection - failed to pick choice: " << choices_error.what();
      throw PagingError(err_stream.str());
    }

    return ret_page;
  }

  const Page* VmAddressSpace::SetupPageMappingForPA(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, GenPageRequest* pPageReq)
  {
    VmPaMapper * pa_mapper = mpVmFactory->VmPaMapperInstance(this);
    std::unique_ptr<VmPaMapper> pa_mapper_storage(pa_mapper);
    string err_msg;
    const Page* ret_page = pa_mapper->SetupPageMapping(PA, bank, size, isInstr, pPageReq, err_msg);

    if (nullptr == ret_page) {
      stringstream err_stream;
      err_stream << "SetupPageMappingForPA: " << err_msg;
      throw PagingError(err_stream.str());
    }
    return ret_page;
  }

  const Page* VmAddressSpace::CreatePage(uint64 VA, uint64 size, GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo, string& rErrMsg)
  {
    rSizeInfo.UpdateStart(VA);
    if (not VirtualMappingAvailable(rSizeInfo.Start(), rSizeInfo.End())) {
      rErrMsg += string_snprintf(128, "{VmAddressSpace::CreatePage} page for VA=%#llx will overlap existing pages with page size: ", VA) + EPteType_to_string(rSizeInfo.mType);
      return nullptr;
    }

    auto size_in_page = rSizeInfo.GetSizeInPage(VA, size);
    LOG(info) << "{VmAddressSpace::CreatePage} page type=" << EPteType_to_string(rSizeInfo.mType) << " VA=0x" << hex << VA << " size=0x" << size << " size-in-page=0x" << size_in_page << endl;

    string pte_id_str = mpControlBlock->PagePteIdentifier(rSizeInfo.mType, VA);
    auto pte_struct = mpGenerator->GetPagingInfo()->LookUpPte(pte_id_str);

    ObjectRegistry* obj_registry = ObjectRegistry::Instance();
    Page* ret_page = obj_registry->TypeInstance<Page>(pte_struct->mClass);
    ret_page->Initialize(pte_struct, mpVmFactory);
    ret_page->SetBoundary(rSizeInfo.Start(), rSizeInfo.End());

    EMemBankType target_mem_bank = mpControlBlock->GetTargetMemoryBank(VA, pPageReq, ret_page, mVmConstraints);
    if (not AllocatePhysicalPage(VA, size, pPageReq, rSizeInfo, target_mem_bank, mpControlBlock->GetChoicesAdapter())) {
      rErrMsg += "{VmAddressSpace::CreatePage} failed to allocate physical page";
      delete ret_page;
      ret_page = nullptr;
      return nullptr;
    }

    RootPageTable* root_table = mpControlBlock->GetRootPageTable(VA);

    ret_page->SetPhysicalBoundary(rSizeInfo.PhysicalStart(), rSizeInfo.PhysicalEnd());
    ret_page->SetMemoryBank(target_mem_bank);
    ret_page->SetRootPageTable(root_table);
    ret_page->Generate(*pPageReq, *this);

    root_table->ConstructPageTableWalk(VA, ret_page, this, *pPageReq);

    return ret_page;
  }

  bool VmAddressSpace::VirtualMappingAvailable(uint64 start, uint64 end) const
  {
    mpLookUpPage->SetBoundary(start, end);

    auto find_iter = lower_bound(mPages.begin(), mPages.end(), mpLookUpPage, compare_pages);
    if (find_iter != mPages.end())
    {
      if ((*find_iter)->Overlaps(*mpLookUpPage))
      {
        return false;
      }
    }

    return true;
  }

  void VmAddressSpace::CommitPage(const Page* pageObj, uint64 size)
  {
    LOG(notice) << "{VmAddressSpace::CommitPage} Committing " << pageObj->ToString() << endl;

    auto mem_manager   = mpGenerator->GetMemoryManager();
    auto mem_bank_type = pageObj->MemoryBank();

    auto last_iter = mPages.end();
    auto find_iter = lower_bound(mPages.begin(), last_iter, pageObj, compare_pages);

    // sanity check, if page being committed overlap with existing pages.
    // case 1 (find_iter == last_iter) means no match found.
    if (find_iter != last_iter) {
      // found lower bound, check if really overlap
      if ((*find_iter)->Overlaps(*pageObj)) {
        LOG(fail) << "{VmAddressSpace::CommitPage} committing page " << pageObj->VaRangeString() << " overlaps with " << (*find_iter)->VaRangeString() << endl;
        FAIL("commit-page-overlaps-existing");
      }
    }

    // not overlapping
    mPages.insert(find_iter, pageObj);
    if (mpVirtualUsable->IsInitialized()) {
      UpdateVirtualUsableByPage(pageObj);
    }

    auto phys_page_manager = mem_manager->GetPhysicalPageManager(mem_bank_type);
    phys_page_manager->CommitPage(pageObj, size);
    mpControlBlock->CommitPage(pageObj, mVmConstraints);
  }

  bool VmAddressSpace::AllocatePhysicalPage(uint64 VA, uint64 size, GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo, EMemBankType memBank, const PagingChoicesAdapter* pChoicesAdapter)
  {
    auto mem_manager   = mpGenerator->GetMemoryManager();
    auto phys_page_manager = mem_manager->GetPhysicalPageManager(memBank);

    return phys_page_manager->AllocatePage(VA, size, pPageReq, rSizeInfo, pChoicesAdapter);
  }

  GenPageRequest* VmAddressSpace::DefaultPageRequest(bool isInstr) const
  {
    if (nullptr != mpDefaultPageRequest) {
      delete mpDefaultPageRequest;
    }

    mpDefaultPageRequest = mpGenerator->GenPageRequestInstance(isInstr);
    if (mFlatMapped) {
      mpDefaultPageRequest->SetGenBoolAttribute(EPageGenBoolAttrType::FlatMap, true);
    }

    auto regulator = mpGenerator->GetPageRequestRegulator();
    if (isInstr) {
      regulator->RegulateBranchPageRequest(this, nullptr, mpDefaultPageRequest);
    }
    else {
      regulator->RegulateLoadStorePageRequest(this, nullptr, mpDefaultPageRequest);
    }

    return mpDefaultPageRequest;
  }

  TablePte* VmAddressSpace::CreateNextLevelTable(uint64 VA, const PageTable* parentTable, const GenPageRequest& pPageReq)
  {
    // find out next level table memory bank
    EMemBankType new_mem_type = GetControlBlock()->NextLevelTableMemoryBank(parentTable, pPageReq);

    // allocate page table
    PageTableConstraint* page_table_constr = mPageTableConstraints[EMemBankTypeBaseType(new_mem_type)];
    uint64 tb_size = mpControlBlock->GetRootPageTable(VA)->TableSize();
    std::unique_ptr<ConstraintSet> exclude_region_ptr(mpControlBlock->GetPageTableExcludeRegion(VA, new_mem_type, mVmConstraints));
    uint64 tb_start = page_table_constr->AllocatePageTable(tb_size, tb_size, exclude_region_ptr.get());

    // Remove any page table block addresses that may have been allocated from the initial virtual
    // usable constraint
    mpVirtualUsable->MarkUsed(*(page_table_constr->BlockAllocated()));

    // instantiate TablePte object
    auto pte_id_str = mpControlBlock->GetRootPageTable(VA)->TableIdentifier();
    auto pte_struct = mpGenerator->GetPagingInfo()->LookUpPte(pte_id_str);
    ObjectRegistry* obj_registry = ObjectRegistry::Instance();
    TablePte* table_obj = obj_registry->TypeInstance<TablePte>(pte_struct->mClass);

    // setup page table details.
    table_obj->Initialize(pte_struct, mpVmFactory);
    table_obj->SetTableBase(tb_start);
    table_obj->SetPhysicalBoundary(tb_start, tb_start + (tb_size - 1));
    uint32 parent_low_bit = parentTable->LowestLookUpBit();
    uint32 table_step = mpControlBlock->GetRootPageTable(VA)->TableStep();
    table_obj->SetLookUpBitRange(parent_low_bit - table_step, parent_low_bit - 1);
    table_obj->SetLevel(parentTable->TableLevel() - 1);
    table_obj->SetMemoryBank(new_mem_type);
    table_obj->Generate(pPageReq, *this);

    mpControlBlock->CommitPageTable(VA, parentTable, table_obj, mVmConstraints);

    // register page table region for mapping afterwards.
    mPhysicalRegions.push_back(new PhysicalRegion(table_obj->PhysicalLower(), table_obj->PhysicalUpper(), EPhysicalRegionType::PageTable, table_obj->MemoryBank(), EMemDataType::Data, VA));
    mpControlBlock->CommitPageTable(VA, parentTable, table_obj, mVmConstraints);

    return table_obj;
  }

  void VmAddressSpace::WriteDescriptor(uint64 descrAddr, EMemBankType memBankType, uint64 descrValue, uint32 descrSize)
  {
    // write to memory with parametes: --   address    memory-bank          bytes      value       data   endian --//
    mpGenerator->InitializeMemoryWithEndian(descrAddr, uint32(memBankType), descrSize, descrValue, false, mpControlBlock->IsBigEndian());
  }

  void VmAddressSpace::UpdatePageTableConstraint(uint64 low, uint64 high)
  {
    if (mVmConstraints[uint32(EVmConstraintType::PageTable)] == nullptr)
    {
      LOG(fail) << "{VmAddressSpace::UpdatePageTableConstraint} page table constraint not initialized" << endl;
      FAIL("no-page-table-constraint");
    }
    mVmConstraints[uint32(EVmConstraintType::PageTable)]->AddRange(low, high);
    LOG(info) << "{VmAddressSpace::UpdatePageTableConstraint} adding range low=0x" << hex << low << " high=0x" << high << endl;
  }

  const PagingChoicesAdapter* VmAddressSpace::GetChoicesAdapter() const
  {
    return mpControlBlock->GetChoicesAdapter();
  }

  bool VmAddressSpace::GetPageInfo(uint64 addr, const std::string& type, uint32 bank, PageInformation& rPageInfo) const
  {
    uint64 VA = addr;

    if (type == "PA" || type == "IPA")
    {
      if (!(ETranslationResultType::Mapped == TranslatePaToVa(addr, EMemBankType(bank), VA)))
      {
        LOG(fail) << "{VmAddressSpace::GetPageInfo} could not find addr in current VMAS, PA=0x" << hex << addr << endl;
        FAIL("get-page-info-invalid-pa");
      }
    }

    auto page = GetPage(VA);

    if (nullptr == page)
    {
      LOG(notice) << "{VmAddressSpace::GetPageInfo} page not found: " << hex << VA << endl;
      return false;
    }

    //make sure address error page isn't returned here, causes fault when doing the root_table->PageTableWalk
    if (VaInAddressErrorRange(VA))
    {
      LOG(notice) << "{VmAddressSpace::GetPageInfo} VA is part of address error range can't return page info, VA=0x" << hex << VA << endl;
      return false;
    }

    LOG(info) << "{VmAddressSpace::GetPageInfo} page found: " << hex << addr << endl;

    mpControlBlock->PopulatePageInfo(VA, this, page, rPageInfo);

    return true;
  }

  void VmAddressSpace::HandleMemoryConstraintUpdate(const MemoryConstraintUpdate& rMemConstrUpdate, const Page* pPage)
  {
    if (!mpVirtualUsable->IsInitialized())
    {
      LOG(fail) << "{VmAddressSpace::HandleMemoryConstraintUpdate} called with empty virtual constraint" << endl;
      FAIL("update-vir-usable-by-range-empty-constr");
    }

    LOG(debug) << "{VmAddressSpace::HandleMemoryConstraintUpdate} page v_lower=0x" << hex << pPage->Lower() << " v_upper=0x"
               << pPage->Upper() << " p_lower=0x" << pPage->PhysicalLower() << " p_upper=0x" << pPage->PhysicalUpper() << endl;

    // Only consider the portion of the range contained in the specified page.

    uint64 vaStart = 0;
    uint64 vaEnd = 0;

    uint64 pageStart = max(rMemConstrUpdate.GetPhysicalStartAddress(), pPage->PhysicalLower());
    pPage->TranslatePaToVa(pageStart, pPage->MemoryBank(), vaStart);

    uint64 pageEnd = min(rMemConstrUpdate.GetPhysicalEndAddress(), pPage->PhysicalUpper());
    pPage->TranslatePaToVa(pageEnd, pPage->MemoryBank(), vaEnd);

    rMemConstrUpdate.UpdateVirtualConstraint(vaStart, vaEnd, mpVirtualUsable);

    LOG(info) << "{VmAddressSpace::HandleMemoryConstraintUpdate} updating virtual constraint lower=0x" << hex << vaStart
              << " upper=0x" << vaEnd << endl;

    //auto cset_s_vir_usable = ConstraintSetSerializer(*mpVirtualUsable->Usable(), FORCE_CSET_DEFAULT_PERLINE);
    //LOG(debug) << "{VmAddressSpace::UpdateVirtualUsableByPage} end vir_usable: " << cset_s_vir_usable.ToDebugString() << endl;
  }

  void VmAddressSpace::AddPhysicalRegion(PhysicalRegion* pRegion, bool map)
  {
    mPhysicalRegions.push_back(pRegion);
    if (map)
    {
      MapPhysicalRegions();
    }
  }

  bool VmAddressSpace::VaInAddressErrorRange(const uint64 VA) const
  {
    auto addr_error_constr = mVmConstraints[uint32(EVmConstraintType::AddressError)];
    if (addr_error_constr->ContainsValue(VA))
    {
      return true;
    }

    return false;
  }

  EExceptionConstraintType VmAddressSpace::GetExceptionConstraintType(const std::string& rExceptName) const
  {
    // << "[VmAddressSpace::GetExceptionConstraintType] rExceptName=" << rExceptName << endl;
    std::unique_ptr<ChoiceTree> choices_tree(mpControlBlock->GetChoicesAdapter()->GetPagingChoiceTree(rExceptName));
    auto choice_ptr = choices_tree->Choose();
    uint32 available_choices = choices_tree->AvailableChoices();
    // << "[VmAddressSpace::GetExceptionConstraintType] found choice tree: " << choices_tree->Name() << " chosen value: " << choice_ptr->Value() << " available choices: " << available_choices << endl;

    EExceptionConstraintType except_constr_type = EExceptionConstraintType::Invalid;
    switch (choice_ptr->Value()) {
    case 0:
      except_constr_type = (available_choices == 1) ? EExceptionConstraintType::PreventHard : EExceptionConstraintType::Prevent;
      break;
    case 1:
      except_constr_type = EExceptionConstraintType::Allow;
      break;
    case 2:
      except_constr_type = (available_choices == 1) ? EExceptionConstraintType::TriggerHard : EExceptionConstraintType::Trigger;
      break;
    default:
      LOG(fail) << "[VmAddressSpace::GetExceptionConstraintType] unexpected exception constraint choice value: " << dec << choice_ptr->Value() << endl;
      FAIL("unexpected-exception-constraint-value");
    }

    return except_constr_type;
  }

  bool VmAddressSpace::VerifyStreamingPageCrossing(uint64 start, uint64 end) const
  {
    auto start_page = GetPageWithAssert(start); // page got to exist by this time, or investigate what is wrong.

    if (not start_page->Contains(end)) {
      const GenPageRequest* page_req = DefaultPageRequest(true);
      auto addr_regulator = mpGenerator->GetAddressFilteringRegulator();
      VmConstraintValidator vm_constr_validator(page_req, addr_regulator, this);
      bool is_valid = vm_constr_validator.IsValid(end);
      LOG(notice) << "[VmAddressSpace::VerifyStreamingPageCrossing] page crossing: 0x" << hex << start << " - 0x" << end << " is next page valid? " << is_valid << endl;
      return is_valid;
    }
    return true;
  }

  void VmAddressSpace::ApplyVmConstraints(const GenPageRequest* pPageReq, ConstraintSet& rConstr) const
  {
    auto addr_regulator = mpGenerator->GetAddressFilteringRegulator();
    VmConstraintValidator vm_constr_validator(pPageReq, addr_regulator, this);
    vm_constr_validator.ApplyOn(rConstr);
  }

  EMemBankType VmAddressSpace::DefaultMemoryBank() const
  {
    return mpControlBlock->DefaultMemoryBank();
  }

  void VmAddressSpace::DumpPage(const EDumpFormat dumpFormat, ofstream& os) const
  {
    switch (dumpFormat) {
    case EDumpFormat::Text:
      DumpPageText(os);
      break;
    case EDumpFormat::JSON:
      DumpPageJson(os);
      break;
    default:
      LOG(fail) << "{VmAddressSpace::DumpPage} Unknown dump format " << EDumpFormat_to_string(dumpFormat) << endl;
      FAIL("unknown-dump-format");
    }
  }

  void VmAddressSpace::DumpPageText(ofstream& os) const
  {
    for (auto page : mPages) {
      DumpPageSummaryText(os, page);
    }
  }

  void VmAddressSpace::DumpPageJson(ofstream& os) const
  {
    bool first_entry = true;
    os << "[";
    for (auto page : mPages) {
      if (not first_entry) {
        os << ",\n";
      }

      DumpPageSummaryJson(os, page);

      first_entry = false;
    }

    os << "]";
  }

  void VmAddressSpace::DumpPageSummaryText(std::ofstream& os, const Page* pPage) const
  {
    os << pPage->VaRangeString() << endl;
    os << "Memory bank:" <<  EMemBankType_to_string(pPage->MemoryBank()) << ", PA range:0x" << hex << pPage->PhysicalLower() << " -- 0x" << pPage->PhysicalUpper() << endl;

    os << pPage->DescriptorDetails() << endl;
  }

  void VmAddressSpace::DumpPageSummaryJson(std::ofstream& os, const Page* pPage) const
  {
    PageInformation page_info;
    mpControlBlock->PopulatePageInfo(pPage->Lower(), this, pPage, page_info);
    const PageInfoRec& page_info_rec = page_info.GetPageInfo();

    os << hex;
    os << "Page: {";
    os << "Lower: " << page_info_rec.lower << ", ";
    os << "Upper: " << page_info_rec.upper << ", ";
    os << "PhysLower: " << page_info_rec.physical_lower << ", ";
    os << "PhysUpper: " << page_info_rec.physical_upper << ", ";
    os << "Bank: " << EMemBankType_to_string(pPage->MemoryBank()) << ", ";

    RootPageTable* root_page_table = pPage->GetRootPageTable();
    os << "RootTable: " << root_page_table->TableBase() << ", ";

    // TODO(Noah): Implement dumping of the table walk information when a good way to do so is
    // determined.
    os << "TableWalk: " << "To Be Determined";  // TableWalk – PA and descriptor value for each entry in the mapping’s walk (including final leaf node, to avoid replication of leaf descriptor value)

    os << "}";
  }

  bool VmAddressSpace::UpdateContext(const VmContext* pVmContext)
  {
    return mpControlBlock->UpdateContext(pVmContext);
  }

  void VmAddressSpace::UpdateVirtualSharedByPage(const Page* pPage)
  {
    MemoryManager* mem_manager = mpGenerator->GetMemoryManager();
    MemoryBank* mem_bank = mem_manager->GetMemoryBank(EMemBankTypeBaseType(pPage->MemoryBank()));
    const ConstraintSet* shared_constr = mem_bank->Shared();

    if (not shared_constr->IsEmpty()) {
      ConstraintSet page_shared_constr(pPage->PhysicalLower(), pPage->PhysicalUpper());
      page_shared_constr.ApplyLargeConstraintSet(*shared_constr);

      if (not page_shared_constr.IsEmpty())
      {
        uint64 mask = pPage->PageSize() - 0x1ull;
        uint64 frame = pPage->Lower() & ~mask;
        page_shared_constr.Translate(mask, frame);
        mpVirtualUsable->MarkShared(page_shared_constr);
      }
    }
  }

  void VmAddressSpace::GetVmContextDelta(std::map<std::string, uint64> & rDeltaMap) const
  {
    return mpControlBlock->GetContextDelta(rDeltaMap, mpGenerator);
  }

  uint32 VmAddressSpace::GenContextId() const
  {
    return mpControlBlock->GenContextId();
  }

}
