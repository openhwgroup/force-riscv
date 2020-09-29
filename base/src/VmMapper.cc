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
#include <VmMapper.h>
#include <VmFactory.h>
#include <Generator.h>
#include <MemoryManager.h>
#include <Constraint.h>
#include <VmAddressSpace.h>
#include <VmasControlBlock.h>
#include <VmDirectMapControlBlock.h>
#include <VmUtils.h>
#include <Config.h>
#include <VmInfo.h>
#include <RegisterReload.h>
#include <Register.h>
#include <AddressTagging.h>
#include <PageRequestRegulator.h>
#include <Variable.h>
#include <PcSpacing.h>
#include <GenRequest.h>
#include <UtilityAlgorithms.h>
#include <UtilityFunctions.h>
#include <Log.h>

#include <iomanip>
#include <memory>
#include <algorithm>

using namespace std;

/*!
  \file VmMapper.cc
  \brief Code for VmMapper module.
*/

namespace Force {

  const AddressFilteringRegulator* VmMapper::GetAddressFilteringRegulator() const
  {
    return mpGenerator->GetAddressFilteringRegulator(); 
  }

  GenPageRequest* VmMapper::GenPageRequestInstance(bool isInstr, EMemAccessType memAccessType) const
  {
    return mpGenerator->GenPageRequestInstance(isInstr, memAccessType);
  }

  GenPageRequest* VmMapper::GenPageRequestRegulated(bool isInstr, EMemAccessType memAccessType, bool noFault) const
  {
    auto ret_req = mpGenerator->GenPageRequestInstance(isInstr, memAccessType);
    auto regulator = mpGenerator->GetPageRequestRegulator();

    if (isInstr) {
      if (noFault) ret_req->SetGenBoolAttribute(EPageGenBoolAttrType::NoInstrPageFault, true);
      regulator->RegulateBranchPageRequest(this, nullptr, ret_req);
    }
    else {
      if (noFault) ret_req->SetGenBoolAttribute(EPageGenBoolAttrType::NoDataPageFault, true);
      regulator->RegulateLoadStorePageRequest(this, nullptr, ret_req);
    }

    return ret_req;
  }

  bool VmMapper::VerifyStreamingVa(uint64 va, uint64 size, bool isInstr) const
  {
    if (size == 0) {
      LOG(fail) << "{VmMapper::VerifyStreamingVa} unexpected size 0 request." << endl;
      FAIL("unexpected-size-0-request");
    }

    auto virtual_constr = VirtualUsableConstraintSet(isInstr);
    const AddressTagging* addr_tagging = GetAddressTagging();
    uint64 untagged_va = addr_tagging->UntagAddress(va, isInstr);
    uint64 va_end = untagged_va + (size - 1);
    // << "{ VmMapper::VerifyStreamingVa }" << va << " : " << untagged_va << " : " << virtual_constr->ToString() << endl;
    ConstraintSet verify_ranges;
    if (va_end < untagged_va) {
      // wrap around
      verify_ranges.AddRange(0, va_end);
      verify_ranges.AddRange(untagged_va,  MAX_UINT64);
    }
    else {
      verify_ranges.AddRange(untagged_va, va_end);
    }

    if (not virtual_constr->ContainsConstraintSet(verify_ranges)) {
      do {
        auto addr_err_constr = GetVmConstraint(EVmConstraintType::AddressError);
        const VariableModerator* var_mod = mpGenerator->GetVariableModerator(EVariableType::Value);
        auto addr_err_var = dynamic_cast<const ValueVariable*>(var_mod->GetVariableSet()->FindVariable("Streaming address error"));
        if (nullptr != addr_err_constr && addr_err_var->Value()) {
          verify_ranges.SubConstraintSet(*addr_err_constr);
          if (virtual_constr->ContainsConstraintSet(verify_ranges)) {
            break;
          }
        }
        return false;
      }
      while (0);
    }

    return VerifyStreamingPageCrossing(untagged_va, va_end);
  }

  bool VmMapper::VerifyVirtualAddress(uint64 va, uint64 size, bool isInstr, const GenPageRequest* pPageReq) const
  {
    if (size == 0) {
      LOG(fail) << "{VmMapper::VerifyVirtualAddress} unexpected size 0 request." << endl;
      FAIL("unexpected-size-0-request");
    }

    auto virtual_constr = VirtualUsableConstraintSet(isInstr);
    const AddressTagging* addr_tagging = GetAddressTagging();
    uint64 untagged_va = addr_tagging->UntagAddress(va, isInstr);
    uint64 va_end = untagged_va + (size - 1);
    // << va << " : " << untagged_va << " : " << virtual_constr->ToString() << endl;
    ConstraintSet verify_ranges;
    if (va_end < untagged_va) {
      // wrap around
      verify_ranges.AddRange(0, va_end);
      verify_ranges.AddRange(untagged_va,  MAX_UINT64);
    }
    else {
      verify_ranges.AddRange(untagged_va, va_end);
    }

    if (not virtual_constr->ContainsConstraintSet(verify_ranges)) {
      return false;
    }

    auto current_size = verify_ranges.Size();
    auto pc_spacing = PcSpacing::Instance();
    auto pc_constr = pc_spacing->GetPcSpaceConstraint();
    verify_ranges.SubConstraintSet(*pc_constr);
    if (verify_ranges.Size() != current_size)
      return false;

    ApplyVmConstraints(pPageReq, verify_ranges);
    if (verify_ranges.Size() != current_size) {
      return false;
    }

    return true;
  }

  const std::vector<Register* > VmRegime::RegisterContext() const
  {
    auto reg_file = mpGenerator->GetRegisterFile();
    std::vector<string> reg_names;
    std::vector<Register* > reg_ptr_context;
    mpVmFactory->GetRegisterContext(reg_names, PagingEnabled());
    if (not mpGenerator->DelayInit())
    {
      mpVmFactory->GetDelayedRegisterContext(reg_names);
    }

    for (auto reg_name : reg_names)
    {
      insert_sorted<Register* >(reg_ptr_context, reg_file->RegisterLookup(reg_name));
    }

    return reg_ptr_context;
  }

  void VmRegime::FinalizeRegisterContext() const
  {
    auto reg_file = mpGenerator->GetRegisterFile();
    std::vector<string> reg_names;
    mpVmFactory->GetRegisterContext(reg_names, PagingEnabled());
    if (not mpGenerator->DelayInit())
    {
      mpVmFactory->GetDelayedRegisterContext(reg_names);
    }

    for (auto reg_name : reg_names)
    {
      mpGenerator->RandomInitializeRegister(reg_file->RegisterLookup(reg_name));
    }
  }

  bool VmRegime::PagingEnabled() const
  {
    return mpVmFactory->PagingEnabled(*mpGenerator);
  }

  VmDirectMapper::~VmDirectMapper()
  {
    if (mpVmDirectMapControlBlock) {
      delete mpVmDirectMapControlBlock;
      mpVmDirectMapControlBlock = nullptr;
    }

    if (mpAddressTagging) {
      delete mpAddressTagging;
      mpAddressTagging = nullptr;
    }

    if (mpAddressErrorConstraint) {
      delete mpAddressErrorConstraint;
      mpAddressErrorConstraint = nullptr;
    }
  }

  const ConstraintSet* VmDirectMapper::VirtualUsableConstraintSet(bool isInstr) const
  {
    auto mem_manager = mpGenerator->GetMemoryManager();
    auto mem_bank = mem_manager->GetMemoryBank(uint32(mMemoryBankType));
    auto usable_constr = mem_bank->Usable();

    return usable_constr;
  }

  ConstraintSet* VmDirectMapper::VirtualUsableConstraintSetClone(bool isInstr)
  {
    auto mem_manager = mpGenerator->GetMemoryManager();
    auto mem_bank = mem_manager->GetMemoryBank(uint32(mMemoryBankType));
    auto usable_constr = mem_bank->Usable();

    return usable_constr->Clone();
  }

  const ConstraintSet* VmDirectMapper::VirtualSharedConstraintSet() const
  {
    MemoryManager* mem_manager = mpGenerator->GetMemoryManager();
    MemoryBank* mem_bank = mem_manager->GetMemoryBank(uint32(mMemoryBankType));
    return mem_bank->Shared();
  }

  void VmDirectMapper::ApplyVirtualUsableConstraint(const EMemDataType memDataType, const EMemAccessType memAccessType, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const
  {
    MemoryManager* mem_manager = mpGenerator->GetMemoryManager();
    MemoryBank* mem_bank = mem_manager->GetMemoryBank(uint32(mMemoryBankType));
    mem_bank->ApplyUsableConstraint(memDataType, memAccessType, mpGenerator->ThreadId(), rAddrReuseMode, constrSet);
  }

  void VmDirectMapper::Activate()
  {
    if (mState == EVmStateType::Uninitialized)
    {
      Initialize();
    }

    if (mState == EVmStateType::Initialized)
    {
      mState = EVmStateType::Active;
    }
  }

  void VmDirectMapper::Initialize()
  {
    const VmFactory* vm_factory = mpPagingRegime->GetVmFactory();
    if (mState == EVmStateType::Uninitialized)
    {
      if (not mpVmDirectMapControlBlock)
      {
        mpVmDirectMapControlBlock = vm_factory->CreateVmDirectMapControlBlock(mMemoryBankType);
        mpVmDirectMapControlBlock->Setup(mpGenerator);
      }

      mpPagingRegime->FinalizeRegisterContext();
      mState = EVmStateType::Initialized;
    }

    mpVmDirectMapControlBlock->Initialize();

    if (mpAddressTagging) {
      delete mpAddressTagging;
    }

    mpAddressTagging = vm_factory->CreateAddressTagging(*mpVmDirectMapControlBlock);

    string err_msg;
    bool valid = mpVmDirectMapControlBlock->Validate(err_msg);

    if (not valid)
    {
      LOG(fail) << "{VmDirectMapper::Initialize} Bad register context: " << err_msg << endl;
      FAIL("init-direct-mapper-bad-context");
    }
    InitializeConstraint();
  }

  ETranslationResultType VmDirectMapper::TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const
  {
    PA = VA;
    bank = uint32(mMemoryBankType);
    return ETranslationResultType::Mapped;
  }

  ETranslationResultType VmDirectMapper::TranslatePaToVa(uint64 PA, EMemBankType bank, uint64& VA) const
  {
    VA = PA;
    return ETranslationResultType::Mapped;
  }

  bool VmDirectMapper::GetTranslationRange(uint64 VA, TranslationRange& rTransRange) const
  {
    uint64 pa_limit = mpVmDirectMapControlBlock->MaxPhysicalAddress();
    if (VA > pa_limit) {
      auto lower_bound = mpAddressErrorConstraint->LowerBound();
      auto upper_bound = mpAddressErrorConstraint->UpperBound();
      rTransRange.SetPhysicalBoundary(lower_bound, upper_bound);
      rTransRange.SetMemoryBank(mMemoryBankType);
      rTransRange.SetBoundary(lower_bound, upper_bound);
      rTransRange.SetTranslationResult(ETranslationResultType::AddressError);
      return true;
    }

    rTransRange.SetPhysicalBoundary(0, pa_limit);
    rTransRange.SetMemoryBank(mMemoryBankType);
    rTransRange.SetBoundary(0, pa_limit);
    rTransRange.SetTranslationResult(ETranslationResultType::Mapped);
    return true;
  }

  bool VmDirectMapper::VaInAddressErrorRange(const uint64 VA) const
  {
    return mpAddressErrorConstraint->ContainsValue(VA);
  }

  const ConstraintSet* VmDirectMapper::GetVmConstraint(EVmConstraintType constrType) const
  {
    switch (constrType) {
    case EVmConstraintType::AddressError:
      return mpAddressErrorConstraint;
    default:
      return nullptr;
    }
  }

  uint64 VmDirectMapper::MaxVirtualAddress() const
  {
    return mpVmDirectMapControlBlock->MaxPhysicalAddress();
  }

  uint64 VmDirectMapper::MaxPhysicalAddress() const
  {
    return mpVmDirectMapControlBlock->MaxPhysicalAddress();
  }

  void VmDirectMapper::InitializeConstraint()
  {
    mpAddressErrorConstraint = new ConstraintSet();
    mpAddressErrorConstraint->AddRange(MaxPhysicalAddress() + 1, MAX_UINT64);
  }

  bool VmDirectMapper::ValidateContext(std::string& rErrMsg) const
  {
    bool valid = true;
    if (mpVmDirectMapControlBlock != nullptr) {
      valid = mpVmDirectMapControlBlock->Validate(rErrMsg);
    }

    return valid;
  }

  RegisterReload* VmDirectMapper::GetRegisterReload() const
  {
    if (mpVmDirectMapControlBlock != nullptr) {
      return mpVmDirectMapControlBlock->GetRegisterReload();
    }

    return nullptr;
  }

  EMemBankType VmDirectMapper::DefaultMemoryBank() const
  {
    return mpVmDirectMapControlBlock->DefaultMemoryBank();
  }

  uint32 VmDirectMapper::GenContextId() const
  {
    return mpVmDirectMapControlBlock->GenContextId();
  }

  void VmDirectMapper::GetVmContextDelta(std::map<std::string, uint64> & rDeltaMap) const
  {
    mpVmDirectMapControlBlock->GetContextDelta(rDeltaMap, mpGenerator);
  }

  void VmPagingMapper::Setup(Generator* gen)
  {
    VmMapper::Setup(gen);

    MemoryManager* mem_manager = mpGenerator->GetMemoryManager();
    mem_manager->SignUp(this);
  }

  const ConstraintSet* VmPagingMapper::VirtualUsableConstraintSet(bool isInstr) const
  {
    return mpCurrentAddressSpace->VirtualUsableConstraintSet(isInstr);
  }

  ConstraintSet* VmPagingMapper::VirtualUsableConstraintSetClone(bool isInstr)
  {
    return mpCurrentAddressSpace->VirtualUsableConstraintSetClone(isInstr);
  }

  const ConstraintSet* VmPagingMapper::VirtualSharedConstraintSet() const
  {
    return mpCurrentAddressSpace->VirtualSharedConstraintSet();
  }

  void VmPagingMapper::ApplyVirtualUsableConstraint(const EMemDataType memDataType, const EMemAccessType memAccessType, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const
  {
    return mpCurrentAddressSpace->ApplyVirtualUsableConstraint(memDataType, memAccessType, rAddrReuseMode, constrSet);
  }

  VmPagingMapper::~VmPagingMapper()
  {
    mpCurrentAddressSpace = nullptr;

    // delete address spaces.
    for (auto as_item : mAddressSpaces) {
      delete as_item;
    }
    delete mpAddressTagging;
  }

  bool VmPagingMapper::ValidateContext(std::string& rErrMsg) const
  {
    if (nullptr != mpCurrentAddressSpace)
    {
      return mpCurrentAddressSpace->GetControlBlock()->Validate(rErrMsg);
    }
    else
    {
      LOG(fail) << "{VmPagingMapper::ValidateContext} VmAddressSpace has not been initialized." << endl;
      FAIL("vm-address-space-not-init");
      return false;
    }
  }

  void VmPagingMapper::Activate()
  {
    if (mState == EVmStateType::Uninitialized)
    {
      Initialize();
    }

    if (mState == EVmStateType::Initialized)
    {
      mState = EVmStateType::Active;
    }

    UpdateCurrentAddressSpace();

    mpCurrentAddressSpace->Activate();

    LOG(notice) << "Activated address space (Type=" << mpCurrentAddressSpace->Type() << ") : " << mpCurrentAddressSpace->ControlBlockInfo() << endl;
  }

  void VmPagingMapper::Initialize()
  {
    if (mState == EVmStateType::Uninitialized)
    {
      if (nullptr == mpCurrentAddressSpace)
      {
        mpCurrentAddressSpace = CreateAddressSpace();
        const VmFactory* vm_factory = mpPagingRegime->GetVmFactory();
        mpAddressTagging = vm_factory->CreateAddressTagging(*(mpCurrentAddressSpace->GetControlBlock()));
      }
      mpPagingRegime->FinalizeRegisterContext();
      mpCurrentAddressSpace->Initialize();
      mState = EVmStateType::Initialized;

      std::string err_msg;
      bool vld = mpCurrentAddressSpace->GetControlBlock()->Validate(err_msg);

      if (!vld) {
        LOG(fail) << "{VmPagingMapper::Initialize} Bad VM context: " << err_msg << endl;
        FAIL("init-paging-mapper-bad-context");
      }
    }
  }

  void VmPagingMapper::Deactivate()
  {
    mState = EVmStateType::Uninitialized;
    if (nullptr != mpCurrentAddressSpace)
    {
      mpCurrentAddressSpace->Deactivate();
    }
  }

  VmAddressSpace* VmPagingMapper::CreateAddressSpace(const VmContext* pVmContext)
  {
    const VmFactory* vm_factory = mpPagingRegime->GetVmFactory();
    auto new_ascb = vm_factory->CreateVmasControlBlock(mpPagingRegime->DefaultMemoryBank());
    auto new_as   = AddressSpaceInstance(new_ascb);
    new_as->Setup(mpGenerator);
    new_as->UpdateContext(pVmContext);
    AddAddressSpace(new_as);
    return new_as;
  }

  bool compare_address_space_with_context(const VmAddressSpace* pVmas, const VmContext* pContext)
  {
    return *(pVmas->GetControlBlock()) < (*pContext);
  }

  void VmPagingMapper::AddAddressSpace(VmAddressSpace* pAddressSpace)
  {
    const VmContext* as_context = pAddressSpace->GetControlBlock();
    auto find_iter = lower_bound(mAddressSpaces.begin(), mAddressSpaces.end(), as_context, compare_address_space_with_context);

    if (find_iter == mAddressSpaces.end()) { // no match found.
      mAddressSpaces.insert(mAddressSpaces.end(), pAddressSpace);
    }
    else {
      if ((*find_iter)->GetControlBlock()->Matches(*as_context)) {
        LOG(fail) << "{VmPagingMapper::AddAddressSpace} adding address space context with ID: " << dec << as_context->GenContextId() << " matches existing context with ID: " << (*find_iter)->GetControlBlock()->GenContextId() << endl;
        FAIL("adding-duplicated-address-space-context");
      }
    }
    LOG(notice) << "{VmPagingMapper::AddAddressSpace} Created address space context ID: " << dec << as_context->GenContextId() << endl;
  }

  VmAddressSpace* VmPagingMapper::FindAddressSpace(const VmContext& rVmContext)
  {
    auto find_iter = lower_bound(mAddressSpaces.begin(), mAddressSpaces.end(), &rVmContext, compare_address_space_with_context);
    if (find_iter != mAddressSpaces.end()) {
      // found lower bound.
      auto lower_bound_as = *find_iter;
      if (lower_bound_as->GetControlBlock()->Matches(rVmContext)) {
        return lower_bound_as;
      }
    }
    return nullptr;
  }

  void VmPagingMapper::UpdateCurrentAddressSpace()
  {
    if (nullptr == mpCurrentAddressSpace) {
      LOG(fail) << "{VmPagingMapper::UpdateCurrentAddressSpace} current VmAddressSpace not set." << endl;
      FAIL("current-vm-address-space-not-set");
    }

    string err_msg;
    bool vld = mpCurrentAddressSpace->GetControlBlock()->Validate(err_msg);
    if (!vld) {
      uint32 current_context_id = mpCurrentAddressSpace->GetControlBlock()->GenContextId();
      mpCurrentAddressSpace->Deactivate();
      LOG(notice) << "{VmPagingMapper::UpdateCurrentAddressSpace} current address space (" << current_context_id << ") validation not sucessful due to: " << err_msg << ".  Search for matching address space." << endl;
      const VmFactory* vm_factory = mpPagingRegime->GetVmFactory();
      std::unique_ptr<VmContext> context_ptr_storage(vm_factory->CreateVmContext());
      VmContext* tmp_vm_context = context_ptr_storage.get();
      tmp_vm_context->InitializeContext(mpGenerator);

      mpCurrentAddressSpace = FindAddressSpace(*tmp_vm_context);
      if (nullptr == mpCurrentAddressSpace) {
        LOG(fail) << "{VmPagingMapper::UpdateCurrentAddressSpace} failed to find a VmAddressSpace to switch to." << endl;
        FAIL("failed-switching-address-space");
      }
      else {
        LOG(notice) << "{VmPagingMapper::UpdateCurrentAddressSpace} sucessfully switched from address space: " << dec << current_context_id << " to: " << mpCurrentAddressSpace->GetControlBlock()->GenContextId() << endl;
      }
    }
  }

  RegisterReload* VmPagingMapper::GetRegisterReload() const
  {
    if (nullptr != mpCurrentAddressSpace) {
      return mpCurrentAddressSpace->GetControlBlock()->GetRegisterReload();
    } else {
      return nullptr;
    }
  }

  VmAddressSpace* VmPagingMapper::AddressSpaceInstance(VmasControlBlock* pVmasCtlrBlock) const
  {
    return new VmAddressSpace(mpVmFactory, pVmasCtlrBlock);
  }

  bool VmPagingMapper::MapAddressRange(uint64 VA, uint64 size, bool isInstr, const GenPageRequest* pPageReq)
  {
    uint64 untagged_VA = mpAddressTagging->UntagAddress(VA, isInstr);
    return mpCurrentAddressSpace->MapAddressRange(untagged_VA, size, isInstr, pPageReq);
  }

  uint64 VmPagingMapper::MapAddressRangeForPA(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, const GenPageRequest* pPageReq)
  {
    return mpCurrentAddressSpace->MapAddressRangeForPA(PA, bank, size, isInstr, pPageReq);
  }

  void VmPagingMapper::AddPhysicalRegion(PhysicalRegion* pRegion, bool map)
  {
    mpCurrentAddressSpace->AddPhysicalRegion(pRegion, map);
  }

  uint64 VmPagingMapper::MaxVirtualAddress() const
  {
    return mpCurrentAddressSpace->MaxVirtualAddress();
  }

  uint64 VmPagingMapper::MaxPhysicalAddress() const
  {
    return mpCurrentAddressSpace->MaxPhysicalAddress();
  }

  bool VmPagingMapper::GetPageInfo(uint64 addr, const std::string& type, uint32 bank, PageInformation& page_info) const
  {
    return mpCurrentAddressSpace->GetPageInfo(addr, type, bank, page_info);
  }

  ETranslationResultType VmPagingMapper::TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const
  {
    return mpCurrentAddressSpace->TranslateVaToPa(VA, PA, bank);
  }

  ETranslationResultType VmPagingMapper::TranslatePaToVa(uint64 PA, EMemBankType bank, uint64& VA) const
  {
    return mpCurrentAddressSpace->TranslatePaToVa(PA, bank, VA);
  }

  const Page* VmPagingMapper::GetPage(uint64 VA) const
  {
    return mpCurrentAddressSpace->GetPage(VA);
  }

  bool VmPagingMapper::GetTranslationRange(uint64 VA, TranslationRange& rTransRange) const
  {
    return mpCurrentAddressSpace->GetTranslationRange(VA, rTransRange);
  }

  bool VmPagingMapper::VaInAddressErrorRange(const uint64 VA) const
  {
    return mpCurrentAddressSpace->VaInAddressErrorRange(VA);
  }

  const ConstraintSet* VmPagingMapper::GetVmConstraint(EVmConstraintType constrType) const
  {
    return mpCurrentAddressSpace->GetVmConstraint(constrType);
  }

  EExceptionConstraintType VmPagingMapper::GetExceptionConstraintType(const std::string& rExceptName) const
  {
    return mpCurrentAddressSpace->GetExceptionConstraintType(rExceptName);
  }

  bool VmPagingMapper::VerifyStreamingPageCrossing(uint64 start, uint64 end) const
  {
    // a slight optimization to exclude most non-page-crossing cases, only check if the start end addresses crosses the smallest page size.
    if ((start & PageCrossMask()) != (end & PageCrossMask())) {
      return mpCurrentAddressSpace->VerifyStreamingPageCrossing(start, end);
    }

    return true;
  }

  void VmPagingMapper::ApplyVmConstraints(const GenPageRequest* pPageReq, ConstraintSet& rConstr) const
  {
    mpCurrentAddressSpace->ApplyVmConstraints(pPageReq, rConstr);
  }

  EMemBankType VmPagingMapper::DefaultMemoryBank() const
  {
    return mpCurrentAddressSpace->DefaultMemoryBank();
  }

  void VmPagingMapper::DumpPage(ofstream& os) const
  {
    if (mpCurrentAddressSpace != nullptr)
      mpCurrentAddressSpace->DumpPage(os);
  }

  void VmPagingMapper::HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* payload)
  {
    switch (eventType) {
    case ENotificationType::PhysicalRegionAdded:
      if (mState != EVmStateType::Uninitialized) {
        auto phys_region = dynamic_cast<PhysicalRegion*>(payload);
        mpCurrentAddressSpace->MapPhysicalRegion(phys_region);
        LOG(info) << "{VmPagingMapper::HandleNotification} new physical region mapped: " << phys_region->ToString() << endl;
      }

      break;
    default:
      LOG(fail) << "{VmPagingMapper::HandleNotification} unexpected event: " << ENotificationType_to_string(eventType) << endl;
      FAIL("unexpected-event-type");
    }
  }

  uint32 VmPagingMapper::GenContextId() const
  {
    return mpCurrentAddressSpace->GetControlBlock()->GenContextId();
  }

  VmPagingRegime::VmPagingRegime(const VmFactory* pVmFactory, EVmRegimeType regimeType, EPrivilegeLevelType privType, EMemBankType bankType)
    : VmRegime(pVmFactory, regimeType, bankType), mPrivilegeLevelType(privType), mpDirectMapper(nullptr), mpPagingMapper(nullptr)
  {
  }

  VmPagingRegime::~VmPagingRegime()
  {
    delete mpDirectMapper;
    delete mpPagingMapper;
  }

  void VmPagingRegime::Setup(Generator* gen)
  {
    VmRegime::Setup(gen);

    mpDirectMapper = DirectMapperInstance();
    mpPagingMapper = PagingMapperInstance();
    mpDirectMapper->SetPagingRegime(this);
    mpPagingMapper->SetPagingRegime(this);
    mpDirectMapper->Setup(gen);
    mpPagingMapper->Setup(gen);

    mpCurrentMapper = mpDirectMapper;
  }

  VmDirectMapper* VmPagingRegime::DirectMapperInstance() const
  {
    return new VmDirectMapper(mpVmFactory, mDefaultBankType);
  }

  VmPagingMapper* VmPagingRegime::PagingMapperInstance() const
  {
    return new VmPagingMapper(mpVmFactory);
  }

  VmMapper* VmPagingRegime::GetVmMapper(const VmInfo& rVmInfo)
  {
    if (rVmInfo.PagingEnabled()) {
      return mpPagingMapper;
    }
    else {
      return mpDirectMapper;
    }
  }

  void VmPagingRegime::UpdateCurrentMapper()
  {
    VmMapper* new_mapper = nullptr;
    if (PagingEnabled())
    {
      new_mapper = mpPagingMapper;
    }
    else
    {
      new_mapper = mpDirectMapper;
    }

    if (mpCurrentMapper != new_mapper)
    {
      if (nullptr != mpCurrentMapper)
      {
        mpCurrentMapper->Deactivate();
      }
      mpCurrentMapper = new_mapper;
    }
  }

  void VmPagingRegime::Activate()
  {
    if (mState == EVmStateType::Uninitialized)
    {
      Initialize();
    }

    if (mState == EVmStateType::Initialized)
    {
      mState = EVmStateType::Active;
    }

    UpdateCurrentMapper();

    mpCurrentMapper->Activate();
  }

  void VmPagingRegime::Initialize()
  {
    if (mState == EVmStateType::Uninitialized)
    {
      UpdateCurrentMapper();
      mState = EVmStateType::Initialized;
    }

    mpCurrentMapper->Initialize();
  }

  void VmPagingRegime::Deactivate()
  {
    mState = EVmStateType::Uninitialized;
    mpPagingMapper->Deactivate();
    mpDirectMapper->Deactivate();
  }

  void VmPagingRegime::DumpPage(std::ofstream& os) const
  {
    if (mpPagingMapper != nullptr)
      mpPagingMapper->DumpPage(os);
  }

  VmMapper* VmPagingRegime::FindVmMapper(const VmContext& rVmContext)
  {
    if (PagingEnabled()) {
      return mpPagingMapper->FindAddressSpace(rVmContext);
    }
    else {
      LOG(fail) << "{VmPagingRegime::FindVmMapper} not expecting to be called in paging-off mode." << endl;
      FAIL("find-vm-mapper-not-expected-when-paging-off");
    }
    return nullptr;
  }

  VmMapper* VmPagingRegime::CreateVmMapper(const VmContext* pVmContext)
  {
    if (PagingEnabled()) {
      return mpPagingMapper->CreateAddressSpace(pVmContext);
    }
    else {
      LOG(fail) << "{VmPagingRegime::CreateVmMapper} not expecting to be called in paging-off mode." << endl;
      FAIL("create-vm-mapper-not-expected-when-paging-off");
    }
    return nullptr;
  }

}
