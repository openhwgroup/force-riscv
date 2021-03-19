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
#include <ThreadGroup.h>
#include <ThreadGroupPartitioner.h>
#include <Constraint.h>
#include <Log.h>

#include <algorithm>
#include <sstream>

using namespace std;

/*!
  \ file ThreadGroup.cc
  \ brief Code for thread group related.
*/

namespace Force {

  ThreadGroup::ThreadGroup(uint32 id) : mId(id), mJob(), mpThreads(nullptr)
  {

  }

  ThreadGroup::ThreadGroup(uint32 id, const std::string& job, const ConstraintSet& threads) : mId(id), mJob(job), mpThreads(nullptr)
  {
    mpThreads = threads.Clone();
  }

  ThreadGroup::~ThreadGroup()
  {
    delete mpThreads;
  }

  void ThreadGroup::SetThreadGroup(const std::string& job, const ConstraintSet& threads)
  {
    mJob = job;
    if (mpThreads != nullptr)
      delete mpThreads;

    mpThreads = threads.Clone();
  }

  const std::string ThreadGroup::ToString() const
  {
    stringstream sstream;
    sstream << "Group Id: " << dec << mId << ", job: " << mJob << ", threads: " << mpThreads->ToSimpleString();
    return sstream.str();
  }

  ThreadGroupModerator::ThreadGroupModerator(uint32 numChip, uint32 numCore, uint32 numThread) : mThreadGroups(), mThreadGroupIDs(), mpFreeThreads(nullptr), mThreadGroupPartitioners(), mThreadsNum(0u)
  {
    mpFreeThreads = new ConstraintSet();
    auto part_factory = ThreadPartitionerFactory::Instance();

    for (EPartitionThreadPolicyBaseType i = 0; i < EPartitionThreadPolicySize; i++) {
      mThreadGroupPartitioners.push_back(part_factory->CreatePartitioner(EPartitionThreadPolicy(i)));
    }

    Setup(numChip, numCore, numThread);
  }

  ThreadGroupModerator::~ThreadGroupModerator()
  {
    for (auto thread_group : mThreadGroups)
      delete thread_group.second;

    delete mpFreeThreads;

    for (auto part : mThreadGroupPartitioners)
      delete part;

  }

  void ThreadGroupModerator::Setup(uint32 numChip, uint32 numCore, uint32 numThread)
  {
    for (EPartitionThreadPolicyBaseType i = 0; i < EPartitionThreadPolicySize; i++) {
      mThreadGroupPartitioners[i]->Setup(numChip, numCore, numThread);
    }

    mThreadsNum = numChip * numCore * numThread;
    mpFreeThreads->AddRange(0u, mThreadsNum - 1);

  }

  void ThreadGroupModerator::PartitionThreadGroup(EPartitionThreadPolicy policy, const PartitionArgument* pPartitionArg)
  {
    vector<ThreadGroup*> thread_groups;

    LOG(info) << "{ThreadGroupModerator::PartitionThreadGroup} do partition " << EPartitionThreadPolicy_to_string(policy) << endl;

    mThreadGroupPartitioners[EPartitionThreadPolicyBaseType(policy)]->DoPartition(thread_groups, pPartitionArg);

    for (auto group : thread_groups) {
      //<< " Partitioned thread group " << group->ToString() << endl;
      MapThreads(group);
    }

  }

  void ThreadGroupModerator::SetThreadGroup(uint32 id, const string& job, const ConstraintSet& threads)
  {
    ConstraintSet total_thread_constr(0, mThreadsNum - 1);
    if (not total_thread_constr.ContainsConstraintSet(threads)) {
      LOG(fail) << "{ThreadGroupModerator::SetThreadGroup} Invalid thread constraint:" << threads.ToSimpleString() << endl;
      FAIL("Invalid-thread-constriant");
    }

    auto iter = mThreadGroups.find(id);
    if (iter == mThreadGroups.end()) {
      LOG(fail) << "{ThreadGroupModerator::SetThreadGroup} Not find group id: 0x" << hex << id << endl;
      FAIL("Not-find-group-id");
    }

    // UnmapThreads will invalidate the iterator, so retrieve the thread group pointer before then
    ThreadGroup* group = iter->second;
    UnmapThreads(group);

    group->SetThreadGroup(job, threads);
    MapThreads(group);
  }

  void ThreadGroupModerator::QueryThreadGroup(uint32 groupId, vector<ThreadGroup* >& threadGroups) const
  {
    if (groupId == -1u) { // all thread groups
      transform(mThreadGroups.cbegin(), mThreadGroups.cend(), back_inserter(threadGroups),
        [](const pair<uint32, ThreadGroup*>& rThreadGroupEntry) { return rThreadGroupEntry.second; });
    }
    else {
      auto it = mThreadGroups.find(groupId);
      if (it != mThreadGroups.end())
        threadGroups.push_back(it->second);
    }

  }

  void ThreadGroupModerator::QueryThreadGroup(const std::string& job, std::vector<ThreadGroup* >& threadGroups) const
  {
    for (const auto item : mThreadGroups)
    {
      if (item.second->GetJob() == job) {
        threadGroups.push_back(item.second);
      }
    }
  }

  void ThreadGroupModerator::GetFreeThreads(vector<uint32>& freeThreads) const
  {
    vector<uint64> free_threads;
    mpFreeThreads->GetValues(free_threads);

    transform(free_threads.cbegin(), free_threads.cend(), back_inserter(freeThreads),
      [](cuint64 threadId) { return static_cast<uint32>(threadId); });
  }

  uint32 ThreadGroupModerator::GetThreadGroupId(uint32 threadId) const
  {
    auto iter = mThreadGroupIDs.find(threadId);
    if (iter == mThreadGroupIDs.end()) {
      LOG(notice) << "{ThreadGroupModerator::GetThreadGroupId} no group as free thread id: 0x"<< hex << threadId << endl;
      return -1u;
    }

    return iter->second;
  }

  void ThreadGroupModerator::MapThreads(const ThreadGroup* group)
  {
    auto group_id = group->GetId();
    if (mThreadGroups.find(group_id) != mThreadGroups.end()) {
      LOG(fail) << "{ThreadGroupModerator::MapThreads} Duplicated thread group id: 0x" << hex << group_id << endl;
      FAIL("Duplicated-thread-group");
    }
    mThreadGroups[group_id] = const_cast<ThreadGroup* >(group);

    auto used_threads_constr = group->GetThreads();
    mpFreeThreads->SubConstraintSet(*used_threads_constr);

    vector<uint64> used_threads;
    used_threads_constr->GetValues(used_threads);
    for (auto thread_id : used_threads) {
      if (mThreadGroupIDs.find(thread_id) != mThreadGroupIDs.end()) {
        LOG(fail) << "{ThreadGroupModerator::MapThreads} Duplicated thread id: 0x" << hex << thread_id << endl;
        FAIL("Duplicated-thread");
      }
      mThreadGroupIDs[thread_id] = group_id;
    }
  }

  void ThreadGroupModerator::UnmapThreads(const ThreadGroup* group)
  {
    auto id = group->GetId();
    mThreadGroups.erase(id);

    auto thread_constr = group->GetThreads();
    mpFreeThreads->MergeConstraintSet(*thread_constr);

    vector<uint64> threads;
    thread_constr->GetValues(threads);
    for (auto thread : threads)
      mThreadGroupIDs.erase(thread);
  }

}
