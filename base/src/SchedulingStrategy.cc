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
#include <SchedulingStrategy.h>
#include <Random.h>
#include <Log.h>
#include <Constraint.h>

#include <algorithm>

using namespace std;

/*!
  \file SchedulingStrategy.cc
  \brief Code for scheduling strategy classes.
*/

namespace Force {

  ShuffledRoundRobinSchedulingStrategy::ShuffledRoundRobinSchedulingStrategy()
    : SchedulingStrategy(), mState(ESchedulingState::Random), mCurrentThreadId(-1), mNextIndex(0), mLockingState(ESchedulingState::Random), mLockingLevel(0), mLockingThreadId(-1), mThreadIds(), mActiveThreadIds()
  {

  }

  void ShuffledRoundRobinSchedulingStrategy::AddThreadId(uint32 threadId)
  {
    if (find(mThreadIds.begin(), mThreadIds.end(), threadId) == mThreadIds.end()) {
      mThreadIds.push_back(threadId);
      mActiveThreadIds.push_back(threadId);
    }
    else {
      LOG(info) << "{ShuffledRoundRobinSchedulingStrategy::AddThreadId} thread " << threadId << " already exist"  << endl;
    }
  }

  void ShuffledRoundRobinSchedulingStrategy::RemoveThreadId(uint32 threadId)
  {
    mThreadIds.erase(std::remove(mThreadIds.begin(), mThreadIds.end(), threadId), mThreadIds.end());
    mActiveThreadIds.erase(std::remove(mActiveThreadIds.begin(), mActiveThreadIds.end(), threadId), mActiveThreadIds.end());
    -- mNextIndex;
    mState = ESchedulingState::Finishing;
  }

  void ShuffledRoundRobinSchedulingStrategy::NextThread()
  {
    auto thread_list_size = mActiveThreadIds.size();

    switch (mState) {
    case ESchedulingState::Finishing:
      if (thread_list_size == 0) {
        mNextIndex = 0;
        mCurrentThreadId = 0;
        return;
      }
      // else fall through
    case ESchedulingState::Random:
      if (mNextIndex >= thread_list_size) {
        RefreshSchedule();
        return;
      }

      SimpleNextThread();
      break;
    case ESchedulingState::Locked:
      // If locked, don't advance thread.
      break;
    default:
      ;
    }
  }

  void ShuffledRoundRobinSchedulingStrategy::RefreshSchedule()
  {
    //LOG(notice) << "[ShuffledRoundRobinSchedulingStrategy::RefreshSchedule] shuffling happend." << endl;
    mNextIndex = 0;

    RandomURBG32 urbg32(Random::Instance());
    std::shuffle(mActiveThreadIds.begin(), mActiveThreadIds.end(), urbg32);
    SimpleNextThread();
  }


  void ShuffledRoundRobinSchedulingStrategy::LockSchedule(uint32 threadId)
  {
    LOG(notice) << "[ShuffledRoundRobinSchedulingStrategy::LockSchedule] locking from thread: 0x" << hex << threadId << " current locking level=" << mLockingLevel << " current state=" << ESchedulingState_to_string(mState) << endl;

    if (mLockingLevel == 0) {
      // First level lock.  Save scheduler state
      mLockingState = mState;
      mLockingThreadId = threadId;
      ++ mLockingLevel;
      mState = ESchedulingState::Locked;
    }
    else {
      if (mState != ESchedulingState::Locked) {
        LOG(fail) << "[ShuffledRoundRobinSchedulingStrategy::LockSchedule] expect scheduler state to be Locked when locking level (" << dec << mLockingLevel << ") > 0" << endl;
        FAIL("inconsistent-thread-scheduler-state");
      }
      else if (threadId != mLockingThreadId) {
        LOG(fail) << "[ShuffledRoundRobinSchedulingStrategy::LockSchedule] expect mLockingThreadId (0x" << hex << mLockingThreadId << ") to be the same with incoming threadId (0x"
          << threadId << ") when locking level (" << dec << mLockingLevel << ") > 0" << endl;
        FAIL("inconsistent-thread-locking-id");
      }
      ++ mLockingLevel;
    }
  }

  void ShuffledRoundRobinSchedulingStrategy::UnlockSchedule(uint32 threadId)
  {
    if (threadId != mLockingThreadId) {
      LOG(fail) << "[ShuffledRoundRobinSchedulingStrategy::UnlockSchedule] expect incoming thread ID (0x" << hex << threadId << ") to be the same with saved mLockingThreadId (0x" << mLockingThreadId << ")." << endl;
      FAIL("inconsistent-thread-unlocking-id");
    }
    else if (mState != ESchedulingState::Locked) {
      LOG(fail) << "[ShuffledRoundRobinSchedulingStrategy::UnlockSchedule] expect scheduler state to be Locked when unlocking called from thread ID (0x" << hex << threadId << ")." << endl;
      FAIL("inconsistent-thread-unlocking-state");
    }
    else if (mLockingLevel == 0) {
      LOG(fail) << "[ShuffledRoundRobinSchedulingStrategy::UnlockSchedule] expect scheduler mLockingLevel (" << dec << mLockingLevel << ") to be > 0 when unlocking called from thread ID (0x" << hex << threadId << ")." << endl;
      FAIL("inconsistent-thread-unlocking-level");
    }

    -- mLockingLevel;
    if (mLockingLevel == 0) {
      // Last level unlock.  Restore scheduler state
      mState = mLockingState;
      mLockingThreadId = -1;
    }

    LOG(notice) << "[ShuffledRoundRobinSchedulingStrategy::LockSchedule] unlocked from thread: 0x" << hex << threadId << " current locking level=" << mLockingLevel << " now state=" << ESchedulingState_to_string(mState) << endl;
  }

  void ShuffledRoundRobinSchedulingStrategy::ActivateThread(uint32 threadId)
  {
    auto result = find(mActiveThreadIds.begin(), mActiveThreadIds.end(), threadId);
    if (result != mActiveThreadIds.end()) {
      LOG(fail) << "[ShuffledRoundRobinSchedulingStrategy::ActivateThread] expect " << threadId << " not in active vector when activate called" << endl;
      FAIL("inconsistent-thread-active-state");
    }

    result = find(mThreadIds.begin(), mThreadIds.end(), threadId);
    if (result == mThreadIds.end()) {
      LOG(fail) << "[ShuffledRoundRobinSchedulingStrategy::ActivateThread] thread " << threadId << " not existing" << endl;
      FAIL("thread-not-existing");
    }

    LOG(info) << "[ShuffledRoundRobinSchedulingStrategy::ActivateThread] activate thread: 0x" << hex << threadId << endl;
    mActiveThreadIds.push_back(threadId);
  }

  void ShuffledRoundRobinSchedulingStrategy::DeactivateThread(uint32 threadId)
  {
    auto result = find(mActiveThreadIds.begin(), mActiveThreadIds.end(), threadId);
    if (result == mActiveThreadIds.end()) {
      LOG(fail) << "[ShuffledRoundRobinSchedulingStrategy::DeactivateThread] expect " << threadId << " in active vector when deactivate called" << endl;
      FAIL("inconsistent-thread-active-state");
    }

    if (mState == ESchedulingState::Locked) {
      LOG(fail) << "[ShuffledRoundRobinSchedulingStrategy::DeactivateThread] scheduler state to be Locked when deactivate " << threadId << endl;
      FAIL("inconsistent-thread-scheduler-state");
    }

    LOG(info) << "[ShuffledRoundRobinSchedulingStrategy::DeactivateThread] deactivate thread: 0x" << hex << threadId << endl;
    mActiveThreadIds.erase(std::remove(mActiveThreadIds.begin(), mActiveThreadIds.end(), threadId), mActiveThreadIds.end());
  }

  void ShuffledRoundRobinSchedulingStrategy::GetScheduledThreadIds(ConstraintSet& constr) const
  {
    std::for_each(mThreadIds.begin(), mThreadIds.end(), [&constr](uint32 id){constr.AddValue(id);});
  }

  void ShuffledRoundRobinSchedulingStrategy::GetActiveThreadIds(ConstraintSet& constr) const
  {
    std::for_each(mActiveThreadIds.begin(), mActiveThreadIds.end(), [&constr](uint32 id){constr.AddValue(id);});
  }
}
