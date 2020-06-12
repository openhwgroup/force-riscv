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
#ifndef Force_PyConfig_H
#define Force_PyConfig_H

#include <Config.h>

#include <Enums.h>
#include <Log.h>
#include <ThreadContext.h>

#include <memory>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(Config, mod) {
    py::class_<Config, std::unique_ptr<Config, py::nodelete>>(mod, "Config")
      .def_static("getInstance", &Config::Instance, py::return_value_policy::reference, py::call_guard<ThreadContext>())
      .def("getLimitValue", &Config::LimitValue, py::call_guard<ThreadContext>())
      .def("setGlobalState", &Config::SetGlobalStateValue, py::call_guard<ThreadContext>())
      .def("setGlobalState", &Config::SetGlobalStateString, py::call_guard<ThreadContext>())
      .def("getGlobalState",
        [](const Config& rSelf, const EGlobalStateType globalStateType) -> py::object {
          bool exists = false;
          uint64 state_value = rSelf.GlobalStateValue(globalStateType, exists);
          if (exists) {
            return py::int_(state_value);
          }

          std::string state_str = rSelf.GlobalStateString(globalStateType, exists);
          if (exists) {
            return py::str(state_str);
          }

          LOG(warn) << "{PyConfig::getGlobalState} unset state \"" << EGlobalStateType_to_string(globalStateType) << "\"." << std::endl;
          return py::none();
        },
        py::call_guard<ThreadContext>())
      ;
  }

}

#endif  // Force_PyConfig_H
