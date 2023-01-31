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

#include <chrono>

#include "lest/lest.hpp"

#include "Log.h"
#include "Random.h"

using text = std::string;
using namespace Force;
using namespace std::chrono;

void gen_random_constraint_set(ConstraintSet& constrSet)
{
  const uint32 MAX_CONSTR_SET_SIZE = 10;
  const uint64 MAX_CONSTR_VALUE = 1000000000;

  Force::Random* rand_instance =  Force::Random::Instance();

  uint32 constr_set_size = rand_instance->Random32(0, MAX_CONSTR_SET_SIZE);
  for (uint32 i = 0; i < constr_set_size; i++) {
    uint32 constr_type = rand_instance->Random32(0, 1);

    if (constr_type == 0) {
      uint64 value = rand_instance->Random64(0, MAX_CONSTR_VALUE);
      constrSet.AddValue(value);
    } else {
      uint64 valueA = rand_instance->Random64(0, MAX_CONSTR_VALUE);
      uint64 valueB = rand_instance->Random64(0, MAX_CONSTR_VALUE);

      if (valueA < valueB) {
        constrSet.AddRange(valueA, valueB);
      } else if (valueA > valueB) {
        constrSet.AddRange(valueB, valueA);
      } else {
        constrSet.AddRange(valueA, valueB + 1);
      }
    }
  }
}

void gen_random_constraint_sets(std::vector<ConstraintSet>& constrSets, size_t genCount)
{
  constrSets.reserve(constrSets.size() + genCount);
  for (size_t i = 0; i < genCount; i++) {
    ConstraintSet constr_set;
    gen_random_constraint_set(constr_set);
    constrSets.push_back(constr_set);
  }
}

const lest::test specification[] = {

CASE( "performance tests for Constraint" ) {

  SETUP ( "setup Constraint" )  {
    std::vector<ConstraintSet> constr_sets;
    gen_random_constraint_sets(constr_sets, 30000);

    SECTION( "test performance of Constraint::AlignWithSize" ) {
      high_resolution_clock::time_point start_time = high_resolution_clock::now();

      for (ConstraintSet& constr_set : constr_sets) {
        constr_set.AlignWithSize(0xfffffffffffffffc, 4);
      }

      high_resolution_clock::time_point end_time = high_resolution_clock::now();
      duration<double> exec_time = duration_cast<duration<double>>(end_time - start_time);

#ifdef PERF_ASSERT
      EXPECT(exec_time.count() < 0.004);
#endif
    }

    SECTION( "test performance of Constraint::AlignOffsetWithSize" ) {
      high_resolution_clock::time_point start_time = high_resolution_clock::now();

      for (ConstraintSet& constr_set : constr_sets) {
        constr_set.AlignOffsetWithSize(0xfffffffffffffffc, 0, 4);
      }

      high_resolution_clock::time_point end_time = high_resolution_clock::now();
      duration<double> exec_time = duration_cast<duration<double>>(end_time - start_time);

#ifdef PERF_ASSERT
      EXPECT(exec_time.count() < 0.004);
#endif
    }
  }
}

};

int main( int argc, char * argv[] )
{
    Logger::Initialize();
    Force::Random::Initialize();
    Force::Random* rand_instance =  Force::Random::Instance();
    rand_instance->Seed(1);

    int ret = lest::run( specification, argc, argv );

    Force::Random::Destroy();
    Logger::Destroy();

    return ret;
}
