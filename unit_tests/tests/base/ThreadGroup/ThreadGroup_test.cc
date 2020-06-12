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
#include <lest/lest.hpp>
#include <Log.h>
#include <Random.h>
//------------------------------------------------
// include necessary header files here
//------------------------------------------------
#include <Enums.h>
#include <ThreadGroup.h>
#include <Constraint.h>
#include <ThreadGroupPartitioner.h>

#include <vector>

using text = std::string;

const lest::test specification[] = {

CASE( "Test Thread Group Moderator APIs" ) {
   SETUP( "Set up class" ) {
     using namespace Force;
     ThreadGroupModerator tg_mod(2,2,4); // 2 chip, 2 core and 4 thread

     SECTION ("Test default behavior") {
      EXPECT(tg_mod.GetThreadGroupId(0) == -1u);
       
      std::vector<uint32> free_threads;
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 16u);
      for (uint32 i = 0u; i < 16u; i++)
        EXPECT(free_threads[i] == i);

      std::vector<ThreadGroup* > thread_groups;
      tg_mod.QueryThreadGroup(-1u, thread_groups);
      EXPECT(thread_groups.size() == 0u);
     }
     
     SECTION ("Test partition: same chip") {
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::SameChip);
      
      std::vector<ThreadGroup* > thread_groups;
      tg_mod.QueryThreadGroup(-1, thread_groups);
      EXPECT(thread_groups.size() == 2u);
      EXPECT(thread_groups[0]->GetThreads()->ToSimpleString() == "0x0-0x7");
      EXPECT(thread_groups[1]->GetThreads()->ToSimpleString() == "0x8-0xf");

      std::vector<uint32> free_threads;
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 0u);

      EXPECT(tg_mod.GetThreadGroupId(7u) == 0u); // thread id 7 belongs to group 0
      EXPECT(tg_mod.GetThreadGroupId(8u) == 1u); // thread id 8 belongs to group 1
     }
     
     SECTION ("Test partition: same core") {
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::SameCore);
      
      std::vector<ThreadGroup* > thread_groups;
      tg_mod.QueryThreadGroup(-1, thread_groups);
      EXPECT(thread_groups.size() == 4u);
      EXPECT(thread_groups[0]->GetThreads()->ToSimpleString() == "0x0-0x3");
      EXPECT(thread_groups[1]->GetThreads()->ToSimpleString() == "0x4-0x7");
      EXPECT(thread_groups[2]->GetThreads()->ToSimpleString() == "0x8-0xb");
      EXPECT(thread_groups[3]->GetThreads()->ToSimpleString() == "0xc-0xf");

      std::vector<uint32> free_threads;
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 0u);

      EXPECT(tg_mod.GetThreadGroupId(3u) == 0u); // thread id 0 belongs to group 0
      EXPECT(tg_mod.GetThreadGroupId(7u) == 1u); // thread id 7 belongs to group 1
      EXPECT(tg_mod.GetThreadGroupId(8u) == 2u); // thread id 8 belongs to group 2
      EXPECT(tg_mod.GetThreadGroupId(15u) == 3u); // thread id 15 belongs to group 3
     }

     SECTION ("Test partition: diff chip") {
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::DiffChip);
      
      std::vector<ThreadGroup* > thread_groups;
      tg_mod.QueryThreadGroup(-1, thread_groups);
      EXPECT(thread_groups.size() == 8u);
      
      for (unsigned i = 0; i < 8u ; i++) {
        auto thread_constr = thread_groups[i]->GetThreads();
        LOG(notice) << "Diff chip thread constraint:" << thread_constr->ToSimpleString() << ", group id: " << thread_groups[i]->GetId() << std::endl;
        EXPECT(thread_constr->Size() == 2u);
        std::vector<uint64> threads;
        thread_constr->GetValues(threads);
        if (threads[0] < 8u) {
          EXPECT(threads[1] >=8u);
          EXPECT(threads[1] <= 15u);
        }
        else
          EXPECT(threads[1] < 8u);
      }
      
      std::vector<uint32> free_threads;
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 0u);

     }
     
     SECTION ("Test partition: diff core") {
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::DiffCore);
      
      std::vector<ThreadGroup* > thread_groups;
      tg_mod.QueryThreadGroup(-1, thread_groups);
      EXPECT(thread_groups.size() == 4u);
      
      for (unsigned i = 0; i < 4u ; i++) {
        auto thread_constr = thread_groups[i]->GetThreads();
        LOG(notice) << "Diff core thread constraint:" << thread_constr->ToSimpleString() << ", group id: " << thread_groups[i]->GetId() << std::endl;
        EXPECT(thread_constr->Size() == 4u);
        std::vector<uint64> threads;
        thread_constr->GetValues(threads);
          EXPECT(threads[0] < 4u);
          EXPECT(threads[1] >=4u);
          EXPECT(threads[1] < 8u);
          EXPECT(threads[2] >= 8u);
          EXPECT(threads[2] < 12u);
          EXPECT(threads[3] >= 12u);
          EXPECT(threads[3] < 16u);
      }
      
      std::vector<uint32> free_threads;
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 0u);

     }

     SECTION("Test partition: random all") {
      PartitionArgument arg(1); // one group
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::Random, &arg);

      std::vector<ThreadGroup* > thread_groups;
      tg_mod.QueryThreadGroup(-1, thread_groups);
      EXPECT(thread_groups.size() == 1u);
      
      auto thread_constr = thread_groups[0]->GetThreads();
      EXPECT(thread_constr->ToSimpleString() == "0x0-0xf");

      std::vector<uint32> free_threads;
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 0u);
     }

     SECTION("Test partition: random gradual") {
      PartitionArgument arg0(2, 2); // 2 groups, 2 threads in a group
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::Random, &arg0);

      std::vector<uint32> free_threads;
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 12u);
      
      PartitionArgument arg1(4, 1); // 4 groups, 1 thread in a group
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::Random, &arg1);
      
      free_threads.clear();
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 8u);
      
      PartitionArgument arg2(2); // 2 groups
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::Random, &arg2);
      
      free_threads.clear();
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 0u);
      
      std::vector<ThreadGroup* > thread_groups;
      tg_mod.QueryThreadGroup(-1, thread_groups);
      EXPECT(thread_groups.size() == 8u);

      EXPECT(thread_groups[0]->GetThreads()->Size() == 2u);
      EXPECT(thread_groups[2]->GetThreads()->Size() == 1u);
      EXPECT(thread_groups[6]->GetThreads()->Size() == 4u);

      for (auto thread_group : thread_groups) {
        LOG(notice) << "Random gradual group id:" << std::dec << thread_group->GetId() << ", job: " << thread_group->GetJob() 
        << ", threads: " << thread_group->GetThreads()->ToSimpleString() << std::endl;
      }

     }

     SECTION("Test Set thread group") {
      PartitionArgument arg0(1, 2); // 1 groups with 2 threads
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::Random, &arg0);
      
      std::vector<ThreadGroup*> thread_groups;
      tg_mod.QueryThreadGroup(-1, thread_groups);
      EXPECT(thread_groups.size() == 1u);
      
      auto g_id = thread_groups[0]->GetId();
      tg_mod.SetThreadGroup(g_id, "Set0", ConstraintSet("2-5"));
      EXPECT(thread_groups[0]->GetThreads()->ToSimpleString() == "0x2-0x5");

      std::vector<uint32> free_threads;
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 12u);
      EXPECT(tg_mod.GetThreadGroupId(0x2) == g_id);

      tg_mod.SetThreadGroup(g_id, "Set1", ConstraintSet("8-10"));
      EXPECT(thread_groups[0]->GetThreads()->ToSimpleString() == "0x8-0xa");
      free_threads.clear();
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 13u);
      EXPECT(tg_mod.GetThreadGroupId(0x8) == g_id);
      EXPECT(tg_mod.GetThreadGroupId(0xa) == g_id);
      EXPECT(tg_mod.GetThreadGroupId(0x2) == -1u);
      EXPECT(tg_mod.GetThreadGroupId(0x5) == -1u);
     }

     SECTION("Test set thread group with multiple groups") {
      tg_mod.PartitionThreadGroup(EPartitionThreadPolicy::SameChip);

      std::vector<ThreadGroup*> thread_groups;
      tg_mod.QueryThreadGroup(-1, thread_groups);

      ThreadGroup* thread_group_0 = thread_groups[0];
      tg_mod.SetThreadGroup(thread_group_0->GetId(), "Set0_0", ConstraintSet("1,3,5"));

      const ConstraintSet* group_0_thread_constr = thread_group_0->GetThreads();
      EXPECT(group_0_thread_constr->ToSimpleString() == "0x1,0x3,0x5");
      std::vector<uint64> group_0_threads;
      group_0_thread_constr->GetValues(group_0_threads);
      for (uint64 thread : group_0_threads) {
        EXPECT(tg_mod.GetThreadGroupId(thread) == thread_group_0->GetId());
      }

      ThreadGroup* thread_group_1 = thread_groups[1];
      tg_mod.SetThreadGroup(thread_group_1->GetId(), "Set1", ConstraintSet("2,4,6-10"));

      const ConstraintSet* group_1_thread_constr = thread_group_1->GetThreads();
      EXPECT(group_1_thread_constr->ToSimpleString() == "0x2,0x4,0x6-0xa");
      std::vector<uint64> group_1_threads;
      group_1_thread_constr->GetValues(group_1_threads);
      for (uint64 thread : group_1_threads) {
        EXPECT(tg_mod.GetThreadGroupId(thread) == thread_group_1->GetId());
      }

      tg_mod.SetThreadGroup(thread_group_0->GetId(), "Set0_1", ConstraintSet("3,11,13-14"));

      group_0_thread_constr = thread_group_0->GetThreads();
      EXPECT(group_0_thread_constr->ToSimpleString() == "0x3,0xb,0xd-0xe");
      group_0_threads.clear();
      group_0_thread_constr->GetValues(group_0_threads);
      for (uint64 thread : group_0_threads) {
        EXPECT(tg_mod.GetThreadGroupId(thread) == thread_group_0->GetId());
      }

      std::vector<uint32> free_threads;
      tg_mod.GetFreeThreads(free_threads);
      EXPECT(free_threads.size() == 5u);
      for (uint32 free_thread : free_threads) {
        EXPECT(tg_mod.GetThreadGroupId(free_thread) == -1u);
      }
     }
   }
}

};

int main( int argc, char * argv[] )
{
  Force::Logger::Initialize();
  Force::Random::Initialize();
  int ret = lest::run( specification, argc, argv );
  Force::Random::Destroy();
  Force::Logger::Destroy();
  return ret;

}
