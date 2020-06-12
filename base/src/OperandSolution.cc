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
#include <OperandSolution.h>
#include <OperandSolutionMap.h>
#include <Register.h>
#include <Operand.h>
#include <Constraint.h>
#include <OperandConstraint.h>
#include <InstructionStructure.h>
#include <UtilityFunctions.h>
#include <Choices.h>

using namespace std;

/*!
  \file OperandSolution.cc
  \brief Code supporting wrapping operands to provide a convenient interface for solving for operand values.
*/

namespace Force {

  OperandSolution::OperandSolution(Operand* pOperand)
    : mpReg(nullptr), mpOperand(pOperand), mpConstr(nullptr), mValue(0), mUopParamType(UopParamBool)
  {
    // Non-system register operand constraints are set when the register is set.
    if ((not pOperand->IsRegisterOperand()) || (pOperand->OperandType() == EOperandType::SysReg)) {
      OperandConstraint* opr_constr = pOperand->GetOperandConstraint();
      const ConstraintSet* opr_constr_set = opr_constr->GetConstraint();

      if (opr_constr_set != nullptr) {
        mpConstr = opr_constr_set->Clone();
      }
      else {
        if (pOperand->OperandType() == EOperandType::Immediate)
        {
          opr_constr->SubConstraintValue(0xffffffffffffffff, *(pOperand->GetOperandStructure()));
          mpConstr = opr_constr->GetConstraint()->Clone();
        }
        else if (pOperand->OperandType() == EOperandType::Choices)
        {
          const auto choices_opr_constr = dynamic_cast<ChoicesOperandConstraint*>(opr_constr);
          if (choices_opr_constr)
          {
            const ChoiceTree * p_tree = choices_opr_constr->GetChoiceTree();
            if (p_tree)
            {
              mpConstr = new ConstraintSet;
              auto choices = p_tree->GetChoices();
              for (auto p_choice : choices)
              {
                mpConstr->AddValue(p_choice->ValueAs64());
              }
            }
          }
          else
          {
            mpConstr = new ConstraintSet(0, MAX_UINT64);
          }
        }
        else{
          mpConstr = new ConstraintSet(0, MAX_UINT64);
        }
      }
    }

    const OperandStructure* opr_struct = pOperand->GetOperandStructure();
    mUopParamType = opr_struct->UopParameterType();
  }

  OperandSolution::OperandSolution(const OperandSolution& rOther)
    : mpReg(rOther.mpReg), mpOperand(rOther.mpOperand), mpConstr(nullptr), mValue(rOther.mValue), mUopParamType(rOther.mUopParamType)
  {
    if (rOther.mpConstr != nullptr) {
      mpConstr = rOther.mpConstr->Clone();
    }
  }

  OperandSolution::OperandSolution(OperandSolution&& rOther)
    : mpReg(rOther.mpReg), mpOperand(rOther.mpOperand), mpConstr(rOther.mpConstr), mValue(rOther.mValue), mUopParamType(rOther.mUopParamType)
  {
    rOther.mpReg = nullptr;
    rOther.mpOperand = nullptr;
    rOther.mpConstr = nullptr;
  }

  OperandSolution& OperandSolution::operator=(const OperandSolution& rOther)
  {
    if (this != &rOther)
    {
      mpReg = rOther.mpReg;
      mpOperand = rOther.mpOperand;
      mValue = rOther.mValue;
      mUopParamType = rOther.mUopParamType;

      if (rOther.mpConstr != nullptr) {
        if (mpConstr != nullptr)
        {
          delete mpConstr;
        }
        mpConstr = rOther.mpConstr->Clone();
      }
    }
    return *this;
  }

  OperandSolution::~OperandSolution()
  {
    if (mpConstr != nullptr) {
      delete mpConstr;
      mpConstr = nullptr;
    }
  }

  const Register* OperandSolution::GetRegister() const
  {
    return mpReg;
  }

  Operand* OperandSolution::GetOperand() const
  {
    return mpOperand;
  }

  const ConstraintSet* OperandSolution::GetConstraint() const
  {
    return mpConstr;
  }

  uint64 OperandSolution::GetValue() const
  {
    return mValue;
  }

  EUopParameterType OperandSolution::GetUopParameterType() const
  {
    return mUopParamType;
  }

  string OperandSolution::GetName() const
  {
    return mpOperand->Name();
  }

  void OperandSolution::SetRegister(Register* pReg)
  {
    mpReg = pReg;

    InitializeConstraintFromRegister(*pReg);
  }

  void OperandSolution::SetValue(cuint64 value)
  {
    mValue = value;
  }

  void OperandSolution::InitializeConstraintFromRegister(const Register& rReg)
  {
    if (mpConstr != nullptr) {
      delete mpConstr;
    }

    if (rReg.IsInitialized()) {
      mValue = rReg.Value();
      mpConstr = new ConstraintSet(mValue);
    }
    else {
      mpConstr = new ConstraintSet(0, get_mask64(rReg.Size()));
    }
  }

}
