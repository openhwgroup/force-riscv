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
#include <Generator.h>
#include <GenRequest.h>
#include <GenRequestQueue.h>
#include <GenAgent.h>
#include <GenQuery.h>
#include <InstructionResults.h>
#include <Record.h>
#include <MemoryManager.h>
#include <VirtualMemoryInitializer.h>
#include <Register.h>
#include <RegisterReserver.h>
#include <ChoicesModerator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <Instruction.h>
#include <BootOrder.h>
#include <Constraint.h>
#include <GenMode.h>
#include <MemoryReservation.h>
#include <GenInstructionAgent.h>
#include <GenPC.h>
#include <PcSpacing.h>
#include <ReExecutionManager.h>
#include <ResourceDependence.h>
#include <Variable.h>
#include <InstructionSet.h>
#include <RegisteredSetModifier.h>
#include <RegisterInitPolicy.h>
#include <GenCondition.h>
#include <AddressTagging.h>
#include <PageRequestRegulator.h>
#include <AddressFilteringRegulator.h>
#include <Config.h>
#include <PathUtils.h>
#include <ImageIO.h>
#include <BntHookManager.h>
#include <BntNodeManager.h>
#include <AddressTableManager.h>
#include <StateTransition.h>
#include <Log.h>

#include <algorithm>
#include <memory>
#include <sstream>

using namespace std;

namespace Force {

  Generator::Generator()
    : Object(), mThreadId(0), mMaxInstructions(0), mMaxPhysicalVectorLen(0), mpArchInfo(nullptr), mpInstructionSet(nullptr), mpPagingInfo(nullptr), mpMemoryManager(nullptr), mpSimAPI(nullptr), mpVirtualMemoryInitializer(nullptr), mpRegisterFile(nullptr), mpVmManager(nullptr), mpRequestQueue(nullptr),
      mpThreadInstructionResults(nullptr), mpRecordArchive(nullptr), mpBootOrder(nullptr), mpGenMode(nullptr), mpGenPC(nullptr), mpReExecutionManager(nullptr), mpDependence(nullptr), mpRegisteredSetModifier(nullptr), mpExceptionRecordManager(nullptr), mpChoicesModerators(nullptr),
      mpConditionSet(nullptr), mpPageRequestRegulator(nullptr), mpAddressFilteringRegulator(nullptr), mpBntHookManager(nullptr), mpBntNodeManager(nullptr), mpAddressTableManager(nullptr), mAgents(), mGenStateValues(), mGenStateStrings(), mPreAmbleRequests(), mPostAmbleRequests(),
      mSpeculativeRequests(), mVariableModerators()
  {
    mpRequestQueue = new GenRequestQueue();

    mpThreadInstructionResults = new ThreadInstructionResults(MemoryManager::Instance()->NumberBanks());
    InstructionResults* instructionResults = InstructionResults::Instance();
    instructionResults->AddThreadResults(mpThreadInstructionResults);

    mpRecordArchive = new RecordArchive();
    mpGenPC = new GenPC();
    mpReExecutionManager = new ReExecutionManager();
    mpDependence = new ResourceDependence();
    mpChoicesModerators = new ChoicesModerators();
    mpRegisteredSetModifier = new RegisteredSetModifier();
    mpBntHookManager = new BntHookManager();
    mpBntNodeManager = new BntNodeManager();
  }

  Generator::Generator(uint64 alignmentMask)
    : Object(), mThreadId(0), mMaxInstructions(0), mMaxPhysicalVectorLen(0), mpArchInfo(nullptr), mpInstructionSet(nullptr), mpPagingInfo(nullptr), mpMemoryManager(nullptr), mpSimAPI(nullptr), mpVirtualMemoryInitializer(nullptr), mpRegisterFile(nullptr), mpVmManager(nullptr), mpRequestQueue(nullptr),
      mpThreadInstructionResults(nullptr), mpRecordArchive(nullptr), mpBootOrder(nullptr), mpGenMode(nullptr), mpGenPC(nullptr), mpReExecutionManager(nullptr), mpDependence(nullptr), mpRegisteredSetModifier(nullptr), mpExceptionRecordManager(nullptr), mpChoicesModerators(nullptr),
      mpConditionSet(nullptr), mpPageRequestRegulator(nullptr), mpAddressFilteringRegulator(nullptr), mpBntHookManager(nullptr), mpBntNodeManager(nullptr), mpAddressTableManager(nullptr), mAgents(), mGenStateValues(), mGenStateStrings(), mPreAmbleRequests(), mPostAmbleRequests(),
      mSpeculativeRequests(), mVariableModerators()
  {
    mpRequestQueue = new GenRequestQueue();

    mpThreadInstructionResults = new ThreadInstructionResults(MemoryManager::Instance()->NumberBanks());
    InstructionResults* instructionResults = InstructionResults::Instance();
    instructionResults->AddThreadResults(mpThreadInstructionResults);

    mpRecordArchive = new RecordArchive();
    mpGenPC = new GenPC(alignmentMask);
    mpReExecutionManager = new ReExecutionManager();
    mpDependence = new ResourceDependence();
    mpChoicesModerators = new ChoicesModerators();
    mpRegisteredSetModifier = new RegisteredSetModifier();
    mpBntHookManager = new BntHookManager();
    mpBntNodeManager = new BntNodeManager();
  }

  Generator::Generator(const Generator& rOther)
    : Object(rOther), mThreadId(0), mMaxInstructions(rOther.mMaxInstructions), mMaxPhysicalVectorLen(rOther.mMaxPhysicalVectorLen), mpArchInfo(rOther.mpArchInfo), mpInstructionSet(rOther.mpInstructionSet), mpPagingInfo(rOther.mpPagingInfo), mpMemoryManager(rOther.mpMemoryManager), mpSimAPI(rOther.mpSimAPI),
      mpVirtualMemoryInitializer(nullptr), mpRegisterFile(nullptr), mpVmManager(nullptr), mpRequestQueue(nullptr), mpThreadInstructionResults(nullptr), mpRecordArchive(nullptr), mpBootOrder(nullptr), mpGenMode(nullptr), mpGenPC(nullptr), mpReExecutionManager(nullptr), mpDependence(nullptr),
      mpRegisteredSetModifier(nullptr), mpExceptionRecordManager(nullptr), mpChoicesModerators(nullptr), mpConditionSet(nullptr), mpPageRequestRegulator(nullptr), mpAddressFilteringRegulator(nullptr), mpBntHookManager(nullptr), mpBntNodeManager(), mpAddressTableManager(nullptr),
      mAgents(), mGenStateValues(), mGenStateStrings(), mPreAmbleRequests(), mPostAmbleRequests(), mSpeculativeRequests(), mVariableModerators()
  {
    if (rOther.mpRequestQueue) {
      mpRequestQueue = dynamic_cast<GenRequestQueue* >(rOther.mpRequestQueue->Clone());
    }

    // Each Generator object should only count its own generated instructions, so don't copy the rOther Generator's
    // results.
    mpThreadInstructionResults = new ThreadInstructionResults(MemoryManager::Instance()->NumberBanks());
    InstructionResults* instructionResults = InstructionResults::Instance();
    instructionResults->AddThreadResults(mpThreadInstructionResults);

    if (rOther.mpRecordArchive) {
      mpRecordArchive = dynamic_cast<RecordArchive* >(rOther.mpRecordArchive->Clone());
    }

    if (rOther.mpRegisterFile) {
      mpRegisterFile = dynamic_cast<RegisterFile* >(rOther.mpRegisterFile->Clone());
    }

    if (rOther.mpVmManager) {
      mpVmManager = dynamic_cast<VmManager* >(rOther.mpVmManager->Clone());
    }

    if (rOther.mpBootOrder) {
      mpBootOrder = dynamic_cast<BootOrder* > (rOther.mpBootOrder->Clone());
    }

    if (rOther.mpGenMode) {
      mpGenMode = dynamic_cast<GenMode* > (rOther.mpGenMode->Clone());
    }

    if (rOther.mpGenPC) {
      mpGenPC = dynamic_cast<GenPC* > (rOther.mpGenPC->Clone());
    }

    if (rOther.mpReExecutionManager) {
      mpReExecutionManager = dynamic_cast<ReExecutionManager* > (rOther.mpReExecutionManager->Clone());
    }

    if (rOther.mpDependence) {
      mpDependence = dynamic_cast<ResourceDependence* >(rOther.mpDependence->Clone());
    }

    if (rOther.mpRegisteredSetModifier) {
      mpRegisteredSetModifier = dynamic_cast<RegisteredSetModifier* >(rOther.mpRegisteredSetModifier->Clone());
    }

    if (rOther.mpExceptionRecordManager) {
      mpExceptionRecordManager = dynamic_cast<ExceptionRecordManager* >(rOther.mpExceptionRecordManager->Clone());
    }

    if (rOther.mpChoicesModerators) {
      mpChoicesModerators = dynamic_cast<ChoicesModerators* >(rOther.mpChoicesModerators->Clone());
    }

    if (rOther.mpBntHookManager) {
      mpBntHookManager = dynamic_cast<BntHookManager* >(rOther.mpBntHookManager->Clone());
    }

    if (rOther.mpBntNodeManager) {
      mpBntNodeManager = dynamic_cast<BntNodeManager* >(rOther.mpBntNodeManager->Clone());
    }

    if (rOther.mpAddressTableManager) {
      mpAddressTableManager = dynamic_cast<AddressTableManager* >(rOther.mpAddressTableManager->Clone());
    }

    if (rOther.mpConditionSet) {
      mpConditionSet = dynamic_cast<GenConditionSet* >(rOther.mpConditionSet->Clone());
    }

    if (rOther.mpPageRequestRegulator) {
      mpPageRequestRegulator = dynamic_cast<PageRequestRegulator* >(rOther.mpPageRequestRegulator->Clone());
    }

    if (rOther.mpAddressFilteringRegulator) {
      mpAddressFilteringRegulator = dynamic_cast<AddressFilteringRegulator* >(rOther.mpAddressFilteringRegulator->Clone());
    }

    if (rOther.mAgents.size()) {
      mAgents.assign(rOther.mAgents.size(), nullptr);
      for (auto const agent_ptr : rOther.mAgents) {
        GenAgent* clone_agent = dynamic_cast<GenAgent* >(agent_ptr->Clone());
        clone_agent->SetGenerator(this);
        mAgents[int(clone_agent->GenAgentType())] = clone_agent;
      }
    }

    if (rOther.mVariableModerators.size()) {
      transform(rOther.mVariableModerators.begin(), rOther.mVariableModerators.end(), back_inserter(mVariableModerators), [](const VariableModerator* var_ptr) { return dynamic_cast<VariableModerator*>(var_ptr->Clone()); });
    }

    if (rOther.mpVirtualMemoryInitializer) {
      mpVirtualMemoryInitializer = new VirtualMemoryInitializer(mpVmManager, mpRecordArchive, mpMemoryManager);
    }
  }

  Generator::~Generator()
  {
    mpArchInfo = nullptr;
    mpInstructionSet = nullptr;
    mpPagingInfo = nullptr;
    mpMemoryManager = nullptr;
    mpSimAPI = nullptr;
    delete mpVirtualMemoryInitializer;
    delete mpRequestQueue;
    delete mpThreadInstructionResults;
    delete mpRecordArchive;
    delete mpRegisterFile;
    delete mpVmManager;
    delete mpBootOrder;
    delete mpGenMode;
    delete mpGenPC;
    delete mpReExecutionManager;
    delete mpDependence;
    delete mpRegisteredSetModifier;
    delete mpExceptionRecordManager;
    delete mpChoicesModerators;
    delete mpConditionSet;
    delete mpPageRequestRegulator;
    delete mpAddressFilteringRegulator;
    delete mpBntHookManager;
    delete mpBntNodeManager;
    delete mpAddressTableManager;

    for (auto agent_ptr : mAgents) {
      delete agent_ptr;
    }

    for (auto var_ptr : mVariableModerators) {
      delete var_ptr;
    }

    if (not mPreAmbleRequests.empty()) {
      LOG(fail) << "{Generator::~Generator} pre amble requests vector not empty." << endl;
      FAIL("preamble-requests-not-empty");
    }

    if (not mPostAmbleRequests.empty()) {
      LOG(fail) << "{Generator::~Generator} post amble requests vector not empty." << endl;
      FAIL("post-requests-not-empty");
    }

    if (not mSpeculativeRequests.empty()) {
      LOG(fail) << "{Generator::~Generator} speculative requests vector not empty." << endl;
      FAIL("speculative-requests-not-empty");
    }
  }

  void Generator::Setup(uint32 threadId)
  {
    mThreadId = threadId;

    mpConditionSet->Setup(this);
    mpVirtualMemoryInitializer->SetThreadId(threadId);

    auto unpredict_registers = GetVariable("Unpredictable Registers", EVariableType::String);
    mpRegisterFile->Setup(mpConditionSet, unpredict_registers);

    // setup register-file
    auto instr_agent = static_cast<GenInstructionAgent*>(mAgents[int(EGenAgentType::GenInstructionAgent)]);
    const map<string, RegisterInitPolicy* >& init_policies = mpRegisterFile->GetInitPolicies();
    for (auto initp_item : init_policies) {
      initp_item.second->Setup(this);
    }
    if (HasISS()) {
      auto readonly_regs = mpRegisterFile->GetReadOnlyRegisters();
      instr_agent->InitializeReadOnlyRegistersWithISS(readonly_regs);
      auto readonly_regfields = mpRegisterFile->GetReadOnlyRegisterFields();
      instr_agent->ResetReadOnlyRegisterFieldsWithISS(readonly_regfields);
    }
    else {
      mpRegisterFile->InitializeReadOnlyRegistersNoISS();
    }
    mpRegisterFile->SignUp(instr_agent);

    mpRegisterFile->GetConditionSet()->SignUp();

    mpRegisteredSetModifier->Setup(this);

    mpVmManager->Setup(this);
    mpGenPC->SetInstructionSpace(InstructionSpace());
    PcSpacing::Instance()->SignUp(this);
    mpDependence->Setup(this);

    mpPageRequestRegulator->Setup(this);
    mpAddressFilteringRegulator->Setup(this);
    mpBntHookManager->Setup(this);
    mpAddressTableManager->Setup(this);

    StateTransitionManagerRepository* state_trans_manager_repo = StateTransitionManagerRepository::Instance();
    state_trans_manager_repo->AddStateTransitionManager(mThreadId, new StateTransitionManager(this));
  }

  const string Generator::ToString() const
  {
    return string("Generator of type: ") + Type();
  }

  uint64 Generator::PC() const
  {
    return mpGenPC->Value();
  }

  uint64 Generator::LastPC() const
  {
    return mpGenPC->LastValue();
  }

  void Generator::AdvancePC(uint32 bytes)
  {
    mpGenPC->Advance(bytes);
  }

  void Generator::SetPC(uint64 newPC)
  {
    mpGenPC->Set(newPC);
  }

  void Generator::MapPC()
  {
    mpGenPC->MapPC(this);
    // << "{Generator::MapPC} " << mpGenPC->ToString() << endl;
  }

  /*!
    Generate a round of instructions.  Normally we will be generating one instruction per GenInstruction call, but if there are pre-amble instructions added, there might be multiple instructions generated per generation round.
   */
  void Generator::GenInstruction(GenInstructionRequest * instrReq, string& rec_id)
  {
    auto round_id = mpRequestQueue->StartRound(); // mark start of the generation round.
    mpRequestQueue->PrependRequest(instrReq);

    // loop till end of the round.
    while (!mpRequestQueue->RoundFinished(round_id)) {
      GenRequest* gen_req = mpRequestQueue->PopFront();
      ProcessGenRequest(gen_req);
    }

    mpThreadInstructionResults->GetCurrentInstructionRecordId (rec_id);

    ProcessSpeculativeRequests();
  }

  void Generator::ProcessSpeculativeRequests()
  {
    if (mSpeculativeRequests.empty())
      return;

    auto round_id = mpRequestQueue->StartRound();
    mpRequestQueue->PrependRequests(mSpeculativeRequests);

    while ( !mpRequestQueue->RoundFinished(round_id) )
    {
      GenRequest* gen_req = mpRequestQueue->PopFront();
      ProcessGenRequest(gen_req);
    }
  }

  void Generator::GenSequence(GenRequest* pGenRequest)
  {
    auto round_id = mpRequestQueue->StartRound();
    mpRequestQueue->PrependRequest(pGenRequest);

    while ( !mpRequestQueue->RoundFinished(round_id) )
    {
      GenRequest* gen_req = mpRequestQueue->PopFront();
      ProcessGenRequest(gen_req);
    }
 }

  void Generator::GenSummary()
  {
    mpThreadInstructionResults->GenSummary();
  }

  void Generator::ProcessGenRequest(GenRequest* genRequest)
  {
    if (genRequest->AddingInstruction()) {
      auto pc_value = mpGenPC->Value();
      auto vm_mapper = GetVmManager()->CurrentVmMapper();
      if (mpGenMode->CheckEscape()) {
        if (not vm_mapper->VerifyStreamingVa(pc_value, InstructionSpace(), true)) {
          LOG(notice) << "{Generator::ProcessGenRequest} instruction space not free at 0x" << hex << pc_value << " pending request: " << genRequest->ToString() << endl;
          mpRequestQueue->PrependRequest(genRequest); // add back the original request.
          mpRequestQueue->PrependRequest(new GenEscapeCollision()); // add an escape-collision request before the original request.
          return;
        }
      }
      else if (mpGenMode->IsFiller()) {
        if (not vm_mapper->VerifyStreamingVa(pc_value, DefaultInstructionSize(), true)) {
          LOG(notice) << "{Generator::ProcessGenRequest} filler instruction, address not usable at 0x"<< hex << pc_value << ", skipping request: "
                      << genRequest->ToString() << ", try to generate instruction on next pc." << endl;
          mpThreadInstructionResults->InvalidCurrentBankAddress();
          genRequest->CleanUp();
          delete genRequest;

          auto next_pc = pc_value + DefaultInstructionSize();
          mpRequestQueue->PrependRequest(new GenStateRequest(EGenStateActionType::Set, EGenStateType::PC, next_pc));
          return;
        }
      }
    }

    if (genRequest->DelayHandle()) {
      if (mpGenMode->CheckAddressShortage() && mpGenMode->IsAddressShortage()) {
        mpRequestQueue->PrependRequest(genRequest); // add back the original request.
        SolveAddressShortage();
        return;
      }
    }

    GenAgent* gen_agent = mAgents[int(genRequest->GenAgentType())];
    gen_agent->ProcessRequest(genRequest);
  }

  bool Generator::CommitInstruction(Instruction* instr, GenInstructionRequest* instrReq)
  {
    bool final_commit = false;

    if (instr->GetPrePostAmbleRequests(*this)) {
      if (not mPostAmbleRequests.empty()) {
        // add post amble requests.
        mpRequestQueue->PrependRequests(mPostAmbleRequests);
      }
      mpRequestQueue->PrependRequest(new GenCommitInstruction(instr, instrReq));
      if (not mPreAmbleRequests.empty()) {
        // add pre amble requests.
        mpRequestQueue->PrependRequests(mPreAmbleRequests);
      }
    } else {
      CommitInstructionFinal(instr, instrReq);
      final_commit = true;
    }

    return final_commit;
  }

  void Generator::CommitInstructionFinal(Instruction* instr, GenInstructionRequest* instrReq)
  {
    instr->Commit(*this);
    GenInstructionAgent * gen_instr_agent = static_cast<GenInstructionAgent*>(mAgents[int(EGenAgentType::GenInstructionAgent)]);
    if (not mpThreadInstructionResults->Commit(this, instr)) {
      if (SimulationEnabled()) {
        mpRequestQueue->PrependRequest(new GenCommitInstruction(instr, instrReq));
        gen_instr_agent->StepInstruction(instr); // step to generate address fault
      }
      else {
        LOG(notice) << "{Generator::CommitInstructionFinal} Skipped address-error instruction:" << instr->FullName() << "at 0x" << hex << GetGenPC()->Value() << endl;
        mpThreadInstructionResults->InvalidCurrentBankAddress();
        instr->CleanUp();
        delete instr;
        delete instrReq;
      }
      return;
    }

    mpDependence->Commit(instr->GiveHotResource());
    gen_instr_agent->StepInstruction(instr);
    instrReq->NotAppliedOperandRequests();
    instr->CleanUp();
    delete instrReq;
  }

  void Generator::PrependRequests(vector<GenRequest* >& reqVec)
  {
    mpRequestQueue->PrependRequests(reqVec);
  }

  void Generator::PrependRequest(GenRequest* req)
  {
    mpRequestQueue->PrependRequest(req);
  }

  void Generator::InitializeMemory(const MemoryInitRecord* memInitRecord)
  {
    mpMemoryManager->InitializeMemory(memInitRecord);
  }

  void Generator::SetStateValue(EGenStateType stateType, uint64 value)
  {
    mGenStateValues[stateType] = value;
    LOG(notice) << "{Generator::SetStateValue} setting " << EGenStateType_to_string(stateType) << " to 0x" << hex << value << endl;
  }

  bool Generator::GetStateValue(EGenStateType stateType, uint64& stateValue) const
  {
    auto state_finder = mGenStateValues.find(stateType);
    if (state_finder != mGenStateValues.end()) {
      LOG(info) << "{Generator::GetStateValue} state: '" << EGenStateType_to_string(stateType) << "=0x" << hex << state_finder->second << endl;
      stateValue = state_finder->second;
      return true;
    }
    LOG(warn) << "{Generator::GetStateValue} state: '" << EGenStateType_to_string(stateType) << " not found." << endl;
    return false;
  }

  void Generator::InitializeMemory(uint64 addr, uint32 bank, uint32 size, uint64 data, bool isInstr, bool isVirtual)
  {
    // TODO isVirtual not supported
    if (isVirtual)
    {
      auto vm_mapper = GetVmManager()->CurrentVmMapper();
      const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
      uint64 untagged_addr = addr_tagging->UntagAddress(addr, isInstr);
      uint64 PA = 0;
      uint32 mem_bank = 0;
      ETranslationResultType result = vm_mapper->TranslateVaToPa(untagged_addr, PA, mem_bank);
      if (result != ETranslationResultType::Mapped)
      {
        LOG(fail) << "{Generator::InitializeMemory} virtual address can't be translated addr=0x" << hex << addr << endl;
        FAIL("va_cant_translate_init_mem");
      }
      InitializeMemoryWithEndian(PA, mem_bank, size, data, isInstr, (isInstr) ? true : IsDataBigEndian());
    }
    else
    {
      InitializeMemoryWithEndian(addr, bank, size, data, isInstr, true /* big-endian */);
    }
  }

  void Generator::InitializeMemoryWithEndian(uint64 addr, uint32 bank, uint32 size, uint64 data, bool isInstr, bool bigEndian)
  {
    if ((size == 0) || (size > 8)) {
      LOG(fail) << "{Generator::InitializeMemoryWithEndian} unsupported size : " << dec << size << endl;
      FAIL("initialize-memory-with-unsupported-size");
    }
    if ((addr + size) < addr) {
      LOG(fail) << "{Generator::InitializeMemoryWithEndian} address with size wrap around: 0x" << hex << addr << " + 0x" << size << endl;
      FAIL("initialize-memory-wrap-around");
    }
    EMemDataType init_type = isInstr ? EMemDataType::Instruction : EMemDataType::Data;
    MemoryInitRecord* mem_init_data = mpRecordArchive->GetMemoryInitRecord(mThreadId, size, size, init_type);
    mem_init_data->SetData(addr, bank, data, size, bigEndian);
    InitializeMemory(mem_init_data);
  }

  void Generator::ReserveMemory(const string& name, const string& range, uint32 bank, bool isVirtual)
  {
    MemoryReservation * mem_reserv = new MemoryReservation(name);
    if (isVirtual) {
      mem_reserv->AddVirtualConstraint(range, GetVmManager()->CurrentVmMapper());
    }
    else {
      mem_reserv->AddConstraint(range, EMemBankType(bank));
    }
    ReserveMemory(mem_reserv);
  }

  void Generator::UnreserveMemory(const string& name, const string& range, uint32 bank, bool isVirtual)
  {
    MemoryReservation * mem_reserv = new MemoryReservation(name);
    if (isVirtual) {
      mem_reserv->AddVirtualConstraint(range, GetVmManager()->CurrentVmMapper());
    }
    else {
      mem_reserv->AddConstraint(range, EMemBankType(bank));
    }
    UnreserveMemory(mem_reserv);
  }

  void Generator::ReserveMemory(const string& name, uint64 start, uint64 size, uint32 bank, bool isVirtual)
  {
    MemoryReservation * mem_reserv = new MemoryReservation(name);
    if (isVirtual) {
      mem_reserv->AddVirtualRange(start, size, GetVmManager()->CurrentVmMapper());
    }
    else {
      mem_reserv->AddRange(start, size, EMemBankType(bank));
    }
    ReserveMemory(mem_reserv);
  }

  void Generator::UnreserveMemory(const string& name, uint64 start, uint64 size, uint32 bank, bool isVirtual)
  {
    MemoryReservation * mem_reserv = new MemoryReservation(name);
    if (isVirtual) {
      mem_reserv->AddVirtualRange(start, size, GetVmManager()->CurrentVmMapper());
    }
    else {
      mem_reserv->AddRange(start, size, EMemBankType(bank));
    }
    UnreserveMemory(mem_reserv);
  }

  void Generator::ReserveMemory(MemoryReservation* pMemReserv)
  {
    mpMemoryManager->ReserveMemory(pMemReserv);
  }

  void Generator::UnreserveMemory(MemoryReservation* pMemReserv)
  {
    mpMemoryManager->UnreserveMemory(pMemReserv);
  }

  void Generator::UpdateVm()
  {
    GetVmManager()->Update();
    mpGenPC->Update(InstructionSpace());
  }

  bool Generator::GetRandomRegisters(cuint32 number, const ERegisterType regType, const string& rExcludes, vector<uint64>& rRegIndices) const
  {
    return mpRegisterFile->GetRandomRegisters(number, regType, rExcludes, rRegIndices);
  }

  bool Generator::GetRandomRegistersForAccess(cuint32 number, const ERegisterType regType, const ERegAttrType access, const string& rExcludes, vector<uint64>& rRegIndices) const
  {
    return mpRegisterFile->GetRandomRegistersForAccess(number, regType, access, rExcludes, rRegIndices);
  }

  uint32 Generator::GetRandomRegister(const ERegisterType regType, const string& rExcludes, bool* status) const
  {
    vector<uint64> reg_indices;
    if (not GetRandomRegisters(1, regType, rExcludes, reg_indices)) {
      if (nullptr != status) {
        *status = false;
      } else {
        LOG(fail) << "{Generator::GetRandomRegister} failed to obtain one random " << ERegisterType_to_string(regType) << " register." << endl;
        FAIL("failed-to-get-one-register");
      }
      return 0;
    }
    else {
      if (nullptr != status) {
        *status = true;
      }
      // << "gotten " << reg_indices.size() << " registers, front is " << dec << reg_indices.front() << endl;
      return reg_indices.front();
    }
  }

  uint32 Generator::GetRandomRegisterForAccess(const ERegisterType regType, const ERegAttrType access, const string& rExcludes, bool *status) const
  {
    vector<uint64> reg_indices;
    if (not GetRandomRegistersForAccess(1, regType, access, rExcludes, reg_indices)) {
      if (nullptr != status) {
        *status = false;
      } else {
        LOG(fail) << "{Generator::GetRandomRegisterForAccess} failed to obtain one random " << ERegisterType_to_string(regType) << " register." << endl;
        FAIL("failed-to-get-one-register");
      }
      return 0;
    }
    else {
      if (nullptr != status) {
        *status = true;
      }
      // << "gotten " << reg_indices.size() << " registers, front is " << dec << reg_indices.front() << endl;
      return reg_indices.front();
    }
  }

  bool Generator::IsRegisterReserved(const string& name, const ERegAttrType access, const ERegReserveType reserveType) const
  {
    Register* pRegister = mpRegisterFile->RegisterLookup( name );
    return mpRegisterFile->GetRegisterReserver()->IsRegisterReserved(pRegister, access, reserveType);
  }

  void Generator::ReserveRegisterByIndex(uint32 size, uint32 index, const ERegisterType reg_type, const ERegAttrType access, const ERegReserveType reserveType)
  {
    Register* pRegister =  mpRegisterFile->RegisterLookupByIndex( index, reg_type, size);
    mpRegisterFile->GetRegisterReserver()->ReserveRegister( pRegister, access);
  }

  void Generator::ReserveRegister(const string& name, const ERegAttrType access, const ERegReserveType reserveType)
  {
    Register* pRegister = mpRegisterFile->RegisterLookup( name );
    mpRegisterFile->GetRegisterReserver()->ReserveRegister(pRegister, access, reserveType);
  }

  void Generator::UnreserveRegisterByIndex(uint32 size, uint32 index, const ERegisterType reg_type, const ERegAttrType access, const ERegReserveType reserveType)
  {
    Register* pRegister = mpRegisterFile->RegisterLookupByIndex( index, reg_type, size);
    mpRegisterFile->GetRegisterReserver()->UnreserveRegister( pRegister, access, reserveType);
  }

  void Generator::UnreserveRegister(const string& name, const ERegAttrType access, const ERegReserveType reserveType)
  {
    Register* pRegister = mpRegisterFile->RegisterLookup(name );
    mpRegisterFile->GetRegisterReserver()->UnreserveRegister(pRegister, access, reserveType);
  }

  bool Generator::ReadRegister(const string& name, const string& field, uint64& reg_value) const
  {
    //field is optional, could be empty
    if (field.compare("") == 0)
      reg_value = mpRegisterFile->RegisterLookup(name)->Value();
    else
      reg_value = mpRegisterFile->RegisterLookup(name)->RegisterFieldLookup(field)->FieldValue();
    return true; //RegisterLookup did name check and RegisterFieldLookup did field check already
  }

  void Generator::WriteRegisterWithUpdate(const string& name, const string& field, uint64 value)
  {
    // Update register if ISS is not integrated, otherwise verify if the register has the same value
    if (HasISS()==true) {
      if (not SimulationEnabled()) return; // simulation not enabled, cannot check.

      auto reg_ptr = mpRegisterFile->RegisterLookup(name);
      if (field.compare("")==0) {
        if (reg_ptr->Value() != value) {
          LOG(fail) << "{Generator::WriteRegisterWithUpdate} ISS integrated. Current register: " << name << " value: 0x" << hex << reg_ptr->Value() << " doesn't match given value: 0x" << value << endl;
          FAIL("iss_not_update_new_reg_value");
        }
      }
      else {
        auto reg_field_ptr = reg_ptr->RegisterFieldLookup(field);
        uint64 field_value = reg_field_ptr->FieldValue();
        if (field_value != value) {
          auto ignore_reg_fields = GetVariable("Register Fields Ignored update",  EVariableType::String);
          if (reg_field_ptr->IgnoreUpdate(ignore_reg_fields)) {
            LOG(warn) << "{Generator::WriteRegisterWithUpdate} ISS integrated. Current register: " << name << " field:" << field << " value: 0x" << hex << field_value << " doesn't match given value:0x" << value << endl;
          }
          else if (!reg_field_ptr->IgnoreUpdate()) {
            LOG(fail) << "{Generator::WriteRegisterWithUpdate} ISS integrated. Current register: " << name << " field:" << field << " value: 0x" << hex << field_value << " doesn't match given value:0x" << value << endl;
            FAIL("iss_not_update_new_reg_value");
          }
        }
      }
    }
    else {
      if (DelayInit() ) return; // exception handler generation etc, shouldn't write register in this mode.

      WriteRegister(name, field, value);
    }

    UpdateSystemWithRegister(name, field, value);
  }

  void Generator::WriteRegister(const string& name, const string& field, uint64 value)
  {
    //RegisterLookup did name check and RegisterFieldLookup did field check already
    //field is optional, could be empty
    if (field.compare("") == 0) {
      mpRegisterFile->RegisterLookup(name)->SetValue(value);
    } else {
      mpRegisterFile->RegisterLookup(name)->RegisterFieldLookup(field)->SetFieldValue(value);
    }
  }

  /*!
    This can be called from the front end, so need to check if the register/register-field is already initialized.
  */
  uint64 Generator::InitializeRegister(const string& name, const string& field, uint64 value) const
  {
    ChoicesModerator* choices_mod = GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);
    uint64 return_value = value;

    //RegisterLookup did name check and RegisterFieldLookup did field check already
    //field is optional, could be empty
    auto reg_ptr = mpRegisterFile->RegisterLookup(name);
    if (field.size() == 0) {
      if (reg_ptr->IsInitialized()) {
        return_value = reg_ptr->Value();
        LOG(warn) << "{Generator::InitializeRegister} register: " << name << " already initialized to 0x" << hex << return_value << " new init value ignored: 0x" << value << endl;
      } else {
        mpRegisterFile->InitializeRegister(name, value, choices_mod);
      }
    } else {
      auto reg_field = reg_ptr->RegisterFieldLookup(field);
      if (reg_field->IsInitialized()) {
        return_value = reg_field->FieldValue();
        LOG(warn) << "{Generator::InitializeRegister} register field: " << name << "." << field << " already initialized to 0x" << hex << return_value << " new init value ignored: 0x" << value << endl;
      } else {
        mpRegisterFile->InitializeRegisterField(reg_ptr, field, value, choices_mod);
      }
    }
    return return_value;
  }

  void Generator::InitializeRegister(const string& name, vector<uint64> values) const
  {
    ChoicesModerator* choices_mod = GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);

    //RegisterLookup did name check and RegisterFieldLookup did field check already
    //field is optional, could be empty

    auto reg = mpRegisterFile->RegisterLookup(name);
    if (string("LargeRegister") == reg->Type())
    {
      mpRegisterFile->InitializeRegister(name, values, choices_mod);
    }
    else
    {
      LOG(fail) << "attempting to initialize 64 bit register with vector of 64 bit values for register=" << name << endl;
      FAIL("init_non_large_reg_with_vector_of_values");
    }
  }

  void Generator::InitializeRegisterFields(const string& registerName, const map<string, uint64>& field_value_map) const
  {
    ChoicesModerator* choices_mod = GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);

    // TODO move this into Register module.
    Register* current_reg = mpRegisterFile->RegisterLookup(registerName);
    current_reg->Block();
    for (auto item : field_value_map) {
      const auto & fld_name = item.first;
      auto fld_value = item.second;
      RegisterField* reg_field = current_reg->RegisterFieldLookup(fld_name);
      if (reg_field->IsInitialized()) {
        LOG(warn) << "{Generator::InitializeRegisterFields} register field: " << registerName << "." << fld_name << " has been initialized to 0x" << reg_field->InitialFieldValue() << " new initial value skipped: 0x" << fld_value << endl;
      }
      else {
        mpRegisterFile->InitializeRegisterField(current_reg, item.first, item.second, choices_mod);
      }
    }
    current_reg->Unblock();
  }

  void Generator::RandomInitializeRegister(Register* pReg) const
  {
    if (pReg->IsInitialized()) return; // if the whole register is initialized, return;

    ChoicesModerator* pChoicesModerator = GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);
    mpRegisterFile->InitializeRegisterRandomly(pReg, pChoicesModerator); // only initiate for uninitialized fields
  }

  void Generator::RandomInitializeRegister(const string& name, const string& field) const
  {
    ChoicesModerator* pChoicesModerator = GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);
    Register* pRegister = mpRegisterFile->RegisterLookup(name);

    if (field.length() > 0)
    {
      mpRegisterFile->InitializeRegisterFieldRandomly(pRegister, field, pChoicesModerator);
    }
    else
    {
      if (pRegister->IsInitialized()) return; //performance advantage if IsInitialized() caches results
      mpRegisterFile->InitializeRegisterRandomly(pRegister, pChoicesModerator); // only initiate for uninitialized fields
    }
  }

  void Generator::RandomInitializeRegisterFields(const string& registerName, const vector<string>& fieldList) const
  {
    Register* pRegister = mpRegisterFile->RegisterLookup(registerName);
    ChoicesModerator* pChoicesModerator = GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);

    if (pRegister->IsInitialized()) return;

    for (auto field : fieldList)
    {
      mpRegisterFile->InitializeRegisterFieldRandomly(pRegister, field, pChoicesModerator);
    }
  }

  uint64 Generator::GetRegisterFieldMask(const string& regName, const vector<string>& fieldList)
  {
    return mpRegisterFile->GetRegisterFieldMask(regName, fieldList);
  }

  void Generator::GenVmRequest(GenRequest* pVmReq)
  {
    GenAgent* gen_agent = mAgents[int(pVmReq->GenAgentType())];
    gen_agent->ProcessRequestWithResult(pVmReq);
  }

  void Generator::StateRequest(GenRequest* pStateReq)
  {
    GenAgent* gen_agent = mAgents[int(pStateReq->GenAgentType())];
    gen_agent->ProcessRequestWithResult(pStateReq);
  }

  void Generator::ExceptionRequest(GenRequest* pExceptReq)
  {
    GenAgent* gen_agent = mAgents[int(pExceptReq->GenAgentType())];
    gen_agent->ProcessRequestWithResult(pExceptReq);
  }

  void Generator::Query(const GenQuery& rGenQuery) const
  {
    const GenAgent* gen_agent = mAgents[int(rGenQuery.GenAgentType())];
    gen_agent->ProcessQuery(&rGenQuery);
  }

  void Generator::ExecuteHandler()
  {
    auto instr_agent = static_cast<GenInstructionAgent*>(mAgents[int(EGenAgentType::GenInstructionAgent)]);
    instr_agent->ExecuteHandler();
  }

  void Generator::ExceptionReturn()
  {
    auto instr_agent = static_cast<GenInstructionAgent*>(mAgents[int(EGenAgentType::GenInstructionAgent)]);
    instr_agent->ExceptionReturn();
  }

  void Generator::SleepOnLowPower()
  {
    if (!InLowPower()) {
      auto state_req = new GenStateRequest(EGenStateActionType::Push, EGenStateType::GenMode, EGenModeTypeBaseType(EGenModeType::LowPower));
      unique_ptr<GenRequest> storage_ptr(state_req); // responsible for releasing the storage when going out of scope.
      StateRequest(state_req);
    }
    LOG(info) << "Generator sleep on low power" << endl;
  }

  void Generator::ReExecute(uint64 addr)
  {
    auto instr_agent = static_cast<GenInstructionAgent*>(mAgents[int(EGenAgentType::GenInstructionAgent)]);
    instr_agent->ReExecute(addr);
  }

  bool Generator::VerifyVirtualAddress(uint64 va, uint64 size, bool isInstr) const
  {
    auto vm_mapper = GetVmManager()->CurrentVmMapper();
    auto page_req = vm_mapper->GenPageRequestRegulated(isInstr, EMemAccessType::ReadWrite); // TODO need to add ways to pass in Read/Write attribute.
    return vm_mapper->VerifyVirtualAddress(va, size, isInstr, page_req);
  }

  void Generator::GetTranslationRange(uint64 VA, TranslationRange& rTransRange) const
  {
    auto vm_mapper = mpVmManager->CurrentVmMapper();
    if (not vm_mapper->GetTranslationRange(VA, rTransRange)) {
      LOG(fail) << "{Generator::GetTranslationRange} expecting translation range for 0x" << hex << VA << " to be valid." << endl;
      FAIL("cannot-get-valid-transliation-range");
    }
  }

  uint64 Generator::GetRegisterFieldValue(const string& regName, const string& fieldName)
  {
    Register* reg_ptr = mpRegisterFile->RegisterLookup(regName);
    //reg_ptr->BlockNotification();

    ChoicesModerator* choices_mod = GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);

    mpRegisterFile->InitializeRegisterFieldRandomly(reg_ptr, fieldName, choices_mod);

    return reg_ptr->RegisterFieldLookup(fieldName)->FieldValue();

    //reg_ptr->ClearNotification();
  }

  uint64 Generator::RegisterFieldReloadValue(const string& regName, const string& fieldName) const
  {
    Register* reg_ptr = mpRegisterFile->RegisterLookup(regName);

    ChoicesModerator* choices_mod = GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);

    return reg_ptr->FieldReloadValue (fieldName, choices_mod);
  }

  void Generator::SetupPageTableRegions()
  {
    mpMemoryManager->SetupPageTableRegions();
  }

  GenPageRequest* Generator::GenPageRequestInstance(bool isInstr, EMemAccessType memAccessType) const
  {
    auto ret_req = new GenPageRequest();

    ret_req->SetGenBoolAttribute(EPageGenBoolAttrType::InstrAddr, isInstr);
    ret_req->SetMemoryAccessType(memAccessType);

    return ret_req;
  }

  GenPageRequest* Generator::GenPageRequestRegulated(const VmMapper *pVmMapper, bool isInstr, EMemAccessType memAccessType) const
  {
    auto ret_req = GenPageRequestInstance(isInstr, memAccessType);
    if (isInstr) {
      GetPageRequestRegulator()->RegulateBranchPageRequest(pVmMapper, nullptr, ret_req);
    }
    else {
      GetPageRequestRegulator()->RegulateLoadStorePageRequest(pVmMapper, nullptr, ret_req);
    }

    return ret_req;
  }

  void Generator::AddLoadRegisterAmbleRequests(const string& regName, uint64 loadValue)
  {
    ERegAttrType reserv_acc = ERegAttrType::Write;
    Register* pRegister = mpRegisterFile->RegisterLookup(regName );
    string inter_reg_name = "";
    switch (pRegister->RegisterType()) {
    case ERegisterType::SP:
    case ERegisterType::SysReg:
    case ERegisterType::SIMDR:
    case ERegisterType::SIMDVR:
      inter_reg_name =  GetGPRName(GetRandomRegisterForAccess(ERegisterType::GPR, ERegAttrType::Write, GetGPRExcludes()));
      ReserveRegister(inter_reg_name, reserv_acc); // reserve the GPR to avoid being corrupted by others.
      mPostAmbleRequests.push_back(new GenRegisterReservation(inter_reg_name, false, reserv_acc)); // restore the register reservation.
      break;
    default: break; // do nothing
    }

    bool reg_reserved = mpRegisterFile->GetRegisterReserver()->IsRegisterReserved(pRegister, reserv_acc);
    if (not reg_reserved) {
      ReserveRegister(regName, reserv_acc);
      LOG(info) << "{Generator::AddLoadRegisterAmbleRequests} register " << regName << " should be reserved now: " << mpRegisterFile->GetRegisterReserver()->IsRegisterReserved(pRegister, reserv_acc) << endl;
      mPostAmbleRequests.push_back(new GenRegisterReservation(regName, false, reserv_acc));
    }
    mPreAmbleRequests.push_back(new GenLoadRegister(regName, loadValue, inter_reg_name));
  }

  void Generator::AddLoadSysRegistersAmbleRequests(const map<std::string, uint64>& rRegisters)
  {
    if (rRegisters.empty()) {
      LOG(info) << "{Generator::AddLoadSysRegisterAmbleRequests} map is empty" << endl;
      return;
    }

    uint32 random_index = GetRandomRegister(ERegisterType::GPR, GetGPRExcludes()); // getting a random GPR, excluding register index 31.
    auto inter_reg_name = GetGPRName(random_index);

    ERegAttrType reserv_acc = ERegAttrType::Write;
    ReserveRegister(inter_reg_name, reserv_acc); // reserve the GPR to avoid being corrupted by others.
    mPostAmbleRequests.push_back(new GenRegisterReservation(inter_reg_name, false, reserv_acc)); // restore the register reservation.

    for (const auto item : rRegisters) {
      const string& name = item.first;
      uint64 value = item.second;
      LOG(info) << "{Generator::AddLoadSysRegisterAmbleRequests} register " << name << " value: " << hex << value << endl;

      Register* pRegister = mpRegisterFile->RegisterLookup(name);
      bool reg_reserved = mpRegisterFile->GetRegisterReserver()->IsRegisterReserved(pRegister, reserv_acc);
      if (not reg_reserved) {
        ReserveRegister(name, reserv_acc);
      }

      mPreAmbleRequests.push_back(new GenLoadRegister(name, value, inter_reg_name));
      if (not reg_reserved) {
        mPreAmbleRequests.push_back(new GenRegisterReservation(name, false, reserv_acc));
      }
    }
  }
  void Generator::AddLoadLargeRegisterAmbleRequests(const string& regName,std::vector<uint64>& loadValues)
  {
    ERegAttrType reserv_acc = ERegAttrType::Write;
    Register* pRegister = mpRegisterFile->RegisterLookup(regName );
    string inter_reg_name = "";
    bool reg_reserved = mpRegisterFile->GetRegisterReserver()->IsRegisterReserved(pRegister, reserv_acc);
    if (not reg_reserved) {
      ReserveRegister(regName, reserv_acc);
      LOG(info) << "{Generator::AddLoadRegisterAmbleRequests} register " << regName << " should be reserved now: " << mpRegisterFile->GetRegisterReserver()->IsRegisterReserved(pRegister, reserv_acc) << endl;
      mPostAmbleRequests.push_back(new GenRegisterReservation(regName, false, reserv_acc));
    }
    if (ERegisterType::VECREG == pRegister->RegisterType()) {
      mPreAmbleRequests.push_back(new GenLoadLargeRegister(regName, loadValues, inter_reg_name));
    } else {
      LOG(fail) << "Not large register: " <<regName << endl;
      FAIL("failed-find-large-register");
    }
  }
  void Generator::AddPreambleRequest(GenRequest* request)
  {
    mPreAmbleRequests.push_back(request);
  }

  void Generator::AddSpeculativeRequest(GenRequest* request)
  {
    mSpeculativeRequests.push_back(request);
  }

  bool Generator::SimulationEnabled() const
  {
    return mpGenMode->SimulationEnabled();
  }

  bool Generator::HasISS() const
  {
    return mpGenMode->HasISS();
  }

  bool Generator::InException() const
  {
    return mpGenMode->InException();
  }

  bool Generator::NoSkip() const
  {
    return mpGenMode->NoSkip();
  }

  bool Generator::InLowPower() const
  {
    return mpGenMode->LowPower();
  }

  bool Generator::InFiller() const
  {
    return mpGenMode->IsFiller();
  }

  bool Generator::InSpeculative() const
  {
    return mpGenMode->IsSpeculative();
  }

  bool Generator::InLoop() const
  {
    return mpGenMode->InLoop();
  }

  bool Generator::DelayInit() const
  {
    return mpGenMode->DelayInit();
  }

  bool Generator::ReExecution() const
  {
    return mpGenMode->ReExecution();
  }

  bool Generator::NoJump() const
  {
    return mpGenMode->NoJump();
  }

  bool Generator::AddressProtection() const
  {
    return mpGenMode->AddressProtection();
  }

  bool Generator::RecordingState() const
  {
    return mpGenMode->RecordingState();
  }

  bool Generator::RestoreStateLoop() const
  {
    return mpGenMode->RestoreStateLoop();
  }

  void Generator::ModifyVariable(const string& name, const string& value, EVariableType var_type)
  {
    auto var_mod = GetVariableModerator(var_type);
    auto var_set = var_mod->GetVariableSet();
    var_set->ModifyVariable(name, value);
  }

  const string& Generator::GetVariable(const string& name, EVariableType var_type) const
  {
    auto var_mod = GetVariableModerator(var_type);
    auto var_set = var_mod->GetVariableSet();
    auto var = var_set->FindVariable(name);
    if (var == nullptr) {
      LOG(fail) << "Failed to find variable: \"" << name << "\"" << endl;
      FAIL("failed-find-variable");
    }
    return var->GetValue();
  }

  const InstructionStructure* Generator::GetInstructionStructure(const string& instrName) const
  {
    return GetInstructionSet()->LookUpById(instrName);
  }

  ChoicesModerator* Generator::GetChoicesModerator(EChoicesType choiceType) const
  {
    return mpChoicesModerators->GetChoicesModerator(choiceType);
  }

  void Generator::OutputImage(ImageIO* imagePrinter) const
  {
    auto cfg_handle = Config::Instance();
    uint64 initial_seed = 0;
    string output_name_img = get_file_stem(cfg_handle->TestTemplate());
    if (cfg_handle->OutputWithSeed(initial_seed)) {
      stringstream seed_stream;
      seed_stream << "0x"<< hex << initial_seed;
      output_name_img += "_" + seed_stream.str();
    }
    output_name_img += ".Registers.img";
    uint64 initial_pc = 0;
    uint64 boot_pc = 0;
    GetStateValue(EGenStateType::InitialPC, initial_pc);
    GetStateValue(EGenStateType::BootPC, boot_pc);
    map<string, uint64> thread_info;
    thread_info["ThreadID"] = mThreadId;
    thread_info["BootPC"] = boot_pc;
    thread_info["InitialPC"] = initial_pc;
    imagePrinter->PrintRegistersImage(output_name_img, thread_info, mpRegisterFile);
  }

  void Generator::SolveAddressShortage()
  {
    // clear AddressShortage flag to avoid infinite loop.
    mpGenMode->DisableGenMode(EGenModeTypeBaseType(EGenModeType::AddressShortage));

    bool fault = false;
    uint32 bank = 0;
    mpGenPC->GetPA(this, bank, fault);
    if (fault) {
      LOG(notice) << "{Generator::SolveAddressShortage} Current address has fault, skipped." << endl;
      return;
    }

    auto enable_str = GetVariable("Enable register reloading", EVariableType::Value);
    if (enable_str == "0") {
      return;
    }
    LOG(notice) << "{Generator::SolveAddressShortage} try to add some available addresses to solve address shortage" << endl;
    mpRequestQueue->PrependRequest(new GenBatchReloadRegisters());
  }

  void Generator::AdvancePC()
  {
    LOG(fail) << "{Generator::AdvancePC} Unimplemented function." << endl;
    FAIL("unimplemented-function");
  }

  void Generator::SetDependenceInstance(ResourceDependence* pDependence)
  {
    if (mpDependence)
      delete mpDependence;

    mpDependence = pDependence;
  }

}
