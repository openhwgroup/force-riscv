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
#include <ProfilingUtility.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

using namespace std;

namespace Force {

  uint64 max_memoryXused()
  {
    rusage my_usage;

    int ret = getrusage(RUSAGE_SELF, &my_usage);
    if (0 != ret) {
      LOG(fail) << "{max_memory_used} failed to obtain max memory used : " << strerror(errno) << endl;
      FAIL("failed-getting-max-memory-used");
    }

    return my_usage.ru_maxrss;
  }

}
