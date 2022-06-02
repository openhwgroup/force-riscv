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
#include "RestoreLoop.h"

#include "GenExceptionAgent.h"
#include "GenInstructionAgent.h"
#include "GenMode.h"
#include "Generator.h"
#include "Log.h"
#include "ReExecutionManager.h"

using namespace std;

/*!
  \file RestoreLoop.cc
  \brief Code supporting state restore loops.
*/

namespace Force {

  RestoreLoop::RestoreLoop(cuint32 loopRegIndex, cuint32 branchRegIndex, cuint32 simCount, cuint32 restoreCount, const set<ERestoreExclusionGroup>& rRestoreExclusions, cuint64 loopBackAddress, cuint32 loopId, Generator* pGenerator)
    : mpGenerator(pGenerator), mRestoreExclusions(rRestoreExclusions), mLoopRegIndex(loopRegIndex), mBranchRegIndex(branchRegIndex), mLoopBackAddr(loopBackAddress), mSimCount(simCount), mEndRestoreCount(restoreCount), mCurRestoreCount(0), mLoopId(loopId), mRestoreStartAddr(0),  mResourcePeStateGroups()
  {
    mResourcePeStateGroups.emplace(
      piecewise_construct,
      forward_as_tuple(ERestoreGroup::GPR),
      forward_as_tuple(EResourcePeStateType::RegisterPeState));
    mResourcePeStateGroups.emplace(
      piecewise_construct,
      forward_as_tuple(ERestoreGroup::VECREG),
      forward_as_tuple(EResourcePeStateType::RegisterPeState));
    mResourcePeStateGroups.emplace(
      piecewise_construct,
      forward_as_tuple(ERestoreGroup::PREDREG),
      forward_as_tuple(EResourcePeStateType::RegisterPeState));
    mResourcePeStateGroups.emplace(
      piecewise_construct,
      forward_as_tuple(ERestoreGroup::System),
      forward_as_tuple(EResourcePeStateType::RegisterPeState));
    mResourcePeStateGroups.emplace(
      piecewise_construct,
      forward_as_tuple(ERestoreGroup::Memory),
      forward_as_tuple(EResourcePeStateType::MemoryPeState));
  }

  void RestoreLoop::PushResourcePeState(const ResourcePeState* pState)
  {
    IncrementalResourcePeStateStack* inc_resource_pe_state_stack = GetResourcePeStateStack(GetRestoreGroup(pState));
    inc_resource_pe_state_stack->PushResourcePeState(pState);
  }

  void RestoreLoop::SetRestoreStartAddress(cuint64 restoreStartAddress)
  {
    mRestoreStartAddr = restoreStartAddress;
  }

  uint32 RestoreLoop::GetLoopId() const
  {
    return mLoopId;
  }

  uint64 RestoreLoop::GetLoopBackAddress() const
  {
    return mLoopBackAddr;
  }

  uint64 RestoreLoop::GetRestoreStartAddress() const
  {
    return mRestoreStartAddr;
  }

  bool RestoreLoop::OnFirstRestoreIteration() const
  {
    return (mCurRestoreCount == 0);
  }

  bool RestoreLoop::OnLastRestoreIteration() const
  {
    return (mCurRestoreCount == (mEndRestoreCount - 1));
  }

  bool RestoreLoop::HasFinishedRestoreIterations() const
  {
    return (mCurRestoreCount == mEndRestoreCount);
  }

  void RestoreLoop::CommitRestoreInstructions(vector<GenRequest*>& rRequestSeq)
  {
    mCurRestoreCount++;

    mpGenerator->PrependRequests(rRequestSeq);
  }

  IncrementalResourcePeStateStack* RestoreLoop::GetResourcePeStateStack(const ERestoreGroup restoreGroup)
  {
    IncrementalResourcePeStateStack* inc_resource_pe_state_stack = nullptr;
    auto itr = mResourcePeStateGroups.find(restoreGroup);
    if (itr != mResourcePeStateGroups.end()) {
      inc_resource_pe_state_stack = &(itr->second);
    }
    else {
      LOG(fail) << "{RestoreLoop::GetResourcePeStateStack} unable to find ResourcePeStateStack for " << ERestoreGroup_to_string(restoreGroup) << endl;;
      FAIL("unknown-restore-group");
    }

    return inc_resource_pe_state_stack;
  }

  bool RestoreLoop::IsExcluded(const ERestoreExclusionGroup restoreGroup) const
  {
    return (mRestoreExclusions.find(restoreGroup) != mRestoreExclusions.end());
  }

  uint32 RestoreLoop::GetLoopRegisterIndex() const
  {
    return mLoopRegIndex;
  }

  uint32 RestoreLoop::GetBranchRegisterIndex() const
  {
    return mBranchRegIndex;
  }

  uint32 RestoreLoop::GetSimulationCount() const
  {
    return mSimCount;
  }

  uint32 RestoreLoop::GetCurrentRestoreCount() const
  {
    return mCurRestoreCount;
  }

  RestoreLoopManager::RestoreLoopManager(Generator *pGenerator)
    : mpGenerator(pGenerator), mRestoreLoops(), mNestedLoopStartAddresses(), mNestedLoopEndAddresses(), mBranchRegIndex(0)
  {
    auto gen_instruction_agent = pGenerator->GetAgent<GenInstructionAgent>(EGenAgentType::GenInstructionAgent);
    gen_instruction_agent->SignUp(this);
  }

  RestoreLoopManager::~RestoreLoopManager()
  {
    while (not mRestoreLoops.empty()) {
      RestoreLoop *restore_loop = mRestoreLoops.top();
      mRestoreLoops.pop();
      delete restore_loop;
    }
  }

  void RestoreLoopManager::BeginLoop(cuint32 loopRegIndex, cuint32 simCount, cuint32 restoreCount, const set<ERestoreExclusionGroup>& rRestoreExclusions)
  {
    ValidateExceptionMode();

    GenMode* gen_mode = mpGenerator->GetGenMode();
    if (mRestoreLoops.empty()) {
      gen_mode->EnableGenMode(EGenModeTypeBaseType(EGenModeType::InLoop) | EGenModeTypeBaseType(EGenModeType::RestoreStateLoop));

      // Reserve a register for long branches.
      ReserveBranchRegister();
    }
    else {
      // We are starting a nested loop, so disable state recording while we set up the inner loop
      gen_mode->DisableGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));
    }

    ReExecutionManager* re_execution_manager = mpGenerator->GetReExecutionManager();
    uint64 loop_back_address = mpGenerator->PC();
    uint32 loop_id = re_execution_manager->StartLoop(loop_back_address);

    RestoreLoop* restore_loop = CreateRestoreLoop(loopRegIndex, mBranchRegIndex, simCount, restoreCount, rRestoreExclusions, loop_back_address, loop_id);
    restore_loop->BeginLoop();
    mRestoreLoops.push(restore_loop);

    BeginNextIteration();
  }

  void RestoreLoopManager::EndLoop(cuint32 loopId)
  {
    FinalizeCurrentLoop();
  }

  void RestoreLoopManager::GenerateRestoreInstructions(cuint32 loopId)
  {
    RestoreLoop* restore_loop = GetCurrentRestoreLoop();

    if (restore_loop->OnFirstRestoreIteration()) {
      BeginNextRestoreIteration();
      restore_loop->SetRestoreStartAddress(mpGenerator->PC());
    }

    restore_loop->GenerateRestoreInstructions();
  }

  void RestoreLoopManager::PushResourcePeState(const ResourcePeState* pState)
  {
    GenMode* gen_mode = mpGenerator->GetGenMode();
    if (not gen_mode->InException()) {
      RestoreLoop* restore_loop = GetCurrentRestoreLoop();
      restore_loop->PushResourcePeState(pState);
    }
  }

  uint32 RestoreLoopManager::GetCurrentLoopId() const
  {
    RestoreLoop* restore_loop = GetCurrentRestoreLoop();
    return restore_loop->GetLoopId();
  }

  uint64 RestoreLoopManager::GetCurrentLoopBackAddress() const
  {
    RestoreLoop* restore_loop = GetCurrentRestoreLoop();
    return restore_loop->GetLoopBackAddress();
  }

  uint32 RestoreLoopManager::GetBranchRegisterIndex() const
  {
    return mBranchRegIndex;
  }

  void RestoreLoopManager::HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload)
  {
    switch (eventType) {
    case ENotificationType::PCUpdate:
      HandlePcUpdate(mpGenerator->PC());
      break;
    default:
      LOG(fail) << "{RestoreLoopManager::HandleNotification} unexpected event: " << ENotificationType_to_string(eventType) << endl;
      FAIL("unexpected-event-type");
    }
  }

  Generator* RestoreLoopManager::GetGenerator() const
  {
    return mpGenerator;
  }

  void RestoreLoopManager::SetBranchRegisterIndex(cuint32 branchRegIndex)
  {
    mBranchRegIndex = branchRegIndex;
  }

  void RestoreLoopManager::ValidateExceptionMode()
  {
    auto gen_except_agent = mpGenerator->GetAgent<GenExceptionAgent>(EGenAgentType::GenExceptionAgent);
    if (gen_except_agent->IsFastMode()) {
      LOG(fail) << "{RestoreLoopManager::ValidateExceptionMode} restore loops are not supported when using fast exception mode" << endl;
      FAIL("unsupported-exception-mode");
    }
  }

  void RestoreLoopManager::HandlePcUpdate(cuint64 curPc)
  {
    if (not mRestoreLoops.empty()) {
      RestoreLoop* restore_loop = mRestoreLoops.top();

      if ((not restore_loop->OnFirstRestoreIteration()) and (not restore_loop->HasFinishedRestoreIterations())) {
        if (curPc == restore_loop->GetLoopBackAddress()) {
          BeginNextIteration();
        }

        // No need to do anything if the beginning of the restore instructions for the outer loop coincides with the end
        // of an inner loop, as recording is already disabled
        else if ((curPc == restore_loop->GetRestoreStartAddress()) and (not IsNestedLoopEndAddress(curPc))) {
          BeginNextRestoreIteration();
        }

        else if (IsNestedLoopStartAddress(curPc)) {
          BeginNestedLoop();
        }

        // No need to do anything if the end of an inner loop conincides with the beginning of the restore instructions
        // for the outer loop, as recording is already disabled
        else if (IsNestedLoopEndAddress(curPc) and (curPc != restore_loop->GetRestoreStartAddress())) {
          EndNestedLoop();
        }
      }
    }
  }

  void RestoreLoopManager::BeginNextIteration()
  {
    GenMode* gen_mode = mpGenerator->GetGenMode();
    gen_mode->EnableGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));
  }

  void RestoreLoopManager::BeginNextRestoreIteration()
  {
    GenMode* gen_mode = mpGenerator->GetGenMode();
    gen_mode->DisableGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));
  }

  void RestoreLoopManager::BeginNestedLoop()
  {
    GenMode* gen_mode = mpGenerator->GetGenMode();
    gen_mode->DisableGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));
  }

  void RestoreLoopManager::EndNestedLoop()
  {
    GenMode* gen_mode = mpGenerator->GetGenMode();
    gen_mode->EnableGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));
  }

  void RestoreLoopManager::FinalizeCurrentLoop()
  {
    RestoreLoop* restore_loop = GetCurrentRestoreLoop();
    mRestoreLoops.pop();
    restore_loop->EndLoop();

    ReExecutionManager* re_execution_manager = mpGenerator->GetReExecutionManager();
    re_execution_manager->EndLoop(restore_loop->GetLoopId(), mpGenerator->PC());

    GenMode* gen_mode = mpGenerator->GetGenMode();
    if (mRestoreLoops.empty()) {
      mNestedLoopStartAddresses.clear();
      mNestedLoopEndAddresses.clear();

      gen_mode->DisableGenMode(EGenModeTypeBaseType(EGenModeType::InLoop) | EGenModeTypeBaseType(EGenModeType::RestoreStateLoop));

      UnreserveBranchRegister();

      // The user may specify instructions that terminate the loop prematurely. We can't always detect this, but we
      // should issue a warning when we can.
      if (gen_mode->RecordingState()) {
        LOG(warn) << "{RestoreLoopManager::FinalizeCurrentLoop} unexpectedly skipped the loop restore instructions" << endl;;

        gen_mode->DisableGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));
      }
    }
    else {
      AddNestedLoopAddresses((restore_loop->GetLoopBackAddress() - 4), mpGenerator->PC());

      // We are returning control to an outer loop, so we need to re-enable state recording for the outer loop
      gen_mode->EnableGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));
    }

    delete restore_loop;
    restore_loop = nullptr;
  }

  bool RestoreLoopManager::IsNestedLoopStartAddress(cuint64 addr) const
  {
    auto start_addr_set_itr = mNestedLoopStartAddresses.find(GetCurrentLoopId());
    if (start_addr_set_itr != mNestedLoopStartAddresses.end()) {
      const set<uint64>& start_addr_set = start_addr_set_itr->second;
      auto start_addr_itr = start_addr_set.find(addr);
      return (start_addr_itr != start_addr_set.end());
    }

    return false;
  }

  bool RestoreLoopManager::IsNestedLoopEndAddress(cuint64 addr) const
  {
    auto end_addr_set_itr = mNestedLoopEndAddresses.find(GetCurrentLoopId());
    if (end_addr_set_itr != mNestedLoopEndAddresses.end()) {
      const set<uint64>& end_addr_set = end_addr_set_itr->second;
      auto end_addr_itr = end_addr_set.find(addr);
      return (end_addr_itr != end_addr_set.end());
    }

    return false;
  }

  void RestoreLoopManager::AddNestedLoopAddresses(cuint64 nestedLoopStartAddr, cuint64 nestedLoopEndAddr)
  {
    uint32 cur_loop_id = GetCurrentLoopId();
    auto start_addr_set_itr = mNestedLoopStartAddresses.find(cur_loop_id);
    if (start_addr_set_itr != mNestedLoopStartAddresses.end()) {
      set<uint64>& start_addr_set = start_addr_set_itr->second;
      auto insert_res = start_addr_set.insert(nestedLoopStartAddr);

      if (not insert_res.second) {
        LOG(fail) << "{RestoreLoopManager::AddNestedLoopAddresses} the nested loop start address 0x" << hex << nestedLoopStartAddr << " has already been used" << endl;
        FAIL("duplicate-nested-loop-address");
      }
    }
    else {
      set<uint64> start_addr_set = {nestedLoopStartAddr};
      mNestedLoopStartAddresses.emplace(cur_loop_id, move(start_addr_set));
    }

    auto end_addr_set_itr = mNestedLoopEndAddresses.find(cur_loop_id);
    if (end_addr_set_itr != mNestedLoopEndAddresses.end()) {
      set<uint64>& end_addr_set = end_addr_set_itr->second;
      auto insert_res = end_addr_set.insert(nestedLoopEndAddr);

      if (not insert_res.second) {
        LOG(fail) << "{RestoreLoopManager::AddNestedLoopAddresses} the nested loop end address 0x" << hex << nestedLoopEndAddr << " has already been used" << endl;
        FAIL("duplicate-nested-loop-address");
      }
    }
    else {
      set<uint64> end_addr_set = {nestedLoopEndAddr};
      mNestedLoopEndAddresses.emplace(cur_loop_id, move(end_addr_set));
    }
  }

  RestoreLoop* RestoreLoopManager::GetCurrentRestoreLoop() const
  {
    // The behavior of calling top() on an empty stack is undefined, so make sure to verify that the stack is not empty
    // first
    if (mRestoreLoops.empty()) {
      LOG(fail) << "{RestoreLoopManager::GetCurrentRestoreLoop} not currently within the context of a restore loop" << endl;
      FAIL("no-restore-loops");
    }

    return mRestoreLoops.top();
  }

  RestoreLoopManagerRepository* RestoreLoopManagerRepository::mspRestoreLoopManagerRepository = nullptr;

  RestoreLoopManagerRepository::RestoreLoopManagerRepository()
    : mRestoreLoopManagers()
  {
  }

  RestoreLoopManagerRepository::~RestoreLoopManagerRepository()
  {
    for (const auto& entry : mRestoreLoopManagers) {
      delete entry.second;
    }
  }

  void RestoreLoopManagerRepository::Initialize()
  {
    if (mspRestoreLoopManagerRepository == nullptr) {
      mspRestoreLoopManagerRepository = new RestoreLoopManagerRepository();
    }
  }

  void RestoreLoopManagerRepository::Destroy()
  {
    delete mspRestoreLoopManagerRepository;
    mspRestoreLoopManagerRepository = nullptr;
  }

  void RestoreLoopManagerRepository::AddRestoreLoopManager(cuint32 threadId, RestoreLoopManager* pRestoreLoopManager)
  {
    mRestoreLoopManagers.emplace(threadId, pRestoreLoopManager);
  }

  RestoreLoopManager* RestoreLoopManagerRepository::GetRestoreLoopManager(cuint32 threadId) const
  {
    RestoreLoopManager* restore_loop_manager = nullptr;
    auto itr = mRestoreLoopManagers.find(threadId);
    if (itr != mRestoreLoopManagers.end()) {
      restore_loop_manager = itr->second;
    }
    else {
      LOG(fail) << "{RestoreLoopManagerRepository::GetRestoreLoopManager} unable to find RestoreLoopManager for Thread " << threadId << endl;;
      FAIL("no-restore-loop-manager-found");
    }

    return restore_loop_manager;
  }

}
