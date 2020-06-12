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
#ifndef Force_InstructionConstraintRISCV_H
#define Force_InstructionConstraintRISCV_H

#include <InstructionConstraint.h>

namespace Force {

  class VectorDataTypeOperand;
  class VectorDataTraits;

  /*!
    \class VectorInstructionConstraint
    \brief class for 32-bit imms bit mask instruction constraint.
  */
  class VectorInstructionConstraint : public virtual InstructionConstraint {
  public:
    VectorInstructionConstraint() : InstructionConstraint(), mpDataTypeOperand(nullptr), mpDataTraits(nullptr) { } //!< Constructor.
    ~VectorInstructionConstraint(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(VectorInstructionConstraint);

    void SetDataTypeOperand(const VectorDataTypeOperand* vecDataTypeOpr) { mpDataTypeOperand = vecDataTypeOpr; } //!< Set pointer to data-type operand of the VectorInstruction.
    const VectorDataTypeOperand* DataTypeOperand() const { return mpDataTypeOperand; } //!< Return pointer to the data-type operand of the VectorInstruction.
    const VectorDataTraits* DataTraits() const; //!< Return pointer to vector data traits object.
  protected:
    VectorInstructionConstraint(const VectorInstructionConstraint& rOther) //!< Copy constructor, not meant to be used.
      : InstructionConstraint(rOther), mpDataTypeOperand(nullptr), mpDataTraits(nullptr)
    {
    }
  protected:
    const VectorDataTypeOperand* mpDataTypeOperand; //!< Pointer to the data type operand of the VectorInstruction.
    mutable VectorDataTraits* mpDataTraits; //!< Traits of the vector data.
  };

  /*!
    \class VectorLoadStoreInstructionConstraint
    \brief class for vector load store instruction constraint.
  */
  class VectorLoadStoreInstructionConstraint : public VectorInstructionConstraint, public LoadStoreInstructionConstraint {
  public:
    VectorLoadStoreInstructionConstraint() : VectorInstructionConstraint(), LoadStoreInstructionConstraint() { } //!< Constructor.
    ~VectorLoadStoreInstructionConstraint() { } //!< Destructor.

    protected:
    VectorLoadStoreInstructionConstraint(const VectorLoadStoreInstructionConstraint& rOther) //!< Copy constructor, not meant to be used.
      : VectorInstructionConstraint(rOther), LoadStoreInstructionConstraint(rOther)
    {
    }
  };

  /*!
    \class RetInstructionConstraint
    \brief class for RET instruction constraint.
  */
  class RetInstructionConstraint : public InstructionConstraint {
  public:
    RetInstructionConstraint() : InstructionConstraint() { } //!< Constructor.
    ~RetInstructionConstraint() { } //!< Destructor.

  protected:
    RetInstructionConstraint(const RetInstructionConstraint& rOther) //!< Copy constructor, not meant to be used.
      : InstructionConstraint(rOther)
    {
    }
  protected:
  };
}

#endif //Force_InstructionConstraintRISCV_H
