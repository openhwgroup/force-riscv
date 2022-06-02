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
#ifndef Force_PyLog_H
#define Force_PyLog_H

#include "pybind11/pybind11.h"

#include "Log.h"
#include "ThreadContext.h"

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(Log, mod) {
    mod
      .def("trace",
        [](const std::string &rLogMessage) {
          LOG(trace) << "Front-end: " << rLogMessage << std::endl;
        },
        py::call_guard<ThreadContext>())
      .def("debug",
        [](const std::string &rLogMessage) {
          LOG(debug) << "Front-end: " << rLogMessage << std::endl;
        },
        py::call_guard<ThreadContext>())
      .def("info",
        [](const std::string &rLogMessage) {
          LOG(info) << "Front-end: " << rLogMessage << std::endl;
        },
        py::call_guard<ThreadContext>())
      .def("warn",
        [](const std::string &rLogMessage) {
          LOG(warn) << "Front-end: " << rLogMessage << std::endl;
        },
        py::call_guard<ThreadContext>())
      .def("error",
        [](const std::string &rLogMessage) {
          LOG(error) << "Front-end: " << rLogMessage << std::endl;
        },
        py::call_guard<ThreadContext>())
      .def("fail",
        [](const std::string &rLogMessage) {
          LOG(fail) << "Front-end error reported: " << rLogMessage << std::endl;
          FAIL("front-end-error-reported");
        },
        py::call_guard<ThreadContext>())
      .def("notice",
        [](const std::string &rLogMessage) {
          LOG(notice) << "Front-end: " << rLogMessage << std::endl;
        },
        py::call_guard<ThreadContext>())
      .def("noticeNoBlock",
        [](const std::string &rLogMessage) {
          LOG(notice) << "Front-end: " << rLogMessage << std::endl;
        }
        /* No call guard because only intended for special thread executor use case in between starting dispatcher and starting threads */)
      ;
  }

}

#endif  // Force_PyLog_H
