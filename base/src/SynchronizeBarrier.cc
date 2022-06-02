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
#include "SynchronizeBarrier.h"

#include <algorithm>
#include <sstream>

#include "Constraint.h"
#include "Log.h"
#include "Random.h"
#include "SchedulingStrategy.h"

using namespace std;

namespace Force {

  SynchronizeBarrier::SynchronizeBarrier(const ConstraintSet& rThreadIds, uint32 unreachedCount)
      : mpSynchronizedThreads(rThreadIds.Clone()), mpReachedThreads(nullptr), mUnreachedCount(unreachedCount)
  {
    if (mUnreachedCount > mpSynchronizedThreads->Size()) {
      LOG(fail) << "{SynchronizeBarrier::SychronizeBarrier} unreached count is larger than synchronized threads size" << endl;
      FAIL("count-larger-than-synchronized-threads-size");
    }
    mpReachedThreads = new ConstraintSet();
  }

  SynchronizeBarrier::~SynchronizeBarrier()
  {
    delete mpSynchronizedThreads;
    mpSynchronizedThreads = nullptr;

    delete mpReachedThreads;
    mpReachedThreads = nullptr;
  }

  bool SynchronizeBarrier::Contains(uint32 threadId) const
  {
    return mpSynchronizedThreads->ContainsValue(threadId);
  }

  uint32 SynchronizeBarrier::AddReachedThread(uint32 threadId)
  {
    if (not Contains(threadId)) {
      LOG(fail) << "{SynchronizeBarrier::AddReachedThread}" << "the barrier not include the thread " << threadId << endl;
      FAIL("not-include-thread");
    }

    if (mpReachedThreads->ContainsValue(threadId)) {
      LOG(fail) << "{SynchronizeBarrier::AddReachedThread}" << "the thread(" << threadId << ") has reached" << endl;
      FAIL("thread-has-reached");
    }

    if (mUnreachedCount == 0) {
      LOG(fail) << "{SynchronizeBarrier::AddReachedThread}" << "unreached count has reached to zero, thread(" << threadId << ")" << endl;
      FAIL("unreached-count-has-reached-to-zero");
    }

    mpReachedThreads->AddValue(threadId);

    return --mUnreachedCount;
  }

  const std::string SynchronizeBarrier::ToString() const
  {
    stringstream out_str;
    out_str << "SynchroizeBarrier:"
      << " synchronized threads=" << mpSynchronizedThreads->ToSimpleString()
      << " reached threads="      << mpReachedThreads->ToSimpleString()
      << " unreached count="      << mUnreachedCount;
    return out_str.str();
  }

  void SynchronizeBarrier::ActivateSynchronizedThreads(SchedulingStrategy* pSchedulingStrategy) const
  {
    ConstraintSet synchronized_threads;
    pSchedulingStrategy->GetScheduledThreadIds(synchronized_threads);
    synchronized_threads.ApplyConstraintSet(GetSynchronizedThreads());

    vector<uint64> thread_vec;
    synchronized_threads.GetValues(thread_vec);
    auto activate_thread = [&pSchedulingStrategy](uint64 id) { pSchedulingStrategy->ActivateThread(id); };
    for_each(thread_vec.begin(), thread_vec.end(), activate_thread);
  }

  SynchronizeBarrierManager::~SynchronizeBarrierManager()
  {
    if (not mBarriers.empty()) {
      LOG(fail) << "{~SynchronizeBarrierManager} should not has any barrier" << endl;
      FAIL("should-not-has-any-barrier");
    }
  }

  void SynchronizeBarrierManager::Remove(SynchronizeBarrier* pBarrier)
  {
    auto find_iter = find(mBarriers.begin(), mBarriers.end(), pBarrier);
    if (find_iter == mBarriers.end()) {
      LOG(fail) << "{SynchronizeBarrierManager::Remove} remove the non-existing barrier " << pBarrier->ToString() << endl;
      FAIL("remove-non-existing-barrier");
    }

    mBarriers.erase(find_iter);
    LOG(info) << "{SynchronizeBarrierManager::Remove} remove " << pBarrier->ToString() << endl;
    delete pBarrier;
  }

  void SynchronizeBarrierManager::Add(SynchronizeBarrier* pBarrier)
  {
    auto find_iter = find(mBarriers.begin(), mBarriers.end(), pBarrier);
    if (find_iter != mBarriers.end()) {
      LOG(fail) << "{SynchronizeBarrierManager::Add} add the existing barrier " << pBarrier->ToString() << endl;
      FAIL("add-existing-barrier");
    }

    LOG(info) << "{SynchronizeBarrierManager::Add} add " << pBarrier->ToString() << endl;
    mBarriers.push_back(pBarrier);
  }

  SynchronizeBarrier* SynchronizeBarrierManager::Query(const ConstraintSet& rThreadIds) const
  {
    auto compare_barrier = [&rThreadIds](const SynchronizeBarrier* barrier) {
      return rThreadIds == barrier->GetSynchronizedThreads();
    };
    auto last_iter = mBarriers.end();

    auto find_iter = find_if(mBarriers.begin(), last_iter, compare_barrier);
    if (find_iter == last_iter) {
      return nullptr;
    }
    return *find_iter;
  }

  SynchronizeBarrier* SynchronizeBarrierManager::CreateSynchronizeBarrier(const ConstraintSet& rThreadIds, uint32 unreachedCount) const
  {
    return new SynchronizeBarrier(rThreadIds, unreachedCount);
  }

  void SynchronizeBarrierManager::GetParticipatingBarriers(uint32 threadId, vector<SynchronizeBarrier* >& rBarriers) const
  {
    auto gather_if = [&threadId, &rBarriers] (SynchronizeBarrier* barrier) {
      if (barrier->Contains(threadId)) { rBarriers.push_back(barrier); }
    };
    for_each(mBarriers.begin(), mBarriers.end(), gather_if);
  }

  bool SynchronizeBarrierManager::Validate(const SchedulingStrategy* pSchedulingStrategy, std::string& rErrMsg) const
  {
    if (pSchedulingStrategy == nullptr) {
      LOG(fail) << "{SynchronizeBarrierManager::Validate} SchedulingStrategy should not nullptr" << endl;
      FAIL("sheduling-strategy-should-not-nullptr");
    }

    bool valid = true;
    if (DetectDeadLock(pSchedulingStrategy)) {
      rErrMsg = "barrier order is not reasonalbe, the barriers have dead lock";
      valid = false;
    }
    return valid;
  }

  bool SynchronizeBarrierManager::DetectDeadLock(const SchedulingStrategy* pSchedulingStrategy) const
  {
    bool non_blocked_barrier = true;
    auto barriers = mBarriers;
    ConstraintSet scheduled_threads;
    pSchedulingStrategy->GetScheduledThreadIds(scheduled_threads);
    ConstraintSet free_threads;
    pSchedulingStrategy->GetActiveThreadIds(free_threads);

    while (non_blocked_barrier and not barriers.empty()) {
      for (auto iter = barriers.begin(); iter != barriers.end(); ++iter) {
        ConstraintSet usable_threads(free_threads);
        usable_threads.MergeConstraintSet((*iter)->GetReachedThreads());

        ConstraintSet synchronized_threads((*iter)->GetSynchronizedThreads());
        synchronized_threads.ApplyConstraintSet(scheduled_threads);

        non_blocked_barrier = usable_threads.ContainsConstraintSet(synchronized_threads);
        if (non_blocked_barrier) {
          barriers.erase(iter);
          free_threads.MergeConstraintSet(usable_threads);
          break;
        }
      }
    }

    for (const auto barrier : barriers) {
      LOG(info) << "{SynchronizeBarrierManager::DetectDeadLock} " << barrier->ToString() << endl;
    }
    return not barriers.empty();
  }

}
