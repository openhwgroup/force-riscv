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
#ifndef Force_PyVirtualMemory_H
#define Force_PyVirtualMemory_H

#include "pybind11/pybind11.h"

#include "Generator.h"
#include "PagingInfo.h"
#include "Scheduler.h"
#include "ThreadContext.h"

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(VirtualMemory, mod) {
    mod
      .def("getPagingMode",
        []() {
          ThreadContext thread_context;

          Scheduler* scheduler = Scheduler::Instance();
          Generator* generator = scheduler->LookUpGenerator(thread_context.GetThreadId());
          const PagingInfo* paging_info = generator->GetPagingInfo();
          return paging_info->GetPagingMode();
        })
      ;
  }

}

#endif  // Force_PyVirtualMemory_H
