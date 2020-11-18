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
#include <BaseOffsetConstraint.h>
#include <Constraint.h>
#include <Log.h>
#include <UtilityFunctions.h>

/*!
  \file BaseOffsetConstraint.cc
  \brief Code handling base offset constraint building.
*/

using namespace std;

namespace Force {

  BaseOffsetConstraint::BaseOffsetConstraint(uint64 offsetBase, uint32 offsetSize,  uint32 offsetScale, uint64 maxAddress, bool IsOffsetShift)
    : mOffsetBase(offsetBase), mMaxAddress(maxAddress), mOffsetSize(offsetSize), mOffsetScale(offsetScale), mIsOffsetShift(IsOffsetShift),mSignTestBit(0), mOffsetMask(0), mSignExtendMask(0), mBaseValue(0), mAccessSize(0), mpOffsetConstraint(nullptr)
  {
    mSignTestBit = 1ull << (mOffsetSize - 1);
    mOffsetMask = get_mask64(mOffsetSize);
    mSignExtendMask = ~mOffsetMask;
  }

  BaseOffsetConstraint::BaseOffsetConstraint()
    : mOffsetBase(0), mMaxAddress(0), mOffsetSize(0), mOffsetScale(0),mIsOffsetShift(1), mSignTestBit(0), mOffsetMask(0), mSignExtendMask(0), mBaseValue(0), mAccessSize(0), mpOffsetConstraint(nullptr)
  {
  }

  void BaseOffsetConstraint::AddOffsetValueConstraint(uint64 offsetValue, ConstraintSet& rAdditionalConstr) const
  {
    uint64 addr_start = SignExtendShiftOffset(offsetValue) + mBaseValue;
    uint64 addr_end = addr_start + (mAccessSize - 1);
    if (addr_end < addr_start) {
      // overflowed
      rAdditionalConstr.AddRange(addr_start, mMaxAddress);
      rAdditionalConstr.AddRange(0, addr_end);
    } else {
      rAdditionalConstr.AddRange(addr_start, addr_end);
    }
  }

  // TODO(Noah): Treat an offset range constraint as a range of permissible offset operand values
  // without regard to sign interpretation when there is time to do so. AddOffsetRangeConstraint()
  // currently uses the complement of the range constraint when one of the bounds is negative and
  // the other is positive. For example, if we have a 4-bit signed offset with a range constraint of
  // {0x6-0xC}, the current behavior treats the constraint as if the operand can take on any of the
  // values in {0x0-0x6, 0xC-0xF}, i.e. the 11 values from -4 (0xC) to 6 (0x6). The interpretation
  // that is consistent with the way constraints are treated elsewhere is that the operand can take
  // on any of the values in {0x6-0xC}, i.e. the 7 values from -4 (0xC) to 0 (0x0) and from 6 (0x6)
  // to 7 (0x7). This interpretation makes the size of the result constraint equal to the size of
  // the offset constraint (neglecting the allowance for the access size), which is what we would
  // expect.
  void BaseOffsetConstraint::AddOffsetRangeConstraint(uint64 offsetLower, uint64 offsetUpper, ConstraintSet& rAdditionalConstr) const
  {
    int64 lower_normalized = (int64)SignExtendShiftOffset(offsetLower);
    int64 upper_normalized = (int64)SignExtendShiftOffset(offsetUpper);
    uint64 addr_lower,addr_upper;

     if (upper_normalized < lower_normalized){
        swap(lower_normalized, upper_normalized);
     }

    addr_upper = mBaseValue + upper_normalized + mAccessSize - 1;
    addr_lower = mBaseValue + lower_normalized;

    if (addr_upper < addr_lower){
        // only one address is wrapped around
        rAdditionalConstr.AddRange(addr_lower,mMaxAddress);
        rAdditionalConstr.AddRange(0,addr_upper);
    }
    else rAdditionalConstr.AddRange(addr_lower, addr_upper); // either both addresses wrapped around, or both not wrapped around.

//    LOG(fail) << "{BaseOffsetConstraint::AddOffsetRangeConstraint} unexpected lower-upper combination: " << switch_var << endl;
//    FAIL("unexpected-lower-upper-offset-combination");
  }

  void BaseOffsetConstraint::GetAdditionalConstraint(ConstraintSet& rAdditionalConstr) const
  {
    const std::vector<Constraint* >& constr_vec = mpOffsetConstraint->GetConstraints();

    for (auto constr_ptr : constr_vec) {
      if (constr_ptr->Size() == 1) {
        AddOffsetValueConstraint(constr_ptr->LowerBound(), rAdditionalConstr);
      }
      else {
        AddOffsetRangeConstraint(constr_ptr->LowerBound(), constr_ptr->UpperBound(), rAdditionalConstr);
      }
    }
  }

  void BaseOffsetConstraint::GetConstraint(uint64 baseValue, uint32 accessSize, const ConstraintSet* pOffsetConstr, ConstraintSet& resultConstr) const
  {
    ValidateOffsetSize();

    SetMutables(baseValue, accessSize, pOffsetConstr);

    uint64 min_value = 0ull - mOffsetBase; // produces 1's compliment value if the offset is signed.
    uint64 max_value = get_mask64(mOffsetSize) - mOffsetBase; // this should be positive
    min_value = AdjustOffset(min_value);
    max_value = AdjustOffset(max_value);

    uint64 range_lower = mBaseValue + min_value;
    if (range_lower > mBaseValue) {
      // underflowed
      resultConstr.AddRange(0, mBaseValue);
      resultConstr.AddRange(range_lower, mMaxAddress);
    } else {
      resultConstr.AddRange(range_lower, mBaseValue);
    }

    uint64 range_upper = mBaseValue + max_value + (mAccessSize - 1);
    if ((range_upper < mBaseValue) or ((range_upper - mAccessSize + 1) < mBaseValue)) {
      // overflowed
      resultConstr.AddRange(mBaseValue, mMaxAddress);
      resultConstr.AddRange(0, range_upper);
    } else {
      resultConstr.AddRange(mBaseValue, range_upper);
    }

    if (nullptr != mpOffsetConstraint) {
      ConstraintSet additional_constr;
      GetAdditionalConstraint(additional_constr);
      // << "additional constraint : " << additional_constr.ToSimpleString() << endl;
      resultConstr.ApplyConstraintSet(additional_constr);
    }
    // << "Base-offset constraint: " << resultConstr.ToSimpleString() << endl;
  }

  void BaseOffsetConstraint::ValidateOffsetSize() const
  {
    uint32 max_offset_size = 64;
    if (mOffsetSize > max_offset_size) {
      LOG(fail) << "{BaseOffsetConstraint::ValidateOffsetSize} offset size " << dec << mOffsetSize << " exceeds maximum value of " << max_offset_size << endl;
      FAIL("invalid-parameter-value");
    }

    if (mIsOffsetShift and ((mOffsetSize + mOffsetScale) > max_offset_size)) {
      LOG(fail) << "{BaseOffsetConstraint::ValidateOffsetSize} sum of offset size " << dec << mOffsetSize << " and shift offset scale " << mOffsetScale << " exceeds maximum value of " << max_offset_size << endl;
      FAIL("invalid-parameter-value");
    }
  }

}
