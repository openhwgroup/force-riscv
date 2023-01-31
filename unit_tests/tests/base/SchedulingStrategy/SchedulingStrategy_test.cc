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
#include "SchedulingStrategy.h"

#include "lest/lest.hpp"

#include "Constraint.h"
#include "Log.h"
#include "Random.h"

using namespace std;
using namespace Force;

using text = std::string;

const lest::test specification[] = {
    
CASE( "test case for ShuffledRoundRobinSchedulingStrategy class" ) {

  SETUP( "setup scheduling strategy class" ) {
    ShuffledRoundRobinSchedulingStrategy* strategy_ptr = nullptr;
    strategy_ptr = new ShuffledRoundRobinSchedulingStrategy();
  
    SECTION( "test new class with no thread." ) {
      EXPECT( strategy_ptr->CurrentThreadId() == -1u );
      EXPECT( strategy_ptr->ActiveThreadCount() == 0u );
    }

    // add base thread ids.
    const uint32 scheduled_threads_num = 4;
    for (uint32 i = 0; i < scheduled_threads_num; ++ i) {
      strategy_ptr->AddThreadId(i);
    }
  
    SECTION( "test base add and remove thread opertions" ) {
      strategy_ptr->RemoveThreadId(2);
      EXPECT( strategy_ptr->ActiveThreadCount() == 3u );
      strategy_ptr->AddThreadId(2);
      EXPECT( strategy_ptr->ActiveThreadCount() == 4u );
      strategy_ptr->AddThreadId(5);
      EXPECT( strategy_ptr->ActiveThreadCount() == 5u );
      strategy_ptr->RemoveThreadId(5);
      EXPECT( strategy_ptr->ActiveThreadCount() == 4u );
    }

    SECTION( "test remove non-existing thread" ) {
      uint32 old_num = strategy_ptr->ActiveThreadCount();
      strategy_ptr->RemoveThreadId(8);
      EXPECT( strategy_ptr->ActiveThreadCount() == old_num);
    }

    SECTION( "test re-add and re-remove a thread" ) {
      uint32 old_num = strategy_ptr->ActiveThreadCount();
      strategy_ptr->AddThreadId(8);
      EXPECT( strategy_ptr->ActiveThreadCount() == old_num + 1);
      strategy_ptr->AddThreadId(8);
      EXPECT( strategy_ptr->ActiveThreadCount() == old_num + 1);
      strategy_ptr->RemoveThreadId(8);
      EXPECT( strategy_ptr->ActiveThreadCount() == old_num);
      strategy_ptr->RemoveThreadId(8);
      EXPECT( strategy_ptr->ActiveThreadCount() == old_num);
    }

    SECTION( "test one round normal scheduling threads" ) {
      uint32 scheduled_threads_num = strategy_ptr->ActiveThreadCount();
      ConstraintSet scheduled_threads;
      strategy_ptr->RefreshSchedule();
      
      for (uint32 i = 0; i < scheduled_threads_num; ++ i) {
        uint32 t_id = strategy_ptr->CurrentThreadId();
        EXPECT( not scheduled_threads.ContainsValue(t_id) );
        scheduled_threads.AddValue(t_id);
        strategy_ptr->NextThread();
      }
      EXPECT( scheduled_threads.Size() == scheduled_threads_num );
    }

    SECTION( "test base lock and unlock operations" ) {
      strategy_ptr->RefreshSchedule();
      uint32 locked_thread_id = strategy_ptr->CurrentThreadId();
     
      strategy_ptr->LockSchedule(locked_thread_id);
      strategy_ptr->NextThread();
      EXPECT( strategy_ptr->CurrentThreadId() == locked_thread_id );

      strategy_ptr->UnlockSchedule(locked_thread_id);
      strategy_ptr->NextThread();
      EXPECT( strategy_ptr->CurrentThreadId() != locked_thread_id );
    }

    SECTION( "test nest lock and unlock operations" ) {
      strategy_ptr->RefreshSchedule();
      uint32 locked_thread_id = strategy_ptr->CurrentThreadId();
      
      strategy_ptr->LockSchedule(locked_thread_id);
      strategy_ptr->NextThread();
      EXPECT( strategy_ptr->CurrentThreadId() == locked_thread_id );

      strategy_ptr->LockSchedule(locked_thread_id);
      strategy_ptr->NextThread();
      EXPECT( strategy_ptr->CurrentThreadId() == locked_thread_id );

      strategy_ptr->UnlockSchedule(locked_thread_id);
      strategy_ptr->NextThread();
      EXPECT( strategy_ptr->CurrentThreadId() == locked_thread_id );

      strategy_ptr->UnlockSchedule(locked_thread_id);
      strategy_ptr->NextThread();
      EXPECT( strategy_ptr->CurrentThreadId() != locked_thread_id );
    }
    
    SECTION( "test lock and unlock functions are not used in pairs" ) {
      uint32 current_thread_id = strategy_ptr->CurrentThreadId();
      EXPECT_FAIL( strategy_ptr->UnlockSchedule(current_thread_id), "inconsistent-thread-unlocking-state" );
      
      strategy_ptr->LockSchedule(current_thread_id);
      EXPECT_FAIL( strategy_ptr->UnlockSchedule(current_thread_id + 1), "inconsistent-thread-unlocking-id" );
    }

    SECTION( "test normal activate and deactivate threads" ) {
      strategy_ptr->RefreshSchedule();
      uint32 deactivate_thread_id = strategy_ptr->CurrentThreadId();
      strategy_ptr->DeactivateThread(deactivate_thread_id);
      ConstraintSet scheduled_threads;
      for (uint32 i = 0; i < 10; ++i) {
        strategy_ptr->NextThread();
        uint32 thread_id = strategy_ptr->CurrentThreadId();
        scheduled_threads.AddValue(thread_id);
      }
      EXPECT( scheduled_threads.ContainsValue(deactivate_thread_id) == false );
      strategy_ptr->ActivateThread(deactivate_thread_id);
      for (uint32 i = 0; i < 10; ++i) {
        strategy_ptr->NextThread();
        uint32 thread_id = strategy_ptr->CurrentThreadId();
        scheduled_threads.AddValue(thread_id);
      }
      EXPECT( scheduled_threads.ContainsValue(deactivate_thread_id) == true );
    }

    SECTION( "test activate and deactivate non existing threads" ) {
      uint32 non_existing_thread_id = 8;
      EXPECT_FAIL( strategy_ptr->ActivateThread(non_existing_thread_id), "thread-not-existing");
      EXPECT_FAIL( strategy_ptr->DeactivateThread(non_existing_thread_id), "inconsistent-thread-active-state");
    }
    
    SECTION( "test duplicate activate and deactivate existing threads" ) {
      uint32 existing_thread_id = 0;
      EXPECT_FAIL( strategy_ptr->ActivateThread(existing_thread_id), "inconsistent-thread-active-state");
      strategy_ptr->DeactivateThread(existing_thread_id);
      EXPECT_FAIL( strategy_ptr->DeactivateThread(existing_thread_id), "inconsistent-thread-active-state");
    }
    
    SECTION( "test activate and deactivate threads in Locked state" ) {
      strategy_ptr->RefreshSchedule();
      uint32 locked_thread_id = strategy_ptr->CurrentThreadId();
      strategy_ptr->LockSchedule(locked_thread_id);
      EXPECT_FAIL( strategy_ptr->DeactivateThread(locked_thread_id), "inconsistent-thread-scheduler-state");
    }
  }
}};

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
