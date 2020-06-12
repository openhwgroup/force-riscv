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
#ifndef Force_OperandSolution_H
#define Force_OperandSolution_H

#include <UopInterface.h>
#include <Defines.h>
#include <map>

namespace Force {

  class Operand;
  class ConstraintSet;
  class Register;

  /*!
    \class OperandSolution
    \brief Wrapper around operands that provides a convenient interface for solving for operand values.
  */
  class OperandSolution {
  public:
    explicit OperandSolution(Operand* pOperand); //!< Constructor
    OperandSolution(const OperandSolution& rOther); //!< Copy constructor
    OperandSolution(OperandSolution&& rOther); //!< Move constructor
    OperandSolution& operator=(const OperandSolution& rOther); //!< Assignment operator
    ~OperandSolution(); //!< Destructor

    const Register* GetRegister() const; //!< Return register intended to be assigned to operand if one exists; will return null otherwise.
    Operand* GetOperand() const; //!< Return operand represented by this OperandSolution.
    const ConstraintSet* GetConstraint() const; //!< Return set of values that may be assumed by the operand.
    uint64 GetValue() const; //!< Return value intended to be assigned to operand.
    EUopParameterType GetUopParameterType() const; //!< Return type to be used in the Uop interface.
    std::string GetName() const; //!< Return operand name.
    void SetRegister(Register* pReg); //!< Set register intended to be assigned to operand.
    void SetValue(cuint64 value); //!< Set value intended to be assigned to operand.
  private:
    void InitializeConstraintFromRegister(const Register& rReg); //!< Get the set of values that may be assumed by the specified register.
  private:
    const Register* mpReg; //!< Register intended to be assigned to operand;
    Operand* mpOperand; //!< Operand represented by this OperandSolution.
    ConstraintSet* mpConstr; //!< Set of values that may be assumed by the operand.
    uint64 mValue; //!< Value intended to be assigned to operand.
    EUopParameterType mUopParamType; //!< Type to be used in the Uop interface.
  };
}

#endif  // Force_OperandSolution_H
