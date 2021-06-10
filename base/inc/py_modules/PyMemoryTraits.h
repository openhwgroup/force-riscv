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
#ifndef Force_PyMemoryTraits_H
#define Force_PyMemoryTraits_H

#include <MemoryTraits.h>

#include <Generator.h>
#include <MemoryManager.h>
#include <Scheduler.h>
#include <ThreadContext.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(MemoryTraits, mod) {
    mod
      .def("hasMemoryAttribute",
        [](const std::string& memAttr, cuint64 startAddr, cuint64 endAddr) {
          ThreadContext thread_context;

          Scheduler* scheduler = Scheduler::Instance();
          Generator* generator = scheduler->LookUpGenerator(thread_context.GetThreadId());
          MemoryManager* mam_manager = generator->GetMemoryManager();

          // TODO(Noah): Get the correct memory bank prior to pushing these changes.
          MemoryBank* mem_bank = mam_manager->GetMemoryBank(EMemBankTypeBaseType(0));

          MemoryTraitsManager* mem_traits_manager = mem_bank->GetMemoryTraitsManager();

          return mem_traits_manager->HasTrait(thread_context.GetThreadId(), memAttr, startAddr, endAddr);
        })
      ;
  }

}

#endif  // Force_PyMemoryTraits_H
