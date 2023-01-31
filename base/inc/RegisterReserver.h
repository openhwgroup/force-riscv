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
#ifndef Force_RegisterReserver_H
#define Force_RegisterReserver_H

#include <vector>

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class ReservationConstraint;
  class Register;
  class PhysicalRegister;
  class ConstraintSet;

  /*!
    \class RegisterReserver
    \brief Class to reserve registers.
    An architecture-specific subclass should provide the facility to map from operand and register
    types to register reservation groups.
   */
  class RegisterReserver : public Object
  {
  public:
    RegisterReserver(); //!< Constructor
    ~RegisterReserver() override; //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(RegisterReserver);

    const std::string ToString() const override { return Type(); } //!< Return a string describing the current state of the Object.

    void ReserveRegister(const Register* pRegister, const ERegAttrType access, const ERegReserveType reserveType = ERegReserveType::User) const; //!< Reserve a register for the specified access and reservation type. Access can only be Read, Write or ReadWrite.
    void UnreserveRegister(const Register* pRegister, const ERegAttrType access, const ERegReserveType reserveType = ERegReserveType::User) const; //!< Unreserve a register for the specified access and reservation type. Access can only be Read, Write or ReadWrite.
    bool IsRegisterReserved(const Register* pRegister, const ERegAttrType access, const ERegReserveType reserveType = ERegReserveType::User) const; //!< Return whether a register is reserved for the specified access and reservation type.
    bool HasReservations(const EOperandType oprType, const ERegAttrType access, const ConstraintSet*& prReadConstr, const ConstraintSet*& prWriteConstr) const; //!< Return whether there are reserved registers for the specified access type.
    void UsableIndexConstraint(const ERegisterType regType, const ERegAttrType access, ConstraintSet* pIndexConstr) const; //!< Retrieve register indices of the specified type that are not reserved for the specified access.
  protected:
    RegisterReserver(const RegisterReserver& rOther); //!< Copy constructor
    virtual ERegReserveGroup GetReserveGroupForOperandType(const EOperandType oprType) const = 0; //!< Get the reservation group for the specified operand type.
    virtual const std::vector<ERegReserveGroup>& GetReserveGroupsForRegisterType(const ERegisterType regType) const = 0; //!< Get the reservation groups for the specified register type.
    virtual void GetRegisterIndexRange(const ERegisterType regType, ConstraintSet* pIndexConstr) const = 0; //!< Retrieve the range of valid indices for the specified register type.
    virtual uint32 GetPhysicalRegisterIndex(const PhysicalRegister* pPhysReg, const ERegisterType regType) const = 0; //!< Get the register index used for register reservation purposes.
  private:
    void ReservePhysicalRegisterIndices(const ConstraintSet& rPhysRegIndices, const ERegisterType regType, const ERegAttrType access, const ERegReserveType reserveType) const; //!< Reserve physical register indices for the specified access and reservation type.
    void UnreservePhysicalRegisterIndices(const ConstraintSet& rPhysRegIndices, const ERegisterType regType, const ERegAttrType access, const ERegReserveType reserveType) const; //!< Unreserve physical register indices for the specified access and reservation type.
    void GetPhysicalRegisterIndices(const Register& rReg, ConstraintSet* physRegIndices) const;
  private:
    std::vector<ReservationConstraint*> mReservationConstraints; //!< Objects to track reserved register indices for each reservation group
  };

}

#endif
