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
#include <GenQueryAgent.h>
#include <GenQuery.h>
#include <Generator.h>
#include <Register.h>
#include <Log.h>

#include <InstructionResults.h>
#include <InstructionStructure.h>
#include <Instruction.h>
#include <Operand.h>
#include <StringUtils.h>
#include <string.h>
#include <ExceptionRecords.h>
#include <PageInfoRecord.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <AddressTagging.h>
#include <ChoicesModerator.h>
#include <Choices.h>
#include <MemoryManager.h>
#include <ExceptionManager.h>
#include <ResourceDependence.h>
#include <GenMode.h>
#include <RestoreLoop.h>
#include <Data.h>

#include <sstream>
#include <memory>

using namespace std;

namespace Force {

  Object* GenQueryAgent::Clone() const
  {
    return new GenQueryAgent(*this);
  }

  void GenQueryAgent::HandleQuery() const
  {

    LOG(notice) << "GenQueryAgent::HandleQuery : " << EQueryType_to_string(mpQuery->QueryType()) << endl;

    switch (mpQuery->QueryType()) {
    case EQueryType::RegisterIndex:
      RegisterIndex();
      break;
    case EQueryType::RegisterReloadValue:
      RegisterReloadValue();
      break;
    case EQueryType::RegisterInfo:
      RegisterInfo();
      break;
    case EQueryType::InstructionRecord:
      InstructionRecord();
      break;
    case EQueryType::GenState:
      GenState();
      break;
    case EQueryType::PageInfo:
      PageInfo();
      break;
    case EQueryType::BranchOffset:
      BranchOffset();
      break;
    case EQueryType::RegisterFieldInfo:
      RegisterFieldInfo();
      break;
    case EQueryType::ChoicesTreeInfo:
      ChoicesTreeInfo();
      break;
    case EQueryType::SimpleExceptionsHistory:
      ExceptionsHistory(0);
      break;
    case EQueryType::AdvancedExceptionsHistory:
      ExceptionsHistory(1);
      break;
    case EQueryType::GetVmContextDelta:
      GetVmContextDelta();
      break;
    case EQueryType::GetVmCurrentContext:
      GetVmCurrentContext();
      break;
    case EQueryType::HandlerSetMemory:
      HandlerSetMemory();
      break;
    case EQueryType::ExceptionVectorBaseAddress:
      ExceptionVectorBaseAddress();
      break;
    case EQueryType::ResourceEntropy:
      ResourceEntropy();
      break;
    case EQueryType::RestoreLoopContext:
      RestoreLoopContext();
      break;

    default:
        LOG(notice) << "GenQueryAgent::HandleQuery U" << endl;
        Utility();
        break;
    }
    // << "GenQueryAgent::HandleQuery " << EQueryType_to_string(mpQuery->QueryType()) << endl;
    mpQuery = nullptr;
  }

  void GenQueryAgent::RegisterIndex() const
  {
    auto reg_file = mpGenerator->GetRegisterFile();
    uint32 index = reg_file->RegisterLookup(mpQuery->PrimaryString())->IndexValue();
    auto target_query = dynamic_cast<const GenRegisterIndexQuery* >(mpQuery);
    target_query->SetIndex(index);
  }

  void GenQueryAgent::RegisterReloadValue() const
  {
    auto reg_file = mpGenerator->GetRegisterFile();
    Register* pRegister = reg_file->RegisterLookup(mpQuery->PrimaryString());
    // get choice moderator.
    auto pChoicesModerator = mpGenerator->GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);
    auto reloadValueQuery = dynamic_cast<const GenRegisterReloadValueQuery*>(mpQuery);
    uint64 reloadValue = pRegister->ReloadValue(pChoicesModerator, reloadValueQuery->FieldConstraintMap());
    reloadValueQuery->SetReloadValue(reloadValue);
  }

  void GenQueryAgent::RegisterFieldInfo() const
  {
    auto reg_file = mpGenerator->GetRegisterFile();
    auto reg_name = mpQuery->PrimaryString();
    Register* pRegister = reg_file->RegisterLookup(reg_name);
    // get choice moderator.
    auto pChoicesModerator = mpGenerator->GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);
    auto regFieldInfoQuery = dynamic_cast<const GenRegisterFieldInfoQuery*>(mpQuery);
    uint64 mask=0ull, value=0ull;
    auto fieldMap = regFieldInfoQuery->FieldConstraintMap();
    pRegister->RegisterFieldInfo(pChoicesModerator, fieldMap, mask, value);
    regFieldInfoQuery->SetRegisterFieldInfo(mask, value);
  }

  void GenQueryAgent::RegisterInfo() const
  {
    auto reg_file = mpGenerator->GetRegisterFile();
    auto target_query = dynamic_cast<const GenRegisterInfoQuery*>(mpQuery);
    string name, new_name;
    uint64 index = target_query->Index();
    name = target_query->PrimaryString();
    reg_file->ConvertRegisterName(name, index, new_name);
    auto reg_ptr = reg_file->RegisterLookup(new_name);
    target_query->SetRegisterType(ERegisterType_to_string(reg_ptr->RegisterType()));
    target_query->SetRegisterWidth(reg_ptr->Size());
    if (reg_ptr->IsInitialized() == true)
    {
      if (std::string(reg_ptr->Type()) == "LargeRegister")
      {
        LargeRegister* large_reg_ptr = dynamic_cast<LargeRegister*>(reg_ptr);
        std::vector<uint64> myVals = large_reg_ptr->Values();
        target_query->SetRegisterValues(myVals);
      }
      else
      {
        uint64 myVal = reg_ptr->Value();
        target_query->SetRegisterValue(myVal);
      }
    }
  }

  void GenQueryAgent::UpdateTargetQueryWithOperandInformation(bool check_reg_status, const GenInstructionRecordQuery* pTargetQuery, Operand* pOperand) const
  {
    if (pOperand->IsImmediateOperand())
    // verify if this is immediate operand
    {
      pTargetQuery->AddImms (pOperand->Name(), pOperand->Value());
    }
    if (pOperand->IsRegisterOperand())
    // verify if this is register operand
    {
      const OperandStructure* op_str = pOperand->GetOperandStructure();
      ERegAttrType access = op_str->mAccess;

      if ((access == ERegAttrType::Write)||(access == ERegAttrType::ReadWrite))
      {
        pTargetQuery->AddDests(pOperand->Name(),pOperand->Value());
      }

      if ((access == ERegAttrType::Read)||(access == ERegAttrType::ReadWrite))
      {
        pTargetQuery->AddSrcs(pOperand->Name(),pOperand->Value());
      }

      if (check_reg_status == true)
      {
        if ( (pOperand->Name().compare("Ws")==0) ||
             (pOperand->Name().compare("Xs")==0) )
        {
          pTargetQuery->AddStatus(pOperand->Name(), pOperand->Value());
        }
      }
    }
    AddressingOperand *addr_pOperand = dynamic_cast<AddressingOperand* >(pOperand);
    if (nullptr != addr_pOperand)
    {
      auto addr_str = dynamic_cast<const AddressingOperandStructure* >(pOperand->GetOperandStructure());
      std::string base_name = addr_str->Base();
      if (!base_name.empty())
      {
        auto base_pOperand = addr_pOperand->MatchOperand(base_name);
        if (nullptr != base_pOperand)
        {
          pTargetQuery->AddAddressingName ("Base", base_name);
          pTargetQuery->AddAddressingIndex ("Base", base_pOperand->Value());
        }
      }
      std::string offset_name = addr_str->Offset();
      if (!offset_name.empty())
      {
        auto offset_pOperand = addr_pOperand->MatchOperand(offset_name);
        if (nullptr != offset_pOperand)
        {
          pTargetQuery->AddAddressingName ("Offset", offset_name);
          pTargetQuery->AddAddressingIndex ("Offset", offset_pOperand->Value());
        }
      }

      BranchOperand *br_pOperand = dynamic_cast<BranchOperand* >(pOperand);
      if (nullptr != br_pOperand)
      {
        pTargetQuery->SetBRTarget(br_pOperand->TargetAddress());
      }
      LoadStoreOperand *ls_pOperand = dynamic_cast<LoadStoreOperand* >(pOperand);
      if (nullptr != ls_pOperand)
      {
        pTargetQuery->SetLSTarget(ls_pOperand->TargetAddress());

        auto ls_str = dynamic_cast<const LoadStoreOperandStructure* >(pOperand->GetOperandStructure());

        std::string extend_amount_name = ls_str->ExtendAmount();
        if (!extend_amount_name.empty())
        {
          auto extend_amount_pOperand = ls_pOperand->MatchOperand(extend_amount_name);
          if (nullptr != extend_amount_pOperand)
          {
            pTargetQuery->AddAddressingName ("ExtendAmount", extend_amount_name);
            pTargetQuery->AddAddressingIndex ("ExtendAmount", extend_amount_pOperand->Value());
          }
        }

        std::string index_name = ls_str->Index();
        if (!index_name.empty())
        {
          auto index_pOperand = ls_pOperand->MatchOperand(index_name);
          if (nullptr != index_pOperand)
          {
            pTargetQuery->AddAddressingName ("Index", index_name);
            pTargetQuery->AddAddressingIndex ("Index", index_pOperand->Value());
          }
        }
      }
    }
  }

  void GenQueryAgent::InstructionRecord() const
  {
    auto instr_res = mpGenerator->GetInstructionResults();
    auto vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
    string rec_id = mpQuery->PrimaryString();
    auto target_query = dynamic_cast<const GenInstructionRecordQuery* >(mpQuery);

    if (rec_id == "#") {
      target_query->SetValid(false);
      return;
    }

    StringSplitter ss(rec_id, '#');
    bool err_status = false;
    uint32 bank = 0;
    uint64 phys_addr = 0;
    uint64 vir_addr = 0x0ull;
    uint32 count = 0;

    while (!ss.EndOfString()) {
        std::string sub_str = ss.NextSubString();
        if (count == 0)
        {
            bank = parse_uint32 (sub_str, &err_status);
        }
        else if (count == 1)
        {
            phys_addr = parse_uint64 (sub_str, &err_status);
        }
        else
        {
            LOG(fail) << "Query Instruction Record - Invalid Record ID: " << rec_id << endl;
            FAIL("invalid-instruction-record-id");
        }
        count++;
    }

    const Instruction *instr_ptr = instr_res->LookupInstruction (bank, phys_addr);

    target_query->SetName(instr_ptr->FullName());
    target_query->SetOpcode(instr_ptr->Opcode());
    target_query->SetPA(phys_addr);  // TODO - use the given address for now
    target_query->SetBank(bank); //TODO - check if the memory bank is correct

    if (ETranslationResultType::Mapped == vm_mapper->TranslatePaToVa(phys_addr, EMemBankType(bank), vir_addr))
    {
      target_query->SetVA(vir_addr);
    }

    // set rest of record information inside Instruction
    // BRTarget, LSTarget, Group, RegisterList (Dests, Srcs) and Status, Imms,
    // AddressingOperand information (Base, Index, ExtendAmount, Offset, etc.)
    target_query->SetGroup (EInstructionGroupType_to_string(instr_ptr->Group()));

    bool check_reg_status = false;
    if (strcmp (instr_ptr->Type(), "LoadStoreInstruction") == 0)
    {
      check_reg_status = true;
    }

    for (auto opr_ptr : instr_ptr->GetOperands())
    {
      UpdateTargetQueryWithOperandInformation(check_reg_status, target_query, opr_ptr);

      //In the special case that one of the operands is a DataProcessingOperand we must
      //recurse in order to extract the full operand information. Note we still had
      //to call the Update method on the DataProcessingOperand itself.
      //
      //Also note that a GroupOperand, unless further specialized, does not need
      //this special treatment.
      auto data_proc_opr_ptr = dynamic_cast<DataProcessingOperand*>(opr_ptr);
      if (data_proc_opr_ptr)
      {
        for (auto sub_opr_ptr : data_proc_opr_ptr->GetSubOperands())
        {
          UpdateTargetQueryWithOperandInformation(check_reg_status, target_query, sub_opr_ptr);
        }
      }
    }
  }

  void GenQueryAgent::GenState() const
  {
    auto gen_state_query = mpQuery->ConstCastInstance<GenStateQuery>();
    string state_name = gen_state_query->PrimaryString();
    EGenStateType state_type = string_to_EGenStateType(state_name);
    bool has_state = true;
    uint64 state_value = 0;

    switch (state_type) {
    case EGenStateType::PC:
      state_value = mpGenerator->PC();
      gen_state_query->SetValue(state_value);
      break;
    case EGenStateType::InitialPC:
    case EGenStateType::BootPC:
      has_state = mpGenerator->GetStateValue(state_type, state_value);
      gen_state_query->SetValue(state_value);
      break;
    case EGenStateType::EL:
    case EGenStateType::PrivilegeLevel:
      state_value = mpGenerator->PrivilegeLevel();
      gen_state_query->SetValue(state_value);
      break;
    case EGenStateType::LastPC:
      state_value = mpGenerator->LastPC();
      gen_state_query->SetValue(state_value);
      break;
    case EGenStateType::GenMode:
      state_value = mpGenerator->GetGenMode()->CurrentMode();
      gen_state_query->SetValue(state_value);
      break;
    case EGenStateType::Endianness:
      if (mpGenerator->IsDataBigEndian()) {
        gen_state_query->SetValue(EEndiannessBaseType(EEndianness::BigEndian));
      }
      else {
        gen_state_query->SetValue(EEndiannessBaseType(EEndianness::LittleEndian));
      }
      break;
    default:
      /* Attempt to decode using architecture specific query */
      has_state = GenStateArch();
    }

    if (has_state) {
      LOG(info) << "{GenQueryAgent::GenState} state: " << state_name << "=0x" << hex << state_value << endl;
    } else {
      LOG(warn) << "{GenQueryAgent::GetState} Cannot retreive value for state: " << state_name << endl;
    }
  }

  void GenQueryAgent::PageInfo() const
  {
    //bool err_status;
    //uint64 addr = parse_uint64 (mpQuery->PrimaryString(), &err_status);
    auto target_query = dynamic_cast<const GenPageInfoQuery* >(mpQuery);
    uint64 addr       = target_query->Addr();
    std::string type  = target_query->Type();
    uint32 bank       = target_query->Bank();

    LOG(notice) << "{GenQueryAgent::PageInfo} type: " << type << "=0x" << hex << addr << " bank=0x" << bank << endl;


    PageInformation pageInformation;
    VmMapper* vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
    const auto addr_tagging = vm_mapper->GetAddressTagging();

    uint64 untagged_addr = addr;
    if (type == "VA")
    {
      untagged_addr = addr_tagging->UntagAddress(addr, false); //TODO this needs to reflect whether its instr or data mem, not just unconditionally untag
    }

    bool res = vm_mapper->GetPageInfo(untagged_addr, type, bank, pageInformation);

    if (res == true)
    {
      target_query->SetValid();
      target_query->Copy (pageInformation);
    }
  }

  void GenQueryAgent::BranchOffset() const
  {
    auto offset_query = dynamic_cast<const GenBranchOffsetQuery* >(mpQuery);

    bool offset_valid = false;
    uint64 offset_value = int64(offset_query->TargetAddress() - offset_query->BranchAddress()) >> offset_query->Shift(); // use signed number so the sign extension works after shifting.
    uint64 br_offset = get_offset_field(offset_value, offset_query->OffsetSize(), &offset_valid);

    offset_query->SetValid(offset_valid);
    if (offset_valid) {
      offset_query->SetOffset(br_offset);
    }
    else {
      uint32 num_hws = 1;
      uint64 target_addr = offset_query->TargetAddress();

      for (int i = 0; i < 3; ++ i) {
        target_addr >>= 16;
        if (target_addr & 0xffff) {
          ++ num_hws;
        }
      }

      offset_query->SetHalfWords(num_hws);
    }
  }

  void GenQueryAgent::Utility() const
  {
    auto gen_utility_query = mpQuery->ConstCastInstance<GenUtilityQuery>();
    const string& utility_name = gen_utility_query->PrimaryString();
    bool util_name_is_query_type = false;

    EQueryType query_type = try_string_to_EQueryType(utility_name, util_name_is_query_type);

    if (!util_name_is_query_type)
    {
      query_type = gen_utility_query->QueryType();
    }

    switch (query_type)
    {
      case EQueryType::MaxAddress:
      {
        auto vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
        if      (utility_name == string("Physical")) gen_utility_query->AddValue(vm_mapper->MaxPhysicalAddress());
        else if (utility_name == string("Virtual"))  gen_utility_query->AddValue(vm_mapper->MaxVirtualAddress());
        else
        {
          gen_utility_query->AddValue(vm_mapper->MaxVirtualAddress());
          LOG(warn) << "{GenQueryAgent::Utility} Max Address query type invalid, defaulting to max virtual address" << endl;
        }
        break;
      }
      case EQueryType::ValidAddressMask:
      {
        VmMapper* vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
        uint64 is_instr = gen_utility_query->ValueVariable(2);
        uint64 va = gen_utility_query->ValueVariable(1);
        const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
        uint64 max_address = vm_mapper->MaxVirtualAddress();
        uint64 mask = max_address;

        if (addr_tagging->CanTagAddress(va,is_instr))
            mask = 0xff00000000000000ull | max_address;
        gen_utility_query->AddValue(mask);
        break;
      }
      case EQueryType::PickedValue:
      {
        auto value_range =  gen_utility_query->StringVariable(1);
        ConstraintSet value_constr(value_range);
        gen_utility_query->AddValue(value_constr.ChooseValue());
        break;
      }
      case EQueryType::GenData:
      {
        auto value_str = gen_utility_query->StringVariable(1);
        std::unique_ptr<const Data> data_ptr(DataFactory::Instance()->BuildData(value_str, *mpGenerator));
        uint32 width = data_ptr.get()->GetDataWidth();
        if (width > 8)
        {
          std::vector<uint64> datas;
          data_ptr.get()->ChooseLargeData(datas);
          for (auto data : datas)
          {
            gen_utility_query->AddValue(data);
          }
        }
        else
        {
          gen_utility_query->AddValue(data_ptr.get()->ChooseData());
        }
        break;
      }
      default:
          /* Attempt to decode using architecture specific query */
          util_name_is_query_type = UtilityArch();
          break;
    }

    if (util_name_is_query_type)
        util_name_is_query_type = false;
  }

  void GenQueryAgent::ChoicesTreeInfo() const
  {
    auto tree_name = mpQuery->PrimaryString();
    auto choices_tree_query = dynamic_cast<const GenChoicesTreeInfoQuery* >(mpQuery);
    auto choices_type = string_to_EChoicesType(choices_tree_query->ChoicesType());
    auto choices_mod = mpGenerator->GetChoicesModerator(choices_type);

    auto choices_raw = choices_mod->CloneChoiceTree(tree_name);
    std::unique_ptr<ChoiceTree> choices_tree(choices_raw);

    auto choices = choices_tree->GetChoices();
    for (const auto choice : choices ) {
      auto choice_type = choice->Type();
      if (strcmp(choice_type, "Choice") == 0) {
        choices_tree_query->AddChoiceInfo(choice->Name(), choice->Weight());
      }
      else if (strcmp(choice_type, "RangeChoice") == 0) {
        auto range_choice = dynamic_cast<const RangeChoice* >(choice);
        uint32 lower, high;
        range_choice->GetRange(lower, high);
        stringstream name_stream;
        name_stream << lower << "-"<< high;
        choices_tree_query->AddChoiceInfo(name_stream.str(), range_choice->Weight());
      }
      else {
        LOG(fail) << "Unknown choice type \"" << choice_type << "\"" << endl;
        FAIL("Unknown-choice-type");
      }
    }
  }

  void GenQueryAgent::ExceptionsHistory(int isAdvanced) const
  {
    std::vector<ExceptionRecord> except_records;
    auto gen_exceptions_query = mpQuery->ConstCastInstance<GenExceptionsHistoryQuery>();

    const std::map<std::string, std::string>& inputAgrs = gen_exceptions_query->GetInputArgs();

    if (isAdvanced)
    {
        mpGenerator->GetExceptionRecordManager()->GetExceptionHistoryByArgs(inputAgrs, except_records);
    }
    else
    {
      /* Retrieve the error code parameter */
      uint32 except_class = (uint32) std::stoi(gen_exceptions_query->PrimaryString());
      /* Retrieve the history this event has happened */
      mpGenerator->GetExceptionRecordManager()->GetExceptionHistoryByEC((EExceptionClassType) except_class, except_records);
    }

    gen_exceptions_query->SetErrorRecords(except_records);

    LOG(notice) << "{GenQueryAgent::GenRegisterRecords} record count: " << except_records.size() << endl;
  }

  void GenQueryAgent::GetVmContextDelta() const
  {
    auto get_vm_context_delta_query = mpQuery->ConstCastInstance<GetVmContextDeltaQuery>();
    const std::map<std::string, uint64>& inputAgrs = get_vm_context_delta_query->GetInputArgs();
    std::map<std::string, uint64> & deltaMap = get_vm_context_delta_query->GetDeltaMap();
    auto vm_manager = mpGenerator->GetVmManager();
    auto itr = inputAgrs.find("VmContextId");
    if (itr != inputAgrs.end())
    {
      uint32 contextId = itr->second;
      bool success = vm_manager->GetVmContextDelta(deltaMap, contextId );
      if (!success)
      {
        LOG(fail) << "GetVmContextDelta not successful" << endl;
        FAIL("GetVmContextDelta-failed");
      }
    }
    else
    {
      LOG(fail) << "GetVmContextDelta missing paramter VmContextId" << endl;
      FAIL("GetVmContextDelta-parameter-failed");
    }
  }

  void GenQueryAgent::GetVmCurrentContext() const
  {
    auto get_vm_current_context_query = mpQuery->ConstCastInstance<VmCurrentContextQuery>();
    auto vm_manager = mpGenerator->GetVmManager();
    auto vm_current_context  = vm_manager->GetVmCurrentContext();
    get_vm_current_context_query->SetId(vm_current_context);
    LOG(notice) << "{GenQueryAgent::GetVmCurrentContext} vmCurrentContext  " << vm_current_context << endl;
  }

  void GenQueryAgent::HandlerSetMemory() const
  {
    auto handle_memory_query = mpQuery->ConstCastInstance<GenHandlerSetMemoryQuery>();
    EMemBankType bank = (EMemBankType) std::stoi(handle_memory_query->PrimaryString());
    auto mem_manager = mpGenerator->GetMemoryManager();
    auto physical_regions = mem_manager->GetPhysicalRegions();

    for (auto phy_region : physical_regions ) {
      if (phy_region->RegionType() == EPhysicalRegionType::HandlerMemory
          and phy_region->MemoryBank() == bank) {
        handle_memory_query->SetMemoryBase(phy_region->Lower());
        handle_memory_query->SetMemorySize(phy_region->Size());
        break;
      }
    }
  }

  void GenQueryAgent::ExceptionVectorBaseAddress() const
  {
    auto gen_query = mpQuery->ConstCastInstance<GenExceptionVectorBaseAddressQuery>();
    auto vector_type = string_to_EExceptionVectorType(gen_query->PrimaryString());
    uint64 vbar_data = ExceptionManager::Instance()->ExceptionVectorBaseAddress(vector_type);
    gen_query->SetVectorBaseAddress(vbar_data);
  }

  void GenQueryAgent::ResourceEntropy() const
  {
    auto entropy_query = mpQuery->ConstCastInstance<GenResourceEntropyQuery>();
    auto resource_type = string_to_EResourceType(entropy_query->PrimaryString());
    auto resource_dep = mpGenerator->GetDependenceInstance();
    auto resource_entropies = resource_dep->GetResourceTypeEntropies();
    auto resource_entropy = resource_entropies[EResourceTypeBaseType(resource_type)];
    auto source_entropy = resource_entropy->SourceEntropy();

    EntropyInfo source_info(source_entropy.State(), source_entropy.Entropy(), source_entropy.OnThreshold(), source_entropy.OffThreshold());
    entropy_query->SetSourceEntropy(source_info);

    auto dest_entropy = resource_entropy->DestEntropy();
    EntropyInfo dest_info(dest_entropy.State(), dest_entropy.Entropy(), dest_entropy.OnThreshold(), dest_entropy.OffThreshold());
    entropy_query->SetDestEntropy(dest_info);
  }

  void GenQueryAgent::RestoreLoopContext() const
  {
    RestoreLoopManagerRepository* restore_loop_manager_repository = RestoreLoopManagerRepository::Instance();
    RestoreLoopManager* restore_loop_manager = restore_loop_manager_repository->GetRestoreLoopManager(mpGenerator->ThreadId());
    auto target_query = dynamic_cast<const GenRestoreLoopContextQuery*>(mpQuery);
    target_query->SetLoopId(restore_loop_manager->GetCurrentLoopId());
    target_query->SetLoopBackAddress(restore_loop_manager->GetCurrentLoopBackAddress());
    target_query->SetBranchRegisterIndex(restore_loop_manager->GetBranchRegisterIndex());
  }

}
