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
#include <GenVirtualMemoryAgent.h>
#include <GenRequest.h>
#include <Generator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <VaGenerator.h>
#include <PaGenerator.h>
#include <Constraint.h>
#include <MemoryManager.h>
#include <Config.h>
#include <VmUtils.h>
#include <Random.h>
#include <AddressTagging.h>
#include <VmInfo.h>
#include <PhysicalPageSplitter.h>
#include <UtilityFunctions.h>
#include <FreePageRangeClaimer.h>
#include <Architectures.h>
#include <Log.h>

#include <memory>

using namespace std;

namespace Force {

  Object* GenVirtualMemoryAgent::Clone() const
  {
    return new GenVirtualMemoryAgent(*this);
  }

  void  GenVirtualMemoryAgent::SetGenRequest(GenRequest* genRequest)
  {
    mpVirtualMemoryRequest = dynamic_cast<GenVirtualMemoryRequest* >(genRequest);
  }

  void  GenVirtualMemoryAgent::HandleRequest()
  {
    EVmRequestType req_type = mpVirtualMemoryRequest->VmRequestType();
    // << "EVmRequestType: " << EVmRequestType_to_string(req_type) << endl;
    switch (req_type) {
    case EVmRequestType::GenPA:
      GenPA();
      break;
    case EVmRequestType::GenVA:
      GenVA();
      break;
    case EVmRequestType::GenVMVA:
      GenVMVA();
      break;
    case EVmRequestType::GenVAforPA:
      GenVAforPA();
      break;
    case EVmRequestType::GenVmContext:
      GenVmContext();
      break;
    case EVmRequestType::UpdateVm:
      UpdateVm();
      break;
    case EVmRequestType::PhysicalRegion:
      GetPhysicalRegion();
      break;
    case EVmRequestType::GenFreePage:
      GenFreePageRanges();
      break;
    default:
      LOG(fail) << "{GenVirtualMemoryAgent::HandleRequest} unsupported GenVirtualMemoryRequest: " << EVmRequestType_to_string(req_type) << endl;
      FAIL("unsupported-gen-virtual-memory-request-type");
    }
  }

  void GenVirtualMemoryAgent::CleanUpRequest()
  {
    delete mpVirtualMemoryRequest;
    mpVirtualMemoryRequest = nullptr;
  }

  void GenVirtualMemoryAgent::ResetRequest()
  {
    mpVirtualMemoryRequest = nullptr;
  }

  void GenVirtualMemoryAgent::GetPhysicalRegion()
  {
    auto req =  mpVirtualMemoryRequest->CastInstance<GenPhysicalRegionRequest>();
    EPhysicalRegionType region_type = req->RegionType();

    switch (region_type) {
    case EPhysicalRegionType::HandlerMemory:
      HandlerMemory();
      break;
    case EPhysicalRegionType::ExceptionStack:
      ExceptionStack();
      break;
    case EPhysicalRegionType::BootRegion:
      BootRegion();
      break;
    case EPhysicalRegionType::ResetRegion:
      ResetRegion();
      break;
    default:
      LOG(fail) << "{GenVirtualMemoryAgent::GetPhysicalRegion} unsupported physical region type: " << EPhysicalRegionType_to_string(region_type) << endl;
      FAIL("unsupported-gen-virtual-memory-request-type");
    }
  }

  void GenVirtualMemoryAgent::GenVA()
  {
    uint64 ret_va = 0;
    auto va_req = dynamic_cast<GenVaRequest* > (mpVirtualMemoryRequest);
    bool is_instr = (va_req->DataType() == EMemDataType::Instruction);
    if (is_instr and va_req->SharedMemory())
    {
      LOG(fail) << "{GenVirtualMemoryAgent::GenVA} instruction memory cannot be designated as shared." << endl;
      FAIL("shared-instruction-memory");
    }

    EMemAccessType va_access_type = EMemAccessType::ReadWrite;

    if (is_instr)
    {
      va_access_type = EMemAccessType::Read;
    }

    VmManager* vm_manager = mpGenerator->GetVmManager();
    VmMapper* vm_mapper = vm_manager->CurrentVmMapper();

    if (va_req->VmSpecified())
    {
      std::unique_ptr<VmInfo> vm_info_storage(vm_manager->VmInfoInstance()); // to release vm_info when done.

      vm_info_storage.get()->SetPrivilegeLevel(uint32(va_req->PrivilegeLevel()));
      vm_info_storage.get()->GetOtherStates(*mpGenerator); // obtain other states from the current PE states.

      bool valid_regime = false;
      EVmRegimeType regime_type = vm_info_storage.get()->RegimeType(&valid_regime);
      if (valid_regime)
      {
        LOG(info) << "{GenVirtualMemoryAgent::GenVA} target VM info: " << vm_info_storage.get()->ToString() << " regime type: " << EVmRegimeType_to_string(regime_type) << endl;
        vm_mapper = vm_manager->GetVmMapper(*vm_info_storage.get());
        vm_mapper->Initialize();
      }
      else
      {
        LOG(notice) << "{GenVirtualMemoryAgent::GenVA} target VM not valid, VM info: " << vm_info_storage.get()->ToString() << ". Use current VmRegime." << endl;
      }
    }

    std::unique_ptr<GenPageRequest> local_page_req(vm_mapper->GenPageRequestRegulated(is_instr, va_access_type));
    SetCommonPageRequestAttributes(*va_req, local_page_req.get());

    local_page_req->SetGenBoolAttribute(EPageGenBoolAttrType::ForceAlias, va_req->ForceAlias());
    local_page_req->SetAttributeValue(EPageRequestAttributeType::AliasPageId, va_req->PhysPageId());

    VaGenerator va_gen(vm_mapper, local_page_req.get());
    ret_va = va_gen.GenerateAddress(va_req->Align(), va_req->Size(), is_instr, va_access_type, va_req->MemoryRangesConstraint());

    if (va_req->SharedMemory()) {
      PhysicalPageSplitter page_splitter(vm_mapper);
      PhysicalPageSplit page_split = page_splitter.GetPhysicalPageSplit(ret_va, va_req->Size());

      MemoryManager* mem_man = mpGenerator->GetMemoryManager();
      mem_man->MarkShared(PaTuple(page_split.mPa1, EMemBankTypeBaseType(page_split.mBank1)), page_split.mSize1);
      if (page_split.mSize2 > 0) {
        mem_man->MarkShared(PaTuple(page_split.mPa2, EMemBankTypeBaseType(page_split.mBank2)), page_split.mSize2);
      }
    }

    const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
    if (va_req->Tag() > 255) {
      // No Tag Specified
      ret_va = addr_tagging->TagAddressRandomly(ret_va, is_instr);
    }
    else ret_va = addr_tagging->TagAddress(ret_va, va_req->Tag(), is_instr);
    va_req->mVA = ret_va;
  }

  void GenVirtualMemoryAgent::GenVMVA()
  {
    auto vm_va_req = dynamic_cast<GenVmVaRequest* > (mpVirtualMemoryRequest);
    // << "GenVirtualMemoryAgent::GenVMVA" << endl;
    vm_va_req->mVA = 0x1000;
  }

  void GenVirtualMemoryAgent::GenPA()
  {
    uint64 ret_pa = 0x0ull;
    auto pa_req = mpVirtualMemoryRequest->CastInstance<GenPaRequest>();
    bool is_instr = (pa_req->DataType() == EMemDataType::Instruction);
    if (is_instr and pa_req->SharedMemory()) {
      LOG(fail) << "{GenVirtualMemoryAgent::GenPA} instruction memory cannot be designated as shared." << endl;
      FAIL("shared-instruction-memory");
    }

    auto vm_man = mpGenerator->GetVmManager();
    auto vm_regime = vm_man->CurrentVmRegime();
    auto vm_mapper = vm_man->CurrentVmMapper();

    uint64 max_phys_addr = vm_mapper->MaxPhysicalAddress();

    EMemBankType mem_bank_type = vm_regime->DefaultMemoryBank();
    pa_req->BankSpecified(mem_bank_type);

    MemoryManager* mem_man = mpGenerator->GetMemoryManager();
    MemoryBank* mem_bank = mem_man->GetMemoryBank(EMemBankTypeBaseType(mem_bank_type));
    std::unique_ptr<ConstraintSet> usable_constr(mem_bank->Usable()->Clone());
    usable_constr->SubRange(max_phys_addr + 1, usable_constr->UpperBound());

    if (pa_req->ForceNewAddr()) {
      usable_constr->ApplyConstraintSet(*(mem_bank->Unmapped()));
    }

    PaGenerator pa_gen(usable_constr.get());

    ret_pa = pa_gen.GenerateAddress(pa_req->Align(), pa_req->Size(), is_instr, pa_req->MemoryRangesConstraint());

    if (pa_req->SharedMemory()) {
      mem_man->MarkShared(PaTuple(ret_pa, EMemBankTypeBaseType(mem_bank_type)), pa_req->Size());
    }

    // << "genpa pa=0x" << hex << ret_pa << " bank=" << EMemBankType_to_string(mem_bank_type) << " size=0x" << pa_req->Size() << endl;

    pa_req->mPA = ret_pa;
  }

  //use cases for GenVAforPA
  //PA has not been initialized - get a VA and new page mapping to specified PA
  //   PA has been initialized - get a VA mapping & aliased to specified PA
  //OR PA has been initialized - return existing VA mapping to the PA, if possible and "ForceNewAddr" not set
  void GenVirtualMemoryAgent::GenVAforPA()
  {
    auto vapa_req = dynamic_cast<GenVaForPaRequest* > (mpVirtualMemoryRequest);
    uint64 ret_va = 0;
    bool is_instr = (vapa_req->DataType() == EMemDataType::Instruction);

    VmManager* vm_manager = mpGenerator->GetVmManager();
    VmMapper* vm_mapper = vm_manager->CurrentVmMapper();

    EMemBankType mem_bank = vm_manager->CurrentVmRegime()->DefaultMemoryBank();

    if (vapa_req->VmSpecified()) {
      auto vm_info    = vm_manager->VmInfoInstance();
      std::unique_ptr<VmInfo> vm_info_storage(vm_info); // to release vm_info when done.

      vm_info->SetPrivilegeLevel(uint32(vapa_req->PrivilegeLevel()));
      vm_info->GetOtherStates(*mpGenerator); // obtain other states from the current PE states.

      bool valid_regime = false;
      EVmRegimeType regime_type = vm_info->RegimeType(&valid_regime);
      if (valid_regime) {
        LOG(info) << "{GenVirtualMemoryAgent::GenVAforPA} target VM info: " << vm_info->ToString() << " regime type: " << EVmRegimeType_to_string(regime_type) << endl;
        vm_mapper = vm_manager->GetVmMapper(*vm_info);
        vm_mapper->Initialize();
        mem_bank = vm_mapper->DefaultMemoryBank();
      }
      else {
        LOG(notice) << "{GenVirtualMemoryAgent::GenVAforPA} target VM not valid, VM info: " << vm_info->ToString() << ". Use current VmRegime." << endl;
      }
    }

    std::unique_ptr<GenPageRequest> local_page_req(vm_mapper->GenPageRequestRegulated(is_instr, EMemAccessType::ReadWrite));
    SetCommonPageRequestAttributes(*vapa_req, local_page_req.get());

    //local_page_req->SetGenBoolAttribute(EPageGenBoolAttrType::ForceAlias, vapa_req->ForceAlias());

    local_page_req->SetAttributeValue(EPageRequestAttributeType::PA, vapa_req->PhysAddr());

    if (vapa_req->BankSpecified(mem_bank)) {
      local_page_req->SetBankType(mem_bank);
    }

    LOG(trace) << "{GenVirtualMemoryAgent::GenVAforPA} phys_addr=0x" << hex << vapa_req->PhysAddr() << " bank=" << EMemBankType_to_string(mem_bank)
               << " size=0x" << vapa_req->Size() << " align=0x" << vapa_req->Align()
               << " flatmap=0x" << vapa_req->FlatMap() << " force_attrs=" << vapa_req->ForceMemAttrs()
               << " canalias=" << vapa_req->CanAlias() << " force_new_addr=" << vapa_req->ForceNewAddr() << endl;

    ret_va = vm_mapper->MapAddressRangeForPA(vapa_req->PhysAddr(), mem_bank, vapa_req->Size(), is_instr, local_page_req.get());

    /*if (vapa_req->SharedMemory()) {
      PhysicalPageSplitter page_splitter(vm_mapper);
      PhysicalPageSplit page_split = page_splitter.GetPhysicalPageSplit(ret_va, vapa_req->Size());

       << "page_split pa1=0x" << hex << page_split.mPa1 << " bank1=0x" << page_split.mBank1 << " size1=0x" << page_split.mSize1 << endl;
       << "page_split pa2=0x" << hex << page_split.mPa2 << " bank2=0x" << page_split.mBank2 << " size2=0x" << page_split.mSize2 << endl;

      MemoryManager* mem_man = mpGenerator->GetMemoryManager();
      mem_man->MarkShared(PaTuple(page_split.mPa1, EMemBankTypeBaseType(page_split.mBank1)), page_split.mSize1);
      if (page_split.mSize2 > 0) {
        mem_man->MarkShared(PaTuple(page_split.mPa2, EMemBankTypeBaseType(page_split.mBank2)), page_split.mSize2);
      }
    }*/

    const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
    if (vapa_req->Tag() > 255) {
      // No Tag Specified
      ret_va = addr_tagging->TagAddressRandomly(ret_va, is_instr);
    }
    else ret_va = addr_tagging->TagAddress(ret_va, vapa_req->Tag(), is_instr);
    vapa_req->mVA = ret_va;
  }

  void GenVirtualMemoryAgent::HandlerMemory()
  {
    auto hm_req = mpVirtualMemoryRequest->CastInstance<GenPhysicalRegionRequest>();

    uint64 ret_pa = 0;
    uint64 size = hm_req->Size();
    bool is_instr = (hm_req->DataType() == EMemDataType::Instruction);
    uint32 mem_bank = uint32(hm_req->MemoryBank());
    MemoryManager* mem_man = mpGenerator->GetMemoryManager();

    bool matched_handler_valid = false;
    auto matched_handler = Config::Instance()->GetOptionValue(ESystemOptionType_to_string(ESystemOptionType::MatchedHandler), matched_handler_valid);

    const PhysicalRegion* handler_region = nullptr;
    if (matched_handler_valid && matched_handler) {
      auto phys_regions = mem_man->GetPhysicalRegions();
      for (auto region : phys_regions) {
        if (region->RegionType() == EPhysicalRegionType::HandlerMemory) {
          handler_region = region;
          break;
        }
      }
    }

    if (handler_region != nullptr) {
      ret_pa = handler_region->Lower();
      LOG(info) << "HandlerMemory: option MatchedHandler is set, reuse the handler memory address to allocate region." << endl;
    }
    else {
      auto usable_constr = mem_man->GetMemoryBank(mem_bank)->Usable();
      auto pa_var = mpGenerator->GetVariable("System Physical Address Range", EVariableType::String);
      ConstraintSet min_ps_constr(pa_var);
      min_ps_constr.ApplyConstraintSet(*(hm_req->MemoryRangesConstraint()));

      PaGenerator pa_gen(usable_constr);
      ret_pa = pa_gen.GenerateAddress(hm_req->Align(), size, is_instr, &min_ps_constr);
    }
    LOG(notice) << "HandlerMemory: align=0x" << hex << hm_req->Align() << " size 0x" << size << " is instr? " << is_instr << " handler base address [" << mem_bank << "]0x" << ret_pa << endl;
    hm_req->SetBaseAddress(ret_pa);

    char print_buffer[32];
    snprintf(print_buffer, 32, "HandlerMemory");
    auto new_region = new PhysicalRegion(print_buffer, ret_pa, ret_pa + (size - 1), EPhysicalRegionType::HandlerMemory, EMemBankType(mem_bank), hm_req->DataType());
    mem_man->AddPhysicalRegion(new_region);
  }

  void GenVirtualMemoryAgent::ExceptionStack()
  {
    auto es_req   = mpVirtualMemoryRequest->CastInstance<GenPhysicalRegionRequest>();
    bool is_instr = (es_req->DataType() == EMemDataType::Instruction);
    uint64 size   = es_req->Size();

    uint64 ret_pa = 0;
    MemoryManager* mem_man   = mpGenerator->GetMemoryManager();
    const vector<EMemBankType>& mem_banks = mpGenerator->GetArchInfo()->GetMemoryBanks();
    ConstraintSet usable_constr;

    // get a usable region that is common with all memory banks.
    bool first_done = false;
    for (auto mem_bank: mem_banks) {
      EMemBankTypeBaseType base_type = EMemBankTypeBaseType(mem_bank);
      auto bank_usable_constr = mem_man->GetMemoryBank(base_type)->Usable();
      if (not first_done) {
        usable_constr.MergeConstraintSet(*bank_usable_constr);
        first_done = true;
      }
      else {
        usable_constr.ApplyConstraintSet(*bank_usable_constr);
      }
    }

    auto pa_var = mpGenerator->GetVariable("System Physical Address Range", EVariableType::String);
    ConstraintSet min_ps_constr(pa_var);
    PaGenerator pa_gen(&usable_constr);
    ret_pa = pa_gen.GenerateAddress(es_req->Align(), size, is_instr, &min_ps_constr);
    LOG(notice) << "ExceptionStack: align=0x" << hex << es_req->Align() << " size 0x" << size << " is instr? " << is_instr << " exception stack base address 0x" << ret_pa << endl;
    es_req->SetBaseAddress(ret_pa);

    char print_buffer[32];
    for (auto mem_bank : mem_banks) {
      EMemBankTypeBaseType base_type = EMemBankTypeBaseType(mem_bank);
      snprintf(print_buffer, 32, "ExceptionStack%d_0x%x", base_type, mpGenerator->ThreadId());
      mpGenerator->ReserveMemory(print_buffer, ret_pa, size, base_type, false);
      RandomInitializeMemory(ret_pa, size, base_type, false);
      auto new_region = new PhysicalRegion(print_buffer, ret_pa, ret_pa + (size - 1),  EPhysicalRegionType::ExceptionStack, mem_bank, es_req->DataType());
      mem_man->AddPhysicalRegion(new_region);
    }
  }

  void GenVirtualMemoryAgent::ResetRegion()
  {
    auto mem_man = mpGenerator->GetMemoryManager();
    auto cfg_ptr = Config::Instance();
    auto br_req = mpVirtualMemoryRequest->CastInstance<GenPhysicalRegionRequest>();

    bool reset_pc_set = false;
    uint64 lower = cfg_ptr->GlobalStateValue(EGlobalStateType::ResetPC, reset_pc_set);
    if (not reset_pc_set) {
      LOG(fail) << "{GenVirtualMemoryAgent::ResetRegion} ResetPC value has not been specified" << endl;
      FAIL("reset-pc-undefined");
    }

    uint64 size = br_req->Size();
    uint64 upper = lower + size - 1;

    LOG(notice) << "ResetRegion:" << " size=0x" << hex << size << " start_addr=0x" << lower << endl;
    br_req->SetBaseAddress(lower);

    char print_buffer[32];
    snprintf(print_buffer, 32, "ResetRegion");
    mpGenerator->ReserveMemory(print_buffer, lower, size, 0, false);
    mem_man->AddPhysicalRegion(new PhysicalRegion(print_buffer, lower, upper, EPhysicalRegionType::ResetRegion, EMemBankType(0), EMemDataType::Instruction));
  }

  void GenVirtualMemoryAgent::BootRegion()
  {
    auto mem_man = mpGenerator->GetMemoryManager();
    auto br_req  = mpVirtualMemoryRequest->CastInstance<GenPhysicalRegionRequest>();

    uint64 lower = 0;
    bool boot_pc_set = mpGenerator->GetStateValue(EGenStateType::BootPC, lower);
    if (not boot_pc_set) {
      LOG(fail) << "{GenVirtualMemoryAgent::BootRegion} BootPC value has not been specified" << endl;
      FAIL("boot-pc-undefined");
    }

    uint64 size  = br_req->Size();
    uint64 upper = lower + size - 1;

    LOG(notice) << "BootRegion:" << " size=0x" << hex << size << " start_addr=0x" << lower << endl;
    br_req->SetBaseAddress(lower);

    char print_buffer[32];
    const ArchInfo* arch_info = mpGenerator->GetArchInfo();
    for (EMemBankType mem_bank : arch_info->GetMemoryBanks()) {
      snprintf(print_buffer, 32, "BootRegion%d_0x%x", EMemBankTypeBaseType(mem_bank), mpGenerator->ThreadId());
      mpGenerator->ReserveMemory(print_buffer, lower, size, EMemBankTypeBaseType(mem_bank), false);
      mem_man->AddPhysicalRegion(new PhysicalRegion(print_buffer, lower, upper, EPhysicalRegionType::BootRegion, mem_bank, EMemDataType::Instruction));
    }
  }

  void GenVirtualMemoryAgent::RandomInitializeMemory(uint64 physAddr, uint32 size, uint32 bank, bool isInstr)
  {
    uint32 addr_offset = physAddr & 0x7;
    if (addr_offset) {
      LOG(fail) << "{GenVirtualMemoryAgent::RandomInitializeMemory} address not properly aligned: 0x" << hex << physAddr << endl;
      FAIL("unexpected-unaligned-address");
    }

    uint64 addr_through = physAddr;
    uint64 end_addr = physAddr + (size - 1);
    auto random_handle = Random::Instance();
    for (; addr_through < end_addr; addr_through += 8) {
      uint64 data = random_handle->Random64();
      mpGenerator->InitializeMemory(addr_through, bank, 8, data, isInstr, false);
    }
  }

  void GenVirtualMemoryAgent::GenFreePageRanges()
  {
    FreePageRangeClaimer fp_claimer;
    fp_claimer.Setup(mpGenerator);

    auto fp_req  = mpVirtualMemoryRequest->CastInstance<GenFreePageRequest>();
    fp_req->RegulateRequest();
    fp_req->mValid = fp_claimer.ClaimFreePages(*fp_req->mpRequestRanges, fp_req->mRequestPageSizes, fp_req->mStartAddr, *fp_req->mpResolvedRanges, fp_req->mResolvedPageSizes);
  }
  void GenVirtualMemoryAgent::UpdateVm()
  {
    mpGenerator->SetupPageTableRegions();
    mpGenerator->UpdateVm();
  }

  void GenVirtualMemoryAgent::SetCommonPageRequestAttributes(const GenVirtualMemoryRequest& rGenVmReq, GenPageRequest* pPageReq)
  {
    if (nullptr != rGenVmReq.MemAttrImplConstraint())
    {
      pPageReq->SetMemAttrImplConstraint(rGenVmReq.MemAttrImplConstraint());
      LOG(trace) << "{GenVirtualMemoryAgent::SetPageRequestAttributes} setting mem attr impl from request to: " << hex << pPageReq->MemAttrImplConstraint()->ToSimpleString() << endl;
    }

    if (nullptr != rGenVmReq.TargetAliasAttrsConstraint())
    {
      pPageReq->SetTargetAliasAttrsConstraint(rGenVmReq.MemAttrImplConstraint());
      LOG(trace) << "{GenVirtualMemoryAgent::GenVA} setting target alias attrs from request to: " << hex << pPageReq->TargetAliasAttrsConstraint()->ToSimpleString() << endl;
    }

    if (rGenVmReq.PrivilegeLevelSpecified()) {
      pPageReq->SetPrivilegeLevel(rGenVmReq.PrivilegeLevel());
    }

    pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::FlatMap, rGenVmReq.FlatMap());
    pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::ForceMemAttrs, rGenVmReq.ForceMemAttrs());
    pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::CanAlias, rGenVmReq.CanAlias());
    pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::ForceNewAddr, rGenVmReq.ForceNewAddr());
  }

}
