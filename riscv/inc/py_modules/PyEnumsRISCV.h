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
#ifndef Force_PyEnumsRISCV_H
#define Force_PyEnumsRISCV_H

#include ARCH_ENUM_HEADER

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(EnumsRISCV, mod) {
    py::enum_<EMemBankType>(mod, "EMemBankType")
      .value("Default", EMemBankType::Default)
      ;

    py::enum_<EPagingMode>(mod, "EPagingMode")
      .value("Bare", EPagingMode::Bare)
      .value("Sv32", EPagingMode::Sv32)
      .value("Sv39", EPagingMode::Sv39)
      .value("Sv48", EPagingMode::Sv48)
      ;
  }

}

#endif  // Force_PyEnumsRISCV_H
