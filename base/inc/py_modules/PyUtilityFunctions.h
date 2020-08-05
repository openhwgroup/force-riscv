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
#ifndef Force_PyUtilityFunctions_H
#define Force_PyUtilityFunctions_H

#include <UtilityFunctions.h>

#include <ThreadContext.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(UtilityFunctions, mod) {
    mod
      .def("getAlignMask", &get_align_mask, py::call_guard<ThreadContext>())
      .def("getAlignedValue", &get_aligned_value, py::call_guard<ThreadContext>())
      .def("lowestBitSet", &lowest_bit_set, py::call_guard<ThreadContext>())
      ;
  }

}

#endif  // Force_PyUtilityFunctions_H
