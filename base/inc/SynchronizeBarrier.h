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
#ifndef Force_SynchronizeBarrier_H
#define Force_SynchronizeBarrier_H

#include <string>
#include <vector>

#include "Defines.h"

namespace Force {

  class ConstraintSet;
  class SchedulingStrategy;

  /*!
    \class SynchronizeBarrier
    \brief Manager threads that participate in one synchronize barrier.
   */
  class SynchronizeBarrier {
  public:
    SynchronizeBarrier(const ConstraintSet& rThreadIds, uint32 unreachedCount); //!< Constructor.
    ~SynchronizeBarrier(); //!< Destructor.
    const std::string ToString() const; //<! Return a string representation of the synchronize barrier.

    uint32 AddReachedThread(uint32 threadId); //!< Return the count of threads that unreached the barrier, add reached thread.
    bool Contains(uint32 threadId) const; //!< Return whether barrier are include the thread.
    inline const ConstraintSet& GetSynchronizedThreads() const { return *mpSynchronizedThreads; } //!< Return all synchronize threads.
    inline const ConstraintSet& GetReachedThreads() const { return *mpReachedThreads; } //!< Return all synchronize threads.
    void ActivateSynchronizedThreads(SchedulingStrategy* pSchedulingStrategy) const; //!< Activate the synchronized threads.

    DEFAULT_CONSTRUCTOR_ABSENT(SynchronizeBarrier);
    COPY_CONSTRUCTOR_ABSENT(SynchronizeBarrier);
    ASSIGNMENT_OPERATOR_ABSENT(SynchronizeBarrier);

  private:
    ConstraintSet* mpSynchronizedThreads; //!< All threads that participate in this barrier.
    ConstraintSet* mpReachedThreads; //!< Reached thread ids.
    uint32 mUnreachedCount; //!< The count of participating PEs that have not yet reached the barrier.
  };

  /*!
    \class SynchronizeBarrierManager
    \brief Manager all active synchronize barriers.
   */
  class SynchronizeBarrierManager {
  public:
    SynchronizeBarrierManager() : mBarriers() { }; //!< Constructor.
    ~SynchronizeBarrierManager(); //!< Destructor.

    void Remove(SynchronizeBarrier* pBarrier); //!< Remove the barrier.
    void Add(SynchronizeBarrier* pBarrier); //!< Add the barrier.
    SynchronizeBarrier* Query(const ConstraintSet& rThreadIds) const; //!< Query the barrier that contains all threadIds.
    SynchronizeBarrier* CreateSynchronizeBarrier(const ConstraintSet& rThreadIds, uint32 unreachedCount) const; //!< Build the barrier that contains all threadIds.
    void GetParticipatingBarriers(uint32 threadId, std::vector<SynchronizeBarrier* >& rBarriers) const; //!< Query the barriers that contains the threadId.
    bool Validate(const SchedulingStrategy* pSchedulingStrategy, std::string& rErrMsg) const; //!< Validate the barriers when one thread partcipating barrier.

    COPY_CONSTRUCTOR_ABSENT(SynchronizeBarrierManager);
    ASSIGNMENT_OPERATOR_ABSENT(SynchronizeBarrierManager);

  private:
    bool DetectDeadLock(const SchedulingStrategy* pSchedulingStrategy) const; //!< Return true if there has any dead lock.
  private:
    std::vector<SynchronizeBarrier* > mBarriers; //!< Active synchronize barriers.
  };

}

#endif
