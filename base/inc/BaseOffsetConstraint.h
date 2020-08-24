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
#ifndef Force_BaseOffsetConstraint_H
#define Force_BaseOffsetConstraint_H

#include <Defines.h>

namespace Force {

  class ConstraintSet;

  /*!
    \class BaseOffsetConstraint
    \brief Class for constructing constraint for base-offset type addressing mode.
  */
  class BaseOffsetConstraint {
  public:
    BaseOffsetConstraint(uint64 offsetBase, uint32 offsetSize, uint32 offsetShift, uint64 maxAddress, bool IsOffsetShift = true); //!< Constructor with parameters.
    BaseOffsetConstraint(); //!< Default constructor.
    ~BaseOffsetConstraint() { } //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(BaseOffsetConstraint);
    COPY_CONSTRUCTOR_DEFAULT(BaseOffsetConstraint);
    void GetConstraint(uint64 baseValue, uint32 accessSize, const ConstraintSet* pOffsetConstr, ConstraintSet& resultConstr) const; //!< Get the base offset constraint.
  private:

    inline void SetMutables(uint64 baseValue, uint32 accessSize, const ConstraintSet* pOffsetConstr) const //!< Set the mutable attributes.
    {
      mBaseValue = baseValue;
      mAccessSize = accessSize;
      mpOffsetConstraint = pOffsetConstr;
    }

    inline uint64 AdjustOffset(uint64 offsetvalue) const
    {
      if (mIsOffsetShift){
        offsetvalue <<= mOffsetScale;
      } else {
        offsetvalue *= mOffsetScale;
      }
      return offsetvalue;
    }
    inline uint64 SignExtendShiftOffset(uint64 inValue) const //!< Sign extend and shift an offset value to be ready to add with base value.
    {
      if (inValue & mSignTestBit)
        inValue |= mSignExtendMask;
      inValue = AdjustOffset(inValue);
      return inValue;
    }

    void GetAdditionalConstraint(ConstraintSet& rAdditionalConstr) const; //!< Get additional constraint by processing the constraint on offset field.
    void AddOffsetValueConstraint(uint64 offsetValue, ConstraintSet& rAdditionalConstr) const; //!< Add constraints to cover the specified offset value.
    void AddOffsetRangeConstraint(uint64 offsetLower, uint64 offsetUpper, ConstraintSet& rAdditionalConstr) const; //!< Add constraints to cover the specified offset range.
  private:
    uint64 mOffsetBase; //!< Base value of the offset field.
    uint64 mMaxAddress; //!< Maximum address allowed.
    uint32 mOffsetSize; //!< Size of the offset field.
    uint32 mOffsetScale; //!< Shift value of the offset field.
    bool   mIsOffsetShift;//!< indicat mOffsetScale is shift or multiply.
    uint64 mSignTestBit; //!< Sign testing bit.
    uint64 mOffsetMask; //!< Offset mask.
    uint64 mSignExtendMask; //!< Sign extension mask.
    mutable uint64 mBaseValue; //!< Base value.
    mutable uint32 mAccessSize; //!< Access size.
    mutable const ConstraintSet* mpOffsetConstraint; //!< Constraint set on the offset field.
  };

}

#endif
