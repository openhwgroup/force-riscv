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
#ifndef Force_PyEnums_H
#define Force_PyEnums_H

#include <Enums.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(Enums, mod) {
    py::enum_<EGlobalStateType>(mod, "EGlobalStateType")
      .value("ResetPC", EGlobalStateType::ResetPC)
      .value("PageTableRegionSize", EGlobalStateType::PageTableRegionSize)
      .value("PageTableRegionAlign", EGlobalStateType::PageTableRegionAlign)
      .value("PageTableRegionStart", EGlobalStateType::PageTableRegionStart)
      .value("MemoryFillPattern", EGlobalStateType::MemoryFillPattern)
      .value("ElfMachine", EGlobalStateType::ElfMachine)
      .value("AppRegisterWidth", EGlobalStateType::AppRegisterWidth)
      ;

    py::enum_<ELimitType>(mod, "ELimitType")
      .value("ThreadsLimit", ELimitType::ThreadsLimit)
      .value("CoresLimit", ELimitType::CoresLimit)
      .value("ChipsLimit", ELimitType::ChipsLimit)
      .value("PhysicalAddressLimit", ELimitType::PhysicalAddressLimit)
      .value("MaxInstructions", ELimitType::MaxInstructions)
      .value("PerMonRegisterNumber", ELimitType::PerMonRegisterNumber)
      .value("DependencyHistoryLimit", ELimitType::DependencyHistoryLimit)
      .value("BranchNotTakenLimit", ELimitType::BranchNotTakenLimit)
      .value("SpeculativeBntLevelLimit", ELimitType::SpeculativeBntLevelLimit)
      .value("MaxPhysicalVectorLen", ELimitType::MaxPhysicalVectorLen)
      .value("ErrRegisterNumber", ELimitType::ErrRegisterNumber)
      .value("SpeculativeBntInstructionLimit", ELimitType::SpeculativeBntInstructionLimit)
      .value("MaxVectorElementWidth", ELimitType::MaxVectorElementWidth)
      ;

    py::enum_<ERestoreExclusionGroup>(mod, "ERestoreExclusionGroup")
      .value("GPR", ERestoreExclusionGroup::GPR)
      .value("SIMDFP", ERestoreExclusionGroup::SIMDFP)
      .value("VECREG", ERestoreExclusionGroup::VECREG)
      .value("System", ERestoreExclusionGroup::System)
      .value("Memory", ERestoreExclusionGroup::Memory)
      ;

    py::enum_<EStateElementDuplicateMode>(mod, "EStateElementDuplicateMode")
      .value("Fail", EStateElementDuplicateMode::Fail)
      .value("Replace", EStateElementDuplicateMode::Replace)
      .value("Ignore", EStateElementDuplicateMode::Ignore)
      ;

    py::enum_<EStateTransitionType>(mod, "EStateTransitionType")
      .value("Boot", EStateTransitionType::Boot)
      .value("Explicit", EStateTransitionType::Explicit)
      ;

    py::enum_<EStateTransitionOrderMode>(mod, "EStateTransitionOrderMode")
      .value("UseDefault", EStateTransitionOrderMode::UseDefault)
      .value("AsSpecified", EStateTransitionOrderMode::AsSpecified)
      .value("ByStateElementType", EStateTransitionOrderMode::ByStateElementType)
      .value("ByPriority", EStateTransitionOrderMode::ByPriority)
      ;

    py::enum_<EStateElementType>(mod, "EStateElementType")
      .value("Memory", EStateElementType::Memory)
      .value("SystemRegister", EStateElementType::SystemRegister)
      .value("VectorRegister", EStateElementType::VectorRegister)
      .value("GPR", EStateElementType::GPR)
      .value("VmContext", EStateElementType::VmContext)
      .value("PrivilegeLevel", EStateElementType::PrivilegeLevel)
      .value("PC", EStateElementType::PC)
      .value("FloatingPointRegister", EStateElementType::FloatingPointRegister)
      .value("PredicateRegister", EStateElementType::PredicateRegister)
      ;

    py::enum_<EEndianness>(mod, "EEndianness")
      .value("LittleEndian", EEndianness::LittleEndian)
      .value("BigEndian", EEndianness::BigEndian)
      ;
  }

}

#endif  // Force_PyEnums_H
