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
#ifndef Force_PyStateElement_H
#define Force_PyStateElement_H

#include <StateElement.h>

#include <ThreadContext.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(StateElement, mod) {
    py::class_<StateElement>(mod, "StateElement")
      .def("getName", &StateElement::GetName, py::call_guard<ThreadContextNoAdvance>())
      .def("getStateElementType", &StateElement::GetStateElementType, py::call_guard<ThreadContextNoAdvance>())
      .def("getValues", &StateElement::GetValues, py::call_guard<ThreadContextNoAdvance>())
      .def("getMasks", &StateElement::GetMasks, py::call_guard<ThreadContextNoAdvance>())
      ;

    py::class_<MemoryStateElement, StateElement>(mod, "MemoryStateElement")
      .def("getStartAddress", &MemoryStateElement::GetStartAddress, py::call_guard<ThreadContextNoAdvance>())
      ;

    py::class_<RegisterStateElement, StateElement>(mod, "RegisterStateElement")
      .def("getRegisterIndex", &RegisterStateElement::GetRegisterIndex, py::call_guard<ThreadContextNoAdvance>())
      ;

    py::class_<VmContextStateElement, StateElement>(mod, "VmContextStateElement")
      .def("getRegisterName", &VmContextStateElement::GetRegisterName, py::call_guard<ThreadContextNoAdvance>())
      .def("getRegisterFieldName", &VmContextStateElement::GetRegisterFieldName, py::call_guard<ThreadContextNoAdvance>())
      ;

    py::class_<PrivilegeLevelStateElement, StateElement>(mod, "PrivilegeLevelStateElement")
      ;

    py::class_<PcStateElement, StateElement>(mod, "PcStateElement")
      ;
  }

}

#endif  // Force_PyStateElement_H
