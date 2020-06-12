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
#ifndef Force_RegisterReserverRISCV_H
#define Force_RegisterReserverRISCV_H

#include <RegisterReserver.h>
#include <map>
#include <vector>

namespace Force {

  /*!
    \class RegisterReserverRISCV
    \brief Class providing RISC-specific functionality for reserving registers.
  */
  class RegisterReserverRISCV : public RegisterReserver
  {
  public:
    RegisterReserverRISCV(); //!< Constructor
    SUBCLASS_DESTRUCTOR_DEFAULT(RegisterReserverRISCV); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(RegisterReserverRISCV);

    Object* Clone() const override { return new RegisterReserverRISCV(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "RegisterReserverRISCV"; } //!< Return a string describing the actual type of the Object.
   protected:
    RegisterReserverRISCV(const RegisterReserverRISCV& rOther); //!< Copy constructor
    ERegReserveGroup GetReserveGroupForOperandType(const EOperandType oprType) const override; //!< Get the reservation group for the specified operand type.
    const std::vector<ERegReserveGroup>& GetReserveGroupsForRegisterType(const ERegisterType regType) const override; //!< Get the reservation groups for the specified register type.
    void GetRegisterIndexRange(const ERegisterType regType, ConstraintSet* pIndexConstr) const override; //!< Retrieve the range of valid indices for the specified register type.
    uint32 GetPhysicalRegisterIndex(const PhysicalRegister* pPhysReg, const ERegisterType regType) const override; //!< Get the register index used for register reservation purposes.
  private:
    std::map<const EOperandType, const ERegReserveGroup> mReserveGroupsByOprType;
    std::map<const ERegisterType, const std::vector<ERegReserveGroup>> mReserveGroupsByRegType;
  };

}

#endif
