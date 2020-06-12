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
#include <PyEnvironment.h>

#include <py_modules/PyConfig.h>
#include <py_modules/PyConstraint.h>
#include <py_modules/PyEnums.h>
#include <py_modules/PyLog.h>
#include <py_modules/PyPyInterface.h>
#include <py_modules/PyRandomUtils.h>
#include <py_modules/PyState.h>
#include <py_modules/PyStateElement.h>
#include <py_modules/PyStateTransition.h>
#include <py_modules/PyThreadDispatcher.h>
#include <py_modules/PyUtilityFunctions.h>

#include <pybind11/embed.h>

namespace py = pybind11;

namespace Force {

namespace PyEnvironment {

  void initialize_python()
  {
    PyImport_AppendInittab("PyInterface", PyInit_PyInterface);

    PyImport_AppendInittab("Config", PyInit_Config);
    PyImport_AppendInittab("Constraint", PyInit_Constraint);
    PyImport_AppendInittab("Enums", PyInit_Enums);
    PyImport_AppendInittab("Log", PyInit_Log);
    PyImport_AppendInittab("RandomUtils", PyInit_RandomUtils);
    PyImport_AppendInittab("State", PyInit_State);
    PyImport_AppendInittab("StateElement", PyInit_StateElement);
    PyImport_AppendInittab("StateTransition", PyInit_StateTransition);
    PyImport_AppendInittab("ThreadDispatcher", PyInit_ThreadDispatcher);
    PyImport_AppendInittab("UtilityFunctions", PyInit_UtilityFunctions);

    py::initialize_interpreter();
  }

  void finalize_python()
  {
    py::finalize_interpreter();
  }

}

}
