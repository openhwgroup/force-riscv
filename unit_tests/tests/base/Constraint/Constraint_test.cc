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
#include "Constraint.h"

#include "lest/lest.hpp"

#include "Log.h"
#include "Random.h"

/*
  NOTICE:

  This is how we can debug a big ConstraintSet bug in a real test, by Serialize the involved ConstraintSet objects in the appropriated location,
  reporduce here using the Deserialize method, then try to find the root cause and create a much shorter unit test to include in the Constraint_test files.

    Example code at the problem site:

    static uint32 debug_counter = 0;
    ++ debug_counter;
    LOG(notice) << "on counter: " << dec << debug_counter << endl;
    if ((debug_counter > 3335) && (debug_counter < 3340)) {
      // Dump ConstraintSet objects invloved before operation.
      mpConstraintSet->Serialize("Minuend-old", debug_counter, 8);
      mpCachedSubs->Serialize("Subtrahend-old", debug_counter, 8);
    }

    // This is the actual code that run into the problem
    mpConstraintSet->SubConstraintSet(*mpCachedSubs);

    if ((debug_counter > 3335) && (debug_counter < 3340)) {
      // Dump resulting ConstraintSet object after operation.
      mpConstraintSet->Serialize("Result-old", debug_counter, 8);
    }

    Construct unit test that re-construct the operation scenario by deserializing the data files:

    SECTION("Test on large ConstraintSet that are deserialized from files") {
      ConstraintSet minuend;
      minuend.Deserialize("Minuend", 3339);
      ConstraintSet subtrahend;
      subtrahend.Deserialize("Subtrahend", 3339);

      minuend.SubConstraintSet(subtrahend);
      // << "Result debug: " << endl << minuend.ToMultiLineStringDebug(4) << endl;
    }
*/

#undef CASE
#define CASE( name ) lest_CASE( specification(), name )

lest::tests & specification()
{
    static lest::tests tests;
    return tests;
}

int main( int argc, char * argv[] )
{
    Force::Logger::Initialize();
    Force::Random::Initialize();
    Force::Random* rand_instance = Force::Random::Instance();
    rand_instance->Seed(rand_instance->RandomSeed()); //Seed(0x12345678);
    std::cout << std::dec;
    int ret = lest::run( specification(), argc, argv );
    Force::Random::Destroy();
    Force::Logger::Destroy();
    return ret;
}
