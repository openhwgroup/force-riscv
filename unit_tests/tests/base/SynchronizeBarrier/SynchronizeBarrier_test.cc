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

#include <SynchronizeBarrier.h>
#include <SchedulingStrategy.h>
#include <Constraint.h>
#include <Random.h>

using namespace std;
using namespace Force;

using text = std::string;

const lest::test specification[] = {
    
CASE( "test case for SynchronizeBarrier class" ) {

  SETUP( "setup SynchronizeBarrier" ) {

    SECTION( "test base functions of SynchronizeBarrier class" ) {
      ConstraintSet synchronized_threads(0, 7);
      uint32 non_reached_count = synchronized_threads.Size();
      auto barrier_ptr = new SynchronizeBarrier(synchronized_threads, non_reached_count);
      std::unique_ptr<SynchronizeBarrier> barrier_storage(barrier_ptr);

      uint32 thread = synchronized_threads.ChooseValue();
      EXPECT( barrier_ptr->Contains(thread) == true );
      EXPECT( barrier_ptr->Contains(8) == false );

      uint32 reached_thread = 6; 
      EXPECT( barrier_ptr->AddReachedThread(reached_thread) == 7u );

      const ConstraintSet& reached_threads = barrier_ptr->GetReachedThreads();
      EXPECT( reached_threads.Size() == 1u );
      EXPECT( reached_threads.OnlyValue() == reached_thread );
     
      uint32 second_reached_thread = 3;
      EXPECT( barrier_ptr->AddReachedThread(second_reached_thread) == 6u );
      EXPECT( reached_threads.Size() == 2u );
      EXPECT( reached_threads.ContainsValue(reached_thread) );
      EXPECT( reached_threads.ContainsValue(second_reached_thread) );

      EXPECT_FAIL( barrier_ptr->AddReachedThread(8), "not-include-thread" );
    }
  }
},

CASE( "test case for SynchronizeBarrierManager class" ) {

  SETUP( "setup SynchronizeBarrierManager class" ) {

    SECTION( "test base functions of SynchronizeBarrierManager class" ) {
      SynchronizeBarrierManager* manager = nullptr;
      manager = new SynchronizeBarrierManager();
      std::unique_ptr<SynchronizeBarrierManager> manager_storage(manager);

      ConstraintSet synchronized_threads(0, 7);
      uint32 non_reached_count = synchronized_threads.Size();
      auto barrier_ptr = manager->CreateSynchronizeBarrier(synchronized_threads, non_reached_count);

      manager->Add(barrier_ptr);
      EXPECT( manager->Query(synchronized_threads) == barrier_ptr );
      EXPECT_FAIL( manager->Add(barrier_ptr), "add-existing-barrier" );
      
      ConstraintSet second_synchronized_threads;
      second_synchronized_threads.AddValue(0);
      second_synchronized_threads.AddRange(3, 5);
      non_reached_count = second_synchronized_threads.Size();
      auto second_barrier_ptr = manager->CreateSynchronizeBarrier(second_synchronized_threads, non_reached_count);

      EXPECT( manager->Query(second_synchronized_threads) == nullptr );
      EXPECT_FAIL( manager->Remove(second_barrier_ptr), "remove-non-existing-barrier" );

      manager->Add(second_barrier_ptr);
      std::vector<SynchronizeBarrier* > barriers;
      manager->GetParticipatingBarriers(0, barriers);
      EXPECT( barriers.size() == 2u );

      manager->Remove(barrier_ptr);
      EXPECT( manager->Query(synchronized_threads) == nullptr );

      std::string msg;
      EXPECT_FAIL( manager->Validate(nullptr, msg), "sheduling-strategy-should-not-nullptr" );
      manager->Remove(second_barrier_ptr);
    }
    
    SECTION( "test dead lock of current barriers" ) {
      SynchronizeBarrierManager* manager = nullptr;
      manager = new SynchronizeBarrierManager();
      std::unique_ptr<SynchronizeBarrierManager> manager_storage(manager);
     
      ConstraintSet synchronized_threads_g1;
      synchronized_threads_g1.AddValue(1);
      synchronized_threads_g1.AddValue(3);
      synchronized_threads_g1.AddValue(5);

      ConstraintSet synchronized_threads_g2;
      synchronized_threads_g2.AddValue(3);
      synchronized_threads_g2.AddValue(4);
      synchronized_threads_g2.AddValue(5);
      
      auto barrier_ptr_g1 = manager->CreateSynchronizeBarrier(synchronized_threads_g1, 3);
      auto barrier_ptr_g2 = manager->CreateSynchronizeBarrier(synchronized_threads_g2, 3);

      auto strategy_ptr = new ShuffledRoundRobinSchedulingStrategy();
      std::unique_ptr<ShuffledRoundRobinSchedulingStrategy> strategy_storage(strategy_ptr);

      const uint32 scheduled_threads_num = 8;
      for (uint32 i = 0; i < scheduled_threads_num; ++ i) {
        strategy_ptr->AddThreadId(i);
      }

      manager->Add(barrier_ptr_g1);
      manager->Add(barrier_ptr_g2);

      barrier_ptr_g1->AddReachedThread(1);
      barrier_ptr_g1->AddReachedThread(5);
      strategy_ptr->DeactivateThread(1);
      strategy_ptr->DeactivateThread(5);
      
      barrier_ptr_g2->AddReachedThread(3);
      barrier_ptr_g2->AddReachedThread(4);
      strategy_ptr->DeactivateThread(3);
      strategy_ptr->DeactivateThread(4);
      
      std::string err_msg;
      EXPECT( manager->Validate(strategy_ptr, err_msg) == false );
      EXPECT( err_msg == "barrier order is not reasonalbe, the barriers have dead lock" );
      manager->Remove(barrier_ptr_g1);
      manager->Remove(barrier_ptr_g2);
    }
  }
}
};

int main( int argc, char * argv[] )
{
  Force::Logger::Initialize();
  Force::Random::Initialize();
  if (int failures = lest::run( specification, argc, argv )) {
      return failures;
  }
  Force::Random::Destroy();
  Force::Logger::Destroy();
  return std::cout << "All tests passed\n", EXIT_SUCCESS;
}
