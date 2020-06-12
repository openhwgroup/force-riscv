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
#include <ThreadGroupPartitioner.h>
#include <Constraint.h>
#include <Log.h>
#include <ThreadGroup.h>

#include <sstream>

using namespace std;

/*!
  \ file ThreadGroupPartitioner.cc
  \ brief Code for all thread group partitioners.
*/

namespace Force {
  DomainThreadPartitioner::DomainThreadPartitioner(EPartitionThreadPolicy policy) : ThreadGroupPartitioner(), mPolicy(policy), mThreads()
  {

  }

  DomainThreadPartitioner::~DomainThreadPartitioner()
  {
    for (auto thread : mThreads)
      delete thread;
  }

  void DomainThreadPartitioner::Setup(uint32 numChip, uint32 numCore, uint32 numThread)
  {
    auto total_thread_num = numChip * numCore * numThread;
    auto domain_num = 0ul;
    switch (mPolicy) {
    case EPartitionThreadPolicy::SameCore:
    case EPartitionThreadPolicy::DiffCore:
      domain_num = numCore * numChip;
      break;
    case EPartitionThreadPolicy::SameChip:
    case EPartitionThreadPolicy::DiffChip:
      domain_num = numChip;
      break;
    default:
      LOG(fail) <<"{DomainThreadPartitioner::Setup} Invalid thread policy :" << EPartitionThreadPolicy_to_string(mPolicy) << endl;
      FAIL("invalid-partition-thread-policy");
    }

    auto domain_size = total_thread_num / domain_num;
    for (auto n = 0ul; n < domain_num; n++) {
      mThreads.push_back(new ConstraintSet(n * domain_size, ((n + 1) * domain_size - 1ul)));
    }
  }

  void DomainThreadPartitioner::DoPartition(std::vector<ThreadGroup*>& threadGroups, const PartitionArgument* pPartitionArg)
  {
    uint32 group_id = 0;
    stringstream sname;
    for (auto thread : mThreads) {
      sname << EPartitionThreadPolicy_to_string(mPolicy) << dec << group_id;
      threadGroups.push_back(new ThreadGroup(group_id, sname.str(), *thread));
      sname.str("");
      group_id ++;
    }
  }

  const std::string DomainThreadPartitioner::ToString() const
  {
    return EPartitionThreadPolicy_to_string(mPolicy);
  }

  DiffDomainThreadPartitioner::DiffDomainThreadPartitioner(EPartitionThreadPolicy policy) : DomainThreadPartitioner(policy), mDiffThreads()
  {

  }

  DiffDomainThreadPartitioner::~DiffDomainThreadPartitioner()
  {
    for (auto thread : mDiffThreads)
      delete thread;
  }

  void DiffDomainThreadPartitioner::Setup(uint32 numChip, uint32 numCore, uint32 numThread)
  {
    DomainThreadPartitioner::Setup(numChip, numCore, numThread);

    bool has_thread = false;
    do {
      has_thread = false;
      auto diff_thread_constr = new ConstraintSet();

      for (auto thread : mThreads) {
        if (not thread->IsEmpty()) {
          auto thread_id = thread->ChooseValue();

          diff_thread_constr->AddValue(thread_id);
          thread->SubValue(thread_id);
          has_thread = true;
        }
      }
      if (has_thread)
        mDiffThreads.push_back(diff_thread_constr);
    } while (has_thread);

  }

  void DiffDomainThreadPartitioner::DoPartition(std::vector<ThreadGroup*>& threadGroups, const PartitionArgument* pPartitionArg)
  {
    uint32 group_id = 0;
    stringstream sname;
    for (auto thread : mDiffThreads) {
      sname << EPartitionThreadPolicy_to_string(mPolicy) << dec << group_id;
      threadGroups.push_back(new ThreadGroup(group_id, sname.str(), *thread));
      sname.str("");
      group_id ++;
    }
  }

  uint32 RandomThreadPartitioner::mGroupId = 0;

  RandomThreadPartitioner::RandomThreadPartitioner() : ThreadGroupPartitioner(), mpFreeThreads(nullptr)
  {

  }

  RandomThreadPartitioner::~RandomThreadPartitioner()
  {
    delete mpFreeThreads;
  }

  void RandomThreadPartitioner::Setup(uint32 numChip, uint32 numCore, uint32 numThread)
  {
    auto total_thread_num = numChip * numCore * numThread;
    mpFreeThreads = new ConstraintSet(0, total_thread_num - 1);
  }

  void RandomThreadPartitioner::DoPartition(std::vector<ThreadGroup*>& threadGroups, const PartitionArgument* pPartitionArg)
  {
    auto group_size = pPartitionArg->mGroupSize;
    auto group_num = pPartitionArg->mGroupNum;
    auto thread_num = mpFreeThreads->Size();

    if (group_size == 0 and group_num == 0) {
      LOG(fail) << "{RandomThreadPartitioner::DoPartition} both group size and group number are set to zero" << endl;
      FAIL("Group-size-and-group-number-zero");
    }
    if (group_size == 0)
      group_size = thread_num / group_num;
    else if (group_num == 0)
      group_num = thread_num / group_size;

    if (group_num * group_size > thread_num) {
      LOG(fail) << "{RandomThreadPartitioner::DoPartition} too large group paramater, size:"
                << group_size << ", number:" << group_num << endl;
      FAIL("Too-large-group-parameter");
    }

    ConstraintSet thread_constr;
    stringstream group_name;
    for (auto g_id = mGroupId; g_id < mGroupId + group_num; g_id ++) {
      group_name << "Random" << dec << g_id;
      for (auto sz = 0u; sz < group_size; sz ++) {
        auto thread = mpFreeThreads->ChooseValue();
        thread_constr.AddValue(thread);
        mpFreeThreads->SubValue(thread);
      }
      threadGroups.push_back(new ThreadGroup(g_id, group_name.str(), thread_constr));
      thread_constr.Clear();
      group_name.str("");
    }

    mGroupId += group_num;
  }

  const std::string RandomThreadPartitioner::ToString() const
  {
    return "Random";
  }

  ThreadPartitionerFactory* ThreadPartitionerFactory::mspPartitionFactory = nullptr;

  void ThreadPartitionerFactory::Initialize()
  {
    if (nullptr == mspPartitionFactory) {
      mspPartitionFactory = new ThreadPartitionerFactory();
    }
  }

  void ThreadPartitionerFactory::Destroy()
  {
    delete mspPartitionFactory;
    mspPartitionFactory = nullptr;
  }

  ThreadGroupPartitioner* ThreadPartitionerFactory::CreatePartitioner(EPartitionThreadPolicy policy)
  {
    switch (policy) {
    case EPartitionThreadPolicy::Random:
      return new RandomThreadPartitioner();
    case EPartitionThreadPolicy::SameCore:
    case EPartitionThreadPolicy::SameChip:
      return new DomainThreadPartitioner(policy);
    case EPartitionThreadPolicy::DiffChip:
    case EPartitionThreadPolicy::DiffCore:
      return new DiffDomainThreadPartitioner(policy);
    default:
      LOG(fail) <<"{ThreadPartitionerFactory::CreatePartitioner} Unhandled partition thread policy :" << EPartitionThreadPolicy_to_string(policy) << endl;
      FAIL("unhandled-partition-thread-policy");
    }
    return nullptr;
  }
}
