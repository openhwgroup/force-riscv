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
#include <Scheduler.h>
#include <PyInterface.h>
#include <Config.h>
#include <Generator.h>
#include <ChoicesModerator.h>
#include <Architectures.h>
#include <MemoryManager.h>
#include <Dump.h>
#include <RegisteredSetModifier.h>
#include <UtilityAlgorithms.h>
#include <FrontEndCall.h>
#include <ImageIO.h>
#include <SchedulingStrategy.h>
#include <ThreadGroup.h>
#include <Constraint.h>
#include <algorithm>
#include <Log.h>
#include <SemaphoreManager.h>
#include <SynchronizeBarrier.h>

using namespace std;

/*!
  \file Scheduler.cc
  \brief Code for back-end scheduler class
*/

namespace Force {

  Scheduler* Scheduler::mspScheduler = nullptr;

  void Scheduler::Initialize()
  {
    if (mspScheduler == nullptr) {
      mspScheduler = new Scheduler();
    }
  }

  void Scheduler::Destroy()
  {
    delete mspScheduler;
    mspScheduler = nullptr;
  }

  Scheduler::Scheduler()
    : mNumChips(0), mNumCores(0), mNumThreads(0), mChipsLimit(0), mCoresLimit(0), mThreadsLimit(0), mpPyInterface(nullptr), mpSchedulingStrategy(nullptr), mpGroupModerator(nullptr), mpSemaManager(nullptr), mpSyncBarrierManager(nullptr), mGenerators()
  {
    mpPyInterface = new PyInterface(this);
    Config * config_ptr = Config::Instance();
    mNumChips = config_ptr->NumChips();
    mNumCores = config_ptr->NumCores();
    mNumThreads = config_ptr->NumThreads();
    mChipsLimit = config_ptr->LimitValue(ELimitType::ChipsLimit);
    mCoresLimit = config_ptr->LimitValue(ELimitType::CoresLimit);
    mThreadsLimit = config_ptr->LimitValue(ELimitType::ThreadsLimit);

    mpSchedulingStrategy = new ShuffledRoundRobinSchedulingStrategy();
    mpGroupModerator = new ThreadGroupModerator(mNumChips, mNumCores, mNumThreads);
    mpSemaManager = new SemaphoreManager();
    mpSyncBarrierManager = new SynchronizeBarrierManager();

    Dump* dump_ptr = Dump::Instance();
    dump_ptr->SetScheduler(this);

    FrontEndCall* front_ptr = FrontEndCall::Instance();
    front_ptr->SetInterface(mpPyInterface);
  }

  Scheduler::~Scheduler()
  {
    delete mpSchedulingStrategy;
    delete mpGroupModerator;
    delete mpSemaManager;
    delete mpSyncBarrierManager;
    delete mpPyInterface;

    for (auto & gen_iter : mGenerators) {
      delete gen_iter.second;
    }
  }

  void Scheduler::Run()
  {
    mpPyInterface->RunTest();
    OutputTest();
  }

  void Scheduler::OutputTest()
  {
    Config * config_ptr = Config::Instance();
    uint64 reset_pc = config_ptr->GetGlobalStateValue(EGlobalStateType::ResetPC);
    uint32 machine_type = Config::Instance()->GetGlobalStateValue(EGlobalStateType::ElfMachine);

    MemoryManager::Instance()->OutputTest(mGenerators, reset_pc, machine_type);
    if (config_ptr->OutputImage()) {
      MemoryManager::Instance()->OutputImage();
      ImageIO image_printer;
      for (auto it = mGenerators.begin(); it != mGenerators.end(); ++it) {
        it->second->OutputImage(&image_printer);
      }
    }
  }

  uint32 Scheduler::CreateGeneratorThread(uint32 iThread, uint32 iCore, uint32 iChip)
  {
    if (iThread >= mThreadsLimit) {
      LOG(fail) << "Thread index " << dec << iThread << " >= " << mThreadsLimit << endl;
      FAIL("thread-index-out-of-range");
    }
    if (iCore >= mCoresLimit) {
      LOG(fail) << "Core index " << dec << iCore << " >= " << mCoresLimit << endl;
      FAIL("core-index-out-of-range");
    }
    if (iChip >= mChipsLimit) {
      LOG(fail) << "Chip index " << dec << iChip << " >= " << mChipsLimit << endl;
      FAIL("chip-index-out-of-range");
    }

    uint32 thread_id = ThreadId(iThread, iCore, iChip);
    if (mGenerators.find(thread_id) != mGenerators.end()) {
      LOG(fail) << "Generator with ID 0x" << hex << thread_id << " already exist!." << endl;
      FAIL("duplicate-generator-ID");
    }

    Generator* new_gen = Architectures::Instance()->DefaultArchInfo()->CreateGenerator(thread_id);
    mGenerators[thread_id] = new_gen;

    return thread_id;
  }

  uint32 Scheduler::ThreadId(uint32 iThread, uint32 iCore, uint32 iChip)
  {
    return (iChip * mNumCores * mNumThreads + iCore * mNumThreads + iThread);
  }

  void Scheduler::GenInstruction(uint32 threadId, GenInstructionRequest * instrReq, std::string& rec_id)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->GenInstruction(instrReq, rec_id);
  }

  void Scheduler::InitializeMemory(uint32 threadId, uint64 addr, uint32 bank, uint32 size, uint64 data, bool isInstr, bool isVirtual)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->InitializeMemory(addr, bank, size, data, isInstr, isVirtual);
  }

  uint32 Scheduler::AddChoicesModification(uint32 threadId, EChoicesType choicesType, const std::string& treeName, const std::map<std::string, uint32>& modifications)
  {
    auto gen_instance = LookUpGenerator(threadId);
    auto choices_moderators = gen_instance->GetChoicesModerators();
    return choices_moderators->AddChoicesModification(choicesType, treeName, modifications);
  }

  uint32 Scheduler::DoChoicesModification(uint32 threadId, EChoicesType choicesType, const std::string& treeName, const std::map<std::string, uint32>& modifications)
    {
      auto gen_instance = LookUpGenerator(threadId);
      auto choices_moderator = gen_instance->GetChoicesModerator(choicesType);
      return choices_moderator->DoChoicesModification(treeName, modifications);
    }

  void Scheduler::CommitModificationSet(uint32 threadId,  EChoicesType choicesType, uint32 setId)
  {
    auto gen_instance = LookUpGenerator(threadId);
    auto choices_moderators = gen_instance->GetChoicesModerators();
    choices_moderators->CommitModificationSet(choicesType, setId);
  }

  void Scheduler::RevertModificationSet(uint32 threadId,  EChoicesType choicesType, uint32 setId)
  {
    auto gen_instance = LookUpGenerator(threadId);
    auto choices_moderators = gen_instance->GetChoicesModerators();
    choices_moderators->RevertModificationSet(choicesType, setId);
  }

  void Scheduler::AddMemoryRange(uint32 bank, uint64 start, uint64 end)
  {
    MemoryManager::Instance()->AddMemoryRange(bank, start, end);
  }

  void Scheduler::SubMemoryRange(uint32 bank, uint64 start, uint64 end)
  {
    MemoryManager::Instance()->SubMemoryRange(bank, start, end);
  }

  void Scheduler::AddArchitectureMemoryAttributes(cuint32 threadId, cuint32 bank, cuint64 start, cuint64 end, const vector<EMemoryAttributeType>& rMemAttributes)
  {
    MemoryManager::Instance()->AddArchitectureMemoryAttributes(threadId, bank, start, end, rMemAttributes);
  }

  void Scheduler::AddImplementationMemoryAttributes(cuint32 threadId, cuint32 bank, cuint64 start, cuint64 end, const std::vector<std::string>& rMemAttributes)
  {
    MemoryManager::Instance()->AddImplementationMemoryAttributes(threadId, bank, start, end, rMemAttributes);
  }

  void Scheduler::ConfigureMemoryBanks()
  {
    MemoryManager::Instance()->ConfigureMemoryBanks();
  }

  void Scheduler::GenVmRequest(uint32 threadId, GenRequest* pVmReq)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->GenVmRequest(pVmReq);
  }

  void Scheduler::StateRequest(uint32 threadId, GenRequest* pStateReq)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->StateRequest(pStateReq);
  }

  void Scheduler::ExceptionRequest(uint32 threadId, GenRequest* pExceptReq)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->ExceptionRequest(pExceptReq);
  }

  Generator* Scheduler::LookUpGenerator(uint32 threadId)
  {
    auto gen_finder = mGenerators.find(threadId);
    if (gen_finder == mGenerators.end()) {
      LOG(fail) << "Generator with ID 0x" << hex << threadId << " not found." << endl;
      FAIL("generator-not-found");
    }

    return gen_finder->second;
  }

  const Generator* Scheduler::LookUpGenerator(uint32 threadId) const
  {
    auto gen_finder = mGenerators.find(threadId);
    if (gen_finder == mGenerators.end()) {
      LOG(fail) << "Generator with ID 0x" << hex << threadId << " not found." << endl;
      FAIL("generator-not-found");
    }

    return gen_finder->second;
  }

  // Random module API
  void Scheduler::Sample(uint64 totals, uint64 samples, std::vector<uint64>& sampleList) const
  {
    if (totals == 0)
      return;
    else if (samples > totals)
      return;

    RandomSampler sampler(0, totals-1);
    for (auto i=0; i<(int)samples; ++i)
    {
      sampleList.push_back(sampler.Sample());
    }
  }

  // Register module API
  bool Scheduler::GetRandomRegisters(cuint32 threadId, cuint32 number, const std::string& rRegType, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const
  {
    auto reg_type = string_to_ERegisterType(rRegType);
    auto gen_instance = LookUpGenerator(threadId);
    return gen_instance->GetRandomRegisters(number, reg_type, rExcludes, rRegIndices);
  }

  bool Scheduler::GetRandomRegistersForAccess(cuint32 threadId, cuint32 number, const std::string& rRegType, const std::string& rAccess, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const
  {
    auto reg_type = string_to_ERegisterType(rRegType);
    auto access = string_to_ERegAttrType(rAccess);
    auto gen_instance = LookUpGenerator(threadId);
    return gen_instance->GetRandomRegistersForAccess(number, reg_type, access, rExcludes, rRegIndices);
  }

  bool Scheduler::IsRegisterReserved(uint32 threadId, const std::string& name, const std::string& access, const std::string& type) const
  {
    auto access_type = string_to_ERegAttrType(access);
    auto reserve_type = string_to_ERegReserveType(type);
    auto gen_instance = LookUpGenerator(threadId);
    return gen_instance->IsRegisterReserved(name, access_type, reserve_type);
  }

  void Scheduler::ReserveRegisterByIndex(uint32 threadId, uint32 size, uint32 index, const std::string& regType, const std::string& access)
  {
    auto reg_type = string_to_ERegisterType(regType);
    auto access_type = string_to_ERegAttrType(access);
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->ReserveRegisterByIndex(size, index, reg_type, access_type);
  }

  void Scheduler::ReserveRegister(uint32 threadId, const std::string& name, const std::string& access)
  {
    auto access_type = string_to_ERegAttrType(access);
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->ReserveRegister(name, access_type);
  }

  void Scheduler::UnreserveRegisterByIndex(uint32 threadId, uint32 size, uint32 index, const std::string& regType, const std::string& access)
  {
    auto reg_type = string_to_ERegisterType(regType);
    auto access_type = string_to_ERegAttrType(access);
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->UnreserveRegisterByIndex(size, index, reg_type, access_type);
  }

  void Scheduler::UnreserveRegister(uint32 threadId, const std::string& name, const std::string& access, const std::string& reserveType)
  {
    auto access_type = string_to_ERegAttrType(access);
    auto reserve_type = string_to_ERegReserveType(reserveType);
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->UnreserveRegister(name, access_type, reserve_type);
  }

  bool Scheduler::ReadRegister(uint32 threadId, const std::string& name, const std::string& field, uint64& reg_value) const
  {
    auto gen_instance = LookUpGenerator(threadId);
    return gen_instance->ReadRegister(name, field, reg_value);
  }

  void Scheduler::WriteRegister(uint32 threadId, const std::string& name, const std::string& field, uint64 value, bool update)
  {
    auto gen_instance = LookUpGenerator(threadId);
    if (update == true)
      gen_instance->WriteRegisterWithUpdate(name, field, value);
    else
      gen_instance->WriteRegister(name, field, value);
  }

  void Scheduler::InitializeRegister(uint32 threadId, const std::string& name, const std::string& field, uint64 value)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->InitializeRegister(name, field, value);
  }

  void Scheduler::InitializeRegister(uint32 threadId, const std::string& name, std::vector<uint64> values)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->InitializeRegister(name, values);
  }

  void Scheduler::InitializeRegisterFields(uint32 threadId, const std::string& registerName, const map<string, uint64>& field_value_map)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->InitializeRegisterFields(registerName, field_value_map);
  }

  void Scheduler::RandomInitializeRegister(uint32 threadId, const std::string& name, const std::string& field)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->RandomInitializeRegister(name, field);
  }

  void Scheduler::RandomInitializeRegisterFields(uint32 threadId, const std::string& registerName, const vector<string>& fieldList)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->RandomInitializeRegisterFields(registerName, fieldList);
  }

  uint64 Scheduler::GetRegisterFieldMask(uint32 threadId, const std::string& regName, const vector<string>& fieldList)
  {
    auto gen_instance = LookUpGenerator(threadId);
    return gen_instance->GetRegisterFieldMask(regName, fieldList);
  }

  void Scheduler::GenSequence(uint32 threadId, GenRequest* pGenRequest)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->GenSequence(pGenRequest);
  }

  void Scheduler::Query(uint32 threadId, const GenQuery& rGenQuery)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->Query(rGenQuery);
  }

  void Scheduler::ReserveMemory(uint32 threadId, const string& name, const string& range, uint32 bank, bool isVirtual)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->ReserveMemory(name, range, bank, isVirtual);
  }

  void Scheduler::UnreserveMemory(uint32 threadId, const string& name, const string& range, uint32 bank, bool isVirtual)
  {
    auto gen_instance = LookUpGenerator(threadId);
    gen_instance->UnreserveMemory(name, range, bank, isVirtual);
  }

  void Scheduler::ModifyVariable(uint32 threadId, const string& name, const string& value, const string& var_type)
  {
    auto gen_instance = LookUpGenerator(threadId);
    auto varType = string_to_EVariableType(var_type);
    gen_instance->ModifyVariable(name, value, varType);
  }

  const string& Scheduler::GetVariable(uint32 threadId, const string& name, const string& var_type) const
  {
    auto gen_instance = LookUpGenerator(threadId);
    auto varType = string_to_EVariableType(var_type);
    return gen_instance->GetVariable(name, varType);
  }

  const InstructionStructure* Scheduler::GetInstructionStructure(uint32 threadId, const std::string& instrName) const
  {
    auto gen_instance = LookUpGenerator(threadId);
    return gen_instance->GetInstructionStructure(instrName);
  }

  void Scheduler::RegisterModificationSet(uint32 threadId,  EChoicesType choicesType, uint32 set_id)
  {
    auto gen_instance = LookUpGenerator(threadId);
    return gen_instance->GetRegisteredSetModifier()->RegisterModificationSet(choicesType, set_id);
  }

  bool  Scheduler::VerifyVirtualAddress(uint32 threadId, uint64 va, uint64 size, bool isInstr) const
  {
    auto gen_instance = LookUpGenerator(threadId);
    return gen_instance->VerifyVirtualAddress(va, size, isInstr);
  }

  void Scheduler::AddThreadId(uint32 threadId)
  {
    mpSchedulingStrategy->AddThreadId(threadId);
  }

  void Scheduler::RemoveThreadId(uint32 threadId)
  {
    mpSchedulingStrategy->RemoveThreadId(threadId);

    // remove thread treat as reached the barrier.
    std::vector<SynchronizeBarrier* > barriers;
    mpSyncBarrierManager->GetParticipatingBarriers(threadId, barriers);
    for (auto barrier : barriers) {
      if (barrier->AddReachedThread(threadId) == 0) {
        barrier->ActivateSynchronizedThreads(mpSchedulingStrategy);
        mpSyncBarrierManager->Remove(barrier);
      }
    }
    string err_msg;
    if (not mpSyncBarrierManager->Validate(mpSchedulingStrategy, err_msg)) {
      LOG(fail) << "{Scheduler::RemoveThreadId} invalid barriers state due to: " << err_msg << endl;
      FAIL("invalid-barriers-state");
    }
  }

  void Scheduler::NextThread()
  {
    mpSchedulingStrategy->NextThread();
  }

  void Scheduler::RefreshSchedule()
  {
    mpSchedulingStrategy->RefreshSchedule();
  }

  uint32 Scheduler::ActiveThreadCount() const
  {
    return mpSchedulingStrategy->ActiveThreadCount();
  }

  uint32 Scheduler::CurrentThreadId() const
  {
    return mpSchedulingStrategy->CurrentThreadId();
  }

  void Scheduler::LockThreadScheduler(uint32 threadId)
  {
    mpSchedulingStrategy->LockSchedule(threadId);
  }

  void Scheduler::UnlockThreadScheduler(uint32 threadId)
  {
    mpSchedulingStrategy->UnlockSchedule(threadId);
  }

  void Scheduler::PartitionThreadGroup(EPartitionThreadPolicy policy, const PartitionArgument* pPartitionArg)
  {
    mpGroupModerator->PartitionThreadGroup(policy, pPartitionArg);
  }

  void Scheduler::SetThreadGroup(uint32 groupId, const string& job, const string& threads)
  {
    ConstraintSet threads_constr(threads);
    mpGroupModerator->SetThreadGroup(groupId, job, threads_constr);
  }

  void Scheduler::QueryThreadGroup(uint32 groupId, vector<ThreadGroup* >& threadGroups) const
  {
    mpGroupModerator->QueryThreadGroup(groupId, threadGroups);
  }

  void Scheduler::QueryThreadGroup(const string& groupJob, std::vector<ThreadGroup* >& threadGroups) const
  {
    mpGroupModerator->QueryThreadGroup(groupJob, threadGroups);
  }

  uint32 Scheduler::GetThreadGroupId(uint32 threadId) const
  {
    return mpGroupModerator->GetThreadGroupId(threadId);
  }

  void Scheduler::GetFreeThreads(vector<uint32>& freeThreads) const
  {
    mpGroupModerator->GetFreeThreads(freeThreads);
  }

  bool Scheduler::GenSemaphore(uint32 threadId, const std::string& name, uint64 counter, uint32 bank, uint32 size, uint64& address, bool& reverseEndian)
  {
    auto gen = LookUpGenerator(threadId);
    return mpSemaManager->GenSemaphore(gen, name, counter, bank, size, address, reverseEndian);
  }

  static bool check_non_existing_threads(const map<uint32, Generator *>& existingThreads, const ConstraintSet& rSynchronizedThreads)
  {
    vector<uint64> value_vec;
    rSynchronizedThreads.GetValues(value_vec);
    for (const auto value : value_vec) {
      if (existingThreads.find(value) == existingThreads.end()) {
        return true;
      }
    }
    return false;
  }

  void Scheduler::SynchronizeWithBarrier(uint32 threadId, const ConstraintSet& rSynchronizedThreads)
  {
    LOG(info) << "{Scheduler::SynchronizeWithBarrier} thread " << threadId << " reached " << rSynchronizedThreads.ToString() << endl;
    if (check_non_existing_threads(mGenerators, rSynchronizedThreads)) {
      LOG(fail) << "{Scheduler::SynchronizeWithBarrier} barrier contains non-existing thread id" << endl;
      FAIL("barrier-contains-non-existing-thread");
    }

    if (not rSynchronizedThreads.ContainsValue(threadId)) {
      LOG(fail) << "{Scheduler::SynchronizeWithBarrier} current thread(" << threadId << ") not in current barrier " << endl;
      FAIL("thread-not-in-current-barrier");
    }

    auto current_barrier = mpSyncBarrierManager->Query(rSynchronizedThreads);
    if (current_barrier == nullptr) {
      ConstraintSet scheduled_threads;
      mpSchedulingStrategy->GetScheduledThreadIds(scheduled_threads);
      scheduled_threads.ApplyConstraintSet(rSynchronizedThreads);
      auto count = scheduled_threads.Size();
      current_barrier = mpSyncBarrierManager->CreateSynchronizeBarrier(rSynchronizedThreads, count);
      mpSyncBarrierManager->Add(current_barrier);
    }

    mpSchedulingStrategy->DeactivateThread(threadId);

    if (current_barrier->AddReachedThread(threadId) == 0) {
      current_barrier->ActivateSynchronizedThreads(mpSchedulingStrategy);
      mpSyncBarrierManager->Remove(current_barrier);
    }

    string err_msg;
    if (not mpSyncBarrierManager->Validate(mpSchedulingStrategy, err_msg)) {
      LOG(fail) << "{Scheduler::SynchronizeWithBarrier} invalid barriers state due to: " << err_msg << endl;
      FAIL("invalid-barriers-state");
    }
  }
}
