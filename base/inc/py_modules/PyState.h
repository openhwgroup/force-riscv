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
#ifndef Force_PyState_H
#define Force_PyState_H

#include <State.h>

#include <Generator.h>
#include <Scheduler.h>
#include <ThreadContext.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(State, mod) {
    py::class_<State>(mod, "State")
      .def(py::init([]() {
          ThreadContext thread_context;

          Scheduler* scheduler = Scheduler::Instance();
          Generator* generator = scheduler->LookUpGenerator(thread_context.GetThreadId());
          return new State(generator);
        }))
      .def("addMemoryStateElement", &State::AddMemoryStateElement, py::arg(), py::arg(), py::arg(), py::arg("aPriority") = 1, py::call_guard<ThreadContext>())
      .def("addMemoryStateElementsAsBytes", &State::AddMemoryStateElementsAsBytes, py::arg(), py::arg(), py::arg("aPriority") = 1, py::call_guard<ThreadContext>())
      .def("addRegisterStateElement", &State::AddRegisterStateElement, py::arg(), py::arg(), py::arg("aPriority") = 1, py::call_guard<ThreadContext>())
      .def("addSystemRegisterStateElementByField", &State::AddSystemRegisterStateElementByField, py::arg(), py::arg(), py::arg(), py::arg("aPriority") = 1, py::call_guard<ThreadContext>())
      .def("addVmContextStateElement", &State::AddVmContextStateElement, py::arg(), py::arg(), py::arg(), py::arg("aPriority") = 1, py::call_guard<ThreadContext>())
      .def("addPrivilegeLevelStateElement",
        [](State& rSelf, const EPrivilegeLevelTypeBaseType privLevelValue, cuint32 priority) {
          rSelf.AddPrivilegeLevelStateElement(EPrivilegeLevelType(privLevelValue), priority);
        },
        py::arg(), py::arg("aPriority") = 1, py::call_guard<ThreadContext>())
      .def("addPrivilegeLevelStateElementByName", &State::AddPrivilegeLevelStateElementByName, py::arg(), py::arg("aPriority") = 1, py::call_guard<ThreadContext>())
      .def("addPcStateElement", &State::AddPcStateElement, py::arg(), py::arg("aPriority") = 1, py::call_guard<ThreadContext>())
      .def("setDuplicateMode", &State::SetDuplicateMode, py::call_guard<ThreadContext>())
      ;
  }

}

#endif  // Force_PyState_H
