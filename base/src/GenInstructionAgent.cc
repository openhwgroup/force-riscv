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
#include <Defines.h>
PICKY_IGNORE_BLOCK_START
#include <SimAPI.h>
PICKY_IGNORE_BLOCK_END
#include <GenInstructionAgent.h>
#include <GenRequest.h>
#include <InstructionSet.h>
#include <Generator.h>
#include <InstructionStructure.h>
#include <ObjectRegistry.h>
#include <Instruction.h>
#include <GenException.h>
#include <GenMode.h>
#include <BntNode.h>
#include <Record.h>
#include <Register.h>
#include <UtilityAlgorithms.h>
#include <MemoryManager.h>
#include <GenPC.h>
#include <VmUtils.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <AddressTagging.h>
#include <ExceptionManager.h>
#include <InstructionResults.h>
#include <Variable.h>
#include <BntHookManager.h>
#include <BntNodeManager.h>
#include <RestoreLoop.h>
#include <ResourcePeState.h>
#include <VirtualMemoryInitializer.h>
#include <MemoryInitData.h>
#include <Operand.h>
#include <Config.h>
#include <ReExecutionManager.h>
#include <Log.h>

#include <memory>

using namespace std;

namespace Force {

  GenInstructionAgent::GenInstructionAgent(const GenInstructionAgent& rOther)
    : GenAgent(rOther), Receiver(rOther), mInstrSimulated(0), mpInstructionRequest(nullptr), mRegisterInitializations() { } //!< Copy constructor, do not copy the request pointer.

  GenInstructionAgent::~GenInstructionAgent()
  {
    if (mRegisterInitializations.size() > 0) {
      LOG(warn) << "{GenInstructionAgent::~GenInstructionAgent} register initialization vector not empty." << endl;
      //FAIL("register-initialization-vector-not-empty");
    }
  }

  Object* GenInstructionAgent::Clone() const
  {
    return new GenInstructionAgent(*this);
  }

  void  GenInstructionAgent::SetGenRequest(GenRequest* genRequest)
  {
    mpInstructionRequest = dynamic_cast<GenInstructionRequest* >(genRequest);
  }

  void GenInstructionAgent::SkipRequest(Instruction* pInstr)
  {
    mpGenerator->GetInstructionResults()->InvalidCurrentBankAddress();
    pInstr->CleanUp();
    delete mpInstructionRequest;
    delete pInstr;
    mpInstructionRequest = nullptr; // object ownership passed to generator.
  }

  void  GenInstructionAgent::HandleRequest()
  {
    mpGenerator->MapPC();

    const InstructionStructure* instr_struct = mpGenerator->GetInstructionSet()->LookUpById(mpInstructionRequest->InstructionId());
    Instruction* instr = ObjectRegistry::Instance()->TypeInstance<Instruction>(instr_struct->mClass);
    instr->Initialize(instr_struct);
    LOG(notice) << "Generating: " << instr->FullName() << endl;
    instr->Setup(*mpInstructionRequest, *mpGenerator);
    try {
      instr->Generate(*mpGenerator);
    }
    catch (const OperandError& operand_error) {
      bool skip = !(instr->NoSkip() || mpGenerator->GetGenMode()->CurrentMode() & EGenModeTypeBaseType(EGenModeType::NoSkip));
      if (skip)  {
        LOG(notice) << "Skipping: " << instr->FullName() << " with operand error: " << operand_error.what() << endl;
        SkipRequest(instr);
        return;
      }
      LOG(fail) << "{GenInstructionAgent:HandleRequest} failed to generate instruction "
                << instr->FullName() << ". Error is " << operand_error.what() << endl;
      FAIL("failed-find-choice-by-value");
    }
    catch (const InstructionError& instr_error) {
      LOG(notice) << "Skipping: " << instr->FullName() << " with instruction error: " << instr_error.what() << endl;
      SkipRequest(instr);
      return;
    }
    mpGenerator->CommitInstruction(instr, mpInstructionRequest);
    mpInstructionRequest = nullptr; // object ownership passed to generator.
  }

  void GenInstructionAgent::HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* payload)
  {
    switch (eventType) {
    case ENotificationType::RegisterInitiation:
      {
        auto reg_ptr = dynamic_cast<Register* >(payload);
        if (insert_sorted<Register* >(mRegisterInitializations, reg_ptr)) {
          LOG(info) << "{GenInstructionAgent::HandleNotification} new register initialization: " << reg_ptr->Name() << endl;
        }
      }
      break;
    default:
      LOG(fail) << "{GenInstructionAgent::HandleNotification} unexpected event: " << ENotificationType_to_string(eventType) << endl;
      FAIL("unexpected-event-type");
    }
  }

  void GenInstructionAgent::StepInstruction(const Instruction* pInstr)
  {
    bool has_except = false;
    if (mpGenerator->SimulationEnabled()) {
      has_except = StepInstructionWithSimulation(pInstr);
      if (!has_except and pInstr->IsBranch() and pInstr->SpeculativeBnt()) {
        auto bnt_node = mpGenerator->GetBntNodeManager()->GetHotSpeculativeBntNode();
        if (bnt_node != nullptr) {
          LOG(info) << "{GenInstructionAgent::StepInstruction} Request to execute Bnt node: " << bnt_node->ToString() << endl;
          auto bnt_req = new GenSpeculativeBntRequest(bnt_node, ESpeculativeBntActionType::Execute);
          mpGenerator->AddPostInstructionStepRequest(bnt_req);
        }
      }
    }
    else {
      StepInstructionNoSimulation(pInstr);
    }

    if (not has_except)
      UpdateUnpredictedConstraint(pInstr);
  }

  void GenInstructionAgent::StepInstructionNoSimulation(const Instruction* pInstr)
  {
    BntNode* bnt_node = pInstr->GetBntNode();
    uint32 instr_bytes = pInstr->ByteSize();

    if (nullptr != bnt_node) {
      if (bnt_node->IsSpeculative()) {
        LOG(fail) << "{GenInstructionAgent::StepInstructionNoSimulation} NoSim mode failed to handle speculative Bnt node: " << bnt_node->ToString() << endl;
        FAIL("NoSim-mode-handle-speculative-Bnt");
      }

      bnt_node->SetNextPC(mpGenerator->PC() + instr_bytes);
      if (bnt_node->BranchTaken() and (mpGenerator->NoJump() == false)) {
        uint64 br_target = bnt_node->BranchTarget();
        VmMapper* vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
        const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
        uint64 untagged_target_address = addr_tagging->UntagAddress(br_target, true);
        mpGenerator->GetGenPC()->SetAligned(untagged_target_address);
      }
      else {
        mpGenerator->AdvancePC(instr_bytes);
      }
      if (!pInstr->NoBnt()) {
        if (not mpGenerator->InFiller()) {
          // inaccurate BNT handling, like exception handler
          bnt_node->PreserveNotTakenPath(mpGenerator);
          auto bnt_manager = mpGenerator->GetBntNodeManager();
          bnt_manager->SaveBntNode(bnt_node);
        }
        else {
          // for filler mode, treat branch direction as accurate.
          UpdateAccurateBnt(pInstr);
          delete bnt_node; // bnt node is not used any more
        }
      }
      else {
        delete bnt_node;
      }
    }
    else {
      mpGenerator->AdvancePC(instr_bytes);
    }

    if (not mpGenerator->HasISS()) {
      ReleaseInits();
    }
  }

  void GenInstructionAgent::ReleaseInits()
  {
    std::vector<Register* > reg_inits;
    reg_inits.swap(mRegisterInitializations);
    // These Register pointer should not be deleted
    reg_inits.clear();

    auto record_archive = mpGenerator->GetRecordArchive();
    vector<MemoryInitRecord* > memory_records;
    record_archive->SwapMemoryInitRecords(memory_records);
    for (auto mem_rd : memory_records) {
      delete mem_rd;
    }
  }

  void GenInstructionAgent::SendInitsToISS()
  {
    SimAPI *sim_ptr = mpGenerator->GetSimAPI(); // get handle to simulator...

    // make register inits (if different from current state?)
    std::vector<const PhysicalRegister* > phys_reg_inits;
    auto choice_mod_ptr = mpGenerator->GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);
    auto reg_file = mpGenerator->GetRegisterFile();

    std::vector<Register* > reg_inits;
    uint32 loop_count = 0;
    do {
      reg_inits.swap(mRegisterInitializations);
      for (auto reg_init : reg_inits) {
        if (!reg_init->IsInitialized()) {
          reg_file->InitializeRegisterRandomly(reg_init, choice_mod_ptr);
        }
        auto wider_reg = reg_file->GetContainingRegister(reg_init);
        if ((nullptr != wider_reg) and (not wider_reg->IsInitialized())) {
          LOG(info) << "{GenInstructionAgent::SendInitsToISS} register " << reg_init->Name() 
		    << " value: 0x" << std::hex << reg_init->Value() << std::dec 
		    << " wider register " << wider_reg->Name() << " not fully initialized." << endl;
	  if (reg_file->InitContainingRegister(wider_reg, reg_init)) {
             // was able to directly init the 'wider' reg from the 'init' reg...
	  } else
             reg_file->InitializeRegisterRandomly(wider_reg, choice_mod_ptr);
        }
        std::set<PhysicalRegister* > phyRegisterSet;
        reg_init->GetPhysicalRegisters(phyRegisterSet);
        for (auto phys_register : phyRegisterSet) {
          insert_sorted<const PhysicalRegister* >(phys_reg_inits, phys_register);
        }
      }
      reg_inits.clear();
      ++ loop_count;
      if (loop_count > 2) {
        LOG(fail) << "{GenInstructionAgent::SendInitsToISS} not expecting the register init loop to loop more than 2 times." << endl;
        FAIL("register-init-loop-too-many-times");
      }
    }
    while (mRegisterInitializations.size() > 0); // New inits could be put into the vector.

    // write fully initialized physical registers.
    uint32 thread_id = mpGenerator->ThreadId();
    for (auto phys_reg_ptr : phys_reg_inits) {
      uint64 phys_reg_mask = phys_reg_ptr->Mask();
      if (phys_reg_ptr->HasAttribute(ERegAttrType::UpdatedFromISS)) {
        LOG(info) << "{GenInstructionAgent::SendInitsToISS} ignore register " << phys_reg_ptr->Name() << " as its value was updated from ISS, "
                  << " Initial value 0x"<< hex << phys_reg_ptr->InitialValue(phys_reg_mask) << ", value 0x" << phys_reg_ptr->Value(phys_reg_mask) << endl;
        continue;
      }

      LOG(info) << "{ GenInstructionAgent::SendInitsToISS} writing register " << phys_reg_ptr->Name() << " initial value 0x" << hex << phys_reg_ptr->InitialValue(phys_reg_mask) << endl;
      sim_ptr->WriteRegister(thread_id, phys_reg_ptr->Name().c_str(), phys_reg_ptr->InitialValue(phys_reg_mask), phys_reg_mask);
    }

    // memory inits include the instruction opcode and memory associated with a load/store...
    auto record_archive = mpGenerator->GetRecordArchive();
    vector<MemoryInitRecord* > memory_records;
    record_archive->SwapMemoryInitRecords(memory_records);
    for (auto mem_rd : memory_records) {
      sim_ptr->WritePhysicalMemory(mem_rd->MemoryId(), mem_rd->Address(), mem_rd->Size(), mem_rd->InitData());
      delete mem_rd;
    }
  }

  bool GenInstructionAgent::StepInstructionWithSimulation(const Instruction* pInstr)
  {
    SendInitsToISS();

    SimAPI *sim_ptr = mpGenerator->GetSimAPI(); // get handle to simulator...

    // make register inits (if different from current state?)
    uint32 thread_id = mpGenerator->ThreadId();
    vector<RegUpdate> reg_updates;  //
    vector<MemUpdate> mem_updates;  // simulator will update these during step
    vector<MmuEvent> mmu_events;    //
    vector<ExceptionUpdate> except_updates;        //

    // step instruction on simulator...

    sim_ptr->Step(thread_id, reg_updates, mem_updates, mmu_events, except_updates);

    bool has_eret_event = false;
    bool has_except_event = HasExceptionEvent(except_updates, has_eret_event);

    if (mpGenerator->InSpeculative()) {
      auto hot_bntNode = mpGenerator->GetBntNodeManager()->GetHotSpeculativeBntNode();
      if (hot_bntNode == nullptr) {
        LOG(fail) <<"No hot bnt node when step instruction speculatively" << endl;
        FAIL("No-hot-bnt-node");
      }
      hot_bntNode->RecordExecution(pInstr);
      if (has_except_event && not has_eret_event) {
        RecoverExceptionBeforeUpdate(except_updates, reg_updates, sim_ptr);
        UpdateInstructionCount();
        return true;
      }
      SaveRegisterBeforeUpdate(reg_updates, hot_bntNode);
      SaveMemoryBeforeUpdate(mem_updates, hot_bntNode);
    }
    else if (mpGenerator->InLoop() && mpGenerator->RecordingState()) {
      SaveLoopRegisterBeforeUpdate(reg_updates);
      SaveLoopMemoryBeforeUpdate(mem_updates);
    }

    // update generator register/memory state... PC, Pstate in particular!
    uint64 real_pc = 0;
    UpdateRegisterFromSimulation(reg_updates, has_except_event, real_pc);
    UpdateMemoryFromSimulation(mem_updates);
    if (has_except_event) {
      has_except_event = UpdateExceptionEvent(except_updates);
    }

    UpdateInstructionCount();

    if (pInstr and not has_except_event) {
      UpdateAccurateBnt(pInstr, real_pc);
    }
    return has_except_event;
  }

  void GenInstructionAgent::UpdateAccurateBnt(const Instruction* pInstr, uint64 targetPC)
  {
    BntNode* bnt_node = pInstr->GetBntNode();
    if (nullptr == bnt_node) return;

    //TODO check to see if branch target is in address error space - if so, inject sequence to step simulation
    auto vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
    const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
    uint64 untagged_va = addr_tagging->UntagAddress(bnt_node->BranchTarget(), true);
    if (vm_mapper->VaInAddressErrorRange(untagged_va))
    {
      LOG(notice) << "{GenInstructionAgent::UpdateAccurateBnt} target in error range branch_target=0x" << hex << untagged_va << endl;
      StepInstructionWithSimulation();
    }

    if (!pInstr->NoBnt()) {
      uint32 instr_bytes = pInstr->ByteSize();
      bnt_node->UpdateAccurateState(mpGenerator, instr_bytes);
      auto bnt_hook =  mpGenerator->GetBntHookManager()->GetBntHook();
      bnt_node->SetSequenceName(bnt_hook->SequenceName());
      bnt_node->SetBntFunction(bnt_hook->FunctionName());
      bnt_node->SetRealPath(targetPC);

      // Prevent random instructions from being generated along the not taken path in a loop. If we
      // allow random instructions to be generated along the not taken path, we lose the ability to
      // control execution of the loop and ensure it terminates successfully.
      if (mpGenerator->InLoop() and (not mpGenerator->RestoreStateLoop())) {
        bnt_node->PreserveNotTakenPath(mpGenerator);
      }

      mpGenerator->GetBntNodeManager()->SaveBntNode(bnt_node);
    }
    else {
      delete bnt_node;
    }
  }

  void GenInstructionAgent::UpdateInstructionCount()
  {
    ++ mInstrSimulated;
    if (mInstrSimulated > mpGenerator->MaxInstructions()) {
      LOG(fail) << "Instruction simulation limit : " << dec << mpGenerator->MaxInstructions() << " exceeded." << endl;
      FAIL("instruction-simulation-limit-exceeded");
    }
  }

  void GenInstructionAgent::UpdateRegisterFromSimulation(const vector<RegUpdate>& regUpdates, bool hasExceptEvent, uint64& targetPC)
  {
    for (auto const update : regUpdates) {
      if (update.access_type == "write") {
        if (update.regname == "PC") {
	  LOG(info) << "{GenInstructionAgent::UpdateRegisterFromSimulation} PC: 0x" << std::hex << update.rval << std::dec << std::endl;
          auto gen_pc = mpGenerator->GetGenPC();
          gen_pc->SetAligned(update.rval);
          targetPC = update.rval;
          if (mpGenerator->InLoop()) {
            SendNotification(ENotificationType::PCUpdate);
          }

          LOG(info) << "{GenInstructionAgent::UpdateRegisterFromSimulation} update PC value 0x" << hex << update.rval << endl;
        }
        else {
          auto reg_file = mpGenerator->GetRegisterFile();
          auto phys_register = reg_file->PhysicalRegisterLookup(update.regname);
          LOG(info) << "{GenInstructionAgent::UpdateRegisterFromSimulation} update register " << update.regname << " value 0x" << hex << update.rval << ", mask 0x" << update.mask << endl;
          phys_register->SetAttribute(ERegAttrType::UpdatedFromISS);
          try {
            phys_register->SetValue(update.rval, update.mask & phys_register->Mask());
          }
          catch (const RegisterError& reg_error) {
            string error_msg = reg_error.what();
            auto not_init_str = EGenExceptionDetatilType_to_string(EGenExceptionDetatilType::RegisterNotInitSetValue);
            if (error_msg.find(not_init_str) != string::npos) {
              if (hasExceptEvent || mpGenerator->InException()) {
                LOG(notice) << "{GenInstructionAgent::UpdateRegisterFromSimulation} setting value 0x" << hex << update.rval << " to uninitialized register: " << update.regname << " exception event? " << hasExceptEvent << ", in-exception? "<< mpGenerator->InException() << endl;
                reg_file->SetPhysicalRegisterValueAndInit(phys_register, update.rval, update.mask & phys_register->Mask(), 0, false);
                continue;
              }
              if (mpGenerator->ReExecution() and reg_file->AllowReExecutionInit(phys_register->Name())) {
                LOG(notice) << "{GenInstructionAgent::UpdateRegisterFromSimulation} setting value 0x" << hex << update.rval << " to uninitialized register: " << update.regname << " in re-execution." << endl;
                reg_file->SetPhysicalRegisterValueAndInit(phys_register, update.rval, update.mask & phys_register->Mask(), 0, false);
                continue;
              }
              if (phys_register->RegisterType() == ERegisterType::VECREG and update.rval == 0ull) {
                LOG(info) << "{GenInstructionAgent::UpdateRegisterFromSimulation} setting value 0x" << hex << update.rval << " to uninitialized register: " << update.regname << " as zero extend." << endl;
                reg_file->SetPhysicalRegisterValueAndInit(phys_register, update.rval, update.mask & phys_register->Mask(), 0, false);
                continue;
              }
            }
            LOG(fail) << "{GenInstructionAgent::UpdateRegisterFromSimulation} " << reg_error.what() << endl;
            FAIL("sim-update-register-error");
          }
        }
      }
    }
  }

  void GenInstructionAgent::UpdateMemoryFromSimulation(const vector<MemUpdate>& memUpdates)
  {
    auto memoryManager = mpGenerator->GetMemoryManager();
    for (auto const update : memUpdates) {
      // << "{GenInstructionAgent::UpdateMemoryFromSimulation} update bank=" << update.mem_bank << " PA=0x" << hex << update.physical_address << " access_type=" << update.access_type << endl;
      if (update.access_type == "write") {
        auto *data = update.bytes.data();
        LOG(info) << "{GenInstructionAgent::UpdateMemoryFromSimulation} writing [" << update.mem_bank << "] PA=0x" << hex << update.physical_address << endl;
        memoryManager->GetMemoryBank(update.mem_bank)->WriteMemory(update.physical_address, (cuint8* )data, update.size);
      }
    }
  }

  bool GenInstructionAgent::HasExceptionEvent(const vector<ExceptionUpdate> & rExcepEvents, bool& hasEretEvent) const
  {
    if (rExcepEvents.size() > 0) {
      uint32 exception_id = rExcepEvents.front().mExceptionID;
      hasEretEvent = IsEret(exception_id);

      if (rExcepEvents.size() > 1) {
        LOG(fail) << "{GenInstructionAgent::HasExceptionEvent} not expecting multiple exception event updates." << endl;
        FAIL("multiple-exception-events");
      }
      return true;
    }

    return false;
  }

  bool GenInstructionAgent::UpdateExceptionEvent(vector<ExceptionUpdate> & rExcepEvents)
  {
    const ExceptionUpdate& excep_event = rExcepEvents.front();
    uint32 exception_id = excep_event.mExceptionID;
    LOG(notice) << "{GenInstructionAgent::UpdateExceptionEvent} exception ID: 0x" << hex << exception_id << " attributes: " << dec << excep_event.mExceptionAttributes << " description: " << excep_event.mComments << endl;
    bool single_thread = Config::Instance()->NumPEs() == 1;
    if (IsLowPower(exception_id)) {
      LOG(info) <<  "{GenInstructionAgent::UpdateExceptionEvent} inject VSEVL to wake up" << endl;
      rExcepEvents.erase(rExcepEvents.begin());
      SimAPI *sim_api = mpGenerator->GetSimAPI(); // Get handle to simulator.
      sim_api->WakeUp(mpGenerator->ThreadId()); // Call ISS interface to wake up the thread.
      if (single_thread)
        return false;
    }

    mpGenerator->PrependRequest(new GenHandleException(excep_event.mExceptionID, excep_event.mComments)); // exception handling request.

    return true;
  }

  static cuint32 sMaxExceptionHandlerLength = 1000; // TODO, set a temporary max exception handler length.

  void GenInstructionAgent::ExecuteHandler()
  {
    PaTuple pa_tuple;
    auto gen_pc = mpGenerator->GetGenPC();
    uint64 pc_pa = 0;
    uint32 pc_bank = 0;
    bool fault = false;
    pc_pa = gen_pc->GetPA(mpGenerator, pc_bank, fault);

    auto memory_manager = mpGenerator->GetMemoryManager();
    uint32 mem_attrs = memory_manager->GetMemoryBank(pc_bank)->GetByteMemoryAttributes(pc_pa);
    uint32 expected_attrs = EMemDataTypeBaseType(EMemDataType::Init) | EMemDataTypeBaseType(EMemDataType::Instruction);
    LOG(info) << "{GenInstructionAgent::ExecuteHandler} starting PC=0x" << hex << gen_pc->Value() << "=>[" << EMemBankType_to_string(EMemBankType(pc_bank)) << "]0x" << pc_pa << " memory attributes: 0x" << mem_attrs << endl;

    if (mem_attrs != expected_attrs) {
      LOG(fail) << "{GenInstructionAgent::ExecuteHandler} expect address [" << EMemBankType_to_string(EMemBankType(pc_bank)) << "]0x" << hex << pc_pa << " to be initialized with instruction." << endl;
      FAIL("expect-handler-memory-initialized");
    }

    if (mpGenerator->InException() == false) {
          /* We only want to push a new exception frame when we're entering into a new exception */
          EGenModeTypeBaseType gen_mode_change = EGenModeTypeBaseType(EGenModeType::Exception) | EGenModeTypeBaseType(EGenModeType::NoEscape) | EGenModeTypeBaseType(EGenModeType::ReExe);

          auto state_req = new GenStateRequest(EGenStateActionType::Push, EGenStateType::GenMode, gen_mode_change); // modify GenMode to Exception, NoEscape and ReExe.
          unique_ptr<GenRequest> storage_ptr(state_req); // responsible for releasing the storage when going out of scope.
          mpGenerator->StateRequest(state_req);
    }

    uint32 exc_handler_length = 0;
    do {
      if (StepInstructionWithSimulation()) {
        break;
      }
      ++ exc_handler_length;
      if (exc_handler_length > sMaxExceptionHandlerLength) {
        LOG(fail) << "{GenInstructionAgent::ExecuteHandler} max exception handling instruction count reached: " << dec << sMaxExceptionHandlerLength << endl;
        FAIL("max-handler-length-reached");
        break;
      }

      gen_pc->GetPA(mpGenerator, pa_tuple);
      if (not memory_manager->InstructionPaInitialized(pa_tuple)) {
        LOG(notice) << "{GenInstructionAgent::ExecuteHandler} next PC 0x" << hex << gen_pc->Value() << "=>" << pa_tuple.ToString() << " is not initialized, ending handler execution." << endl;
        ExceptionReturn();
        // TODO detect conditional branch in loop and re-converge back.
        break;
      }

    }
    while (1);
  }

  void GenInstructionAgent::ExceptionReturn()
  {
    if (not mpGenerator->InException()) {
      LOG(notice) << "{GenInstructionAgent::ExceptionReturn} not currently in exception." << endl;
      return;
    }

    EGenModeTypeBaseType possible_mode_changes = EGenModeTypeBaseType(EGenModeType::Exception) | EGenModeTypeBaseType(EGenModeType::NoEscape) | EGenModeTypeBaseType(EGenModeType::ReExe);
    auto gen_mode = mpGenerator->GetGenMode();
    EGenModeTypeBaseType current_mode = gen_mode->CurrentMode();
    auto gen_mode_change = current_mode & possible_mode_changes;

    /* We only want to pop the exception unit when we are returning back to the user code */
    PaTuple pa_tuple;
    auto gen_pc = mpGenerator->GetGenPC();
    uint64 pc_va = gen_pc->Value();
    bool fault = false;
    pa_tuple.mAddress = gen_pc->GetPA(mpGenerator, pa_tuple.mBank, fault);
    if (ExceptionManager::Instance()->IsReturningToUser(pa_tuple)) {
          auto state_req = new GenStateRequest(EGenStateActionType::Pop, EGenStateType::GenMode, gen_mode_change); // revert GenMode back.
          unique_ptr<GenRequest> storage_ptr(state_req); // responsible for releasing the storage when going out of scope.
          mpGenerator->StateRequest(state_req);
    }

    bool re_exe = mpGenerator->GetMemoryManager()->InstructionPaInitialized(pa_tuple);
    LOG(notice) << "{GenInstructionAgent::ExceptionReturn} returned from exception to PC 0x" << hex << pc_va << " re-execution? " << re_exe << endl;

    if (re_exe) {
      auto re_exe_req = new GenReExecutionRequest(pc_va);
      mpGenerator->PrependRequest(re_exe_req);
    }
    else if (fault) {
      StepInstructionWithSimulation();
    }
  }

  void GenInstructionAgent::ReExecute(cuint64 addr, cuint32 maxReExeInstr)
  {
    LOG(notice) << "{GenInstructionAgent::ReExecute} starting address 0x" << hex << addr << endl;
    mpGenerator->SetPC(addr);
    PaTuple pa_tuple;
    auto gen_pc = mpGenerator->GetGenPC();
    gen_pc->GetPA(mpGenerator, pa_tuple);
    auto mem_man = mpGenerator->GetMemoryManager();

    if (not mem_man->InstructionPaInitialized(pa_tuple)) {
      LOG(fail) << "{GenInstructionAgent::ReExecute} PC target 0x" << hex << addr << " not initialized." << endl;
      FAIL("re-execution-target-not-initialized");
    }

    EGenModeTypeBaseType gen_mode_change = EGenModeTypeBaseType(EGenModeType::ReExe);
    auto state_req = new GenStateRequest(EGenStateActionType::Push, EGenStateType::GenMode, gen_mode_change); // modify GenMode to ReExe.
    unique_ptr<GenRequest> storage_ptr(state_req); // responsible for releasing the storage when going out of scope.
    mpGenerator->StateRequest(state_req);

    uint32 re_exe_length = 0;
    uint32 re_exe_step = 0;
    bool try_loop_reconverge = false;
    do {
      if (StepInstructionWithSimulation()) {
        LOG(notice) << "{GenInstructionAgent::ReExecute} exception in re-execution." << endl;
        break;
      }
      ++ re_exe_length;
      ++ re_exe_step;
      if (re_exe_step > 1000) {
        LOG(notice) << "{GenInstructionAgent::ReExecute} re-executed " << dec << re_exe_length << " instructions." << endl;
        re_exe_step = 0;
      }

      bool fault = false;
      pa_tuple.mAddress = gen_pc->GetPA(mpGenerator, pa_tuple.mBank, fault);
      if ((not fault) and (not mem_man->InstructionPaInitialized(pa_tuple))) {
        LOG(notice) << "{GenInstructionAgent::ReExecute} next PC 0x" << hex << gen_pc->Value() << "=>" << pa_tuple.ToString() << " is not initialized, ending re-execution, in loop? " << mpGenerator->InLoop() << " restore state loop? " << mpGenerator->RestoreStateLoop() << endl;
        if (mpGenerator->InLoop() and (not mpGenerator->RestoreStateLoop())) {
          auto reexe_manager = mpGenerator->GetReExecutionManager();
          uint64 post_loop_addr = reexe_manager->PostLoopAddress();
          if (gen_pc->Value() == post_loop_addr) {
            LOG(info) << "{GenInstructionAgent::ReExecute} reached post-loop instruction in loop, finishing loop." << endl;
          }
          else {
            LOG(info) << "Need to do loop reconverge." << endl;
            try_loop_reconverge = true;
          }
        }
        // TODO detect conditional branch in loop and re-converge back.
        break;
      }
      else if (re_exe_length >= maxReExeInstr) {
        break;
      }
    }
    while (1);

    LOG(notice) << "{GenInstructionAgent::ReExecute} re-executed " << dec << re_exe_length << " instructions, try loop reconverge? " << try_loop_reconverge << endl;

    auto state_req_back = new GenStateRequest(EGenStateActionType::Pop, EGenStateType::GenMode, gen_mode_change); // modify GenMode to ReExe.
    unique_ptr<GenRequest> storage_ptr2(state_req_back); // responsible for releasing the storage when going out of scope.
    mpGenerator->StateRequest(state_req_back);

    if (try_loop_reconverge) {
      mpGenerator->PrependRequest(new GenLoopReconvergeRequest(mpGenerator->PC(), mpGenerator->LastPC())); // loop reconverge request.
    }
  }

  void GenInstructionAgent::InitializeReadOnlyRegistersWithISS(const vector<ReadOnlyRegister* >& rRegs)
  {
    uint32 thread_id = mpGenerator->ThreadId();
    auto reg_file = mpGenerator->GetRegisterFile();
    SimAPI *sim_ptr = mpGenerator->GetSimAPI(); // get handle to simulator
    for (auto ro_reg : rRegs) {
      uint64 reg_value = 0;
      uint64 reg_mask = 0;
      sim_ptr->ReadRegister(thread_id, ro_reg->Name().c_str(), &reg_value, &reg_mask);
      LOG(info) << "{GenInstructionAgent::InitializeReadOnlyRegistersWithISS} read-only register " << ro_reg->Name() << " value from ISS: 0x" << hex << reg_value << " mask: 0x" << reg_mask << endl;
      reg_file->InitializeReadOnlyRegisterFromIss(ro_reg, reg_value);
    }
  }

  void GenInstructionAgent::ResetReadOnlyRegisterFieldsWithISS(const std::vector<ReadOnlyRegisterField* >& rRegFields)
  {
    uint32 thread_id = mpGenerator->ThreadId();
    auto reg_file = mpGenerator->GetRegisterFile();
    SimAPI *sim_ptr = mpGenerator->GetSimAPI(); // get handle to simulator
    string last_reg_name;
    uint64 last_reg_val = 0;
    for (auto reg_field: rRegFields) {
      auto reg_name = reg_field->PhysicalRegisterName();
      if (reg_name != last_reg_name) {
        uint64 reg_val = 0;
        uint64 reg_mask = 0;
        sim_ptr->ReadRegister(thread_id, reg_name.c_str(), &reg_val, &reg_mask);
        last_reg_name = reg_name;
        last_reg_val = reg_val;
      }
      auto field_val =(last_reg_val & reg_field->FieldMask()) >> reg_field->Lsb();
      LOG(info) << "{GenInstructionAgent::InitializeReadOnlyRegisterFieldsWithISS} read-only register field " << reg_field->Name() << " value from ISS: 0x" << hex << field_val << endl;
      reg_file->ResetReadOnlyRegisterFieldFromIss(reg_field, field_val);
    }
  }

  void GenInstructionAgent::RecoverExceptionBeforeUpdate(const vector<ExceptionUpdate>& exceptUpdates, const std::vector<RegUpdate>& regUpdates, SimAPI* pSimAPI)
  {
    const ExceptionUpdate& excep_event = exceptUpdates.front();
    uint32 exception_id = excep_event.mExceptionID;
    LOG(info ) << "{GenInstructionAgent::RecoverExceptionBeforeUpdate}  exception ID: 0x" << hex << exception_id << " attributes: " << dec << excep_event.mExceptionAttributes << " description: " << excep_event.mComments << endl;

    if (IsLowPower(exception_id)) {
      SimAPI *sim_api = mpGenerator->GetSimAPI(); // Get handle to simulator.
      sim_api->WakeUp(mpGenerator->ThreadId()); // Call ISS interface to wake up the thread.
    }
    else if (IsSimExit(exception_id)) {
      SimAPI *sim_api = mpGenerator->GetSimAPI(); // Get handle to simulator.
      sim_api->TurnOn(mpGenerator->ThreadId()); // Turn the thread back on.
    }
    else {
      // recovering register
      auto reg_file = mpGenerator->GetRegisterFile();
      for (auto const update : regUpdates) {
        if (update.access_type == "read" || update.regname == "PC")
          continue;
        auto phys_register = reg_file->PhysicalRegisterLookup(update.regname);
        auto mask = update.mask & phys_register->Mask();
        if (phys_register->IsInitialized(mask)) {
          pSimAPI->WriteRegister(mpGenerator->ThreadId(), phys_register->Name().c_str(), phys_register->Value(mask), update.mask);
        }
        else {
          LOG(info) <<"{GenInstructionAgent::RecoverExceptionBeforeUpdate} Give up recover un-initialized register: " <<  phys_register->Name() << endl;
        }

      }

      mpGenerator->AdvancePC();
      pSimAPI->WriteRegister(mpGenerator->ThreadId(), "PC", mpGenerator->PC(), -1ull);
    }
  }

  void GenInstructionAgent::SaveRegisterBeforeUpdate(const vector<RegUpdate>& regUpdates, BntNode* pBntNode)
  {
    auto reg_file = mpGenerator->GetRegisterFile();
    for (auto const update : regUpdates) {
      if (update.access_type == "read" || update.regname == "PC")
        continue;
      auto phys_register = reg_file->PhysicalRegisterLookup(update.regname);
      auto mask = update.mask & phys_register->Mask();
      if (not phys_register->IsInitialized(mask)) {
        LOG(info) << "{GenInstructionAgent::SaveRegisterBeforeUpdate} setting value 0x" << hex << update.rval << " to uninitialized register: " << update.regname << endl;
        reg_file->SetPhysicalRegisterValueAndInit(phys_register, update.rval, update.mask & phys_register->Mask(), 0, false);
      }

      pBntNode->PushResourcePeState(new RegisterPeState(phys_register, update.mask, phys_register->Value(mask)));
    }
  }

  void GenInstructionAgent::SaveLoopRegisterBeforeUpdate(const vector<RegUpdate>& regUpdates)
  {
    auto reg_file = mpGenerator->GetRegisterFile();
    RestoreLoopManagerRepository* restore_loop_manager_repository = RestoreLoopManagerRepository::Instance();
    RestoreLoopManager* restore_loop_manager = restore_loop_manager_repository->GetRestoreLoopManager(mpGenerator->ThreadId());
    for (auto const update : regUpdates) {
      if (update.access_type == "read" || update.regname == "PC") {
        continue;
      }

      auto phys_register = reg_file->PhysicalRegisterLookup(update.regname);
      auto mask = update.mask & phys_register->Mask();
      if (not phys_register->IsInitialized(mask)) {
        LOG(info) << "{GenInstructionAgent::SaveLoopRegisterBeforeUpdate} setting value 0x" << hex << update.rval << " to uninitialized register: " << update.regname << endl;
        reg_file->SetPhysicalRegisterValueAndInit(phys_register, update.rval, update.mask & phys_register->Mask(), 0, false);
      }

      restore_loop_manager->PushResourcePeState(new RegisterPeState(phys_register, update.mask, phys_register->Value(mask)));
    }
  }

  void GenInstructionAgent::SaveMemoryBeforeUpdate(const vector<MemUpdate>& memUpdates, BntNode* pBntNode)
  {
    auto memoryManager = mpGenerator->GetMemoryManager();
    for (auto const update : memUpdates) {
      if (update.access_type == "read")
        continue;

      vector<unsigned char> data_buffer(update.size, 0);
      auto *pData = data_buffer.data();
      memoryManager->GetMemoryBank(update.mem_bank)->ReadMemoryPartiallyInitialized(update.physical_address, update.size,  (uint8*)pData);
      for (unsigned i = 0; i < update.size; i++)
        pBntNode->PushResourcePeState(new ByteMemoryPeState(update.mem_bank, update.physical_address + i, update.virtual_address + i, pData[i]));

    }
  }

  void GenInstructionAgent::SaveLoopMemoryBeforeUpdate(const vector<MemUpdate>& memUpdates)
  {
    VirtualMemoryInitializer* virt_mem_initializer = mpGenerator->GetVirtualMemoryInitializer();
    RestoreLoopManagerRepository* restore_loop_manager_repository = RestoreLoopManagerRepository::Instance();
    RestoreLoopManager* restore_loop_manager = restore_loop_manager_repository->GetRestoreLoopManager(mpGenerator->ThreadId());
    for (auto const update : memUpdates) {
      if (update.access_type == "read") {
        continue;
      }

      VmManager* vm_manager = mpGenerator->GetVmManager();
      VmMapper* vm_mapper = vm_manager->CurrentVmMapper();
      const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
      uint64 untagged_update_va = addr_tagging->UntagAddress(update.virtual_address, false);

      // Break apart the memory update into chunks and record the chunks
      uint32 chunk_size = BlockMemoryPeState::GetMaxBlockSize();
      uint32 align_shift = get_align_shift(chunk_size);
      uint64 cur_va = (untagged_update_va >> align_shift) << align_shift;

      uint64 end_va = untagged_update_va + update.size;
      uint64 mem_range_size = end_va - cur_va;
      if (mem_range_size % chunk_size != 0) {
        mem_range_size = ((mem_range_size / chunk_size) + 1) * chunk_size;
      }

      uint64 last_accessed_va = cur_va + mem_range_size - 1;
      if (last_accessed_va < cur_va) {
        LOG(fail) << "{GenInstructionAgent:SaveLoopMemoryBeforeUpdate} access to VA 0x" << hex << cur_va << " of size 0x" << mem_range_size << " exceeded maximum possible 64-bit address." << endl;
        FAIL("virtual-address-overflow");
      }

      InitializeLoopMemory(cur_va, mem_range_size);

      while (cur_va <= last_accessed_va) {
        vector<uint8> data_buffer(chunk_size, 0);
        virt_mem_initializer->ReadMemory(cur_va, chunk_size, data_buffer.data());

        uint64 data_value = 0;
        if (mpGenerator->IsDataBigEndian()) {
          data_value = data_array_to_element_value_big_endian(data_buffer.data(), chunk_size);
        }
        else {
          data_value = data_array_to_element_value_little_endian(data_buffer.data(), chunk_size);
        }

        restore_loop_manager->PushResourcePeState(new BlockMemoryPeState(update.mem_bank, cur_va, data_value, chunk_size));

        // Avoid overflow
        if ((MAX_UINT64 - cur_va) < chunk_size) {
          break;
        }

        cur_va += chunk_size;
      }
    }
  }

  void GenInstructionAgent::UpdateUnpredictedConstraint(const Instruction* pInstr)
  {
    for (auto opr_ptr : pInstr->GetOperands()) {
      RegisterOperand* reg_opr = dynamic_cast<RegisterOperand* >(opr_ptr);
      if (reg_opr == nullptr)
        continue;
      auto has_write_access = reg_opr->GetOperandStructure()->HasWriteAccess();
      if (not has_write_access)
        continue;

      auto choice_text = reg_opr->ChoiceText();
      auto choice_reg = mpGenerator->GetRegisterFile()->RegisterLookup(choice_text);
      if (choice_reg->RegisterType() == ERegisterType::ZR)
        continue;

      //TBD: skip the register if it is not target one such as base register in load post index.
      if (pInstr->Unpredictable()) {
        if (not mpGenerator->IsRegisterReserved(choice_text, ERegAttrType::Read, ERegReserveType::Unpredictable)) {
          mpGenerator->ReserveRegister(choice_text, ERegAttrType::Read, ERegReserveType::Unpredictable);
          // << "{GenInstructionAgent::UpdateUnpredictedConstraint} reserve read register: " << choice_text << endl;
        }
      }
      else if (not pInstr->IsPartialWriter()) {
        if (mpGenerator->IsRegisterReserved(choice_text, ERegAttrType::Read, ERegReserveType::Unpredictable)) {
          mpGenerator->UnreserveRegister(choice_text, ERegAttrType::Read, ERegReserveType::Unpredictable);
          // << "{GenInstructionAgent::UpdateUnpredictedConstraint} unreserve read register: " << choice_text << endl;
        }
      }
    }
  }

  // This method carefully initializes only uninitialized memory. The reason for this is that the
  // memory is being captured prior to propagating the simulator updates to the Memory module. If we
  // re-initialize memory that has just been written by the simulator with data read from the Memory
  // module, the subsequent memory initialization in the simulator will overwrite the preceding
  // memory write.
  void GenInstructionAgent::InitializeLoopMemory(cuint64 startVa, cuint64 memRangeSize) const
  {
    VirtualMemoryInitializer* virt_mem_initializer = mpGenerator->GetVirtualMemoryInitializer();
    vector<uint8> mem_attrs(memRangeSize);
    virt_mem_initializer->GetMemoryAttributes(startVa, memRangeSize, mem_attrs.data());

    uint64 init_mem_va = startVa;
    uint32 init_mem_size = 0;
    for (uint8 mem_attr : mem_attrs) {
      bool byte_initialized = (mem_attr & EMemDataTypeBaseType(EMemDataType::Init)) == EMemDataTypeBaseType(EMemDataType::Init);

      if (byte_initialized) {
        if (init_mem_size > 0) {
          MemoryInitData init_data(init_mem_va, init_mem_size, 1, EMemAccessType::Write);
          init_data.Setup(*mpGenerator, new ConstraintSet(0, MAX_UINT64));
          init_data.Commit(*mpGenerator);
          init_mem_va += init_mem_size;
        }

        init_mem_va++;
        init_mem_size = 0;
      }
      else {
        init_mem_size++;
      }
    }

    if (init_mem_size > 0) {
      MemoryInitData init_data(init_mem_va, init_mem_size, 1, EMemAccessType::Write);
      init_data.Setup(*mpGenerator, new ConstraintSet(0, MAX_UINT64));
      init_data.Commit(*mpGenerator);
    }
  }

}
