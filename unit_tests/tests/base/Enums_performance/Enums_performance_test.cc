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

#include <Enums.h>
#include <chrono>

using text = std::string;
using namespace Force;
using namespace std::chrono;

const lest::test specification[] = {

// Build with "make PERF_ASSERT=1" to enable performance threshhold assertions.
CASE( "performance tests for enums" ) {

  SETUP ( "setup enums" )  {

    SECTION( "test performance of enum to string conversion" ) {
      high_resolution_clock::time_point start_time = high_resolution_clock::now();

      for (int i = 0; i < 1500000; i++) {
        // Alternate to mitigate branch prediction effects.
        if (i % 2) {
          ESequenceType_to_string(ESequenceType::InitializeAddrTables);
        } else {
          ESequenceType_to_string(ESequenceType::ConfirmSpace);
        }
      }

      high_resolution_clock::time_point end_time = high_resolution_clock::now();
      duration<double> exec_time = duration_cast<duration<double>>(end_time - start_time);

#ifdef PERF_ASSERT
      EXPECT(exec_time.count() < 0.1);
#endif
    }

    SECTION( "test performance of string to enum conversion" ) {
      high_resolution_clock::time_point start_time = high_resolution_clock::now();

      for (int i = 0; i < 500000; i++) {
        // Alternate to mitigate branch prediction effects.
        if (i % 2) {
          string_to_ESequenceType("InitializeAddrTables");
        } else {
          string_to_ESequenceType("ConfirmSpace");
        }
      }

      high_resolution_clock::time_point end_time = high_resolution_clock::now();
      duration<double> exec_time = duration_cast<duration<double>>(end_time - start_time);

#ifdef PERF_ASSERT
      EXPECT(exec_time.count() < 0.1);
#endif
    }

    SECTION( "test performance of failed string to enum conversion" ) {
      high_resolution_clock::time_point start_time = high_resolution_clock::now();

      bool okay = false;
      for (int i = 0; i < 900000; i++) {
        try_string_to_ESequenceType("__non_matching_string__", okay);
      }

      high_resolution_clock::time_point end_time = high_resolution_clock::now();
      duration<double> exec_time = duration_cast<duration<double>>(end_time - start_time);

#ifdef PERF_ASSERT
      EXPECT(exec_time.count() < 0.1);
#endif
    }
  }
}

};

int main( int argc, char * argv[] )
{
    Logger::Initialize();
    int ret = lest::run( specification, argc, argv );
    Logger::Destroy();
    return ret;
}
