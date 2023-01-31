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
#include "RegisterReserver.h"

#include "Constraint.h"
#include "Log.h"
#include "Register.h"
#include "ReservationConstraint.h"

using namespace std;

/*!
  \file RegisterReserver.cc
  \brief Code supporting reserving registers.
*/

namespace Force {

  RegisterReserver::RegisterReserver()
    : Object(), mReservationConstraints()
  {
    for (ERegReserveGroupBaseType i = 0; i < ERegReserveGroupSize; i++) {
      mReservationConstraints.push_back(new ReservationConstraint());
    }
  }

  RegisterReserver::RegisterReserver(const RegisterReserver& rOther)
    : Object(), mReservationConstraints()
  {
    transform(rOther.mReservationConstraints.cbegin(), rOther.mReservationConstraints.cend(), back_inserter(mReservationConstraints),
      [](const ReservationConstraint* pReservationConstr) { return dynamic_cast<ReservationConstraint*>(pReservationConstr->Clone()); });
  }

  RegisterReserver::~RegisterReserver()
  {
    for (ReservationConstraint* reservation_constr : mReservationConstraints) {
      delete reservation_constr;
    }
  }

  void RegisterReserver::ReserveRegister(const Register* pRegister, const ERegAttrType access, const ERegReserveType reserveType) const
  {
    LOG(info) << "{RegisterReserver::ReserveRegister} Reserving register: " << pRegister->Name() << ", access: " << ERegAttrType_to_string(access) << ", reserve type: " <<  ERegReserveType_to_string(reserveType) << endl;

    ConstraintSet phys_reg_indices;
    GetPhysicalRegisterIndices(*pRegister, &phys_reg_indices);
    ReservePhysicalRegisterIndices(phys_reg_indices, pRegister->RegisterType(), access, reserveType);
  }

  void RegisterReserver::UnreserveRegister(const Register* pRegister, const ERegAttrType access, const ERegReserveType reserveType) const
  {
    LOG(info) << "{RegisterReserver::UnreserveRegister} Unreserve register: " << pRegister->Name() << ", access: " << ERegAttrType_to_string(access) << ", reserve type: " <<  ERegReserveType_to_string(reserveType) << endl;

    ConstraintSet phys_reg_indices;
    GetPhysicalRegisterIndices(*pRegister, &phys_reg_indices);
    UnreservePhysicalRegisterIndices(phys_reg_indices, pRegister->RegisterType(), access, reserveType);
  }

  bool RegisterReserver::IsRegisterReserved(const Register* pRegister, const ERegAttrType access, const ERegReserveType reserveType) const
  {
    bool reserved = true;
    ConstraintSet phys_reg_indices;
    GetPhysicalRegisterIndices(*pRegister, &phys_reg_indices);
    for (ERegReserveGroup reserve_group : GetReserveGroupsForRegisterType(pRegister->RegisterType())) {
      ReservationConstraint* reservation_constr = mReservationConstraints[ERegReserveGroupBaseType(reserve_group)];

      if (not reservation_constr->AreRegistersReserved(phys_reg_indices, access, reserveType)) {
        reserved = false;
        break;
      }
    }

    return reserved;
  }

  bool RegisterReserver::HasReservations(const EOperandType oprType, const ERegAttrType access, const ConstraintSet*& prReadConstr, const ConstraintSet*& prWriteConstr) const
  {
    ERegReserveGroup reserve_group = GetReserveGroupForOperandType(oprType);
    ReservationConstraint* reservation_constr = mReservationConstraints[ERegReserveGroupBaseType(reserve_group)];
    return reservation_constr->HasReserved(access, prReadConstr, prWriteConstr);
  }

  void RegisterReserver::UsableIndexConstraint(const ERegisterType regType, const ERegAttrType access, ConstraintSet* pIndexConstr) const
  {
    GetRegisterIndexRange(regType, pIndexConstr);
    for (ERegReserveGroup reserve_group : GetReserveGroupsForRegisterType(regType)) {
      ReservationConstraint* reservation_constr = mReservationConstraints[ERegReserveGroupBaseType(reserve_group)];
      reservation_constr->ExcludeReservedByAccess(access, pIndexConstr);
    }
  }

  void RegisterReserver::ReservePhysicalRegisterIndices(const ConstraintSet& rPhysRegIndices, const ERegisterType regType, const ERegAttrType access, const ERegReserveType reserveType) const
  {
    for (ERegReserveGroup reserve_group : GetReserveGroupsForRegisterType(regType)) {
      ReservationConstraint* reservation_constr = mReservationConstraints[ERegReserveGroupBaseType(reserve_group)];
      reservation_constr->ReserveRegisters(rPhysRegIndices, access, reserveType);
    }
  }

  void RegisterReserver::UnreservePhysicalRegisterIndices(const ConstraintSet& rPhysRegIndices, const ERegisterType regType, const ERegAttrType access, const ERegReserveType reserveType) const
  {
    for (ERegReserveGroup reserve_group : GetReserveGroupsForRegisterType(regType)) {
      ReservationConstraint* reservation_constr = mReservationConstraints[ERegReserveGroupBaseType(reserve_group)];
      reservation_constr->UnreserveRegisters(rPhysRegIndices, access, reserveType);
    }
  }

  void RegisterReserver::GetPhysicalRegisterIndices(const Register& rReg, ConstraintSet* pPhysRegIndices) const
  {
    set<PhysicalRegister*> phys_registers;
    rReg.GetPhysicalRegisters(phys_registers);
    for (PhysicalRegister* phys_reg : phys_registers)
    {
      pPhysRegIndices->AddValue(GetPhysicalRegisterIndex(phys_reg, rReg.RegisterType()));
    }
  }

}
