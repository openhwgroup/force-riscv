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
#ifndef Force_PyThreadDispatcher_H
#define Force_PyThreadDispatcher_H

#include "pybind11/pybind11.h"

#include "Scheduler.h"
#include "ThreadDispatcher.h"

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(ThreadDispatcher, mod) {
    py::class_<ThreadDispatcher>(mod, "ThreadDispatcher")
      .def_static("getCurrentDispatcher", &ThreadDispatcher::GetCurrentDispatcher, py::return_value_policy::reference /* No call guard because never blocks */)
      .def_static("setCurrentDispatcher", &ThreadDispatcher::SetCurrentDispatcher /* No call guard because only intended for specific use cases of thread executor */)
      ;

    py::class_<MultiThreadDispatcher, ThreadDispatcher>(mod, "MultiThreadDispatcher")
      .def(py::init([]() {
          return new MultiThreadDispatcher(Scheduler::Instance());
        }) /* No call guard because never blocks */)
      .def("start", &MultiThreadDispatcher::Start, py::call_guard<py::gil_scoped_release>())
      .def("stop", &MultiThreadDispatcher::Stop, py::call_guard<py::gil_scoped_release>())
      .def("request", &MultiThreadDispatcher::Request, py::call_guard<py::gil_scoped_release>())
      .def("finish", &MultiThreadDispatcher::Finish /* No call guard because never blocks */)
      .def("addThreadId", &MultiThreadDispatcher::AddThreadId, py::call_guard<py::gil_scoped_release>())
      .def("registerExecutionThread", &MultiThreadDispatcher::RegisterExecutionThread, py::call_guard<py::gil_scoped_release>())
      .def("reportThreadDone", &MultiThreadDispatcher::ReportThreadDone, py::call_guard<py::gil_scoped_release>())
      ;

    py::class_<SingleThreadDispatcher, ThreadDispatcher>(mod, "SingleThreadDispatcher")
      .def(py::init() /* No call guard because never blocks */)
      .def("start", &SingleThreadDispatcher::Start /* No call guard because never blocks */)
      .def("stop", &SingleThreadDispatcher::Stop /* No call guard because never blocks */)
      .def("request", &SingleThreadDispatcher::Request /* No call guard because never blocks */)
      .def("finish", &SingleThreadDispatcher::Finish /* No call guard because never blocks */)
      .def("addThreadId", &SingleThreadDispatcher::AddThreadId /* No call guard because never blocks */)
      ;
  }

}

#endif  // Force_PyThreadDispatcher_H
