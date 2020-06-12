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
#ifndef Force_PyRandomUtils_H
#define Force_PyRandomUtils_H

#include <RandomUtils.h>

#include <ThreadContext.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(RandomUtils, mod) {
    mod
      .def("random32", &random_value32, py::arg("aMin") = 0, py::arg("aMax") = MAX_UINT32, py::call_guard<ThreadContext>())
      .def("random64", &random_value64, py::arg("aMin") = 0, py::arg("aMax") = MAX_UINT64, py::call_guard<ThreadContext>())
      .def("randomReal", &random_real, py::arg("aMin") = 0.0, py::arg("aMax") = 1.0, py::call_guard<ThreadContext>())
      ;
  }

}

#endif  // Force_PyRandomUtils_H
