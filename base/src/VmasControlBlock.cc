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
#include <VmasControlBlock.h>
#include <Generator.h>
#include <Page.h>
#include <PageTable.h>
#include <PageInfoRecord.h>
#include <VmUtils.h>
#include <Constraint.h>
#include <PagingChoicesAdapter.h>
#include <Log.h>
#include <VmAddressSpace.h>
#include <MemoryManager.h>
#include <MemoryAttributes.h>
#include <UtilityFunctions.h>

#include <memory>
#include <sstream>

using namespace std;

/*!
  \file VmasControlBlock.cc
  \brief Code supporting address space control block handling.
*/

namespace Force {

  VmasControlBlock::VmasControlBlock(EPrivilegeLevelType privType, EMemBankType memType)
    : VmControlBlock(privType, memType), mGranuleType(EPageGranuleType(0)), mMemoryAttributes(0), mAsid(0), mWriteExecuteNever(false), mpRootPageTable(nullptr), mpChoicesAdapter(nullptr), mGranuleSuffix(), mPteIdentifierSuffix(), mPageTableImplAttr(EMemAttributeImplType(0)), mpPtCompatibleImplAttrs(nullptr)
  {
  }

  VmasControlBlock::VmasControlBlock()
    : VmControlBlock(), mGranuleType(EPageGranuleType(0)), mMemoryAttributes(0), mAsid(0), mWriteExecuteNever(false), mpRootPageTable(nullptr), mpChoicesAdapter(nullptr), mGranuleSuffix(), mPteIdentifierSuffix(), mPageTableImplAttr(EMemAttributeImplType(0)), mpPtCompatibleImplAttrs(nullptr)
  {
  }

  VmasControlBlock::VmasControlBlock(const VmasControlBlock& rOther)
    : VmControlBlock(rOther), mGranuleType(EPageGranuleType(0)), mMemoryAttributes(0), mAsid(0), mWriteExecuteNever(false) , mpRootPageTable(nullptr), mpChoicesAdapter(nullptr), mGranuleSuffix(), mPteIdentifierSuffix(), mPageTableImplAttr(EMemAttributeImplType(0)), mpPtCompatibleImplAttrs(nullptr)
  {
  }

  VmasControlBlock::~VmasControlBlock()
  {
    //delete mpRootPageTable; ---- root table should now be managed and deleted by the page table manager,
    //                             so only 1 delete occurs and we don't delete another contexts table structure thats in use
    delete mpChoicesAdapter;
    delete mpPtCompatibleImplAttrs;
  }

  const string VmasControlBlock::AdditionalAttributesString() const
  {
    stringstream out_str;
    out_str << " : Memory-attributes:0x" << hex << mMemoryAttributes
            << " ASID=0x" << mAsid
            << " Granule=" << EPageGranuleType_to_string(PageGranuleType())
            << " Max-PA=0x" << hex << mMaxPhysicalAddress
            << " Root-table-address=0x" << mpRootPageTable->TableBase()
            << " Table-step=0x" << mpRootPageTable->TableStep()
            << " Root-level=0x" << mpRootPageTable->TableLevel()
            << " Root-lookup=[" << dec << mpRootPageTable->HighestLookUpBit()
            << ":" << mpRootPageTable->LowestLookUpBit()
            << "] Root-table-size=0x" << hex << mpRootPageTable->TableSize();

    return out_str.str();
  }

  void VmasControlBlock::Setup(Generator* pGen)
  {
    VmControlBlock::Setup(pGen);

    // setup paging choices adapter
    mpChoicesAdapter = PagingChoicesAdapterInstance();
    mpChoicesAdapter->Setup(mpGenerator->GetChoicesModerator(EChoicesType::PagingChoices), mpGenerator->GetVariableModerator(EVariableType::Value));
  }

  PagingChoicesAdapter* VmasControlBlock::PagingChoicesAdapterInstance() const
  {
    return new PagingChoicesAdapter(EPrivilegeLevelType_to_string(mPrivilegeLevel), mStage);
  }

  void VmasControlBlock::Initialize()
  {
    VmControlBlock::InitializeContext(mpGenerator);

    //Initialize any other context parameters not managed in mContextParams
    InitializeMemoryAttributes();

    mMemoryAttributes = GetMemoryAttributes();
    mAsid = GetAsid();
    mGranuleType = GetGranuleType();
    mWriteExecuteNever = GetWriteExecuteNever();

    string granule_str =  EPageGranuleType_to_string(mGranuleType);
    mPteIdentifierSuffix = "#";
    mPteIdentifierSuffix += granule_str;
    mPteIdentifierSuffix += "#";
    char print_buffer[32];
    snprintf(print_buffer, 32, "%d", mStage);
    mPteIdentifierSuffix += print_buffer;

    mPageTableImplAttr = GetPageTableAttribute();
    mpPtCompatibleImplAttrs = GetPageTableCompatibleAttributes(mPageTableImplAttr);
    mGranuleSuffix = granule_str.substr(1) + " granule";

    VmControlBlock::Initialize();
  }

  bool VmasControlBlock::InitializeRootPageTable(VmAddressSpace* pVmas, RootPageTable* pRootTable)
  {
    if (nullptr != pRootTable)
    {
      mpRootPageTable = pRootTable;
    }
    else
    {
      mpRootPageTable = RootPageTableInstance();
    }

    return true;
  }

  bool VmasControlBlock::Validate(std::string& rErrMsg) const
  {
    bool valid_context = true;

    // set initial value of error message string, adds on and mismatches as it validates
    rErrMsg = string("{") + Type() + string("::Validate} Validation mismatches:");

    //validate values stored in the base control block implementation
    valid_context &= (mGranuleType == GetGranuleType());
    if (!valid_context)
    {
      rErrMsg += " GranuleType,";
    }

    valid_context &= (mMemoryAttributes == GetMemoryAttributes());
    if (!valid_context)
    {
      rErrMsg += " Memory Attributes,";
    }

    valid_context &= (mAsid == GetAsid());
    if (!valid_context)
    {
      rErrMsg += " ASID,";
    }

    valid_context &= VmControlBlock::Validate(rErrMsg);

    return valid_context;
  }

  ConstraintSet* VmasControlBlock::InitialVirtualConstraint() const
  {
    auto v_constr = new ConstraintSet();
    GetDefaultVaRange(*v_constr);

    if (v_constr->IsEmpty())
    {
      LOG(fail) << "{VmasControlBlock::InitialVirtualConstraint} empty initial virtual constraint" << endl;
      FAIL("empty_initial_virtual_constraint");
    }

    return v_constr;
  }

  void VmasControlBlock::GetDefaultVaRange(ConstraintSet& rConstr) const
  {
    //get highest va bit from vmas control block
    uint64 va_bits = mpRootPageTable->HighestLookUpBit();
    uint64 va_end  = (0x1ull << (va_bits + 1)) - 0x1ull;
    rConstr.AddRange(0, va_end);
  }

  void VmasControlBlock::GetGranuleType(std::vector<EPageGranuleType>& granuleTypes) const
  {
    granuleTypes.push_back(mGranuleType);
  }

  bool VmasControlBlock::GetVmVaRanges(std::vector<VmVaRange* >& rVmVaRanges, uint64* pVA) const
  {
    ConstraintSet* vmva_constr = new ConstraintSet();
    GetDefaultVaRange(*vmva_constr);

    rVmVaRanges.push_back(new VmVaRange(mGranuleType, mGranuleSuffix, vmva_constr));

    if (nullptr != pVA) {
      return vmva_constr->ContainsValue(*pVA);
    }
    return true;
  }

  void VmasControlBlock::GetAddressErrorRanges(vector<TranslationRange>& rRanges) const
  {
    TranslationRange addr_err_range;

    uint64 addr_err_low = 1ull << (mpRootPageTable->HighestLookUpBit() + 1);
    addr_err_range.SetBoundary(addr_err_low, MAX_UINT64);
    addr_err_range.SetMemoryBank(DefaultMemoryBank());

    rRanges.push_back(addr_err_range);
  }

  void VmasControlBlock::CommitPageTable(uint64 VA, const PageTable* pParentTable, const TablePte* pTablePte, std::vector<ConstraintSet* >& rVmConstraints)
  {

  }

  void VmasControlBlock::CommitPage(const Page* pPage, std::vector<ConstraintSet* >& rVmConstraints)
  {
    // Add to existing page constraint.
    rVmConstraints[uint32(EVmConstraintType::Existing)]->AddRange(pPage->Lower(), pPage->Upper());

    // update data access permission constraints.
    EDataAccessPermissionType dap_type = EDataAccessPermissionType(pPage->PageGenAttribute(EPageGenAttributeType::DataAccessPermission));
    switch (dap_type) {
    case EDataAccessPermissionType::ReadOnly:
    case EDataAccessPermissionType::ReadOnlyUserOnly:
      rVmConstraints[uint32(EVmConstraintType::ReadOnly)]->AddRange(pPage->Lower(), pPage->Upper());
      rVmConstraints[uint32(EVmConstraintType::UserAccess)]->AddRange(pPage->Lower(), pPage->Upper());
      break;
    case EDataAccessPermissionType::ReadOnlyNoUser:
      rVmConstraints[uint32(EVmConstraintType::ReadOnly)]->AddRange(pPage->Lower(), pPage->Upper());
      rVmConstraints[uint32(EVmConstraintType::NoUserAccess)]->AddRange(pPage->Lower(), pPage->Upper());
      break;
    case EDataAccessPermissionType::ReadWriteNoUser:
      rVmConstraints[uint32(EVmConstraintType::NoUserAccess)]->AddRange(pPage->Lower(), pPage->Upper());
      break;
    case EDataAccessPermissionType::ReadWrite:
    case EDataAccessPermissionType::ReadWriteUserOnly:
      rVmConstraints[uint32(EVmConstraintType::UserAccess)]->AddRange(pPage->Lower(), pPage->Upper());
      break;
    case EDataAccessPermissionType::NoAccess:
      break;
    default:
      LOG(fail) << "{VmasControlBlock::CommitPage} unexpected data access permission type: " << EDataAccessPermissionType_to_string(dap_type) << endl;
      FAIL("unexpected-data-access-permission-type");
    }

    // update instruction access permission constraints.

    EInstrAccessPermissionType iap_type = EInstrAccessPermissionType(pPage->PageGenAttribute(EPageGenAttributeType::InstrAccessPermission));
    switch (iap_type) {
    case EInstrAccessPermissionType::Execute:
      // bypass
      break;
    case EInstrAccessPermissionType::NoExecute:
      rVmConstraints[uint32(EVmConstraintType::NoExecute)]->AddRange(pPage->Lower(), pPage->Upper());
      break;
    case EInstrAccessPermissionType::PrivilegedNoExecute:
      rVmConstraints[uint32(EVmConstraintType::PrivilegedNoExecute)]->AddRange(pPage->Lower(), pPage->Upper());
      break;
    default:
      LOG(fail) << "{VmasControlBlock::CommitPage} unexpected instruction access permission type: " << EInstrAccessPermissionType_to_string(iap_type) << endl;
      FAIL("unexpected-instruction-access-permission-type");
    }
    // Add to flat map constraint.
    if (pPage->Lower() == pPage->PhysicalLower())
    {
      rVmConstraints[uint32(EVmConstraintType::FlatMap)]->AddRange(pPage->Lower(), pPage->Upper());
    }
  }

  EMemBankType VmasControlBlock::NextLevelTableMemoryBank(const PageTable* parentTable, const GenPageRequest& rPageReq) const
  {
    return parentTable->MemoryBank();
  }

  EMemBankType VmasControlBlock::GetTargetMemoryBank(uint64 VA, GenPageRequest* pPageReq, const Page* pPage, const std::vector<ConstraintSet* >& rVmConstraints)
  {
    return DefaultMemoryBank();
  }

  const string VmasControlBlock::PagePteIdentifier(EPteType pteType, uint64 VA) const
  {
    string str_id = EPteType_to_string(pteType);
    str_id += "#Page";
    str_id += mPteIdentifierSuffix;

    LOG(trace) << "{VmasControlBlock::PagePteIdentifier} str_id=" << str_id << " pte_id_suffix=" << mPteIdentifierSuffix << endl;

    return str_id;
  }

  uint64 VmasControlBlock::MaxVirtualAddress(bool flatMapped) const
  {
    uint64 max_addr = get_mask64(mpRootPageTable->HighestLookUpBit()+1);
    if (flatMapped) max_addr &= MaxPhysicalAddress();
    return max_addr;
  }

  void VmasControlBlock::PopulatePageInfo(uint64 VA, const VmAddressSpace* pVmas, const Page* pPage, PageInformation& rPageInfo) const
  {
    PageTableInfoRec page_table_rec;
    RootPageTable* root_table = GetRootPageTable(VA);
    const TablePte* pte = root_table->PageTableWalk(pPage, pVmas, page_table_rec);
    if (nullptr != pte)
    {
      rPageInfo.SetPageTableRecord(page_table_rec);
      const TablePte* new_pte = nullptr;
      while ((new_pte = pte->PageTableWalk(pPage, pVmas, page_table_rec)) != nullptr)
      {
        rPageInfo.SetPageTableRecord(page_table_rec);
        pte = new_pte;
      }
    }

    PageInfoRec page_info_rec;
    page_info_rec.memoryType = EMemBankType_to_string (pPage->MemoryBank());
    page_info_rec.lower = pPage->Lower();
    page_info_rec.upper = pPage->Upper();
    page_info_rec.physical_lower = pPage->PhysicalLower();
    page_info_rec.physical_upper = pPage->PhysicalUpper();
    page_info_rec.page_size = pPage->PageSize();
    page_info_rec.descr_value = pPage->Descriptor();
    pPage->DescriptorDetails(page_info_rec.descr_details);
    page_info_rec.phys_id = pPage->PhysPageId();
    PopulateAttributeInfo(pPage, page_info_rec);
    rPageInfo.SetPageInfoRecord(page_info_rec);
  }

  RootPageTable* VmasControlBlock::RootPageTableInstance() const
  {
    return new RootPageTable();
  }

  vector<const PhysicalRegion* > VmasControlBlock::FilterEssentialPhysicalRegions(const vector<PhysicalRegion* >& rPhysRegions) const
  {
    return vector<const PhysicalRegion*> (reinterpret_cast<const vector<const PhysicalRegion* >& >(rPhysRegions));
  }

}
