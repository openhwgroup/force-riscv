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
#ifndef Force_SchedulingStrategy_H
#define Force_SchedulingStrategy_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>

namespace Force
{

  class ConstraintSet;

  /*!
    \class SchedulingStrategy
    \brief Abstract base class for SchedulingStrategy
  */
  class SchedulingStrategy {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(SchedulingStrategy); //!< Use default constructor.
    SUPERCLASS_DESTRUCTOR_DEFAULT(SchedulingStrategy); //!< Default virtual destructor.

    virtual void AddThreadId(uint32 threadId) = 0; //!< Add front end thread ID for scheduling.
    virtual void RemoveThreadId(uint32 threadId) = 0; //!< Remove front end thread ID for scheduling.
    virtual void NextThread() = 0; //!< Advance to next thread.
    virtual void RefreshSchedule() = 0; //!< Refresh scheduling for next round.
    virtual uint32 ActiveThreadCount() const = 0; //!< Return number of active threads that are being scheduled.
    virtual uint32 CurrentThreadId() const = 0; //!< Return current thread ID.
    virtual void LockSchedule(uint32 threadId) = 0; //!< Lock schedule.
    virtual void UnlockSchedule(uint32 threadId) = 0; //!< Unlock schedule
    virtual void ActivateThread(uint32 threadId) = 0; //!< Activate thread.
    virtual void DeactivateThread(uint32 threadId) = 0; //!< Deactivate thread.
    virtual void GetScheduledThreadIds(ConstraintSet& constr) const = 0; //!< Vector of the all thread IDs to schedule.
    virtual void GetActiveThreadIds(ConstraintSet& constr) const = 0; //!< Vector of the active thread IDs.
  };


  /*!
    \class ShuffledRoundRobinSchedulingStrategy
    \brief Round robin thread scheduling strategy.
  */
  class ShuffledRoundRobinSchedulingStrategy : public SchedulingStrategy {
  public:
    ShuffledRoundRobinSchedulingStrategy(); //!< Constructor.
    SUBCLASS_DESTRUCTOR_DEFAULT(ShuffledRoundRobinSchedulingStrategy); //!< Use default sub class destructor.

    inline uint32 CurrentThreadId() const override //!< Return current thread ID.
    {
      return mCurrentThreadId;
    }

    void AddThreadId(uint32 threadId) override; //!< Add thread ID to thread ID list.
    void RemoveThreadId(uint32 threadId) override; //!< Remove thread ID from scheduling strategy.
    void NextThread() override; //!< Progress to the next thread.
    void RefreshSchedule() override; //!< Refresh schedule for shuffling.

    uint32 ActiveThreadCount() const override //!< Return number of threads that are still active and waiting for scheduling.
    {
      return mActiveThreadIds.size();
    }

    void LockSchedule(uint32 threadId) override; //!< Lock schedule.
    void UnlockSchedule(uint32 threadId) override; //!< Unlock schedule
    void ActivateThread(uint32 threadId) override; //!< Activate thread.
    void DeactivateThread(uint32 threadId) override; //!< Deactivate thread.
    void GetScheduledThreadIds(ConstraintSet& constr) const override; //!< Vector of the all thread IDs to schedule.
    void GetActiveThreadIds(ConstraintSet& constr) const override; //!< Vector of the active thread IDs.

    COPY_CONSTRUCTOR_ABSENT(ShuffledRoundRobinSchedulingStrategy);
    ASSIGNMENT_OPERATOR_ABSENT(ShuffledRoundRobinSchedulingStrategy);
  protected:

    inline void SimpleNextThread() //!< Simply move to next ID in the list.
    {
      mCurrentThreadId = mActiveThreadIds.at(mNextIndex);
      mNextIndex += 1;
    }

  private:
    ESchedulingState mState; //!< Scheduling state enum.
    uint32 mCurrentThreadId; //!< Current thread ID.
    uint32 mNextIndex; //!< Next index in the thread ID list.
    ESchedulingState mLockingState; //!< The scheduler state when thread locking occurs.
    uint32 mLockingLevel; //!< Schedule locking level.
    uint32 mLockingThreadId; //!< ThreadId of the thread that holds the schedule lock.
    std::vector<uint32> mThreadIds; //!< Vector of the thread IDs to schedule.
    std::vector<uint32> mActiveThreadIds; //!< Vector of the active thread IDs to schedule.
  };

}

#endif
