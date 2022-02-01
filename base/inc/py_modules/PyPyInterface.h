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
#ifndef Force_PyPyInterface_H
#define Force_PyPyInterface_H

#include <PyInterface.h>

#include <ThreadContext.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(PyInterface, mod) {
    mod.doc() = "Force backend library interface plugin";

    py::class_<PyInterface>(mod, "Interface")
      .def("numberOfChips", &PyInterface::NumberOfChips /* No call guard because used when initializing environment, prior to creating thread dispatcher */)
      .def("numberOfCores", &PyInterface::NumberOfCores /* No call guard because used when initializing environment, prior to creating thread dispatcher */)
      .def("numberOfThreads", &PyInterface::NumberOfThreads /* No call guard because used when initializing environment, prior to creating thread dispatcher */)
      .def("createGeneratorThread", &PyInterface::CreateGeneratorThread /* No call guard because used when initializing environment, prior to creating thread dispatcher */)
      .def("genInstruction", &PyInterface::GenInstruction, py::call_guard<ThreadContext>())
      .def("genMetaInstruction", &PyInterface::GenMetaInstruction, py::call_guard<ThreadContext>())
      .def("initializeMemory", &PyInterface::InitializeMemory, py::call_guard<ThreadContext>())
      .def("addChoicesModification", &PyInterface::AddChoicesModification, py::call_guard<ThreadContext>())
      .def("commitModificationSet",  &PyInterface::CommitModificationSet, py::call_guard<ThreadContext>())
      .def("revertModificationSet",  &PyInterface::RevertModificationSet, py::call_guard<ThreadContext>())
      .def("genPA", &PyInterface::GenPA, py::call_guard<ThreadContext>())
      .def("genVA", &PyInterface::GenVA, py::call_guard<ThreadContext>())
      .def("genVMVA", &PyInterface::GenVMVA, py::call_guard<ThreadContext>())
      .def("genVAforPA", &PyInterface::GenVAforPA, py::call_guard<ThreadContext>())
      .def("genFreePagesRange", &PyInterface::GenFreePagesRange, py::call_guard<ThreadContext>())
      .def("sample", &PyInterface::Sample, py::call_guard<ThreadContext>())
      .def("getRandomRegisters", &PyInterface::GetRandomRegisters, py::call_guard<ThreadContext>())
      .def("getRandomRegistersForAccess", &PyInterface::GetRandomRegistersForAccess, py::call_guard<ThreadContext>())
      .def("isRegisterReserved", &PyInterface::IsRegisterReserved, py::call_guard<ThreadContext>())
      .def("reserveRegisterByIndex", &PyInterface::ReserveRegisterByIndex, py::call_guard<ThreadContext>())
      .def("reserveRegister", &PyInterface::ReserveRegister, py::call_guard<ThreadContext>())
      .def("unreserveRegisterByIndex", &PyInterface::UnreserveRegisterByIndex, py::call_guard<ThreadContext>())
      .def("unreserveRegister", &PyInterface::UnreserveRegister, py::call_guard<ThreadContext>())
      .def("readRegister", &PyInterface::ReadRegister, py::call_guard<ThreadContext>())
      .def("writeRegister", &PyInterface::WriteRegister, py::call_guard<ThreadContext>())
      .def("initializeRegister", &PyInterface::InitializeRegister, py::call_guard<ThreadContext>())
      .def("initializeRegisterFields", &PyInterface::InitializeRegisterFields, py::call_guard<ThreadContext>())
      .def("randomInitializeRegister", &PyInterface::RandomInitializeRegister, py::call_guard<ThreadContext>())
      .def("randomInitializeRegisterFields", &PyInterface::RandomInitializeRegisterFields, py::call_guard<ThreadContext>())
      .def("getRegisterFieldMask", &PyInterface::GetRegisterFieldMask, py::call_guard<ThreadContext>())
      .def("genSequence", &PyInterface::GenSequence, py::call_guard<ThreadContext>())
      .def("addMemoryRange", &PyInterface::AddMemoryRange /* No call guard because used when initializing environment, prior to creating thread dispatcher */)
      .def("subMemoryRange", &PyInterface::SubMemoryRange /* No call guard because used when initializing environment, prior to creating thread dispatcher */)
      .def("addArchitectureMemoryAttributes", &PyInterface::AddArchitectureMemoryAttributes, py::arg(), py::arg(), py::arg(), py::arg(), py::arg("thread_id") = 0 /* No call guard because used when initializing environment, prior to creating thread dispatcher */)
      .def("addImplementationMemoryAttributes", &PyInterface::AddImplementationMemoryAttributes, py::arg(), py::arg(), py::arg(), py::arg(), py::arg("thread_id") = 0 /* No call guard because used when initializing environment, prior to creating thread dispatcher */)
      .def("getOption", &PyInterface::GetOption, py::call_guard<ThreadContext>())
      .def("query", &PyInterface::Query, py::call_guard<ThreadContext>())
      .def("virtualMemoryRequest", &PyInterface::VirtualMemoryRequest, py::call_guard<ThreadContext>())
      .def("stateRequest", &PyInterface::StateRequest, py::call_guard<ThreadContext>())
      .def("reserveMemory", &PyInterface::ReserveMemory, py::call_guard<ThreadContext>())
      .def("unreserveMemory", &PyInterface::UnreserveMemory, py::call_guard<ThreadContext>())
      .def("exceptionRequest", &PyInterface::ExceptionRequest, py::call_guard<ThreadContext>())
      .def("beginStateRestoreLoop", &PyInterface::BeginStateRestoreLoop, py::call_guard<ThreadContext>())
      .def("endStateRestoreLoop", &PyInterface::EndStateRestoreLoop, py::call_guard<ThreadContext>())
      .def("generateLoopRestoreInstructions", &PyInterface::GenerateLoopRestoreInstructions, py::call_guard<ThreadContext>())
      .def("modifyVariable", &PyInterface::ModifyVariable, py::call_guard<ThreadContext>())
      .def("getVariable", &PyInterface::GetVariable, py::call_guard<ThreadContext>())
      .def("registerModificationSet", &PyInterface::RegisterModificationSet, py::call_guard<ThreadContext>())
      .def("verifyVirtualAddress", &PyInterface::VerifyVirtualAddress, py::call_guard<ThreadContext>())
      // Multi-threading APIs
      .def("partitionThreadGroup", &PyInterface::PartitionThreadGroup, py::call_guard<ThreadContext>())
      .def("setThreadGroup", &PyInterface::SetThreadGroup, py::call_guard<ThreadContext>())
      .def("queryThreadGroup", &PyInterface::QueryThreadGroup, py::call_guard<ThreadContext>())
      .def("getThreadGroupId", &PyInterface::GetThreadGroupId, py::call_guard<ThreadContext>())
      .def("getFreeThreads", &PyInterface::GetFreeThreads, py::call_guard<ThreadContext>())
      .def("lockThreadScheduler", &PyInterface::LockThreadScheduler, py::call_guard<ThreadContext>())
      .def("unlockThreadScheduler", &PyInterface::UnlockThreadScheduler, py::call_guard<ThreadContext>())
      .def("genSemaphore", &PyInterface::GenSemaphore, py::call_guard<ThreadContext>())
      .def("synchronizeWithBarrier", &PyInterface::SynchronizeWithBarrier, py::call_guard<ThreadContext>())
      ;
  }

}

#endif  // Force_PyPyInterface_H
