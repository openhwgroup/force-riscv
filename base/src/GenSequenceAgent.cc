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
#include "GenSequenceAgent.h"

#include <algorithm>
#include <memory>

#include "AddressTagging.h"
#include "BntNode.h"
#include "BntNodeManager.h"
#include "BootOrder.h"
#include "Choices.h"
#include "ChoicesModerator.h"
#include "Config.h"
#include "Constraint.h"
#include "DataStation.h"
#include "GenRequest.h"
#include "GenRequest.h"
#include "Generator.h"
#include "InstructionResults.h"
#include "Log.h"
#include "MemoryManager.h"
#include "PeStateUpdate.h"
#include "ReExecutionManager.h"
#include "Register.h"
#include "RegisterReserver.h"
#include "ResourceDependence.h"
#include "ResourcePeState.h"
#include "RestoreLoop.h"
#include "SimplePeState.h"
#include "VaGenerator.h"
#include "VmManager.h"
#include "VmMapper.h"

using namespace std;

namespace Force {

  void  GenSequenceAgent::SetGenRequest(GenRequest* genRequest)
  {
    mpSequenceRequest = dynamic_cast<GenSequenceRequest* >(genRequest);
  }

  void  GenSequenceAgent::HandleRequest()
  {
    ESequenceType seq_type = mpSequenceRequest->SequenceType();
    LOG(notice) << "{GenSequenceAgent::HandleRequest} HandleRequest is called with : " << ESequenceType_to_string(seq_type) << endl;

    switch (seq_type) {
    case ESequenceType::LoadRegister:
      LoadRegister();
      break;
    case ESequenceType::CommitInstruction:
      CommitInstruction();
      break;
    case ESequenceType::BootLoading:
      BootLoading();
      break;
    case ESequenceType::EndOfTest:
      EndOfTest();
      break;
    case ESequenceType::JumpToStart:
      JumpToStart();
      break;
    case ESequenceType::BranchToTarget:
      BranchToTarget();
      break;
    case ESequenceType::Summary:
      Summary();
      break;
    case ESequenceType::InitialSetup:
      InitialSetup();
      break;
    case ESequenceType::RegisterReservation:
      RegisterReservation();
      break;
    case ESequenceType::EscapeCollision:
      EscapeCollision();
      break;
    case ESequenceType::BranchNotTaken:
      BranchNotTaken();
      break;
    case ESequenceType::BntNode:
      ProcessBntNode();
      break;
    case ESequenceType::SpeculativeBntNode:
      ProcessSpeculativeBntNode();
      break;
    case ESequenceType::ReExecution:
      ReExecution();
      break;
    case ESequenceType::UpdatePeState:
      UpdatePeState();
      break;
    case ESequenceType::UpdateRegisterField:
      UpdateRegisterField();
      break;
    case ESequenceType::SetRegister:
      SetRegister();
      break;
    case ESequenceType::ReloadRegister:
      ReloadRegister();
      break;
    case ESequenceType::BatchReloadRegisters:
      BatchReloadRegisters();
      break;
    case ESequenceType::ThreadSummary:
      ThreadSummary();
      break;
    case ESequenceType::InitializeAddrTables:
      InitializeAddrTables();
      break;
    case ESequenceType::LoadLargeRegister:
      LoadLargeRegister();
      break;
    case ESequenceType::BeginRestoreLoop:
      BeginRestoreLoop();
      break;
    case ESequenceType::EndRestoreLoop:
      EndRestoreLoop();
      break;
    case ESequenceType::RestoreLoopState:
      RestoreLoopState();
      break;
    case ESequenceType::LoopReconverge:
      LoopReconverge();
      break;
    default:
      LOG(notice) << "{GenSequenceAgent::HandleRequest} utility : " << endl;
      SequenceUtility();
    }

    delete mpSequenceRequest;
    mpSequenceRequest = nullptr;
  }

  void GenSequenceAgent::LoadRegister()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenLoadRegister>();
    auto reg_file = mpGenerator->GetRegisterFile();
    auto reg_ptr = reg_file->RegisterLookup(cast_req->RegisterName());

    vector<GenRequest* > req_seq;

    Register* inter_reg_ptr = nullptr;
    if (!cast_req->InterRegisterName().empty())
    {
      // intermedidate register is provided
      inter_reg_ptr = reg_file->RegisterLookup(cast_req->InterRegisterName());
    }

    GetLoadRegisterSequence(reg_ptr, cast_req->RegisterValue(), req_seq, inter_reg_ptr);

    if (not mpGenerator->HasISS()) {
      // add SetRegister request for NoISS mode.
      auto wider_reg_ptr = reg_file->GetContainingRegister(reg_ptr);
      if (wider_reg_ptr != nullptr)
      {
        if (!wider_reg_ptr->IsInitialized())
        {
          reg_file->InitializeRegisterRandomly(wider_reg_ptr);
        }
        req_seq.push_back(new GenSetRegister(wider_reg_ptr, cast_req->RegisterValue()));
      }
      else
      {
        req_seq.push_back(new GenSetRegister(reg_ptr, cast_req->RegisterValue()));
      }
    }

    mpGenerator->PrependRequests(req_seq);
  }

  void GenSequenceAgent::GetLoadRegisterSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr)
  {
    ERegisterType reg_type = regPtr->RegisterType();
    switch (reg_type) {
    case ERegisterType::GPR:
      GetLoadGPRSequence(regPtr, loadValue, reqSeq);
      break;
    default:
      GetLoadArchRegisterSequence(regPtr, loadValue, reqSeq, interRegPtr);
    }
  }

  void GenSequenceAgent::GetLoadArchRegisterSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr)
  {
    LOG(fail) << "{GenSequenceAgent::LoadArchRegister} not yet supported register type: " << ERegisterType_to_string(regPtr->RegisterType());
    FAIL("unsupported-arch-register-type");
  }

  void GenSequenceAgent::CommitInstruction()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenCommitInstruction>();
    auto instr_ptr = cast_req->GiveInstruction();
    auto instr_req = cast_req->GiveInstructionRequest();
    mpGenerator->CommitInstructionFinal(instr_ptr, instr_req);
  }

  void GenSequenceAgent::InsertToRegisterListByBoot( list<Register*>& registerList, Register* element) const
  {
    auto lower_bound_iter = std::lower_bound(registerList.begin(), registerList.end(), element, boot_comparator);
    registerList.insert(lower_bound_iter, element);    //reg_set_by_boot holding
  }

  void GenSequenceAgent::EndOfTest()
  {
    vector<GenRequest*> req_seq;
    GetEndOfTestSequence(req_seq);
    mpGenerator->PrependRequests(req_seq);

    mHasGenEndOfTest = true;
  }

  void GenSequenceAgent::BootLoading()
  {
    bool skip_boot_valid = false;
    uint64 skip_boot = Config::Instance()->GetOptionValue("SkipBootCode", skip_boot_valid);

    if (!skip_boot_valid || skip_boot == 0)
    {
      RestoreBootStates();

      auto registerFile = mpGenerator->GetRegisterFile();

      auto registers = registerFile->Registers();
      ChoicesModerator* pChoicesModerator = mpGenerator->GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);

      list<Register*> partial_regs_by_boot;
      list<Register*> initialized_regs;

      bool partial = false;
      for ( auto map_it = registers.begin(); map_it != registers.end(); ++map_it )
      {
        if ( 0 != map_it->second->Boot()) {
          if (map_it->second->IsInitialized( &partial )) {
            InsertToRegisterListByBoot(initialized_regs, map_it->second); // insert ordered
          } else if (true == partial) {
            InsertToRegisterListByBoot(initialized_regs, map_it->second); // insert ordered
            partial_regs_by_boot.push_back(map_it->second);
          }
        }
      }

      for ( auto reg_it = partial_regs_by_boot.rbegin(); reg_it != partial_regs_by_boot.rend(); ++reg_it )
      {
        // << " partial register " << (*reg_it)->Name() << endl;
        registerFile->InitializeRegisterRandomly(*reg_it, pChoicesModerator);
      }

      RegulateInitRegisters(initialized_regs);

      BootOrder* bootOrder = mpGenerator->GetBootOrder();

      bootOrder->InitiateRegisterList(initialized_regs);
      Register* boot_load_gpr = GetBootLoadingGPR();
      registerFile->InitializeRegisterRandomly(boot_load_gpr, pChoicesModerator);
      bootOrder->AssignLastGPR(boot_load_gpr);
      uint32 loading_size = bootOrder->GetLoadingAddressSize();
      if (loading_size > 0) {
        uint64 boot_load_addr = GetBootLoadingBaseAddress(loading_size);
        bootOrder->SetLoadingBaseAddress(boot_load_addr);
      }
      bootOrder->AdjustOrder();

      vector<GenRequest*> loadRegisterRequests;
      EGenModeTypeBaseType gen_mode_change = EGenModeTypeBaseType(EGenModeType::SimOff) | EGenModeTypeBaseType(EGenModeType::NoEscape);
      loadRegisterRequests.push_back(new GenStateRequest(EGenStateActionType::Push, EGenStateType::GenMode, gen_mode_change)); // modify GenMode to SimOff and NoEscape.
      GetBootLoadRegisterRequests(*bootOrder, loadRegisterRequests);
      bootOrder->JumpToTestStart(loadRegisterRequests);
      loadRegisterRequests.push_back(new GenStateRequest(EGenStateActionType::Pop, EGenStateType::GenMode, gen_mode_change)); // revert GenMode back.

      mpGenerator->PrependRequests(loadRegisterRequests);
    }
  }

  void GenSequenceAgent::RestoreBootStates()
  {
    RestoreArchBootStates();
    mpGenerator->UpdateVm();

    // move PC to BootPC value to start generating boot loading code.
    uint64 boot_pc = 0;
    if (!mpGenerator->GetStateValue(EGenStateType::BootPC, boot_pc)) {
      LOG(fail) << "{GenSequenceAgent::BootLoading} BootPC not set." << endl;
      FAIL("no-boot-pc-specified");
    }

    mpGenerator->SetPC(boot_pc);
  }

  void GenSequenceAgent::ThreadSummary()
  {
    mpGenerator->GenSummary();
  }

  void GenSequenceAgent::Summary()
  {
    InstructionResults* instructionResults = InstructionResults::Instance();
    instructionResults->GenSummary();
  }

  void GenSequenceAgent::JumpToStart()
  {
    // obtain InitialPC value, issue sequence to jump to the target.
    uint64 init_pc = 0;
    if (!mpGenerator->GetStateValue(EGenStateType::InitialPC, init_pc)) {
      LOG(fail) << "{GenSequenceAgent::BootLoading} InitialPC not set." << endl;
      FAIL("no-initial-pc-specified");
    }

    auto br_req = new GenBranchToTarget(init_pc);
    mpGenerator->PrependRequest(br_req);
  }

  void GenSequenceAgent::RegisterReservation()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenRegisterReservation>();

    if (cast_req->DoReserve()) {
      mpGenerator->ReserveRegister(cast_req->RegisterName(), cast_req->ReservationAttributes());
    }
    else {
      mpGenerator->UnreserveRegister(cast_req->RegisterName(), cast_req->ReservationAttributes());
    }

    LOG(info) << "{GenSequenceAgent::RegisterReservation} register reservation do? " << cast_req->DoReserve() << " reg " << cast_req->RegisterName() << " attrs " << ERegAttrType_to_string(cast_req->ReservationAttributes()) << " is reserved? " << mpGenerator->IsRegisterReserved(cast_req->RegisterName(), cast_req->ReservationAttributes()) << endl;
  }

  void GenSequenceAgent::EscapeCollision()
  {
    auto vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
    VaGenerator va_gen(vm_mapper);
    uint64 pc_value = mpGenerator->PC();
    uint64 inter_size = 0;

    auto free_constr = vm_mapper->VirtualUsableConstraintSet(true);
    uint64 inter_start = free_constr->LeadingIntersectingRange(pc_value, -1ull, inter_size);

    if (inter_size < mpGenerator->DefaultInstructionSize())
    {
      LOG(fail) << "{GenSequenceAgent::EscapeCollision} inter size is less than instruction size inter_size=0x" << hex << inter_size << endl;
      FAIL("inter_size_0_escape_collision");
    }

    // If the free address range isn't at least as large as the BNT reserve space or doesn't start
    // at the current PC value, we have little to no space to generate instructions, so we must use
    // the short branch.
    bool use_short_branch = false;
    if ((inter_size < mpGenerator->BntReserveSpace()) or (inter_start > pc_value)) {
      use_short_branch = true;
    }

    ConstraintSet * pc_offset_constr = nullptr;
    std::unique_ptr<ConstraintSet> pcoffset_storage_ptr;

    if (not use_short_branch) {
      auto choices_mod = mpGenerator->GetChoicesModerator(EChoicesType::OperandChoices);
      auto choices_raw = choices_mod->CloneChoiceTree("Branch type choice");
      std::unique_ptr<ChoiceTree> choices_tree(choices_raw);
      use_short_branch = (choices_tree->Choose()->Value() == 0 );
    }

    //Check if space is limited:
    if ( use_short_branch ) {
      pc_offset_constr = new ConstraintSet();
      GetShortBranchConstraint(pc_value, *pc_offset_constr);
      // cppcheck-suppress unreadVariable // we're using this unique_ptr to handle memory management for pc_offset_constr
      pcoffset_storage_ptr = std::unique_ptr<ConstraintSet>(pc_offset_constr);
    }

    uint64 target_addr = va_gen.GenerateAddress(mpGenerator->InstructionAlignment(), mpGenerator->InstructionSpace(), true, EMemAccessType::Unknown, pc_offset_constr);

    LOG(notice) << "{GenSequenceAgent::EscapeCollision} space left at pc 0x" << hex  << pc_value << " intersection start at 0x" << inter_start << " size 0x" << inter_size << " use short branch? " << use_short_branch << " target address: 0x" << target_addr << endl;
    vector<GenRequest*> escape_requests;

    escape_requests.push_back(new GenStateRequest(EGenStateActionType::Push, EGenStateType::GenMode, EGenModeTypeBaseType(EGenModeType::NoEscape))); // modify GenMode to NoEscape.
    escape_requests.push_back(new GenBranchToTarget(target_addr, use_short_branch, true));
    escape_requests.push_back(new GenStateRequest(EGenStateActionType::Pop, EGenStateType::GenMode, EGenModeTypeBaseType(EGenModeType::NoEscape))); // revert GenMode back.

    mpGenerator->PrependRequests(escape_requests);
  }

  void GenSequenceAgent::SequenceUtility()
  {
    //auto gen_sequence_req = mpSequenceRequest->CastInstance<GenSequenceRequest>();
    //LOG(notice) << "{GenQueryAgent::SequenceUtility} : " << endl;
    ESequenceType sequence_type = mpSequenceRequest->SequenceType();
    LOG(notice) << "{GenSequenceAgent::SequenceUtility}  is called with : " << ESequenceType_to_string(sequence_type) << endl;


    switch (sequence_type) {
    case ESequenceType::ConfirmSpace:
      {
        uint64 confirm_space_size = mpSequenceRequest->ValueVariable(1);//ToDo: Amit: Find  a better solution then hard coding 0? maybe enum? 0 is here for size for confirmspace
        LOG(notice) << "{GenSequenceAgent::SequenceUtility}  confirm_space_size  : " << confirm_space_size << endl;

        auto vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
        uint64 pc_value = mpGenerator->PC();
        uint64 inter_size = 0;

        auto free_constr = vm_mapper->VirtualUsableConstraintSet(true);
        uint64 inter_start = free_constr->LeadingIntersectingRange(pc_value, -1ull, inter_size);
        bool escape_collision_needed = (inter_size  < confirm_space_size);

        LOG(notice) << "{GenSequenceAgent::SequenceUtility} space left at pc 0x" << hex  << pc_value << " intersection start at 0x" << inter_start << " size 0x" << inter_size << " escape_collision_needed? " << escape_collision_needed  << endl;

        if ( escape_collision_needed )
          {
            EscapeCollision();
          }
        break;
      }
    default:
      LOG(fail) << "{GenSequenceAgent::SequenceUtility} Sequence type is not defined" << endl;
      FAIL("unsupported-sequence_type");
    }

  }

  void GenSequenceAgent::BranchNotTaken()
  {
    auto bnt_manager = mpGenerator->GetBntNodeManager();
    vector<BntNode* > bnt_nodes;
    bnt_manager->SwapBntNodes(bnt_nodes);
    if (bnt_nodes.empty())
      return;

    auto bnt_limit = Config::Instance()->LimitValue(ELimitType::BranchNotTakenLimit);
    if (++ mBntLevel > bnt_limit) {
      LOG(notice) << "{GenSequenceAgent::BranchNotTaken} Bnt level " << dec << mBntLevel << " is overflow its limiation: " << bnt_limit << endl;
      for (auto bnt_node: bnt_nodes)
        delete bnt_node;
      return;
    }

    bool any_bnt = false;
    vector<GenRequest*> bnt_requests;
    EGenModeTypeBaseType gen_mode_change = EGenModeTypeBaseType(EGenModeType::SimOff);
    bnt_requests.push_back(new GenStateRequest(EGenStateActionType::Push, EGenStateType::GenMode, gen_mode_change));

    for (auto bnt_node : bnt_nodes) {
      LOG(notice) << "{GenSequenceAgent::BranchNotTaken} processing : " << bnt_node->ToString() << endl;
      if (bnt_node->PathsSame()) {
        LOG(notice) << "{GenSequenceAgent::BranchNotTaken} skipped since Target=Next-PC." << endl;
        delete bnt_node;
      }
      else {
        any_bnt = true;
        if (bnt_node->IsAccurate())
          bnt_requests.push_back(new GenCallBackBntRequest(bnt_node));
        else
          bnt_requests.push_back(new GenBntRequest(bnt_node));
      }
    }

    bnt_requests.push_back(new GenStateRequest(EGenStateActionType::Pop, EGenStateType::GenMode, gen_mode_change)); // revert GenMode back.
    if (any_bnt)
      bnt_requests.push_back(new GenSequenceRequest("BranchNotTaken"));

    mpGenerator->PrependRequests(bnt_requests);
  }

  void GenSequenceAgent::ProcessBntNode()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenBntRequest>();

    BntNode* bnt_node = cast_req->GiveBntNode();

    // restore PE state.
    SimplePeState* pe_state = bnt_node->GetPeState();
    if (pe_state->RestoreState()) {
      mpGenerator->UpdateVm();
    }

    uint64 pa = 0;
    uint32 bank = 0;
    uint64 not_taken_path = bnt_node->NotTakenPath();
    auto vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
    if (vm_mapper->TranslateVaToPa(not_taken_path, pa, bank) != ETranslationResultType::Mapped) {
      LOG(fail) << "{GenSequenceAgent::ProcessBntNode} failed to translate not-taken-path address 0x" << hex << not_taken_path << endl;
      FAIL("failed-to-translate-not-taken-path");
    }

    uint64 inter_start = 0;
    uint64 inter_size = 0;
    auto free_constr = mpGenerator->GetMemoryManager()->GetMemoryBank(bank)->Free();
    inter_start = free_constr->LeadingIntersectingRange(pa, -1ull, inter_size);

    LOG(notice) << "{GenSequenceAgent::ProcessBntNode} generating not-taken-path starting from 0x" << hex << not_taken_path << "=>[" << bank << "]0x" << pa << " intersection start 0x" << inter_start << " size " << dec << inter_size << endl;

    if ((pa == inter_start) && (inter_size >= mpGenerator->BntMinSpace())) {
      mpGenerator->SetPC(not_taken_path);
      auto bnt_branch = new GenBranchToTarget(bnt_node->TakenPath(), false, true); // NoBnt
      mpGenerator->PrependRequest(bnt_branch);
    }
    delete bnt_node;
  }

  void GenSequenceAgent::ReExecution()
  {
    if (mpGenerator->SimulationEnabled()) {
      auto re_exe_req = mpSequenceRequest->CastInstance<GenReExecutionRequest>();
      uint64 re_exe_addr = re_exe_req->ReExecutionAddress();
      uint32 max_re_exe_instr = re_exe_req->MaxReExecutionInstructions();
      if (mHasGenEndOfTest) {
        // We limit the number of re-execution steps to avoid the possibility of infinitely
        // repeating the end of test instructions, particularly in the branch to self case. This
        // could happen if an instruction in the end of test sequence triggers an exception.
        max_re_exe_instr = GetEndOfTestInstructionCount();
      }

      delete mpSequenceRequest;
      mpSequenceRequest = nullptr;
      mpGenerator->ReExecute(re_exe_addr, max_re_exe_instr);
    }
  }

  void GenSequenceAgent::UpdatePeState()
  {
    auto pe_req = mpSequenceRequest->CastInstance<GenPeStateUpdateRequest>();
    uint64 pe_rec_id = pe_req->RecordId();
    auto record = DataStation::Instance()->Get(pe_rec_id);

    if (nullptr == record)
    {
      LOG(fail) << "{GenSequenceAgent::UpdatePeState} invalid record id:" << pe_rec_id << endl;
      FAIL("invalid-record-id-update-pestate");
      return;
    }

    auto pe_state = dynamic_cast<PeStateUpdate*>(record);

    if (nullptr == pe_state)
    {
      LOG(fail) << "{GenSequenceAgent::UpdatePeState} can't find PeStateUpdate from record id:" << pe_rec_id << endl;
      FAIL("not-available-update-pestate");
      return;
    }

    LOG(notice) << "{GenSequenceAgent::UpdatePeState} find a valid PeStateUpdate from record id:" << pe_rec_id << endl;

    std::vector<GenRequest*>& requests = pe_state->GetRequests();
    mpGenerator->PrependRequests(requests);

    DataStation::Instance()->Remove(pe_rec_id);
  }

  void GenSequenceAgent::UpdateRegisterField()
  {
    auto reg_req = mpSequenceRequest->CastInstance<GenRegisterFieldUpdateRequest>();

    uint64 mask = reg_req->Mask();

    if (mask != 0)  // the request targets to update based on given mask and given register value
    {
      const std::map<std::string, RegisterField*> regFields = mpGenerator->GetRegisterFile()->GetRegisterFieldsFromMask (reg_req->RegisterName(), mask);
      uint64 value = reg_req->Value();
      LOG(notice) << "{GenSequenceAgent::UpdateRegisterField} register name:" << reg_req->RegisterName() << " value:0x" << hex << value << " and mask:0x" << mask << endl;

      auto getFieldVal = [this] (uint64 value, uint64 mask)
      {
        uint64 fieldVal = 0;

        for (auto i=0; i<64; ++i)
        {
          uint64 m = mask >> i;
          if ( (m&0x01) != 0 )
          {
            fieldVal = (value>>i) & (mask>>i);
            LOG(notice) << "Get Field Value: value:0x" << value << " mask:0x" << mask << " position:" << i << " actual field value:" << fieldVal << endl;
            return fieldVal;
          }
        }

        return value;
      };

      for (auto item : regFields)
      {
        auto field = item.second;
        uint64 val = getFieldVal(value, field->FieldMask());
        LOG(notice) << "field name:" << field->Name() << " field mask:" << field->FieldMask() << " value:" << val << endl;
        mpGenerator->WriteRegisterWithUpdate(reg_req->RegisterName(), field->Name(), val);
      }
    } else {
      mpGenerator->WriteRegisterWithUpdate(reg_req->RegisterName(), reg_req->FieldName(), reg_req->FieldValue());
    }
  }

  void GenSequenceAgent::SetRegister()
  {
    auto set_reg_req = mpSequenceRequest->CastInstance<GenSetRegister>();

    auto reg_ptr = set_reg_req->GetRegister();
    reg_ptr->SetValue(set_reg_req->RegisterValue());
    LOG(info) << "{GenSequenceAgent::SetRegister} set register: " << reg_ptr->Name() << " value: 0x" << hex << set_reg_req->RegisterValue() << endl;
  }

  void GenSequenceAgent::GetShortBranchConstraint(uint64 pcValue, ConstraintSet& rPcOffsetConstr)
  {
    LOG(fail) << "{GenSequenceAgent::GetShortBranchConstraint} not yet for arch " << endl;
    FAIL("unsupported-arch-register-type");
  }

  EReloadingMethodType GenSequenceAgent::ChooseReloadingMethod() const
  {
    ChoicesModerator* pChoicesModerator = mpGenerator->GetChoicesModerator(EChoicesType::GeneralChoices);
    auto choices_raw = pChoicesModerator->CloneChoiceTree("Reloading register methods");
    std::unique_ptr<ChoiceTree> reload_choice_tree(choices_raw);
    const Choice* reload_choice = reload_choice_tree->Choose();
    EReloadingMethodType reload_method = string_to_EReloadingMethodType(reload_choice->Name());
    if (not mpGenerator->HasISS()) {
      reload_method = EReloadingMethodType::Move; // if NoISS mode, does not using load instruciton.
    }
    return reload_method;
  }

  uint32 GenSequenceAgent::ChooseBatchReloadingNumber() const
  {
    auto number_var = mpGenerator->GetVariable("Reloading registers number", EVariableType::String);
    ConstraintSet number_constr(number_var);
    return number_constr.ChooseValue();
  }

  uint64 GenSequenceAgent::GenerateAvailableAddress() const
  {
    // refill available addresses for data.
    ChoicesModerator* pChoicesModerator = mpGenerator->GetChoicesModerator(EChoicesType::GeneralChoices);
    auto choices_raw = pChoicesModerator->CloneChoiceTree("Reloading register alignment");
    std::unique_ptr<ChoiceTree> align_choice_tree(choices_raw);
    const Choice* align_choice = align_choice_tree->Choose();
    uint32 align_value = align_choice->Value();

    bool is_instr = false;
    auto current_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();

    std::unique_ptr<GenPageRequest> local_page_req(current_mapper->GenPageRequestRegulated(false, EMemAccessType::ReadWrite));

    VaGenerator va_gen(current_mapper);
    return va_gen.GenerateAddress(align_value, align_value, is_instr, EMemAccessType::ReadWrite);
  }

  void GenSequenceAgent::BatchReloadRegisters()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenBatchReloadRegisters>();
    std::vector<std::string> regs_vec(cast_req->ReloadRegisters());

    if (regs_vec.empty()) {
      if (not GetRandomReloadRegisters(ChooseBatchReloadingNumber(), regs_vec)) {
        LOG(notice) << "{Generator::SolveAddressShortage} not enough GPR random registers available" << endl;
        return;
      }
    }

    vector<GenRequest* > req_seq;
    string inter_reg_name = cast_req->InterRegisterName();

    EReloadingMethodType reload_method = ChooseReloadingMethod();
    if (reload_method == EReloadingMethodType::Load) {
      if (inter_reg_name.empty()) {
        inter_reg_name = regs_vec[0]; // remove regs_vec0.
        regs_vec.erase(regs_vec.begin());
      }
      auto inter_reg_ptr = mpGenerator->GetRegisterFile()->RegisterLookup(inter_reg_name);
      GetReloadBaseAddressSequence(inter_reg_ptr, regs_vec.size() << 3, req_seq);
    }

    if (not inter_reg_name.empty()) {
      // intermedidate register is provided
      if (not mpGenerator->IsRegisterReserved(inter_reg_name, ERegAttrType::Write))
      {
        mpGenerator->ReserveRegister(inter_reg_name, ERegAttrType::Write);
        // unreservate intermedidate register.
        mpGenerator->PrependRequest(new GenRegisterReservation(inter_reg_name, false, ERegAttrType::Write));
      }
    }

    uint32 size = regs_vec.size();
    uint32 double_size = (size >> 1) << 1;
    for (uint32 i = 0; i < double_size; i+=2) {
      GenReloadRegister* req = new GenReloadRegister();
      req->SetRegisterType(ERegisterType::GPR);
      req->AddReloadRegister(regs_vec[i], GenerateAvailableAddress());
      req->AddReloadRegister(regs_vec[i+1], GenerateAvailableAddress());
      req->SetInterRegisterName(inter_reg_name);
      req->SetReloadMethod(reload_method);
      req_seq.push_back(req);
    }
    if (double_size < size) {
      GenReloadRegister* req = new GenReloadRegister();
      req->SetRegisterType(ERegisterType::GPR);
      req->AddReloadRegister(regs_vec[double_size], GenerateAvailableAddress());
      //req->SetInterRegisterName(inter_reg_name);
      req->SetReloadMethod(reload_method);
      req_seq.push_back(req);
    }
    mpGenerator->PrependRequests(req_seq);
  }

  void GenSequenceAgent::ReloadRegister()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenReloadRegister>();
    vector<GenRequest* > req_seq;
    GetReloadRegisterSequence(cast_req, req_seq);
    mpGenerator->PrependRequests(req_seq);
  }

  void GenSequenceAgent::GetReloadRegisterSequence(const GenReloadRegister *reqPtr, std::vector<GenRequest* >& reqSeq)
  {
    ERegisterType reg_type = reqPtr->RegisterType();

    const Register* inter_reg_ptr = nullptr;
    auto inter_reg_name = reqPtr->InterRegisterName();
    if (not inter_reg_name.empty()) {
      inter_reg_ptr = mpGenerator->GetRegisterFile()->RegisterLookup(inter_reg_name);
    }

    switch (reg_type) {
    case ERegisterType::GPR:
      GetReloadGPRSequence(reqPtr, reqSeq, inter_reg_ptr);
      break;
    default:
      LOG(fail) << "Unsupported reloading register types:" << ERegisterType_to_string(reg_type) << endl;
      FAIL("unsupported_reloading_register_type_fail");
      break;
    }
  }

  void GenSequenceAgent::ProcessSpeculativeBntNode()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenSpeculativeBntRequest>();
    auto bnt_node = cast_req->GetBntNode();
    if (bnt_node !=  mpGenerator->GetBntNodeManager()->GetHotSpeculativeBntNode()) {
      LOG(fail) << "{GenSequenceAgent::ProcessSpeculativeBntNode} Process not hot Bnt node:" << bnt_node->ToString() << endl;
      FAIL("process-not-hot-BntNode");
    }
    switch (cast_req->GetActionType()) {
    case ESpeculativeBntActionType::Execute:
      ExecuteSpeculativeBntNode(bnt_node);
      break;
    case ESpeculativeBntActionType::Restore:
      RestoreSpeculativeBntNode(bnt_node);
      break;
    case ESpeculativeBntActionType::Pop:
      PopSpeculativeBntNode(bnt_node);
      break;
    default:
      LOG(fail) << "{GenSequenceAgent::ProcessSpeculativeBntNode} Sequence type is not defined" << endl;
      FAIL("unsupported-sequence_type");
    }
  }

  void GenSequenceAgent::ExecuteSpeculativeBntNode(BntNode* pBntNode)
  {
    ++ mSpeculativeBntLevel;
    if (pBntNode->PathsSame()) {
      LOG(notice) << "{GenSequenceAgent::ExecuteSpeculativeBntNode} skipped since Target=Next-PC." << endl;
      PopSpeculativeBntNode(pBntNode);
      return;
    }

    auto bnt_limit = Config::Instance()->LimitValue(ELimitType::SpeculativeBntLevelLimit);
    if (mSpeculativeBntLevel > bnt_limit) {
      LOG(notice) << "{GenSequenceAgent::ExecuteSpeculativeBntNode} Speculative Bnt level " << dec << mSpeculativeBntLevel << " is overflow its limitation: " << bnt_limit << ", Ignored bnt node: " << pBntNode->ToString() << endl;
      PopSpeculativeBntNode(pBntNode);
      return;
    }

    LOG(info) << "{GenSequenceAgent::ExecuteSpeculativeBntNode} Execute Bnt node : " << pBntNode->ToString() << " at Bnt level:" << dec << mSpeculativeBntLevel << endl;

    ReserveBntConstraint(pBntNode);

    const AddressTagging* addr_tagging = mpGenerator->GetVmManager()->CurrentVmMapper()->GetAddressTagging();
    uint64 untagged_pc = addr_tagging->UntagAddress(pBntNode->RealPath(), true);
    pBntNode->PushResourcePeState(new PCPeState( untagged_pc));

    ChoicesModerator* pChoicesModerator = mpGenerator->GetChoicesModerator(EChoicesType::DependenceChoices);
    auto choices_raw = pChoicesModerator->CloneChoiceTree("Recover Speculative Dependency");
    std::unique_ptr<ChoiceTree> recover_choice_tree(choices_raw);
    const Choice* recover_choice = recover_choice_tree->Choose();
    if (recover_choice->Value()) {
     auto cloned_dep = mpGenerator->GetDependenceInstance()->Snapshot();
     pBntNode->PushResourcePeState(new DependencePeState(cloned_dep));
    }

    vector<GenRequest*> bnt_requests;
    EGenModeTypeBaseType gen_mode_change = EGenModeTypeBaseType(EGenModeTypeBaseType(EGenModeType::NoEscape) | EGenModeTypeBaseType(EGenModeType::Speculative));
    bnt_requests.push_back(new GenStateRequest(EGenStateActionType::Push, EGenStateType::GenMode, gen_mode_change));

    untagged_pc = addr_tagging->UntagAddress(pBntNode->NotTakenPath(), true);
    bnt_requests.push_back(new GenStateRequest(EGenStateActionType::Set, EGenStateType::PC, untagged_pc)); // May be aligned PC
    bnt_requests.push_back(new GenCallBackBntRequest(pBntNode));
    bnt_requests.push_back(new GenSpeculativeBntRequest(pBntNode, ESpeculativeBntActionType::Restore));
    bnt_requests.push_back(new GenSpeculativeBntRequest(pBntNode, ESpeculativeBntActionType::Pop));
    bnt_requests.push_back(new GenStateRequest(EGenStateActionType::Pop, EGenStateType::GenMode, gen_mode_change)); // revert GenMode back.
    mpGenerator->PrependRequests(bnt_requests);
  }

  void GenSequenceAgent::RestoreSpeculativeBntNode(BntNode* pBntNode)
  {
    LOG(info) << "{GenSequenceAgent::RestoreSpeculativeBntNode} Restore Bnt node : " << pBntNode->ToString() << endl;
    if (pBntNode->RecoverResourcePeStates(mpGenerator))
      mpGenerator->UpdateVm();

    UnreserveBntConstraint(pBntNode);
  }

  void GenSequenceAgent::PopSpeculativeBntNode(BntNode* pBntNode)
  {
    auto bnt_manager = mpGenerator->GetBntNodeManager();
    LOG(info) << "{GenSequenceAgent::PopSpeculativeBntNode} Pop Bnt node : " << pBntNode->ToString() << endl;
    bnt_manager->PopSpeculativeBntNode();

    uint32 bnt_level = mSpeculativeBntLevel--;

    if (mSpeculativeBntLevel > bnt_level) {
      LOG(fail) << "{GenSequenceAgent::PopSpeculativeBntNode} Speculative Bnt level Underflow" << endl;
      FAIL("Speculative-Bntlevel-Underflow");
    }
  }

  void GenSequenceAgent::ReserveBntConstraint(BntNode* pBntNode)
  {
    pBntNode->ReserveTakenPath(mpGenerator);
  }

  void GenSequenceAgent::UnreserveBntConstraint(BntNode* pBntNode)
  {
    pBntNode->UnreserveTakenPath(mpGenerator);
  }

  void GenSequenceAgent::LoadLargeRegister()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenLoadLargeRegister>();
    auto reg_file = mpGenerator->GetRegisterFile();
    auto reg_ptr = reg_file->RegisterLookup(cast_req->RegisterName());

    vector<GenRequest* > req_seq;
    LOG(notice) << "{LoadLargeRegister} Register Name:" << reg_ptr->Name() << endl;

    Register* inter_reg_ptr = nullptr;
    if (not cast_req->InterRegisterName().empty())
    {
      // intermedidate register is provided
      inter_reg_ptr = reg_file->RegisterLookup(cast_req->InterRegisterName());
    }

    GetLoadLargeRegisterSequence(reg_ptr, cast_req->RegisterValues(), req_seq, inter_reg_ptr, cast_req->ImmOffset());

    mpGenerator->PrependRequests(req_seq);
  }

  void GenSequenceAgent::BeginRestoreLoop()
  {
    RestoreLoopManagerRepository* restore_loop_manager_repository = RestoreLoopManagerRepository::Instance();
    RestoreLoopManager* restore_loop_manager = restore_loop_manager_repository->GetRestoreLoopManager(mpGenerator->ThreadId());
    auto cast_req = mpSequenceRequest->CastInstance<GenRestoreRequest>();
    restore_loop_manager->BeginLoop(cast_req->LoopRegisterIndex(), cast_req->SimulationCount(), cast_req->RestoreCount(), cast_req->RestoreExclusions());
  }

  void GenSequenceAgent::EndRestoreLoop()
  {
    RestoreLoopManagerRepository* restore_loop_manager_repository = RestoreLoopManagerRepository::Instance();
    RestoreLoopManager* restore_loop_manager = restore_loop_manager_repository->GetRestoreLoopManager(mpGenerator->ThreadId());
    auto cast_req = mpSequenceRequest->CastInstance<GenRestoreRequest>();
    restore_loop_manager->EndLoop(cast_req->LoopId());
  }

  void GenSequenceAgent::RestoreLoopState()
  {
    RestoreLoopManagerRepository* restore_loop_manager_repository = RestoreLoopManagerRepository::Instance();
    RestoreLoopManager* restore_loop_manager = restore_loop_manager_repository->GetRestoreLoopManager(mpGenerator->ThreadId());
    auto cast_req = mpSequenceRequest->CastInstance<GenRestoreRequest>();
    restore_loop_manager->GenerateRestoreInstructions(cast_req->LoopId());
  }

  uint64 GenSequenceAgent::GetBootLoadingBaseAddress(uint32 size) const
  {
    auto vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
    //use system page regulated request to avoid boot loading base address range from triggering exceptions
    std::unique_ptr<GenPageRequest> local_page_req(vm_mapper->GenPageRequestRegulated(false, EMemAccessType::Read, true)); // request no fault

    VaGenerator va_gen(vm_mapper, local_page_req.get());
    uint64 base_addr = va_gen.GenerateAddress(0x10, size, false, EMemAccessType::Unknown); // 128 bit alignment.
    return base_addr;
  }

  void GenSequenceAgent::LoopReconverge()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenLoopReconvergeRequest>();
    auto reexe_manager = mpGenerator->GetReExecutionManager();
    uint64 loop_reconverge_addr = reexe_manager->LoopReconvergeAddress();
    uint64 post_loop_addr = reexe_manager->PostLoopAddress();
    LOG(info) << "LoopReconverge: current PC: 0x" << hex << cast_req->CurrentPC() << " last PC: 0x" << cast_req->LastPC() << " loop reconverge: 0x" << loop_reconverge_addr << " post loop: 0x" << post_loop_addr << endl;

    auto branch_req = new GenBranchToTarget(loop_reconverge_addr);
    mpGenerator->PrependRequest(branch_req);
  }

}
