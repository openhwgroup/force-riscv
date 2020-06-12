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
#ifndef Force_ProfilingUtility_H
#define Force_ProfilingUtility_H

#include <Defines.h>
#include <Log.h>
#include <chrono>

namespace Force {

// Wrap a function call in the LOG_FUNCTION_TIME macro in order to log the execution time of the function call. For
// example, "LOG_FUNCTION_TIME(TEST_INFO, constr_set->ApplyConstraintSet(rOther_constr_set));" will log the execution
// time for each invocation of wrapped call to ApplyConstraintSet(). Note that because this macro writes to the log,
// making a lot of calls to it can slow down the overall execution time, although the execution time logged for each
// function call should be unaffected.
#define LOG_FUNCTION_TIME(log_handle, function_call)  \
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();  \
  function_call;  \
  std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();  \
  std::chrono::duration<double> elapsed_time = end_time - start_time;  \
  log_handle << "Elapsed time for call to " << #function_call << ": " << elapsed_time.count() << std::endl;

  uint64 max_memory_used(); //!< Return max memory used by the program.

}

#endif
