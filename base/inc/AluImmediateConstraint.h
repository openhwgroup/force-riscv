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
#ifndef Force_AluImmediateConstraint_H
#define Force_AluImmediateConstraint_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  class ConstraintSet;

  /*!
    \class AluImmediateConstraint
    \brief Class for constructing constraint for ALU immediate type addressing mode.
  */
  class AluImmediateConstraint {
  public:
    AluImmediateConstraint(const EAluOperationType operationType, cuint32 offsetSize, cuint32 offsetShift); //!< Constructor with parameters.
    COPY_CONSTRUCTOR_ABSENT(AluImmediateConstraint);
    DESTRUCTOR_DEFAULT(AluImmediateConstraint);
    ASSIGNMENT_OPERATOR_ABSENT(AluImmediateConstraint);

    void GetConstraint(cuint64 baseValue, cuint32 accessSize, ConstraintSet* pResultConstr) const; //!< Get the ALU immediate constraint.
  private:
    const EAluOperationType mOperationType; //!< ALU operation type.
    uint64 mMaxOffset; //!< Maximum offset value.
  };

}

#endif
