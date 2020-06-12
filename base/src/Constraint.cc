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
#include <Constraint.h>
#include <Random.h>
#include <GenException.h>
#include <UtilityFunctions.h>
#include <StringUtils.h>
#include <Log.h>

#include <sstream>
// C++UP accumulate defined in numeric
#include <numeric>

using namespace std;

#include <ConstraintUtils.h>

//#define DEBUG_CONSTRAINT_DELETE 1

namespace Force {

  Constraint::~Constraint()
  {
#ifdef UNIT_TEST
    ++ ConstraintSet::msConstraintDeleteCount; // increment delete count for unit-test checking.
#endif
  }

  bool Constraint::operator==(const Constraint& rOther) const
  {
    if ((this->LowerBound() == rOther.LowerBound()) && (this->UpperBound() == rOther.UpperBound())) {
      return true;
    }

    return false;
  }

  bool Constraint::operator!=(const Constraint& rOther) const
  {
    return not ((*this) == rOther);
  }

  void Constraint::UnexpectedConstraintType(EConstraintType constrType, const string& callerName) const
  {
    LOG(fail) << "Unexpected constraint type: " << EConstraintType_to_string(constrType) << " in {" << callerName << "}." << endl;
    FAIL("unexpected-constr-type");
  }

  string Constraint::ToSimpleString() const
  {
    char print_buffer[64];
    PrintSimpleString(print_buffer, 64);

    return string(print_buffer);
  }

  string Constraint::ToString() const
  {
    string ret_string = EConstraintType_to_string(Type());
    ret_string += ":";
    char print_buffer[64];
    PrintSimpleString(print_buffer, 64);
    ret_string += print_buffer;

    return ret_string;
  }

  uint64 ValueConstraint::ChosenValue(uint64 offset) const
  {
    if (offset != 0) {
      LOG(fail) << "{ValueConstraint::ChosenValue} expecting the offset to be 0." << endl;
      FAIL("incorrect-chosen-offset-value-constraint");
    }
    return mValue;
  }

  void ValueConstraint::PrintSimpleString(char* print_buffer, uint32 size) const
  {
    snprintf(print_buffer, size, "0x%llx", mValue);
  }

  /*!
    Caller should ensure the passed in Constraint object overlaps with or is right next to the ValueConstraint object.
  */
  void ValueConstraint::MergeConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult)
  {
    switch (pOtherConstr->Type()) {
    case EConstraintType::Value:
      {
        uint64 other_value = pOtherConstr->LowerBound(); // Lower bound same as constraint value.
        if (other_value == mValue) {
          // no changes
          rResult.mType = EConstraintResultType::Consumed;
        }
        else {
          // replace with a RangeConstraint object.
          rResult.mType = EConstraintResultType::Replace;
          if (other_value < mValue) {
            rResult.mpConstraint = new RangeConstraint(other_value, mValue);
          }
          else {
            rResult.mpConstraint = new RangeConstraint(mValue, other_value);
          }
          rResult.mSizeChange = rResult.mpConstraint->Size() - 1;
        }
        DELETE_CONSTRAINT(pOtherConstr);
      }
      break;
    case EConstraintType::Range:
      {
        rResult.mType = EConstraintResultType::Replace;
        rResult.mpConstraint = pOtherConstr;
        uint64 other_size = pOtherConstr->Size();
        RangeConstraint* other_cast = static_cast<RangeConstraint* >(pOtherConstr);
        if (mValue < other_cast->LowerBound()) {
          other_cast->SetLowerBound(mValue);
          rResult.mSizeChange = other_size;
        }
        else if (mValue > other_cast->UpperBound()) {
          other_cast->SetUpperBound(mValue);
          rResult.mSizeChange = other_size;
        }
        else {
          rResult.mSizeChange = other_size - 1;
        }
      }
      break;
    default:
      UnexpectedConstraintType(pOtherConstr->Type(), "ValueConstraint::MergeConstraint");
    }
  }

  // Define a local static inline function to be used in subsequent code block.
  static inline bool replace_with_new_range_constraint(uint64 lower, uint64 upper, Constraint* pOtherConstr, ConstraintOneResult& rResult)
  {
    // replace with a RangeConstraint object.
    rResult.mType = EConstraintResultType::Replace;
    rResult.mpConstraint = new RangeConstraint(lower, upper);
    DELETE_CONSTRAINT(pOtherConstr);
    rResult.mSizeChange = rResult.mpConstraint->Size() - 1;
    return true;
  }

  // Define a local static inline function to be used in subsequent code block.
  static inline bool replace_reusing_range_constraint(uint64 new_size, Constraint* pOtherConstr, ConstraintOneResult& rResult)
  {
    // reuse passed in RangeConstraint.
    rResult.mType = EConstraintResultType::Replace;
    rResult.mpConstraint = pOtherConstr;
    rResult.mSizeChange = new_size;
    return true;
  }

  /*!
    This method is used to process the next Constraint objects in a ConstraintSet after some enflating operation.
    Applicable case will need to make sure this is true: lower bound of pOtherConstr is larger or equals to mValue.
    With this assumption, the merge condition checking is simpler than other more general case.
    This method will check if merge will actually happen.
  */
  bool ValueConstraint::MergeNextConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult)
  {
    uint64 other_lower = pOtherConstr->LowerBound();
    if ((other_lower - mValue) > 1) return false; // no need to merge with the next Constraint.

    switch (pOtherConstr->Type()) {
    case EConstraintType::Value:
      {
        // Lower bound same as constraint value.
        if (other_lower == mValue) {
          // no changes
          rResult.mType = EConstraintResultType::Consumed;
          DELETE_CONSTRAINT(pOtherConstr);
        }
        else { // other_lower - mValue == 1 case
          replace_with_new_range_constraint(mValue, other_lower, pOtherConstr, rResult);
        }
      }
      break;
    case EConstraintType::Range:
      {
        uint64 other_size = pOtherConstr->Size();
        if (other_lower == mValue) { // mValue contained in passed in range
          replace_reusing_range_constraint(other_size - 1, pOtherConstr, rResult);
        }
        else { // mValue will be merged to lower bound of the passed in range.
          RangeConstraint* other_cast = static_cast<RangeConstraint* >(pOtherConstr);
          other_cast->SetLowerBound(mValue);
          replace_reusing_range_constraint(other_size, pOtherConstr, rResult);
        }
      }
      break;
    default:
      UnexpectedConstraintType(pOtherConstr->Type(), "ValueConstraint::MergeNextConstraint");
    }
    return true;
  }

  /*!
    Caller should ensure the passed in Constraint object overlaps with the ValueConstraint object.
   */
  void ValueConstraint::SubConstraint(const Constraint& rOtherConstr, ConstraintTwoResult& cResult)
  {
    switch (rOtherConstr.Type()) {
    case EConstraintType::Value:
    case EConstraintType::Range:
      {
        cResult.MarkRemove(1);
      }
      break;
    default:
      UnexpectedConstraintType(rOtherConstr.Type(), "ValueConstraint::SubConstraint");
    }
  }

  bool ValueConstraint::Contains(uint64 value) const
  {
    return (value == mValue);
  }

  /*!
    Only possible contain case is if otherConstr is also ValueConstraint type.
    If otherConstraint is ValueConstraint as well, then LowerBound returns mValue.
  */
  bool ValueConstraint::Contains(const Constraint& rOtherConstr) const
  {
    if (rOtherConstr.Type() == EConstraintType::Value) {
      return (mValue == rOtherConstr.LowerBound());
    }

    return false;
  }

  bool ValueConstraint::Intersects(const Constraint& rOtherConstr) const
  {
    switch (rOtherConstr.Type()) {
    case EConstraintType::Value:
      {
        const ValueConstraint* other_value_constr = static_cast<const ValueConstraint* >(&rOtherConstr);
        return (mValue == other_value_constr->Value());
      }
      break;
    case EConstraintType::Range:
      {
        const RangeConstraint* other_range_constr = static_cast<const RangeConstraint* >(&rOtherConstr);
        return ((mValue >= other_range_constr->Lower()) and (mValue <= other_range_constr->Upper()));
      }
      break;
    default:
      UnexpectedConstraintType(rOtherConstr.Type(), "ValueConstraint::Intersects");
    }
    return false;
  }

  void ValueConstraint::AlignWithSize(uint64 alignMask, uint64 alignSize, ConstraintOneResult& cResult)
  {
    if (alignSize > 1) {
      cResult.MarkRemove(1);
      return;
    }

    if ((mValue & alignMask) != mValue) {
      cResult.MarkRemove(1);
      return;
    }
  }

  void ValueConstraint::AlignOffsetWithSize(uint64 alignMask, uint64 alignOffset, uint64 alignSize, ConstraintOneResult& cResult)
  {
    if (alignSize > 1) {
      cResult.MarkRemove(1);
      return;
    }

    if ((mValue & ~alignMask) != alignOffset) {
      cResult.MarkRemove(1);
      return;
    }
  }

  void ValueConstraint::AlignMulDataWithSize(uint64 mulData, uint64 baseRemainder, uint64 alignSize, ConstraintOneResult& cResult)
  {
    if (alignSize > 1) {
      cResult.MarkRemove(1);
      return;
    }

    if ((mValue % mulData) != baseRemainder) {
      cResult.MarkRemove(1);
      return;
    }
  }

  uint64 ValueConstraint::AlignWithPage(const PageAlignTraits& rAlignTraits)
  {
    //align value constraint to start of nearest page
    mValue &= rAlignTraits.mAlignMask;
    //shift value to right by page size
    ValueConstraint::ShiftRight(rAlignTraits.mShiftAmount);
    return 0; // no size change.
  }

  void ValueConstraint::Translate(uint64 pageMask, uint64 pageFrame)
  {
    mValue = (mValue & pageMask) | pageFrame;
  }

  uint64 ValueConstraint::GetAlignedSizeFromBottom(uint64 alignMask, uint64 alignSize, bool &hasMatch) const
  {
    hasMatch = ((alignSize == 1) and ((mValue & alignMask) == mValue));
    return mValue;
  }

  uint64 ValueConstraint::GetAlignedSizeFromTop(uint64 alignMask, uint64 alignSize, bool &hasMatch) const
  {
    hasMatch = ((alignSize == 1) and ((mValue & alignMask) == mValue));
    return mValue;
  }

  Constraint* ValueConstraint::GetIntersection(const Constraint& rOtherConstr) const
  {
    if (rOtherConstr.Contains(*this)) {
      return Clone();
    }
    LOG(fail) << "{ValueConstraint::GetIntersection} no intersection found." << endl;
    FAIL("no-intersection-found");
    return nullptr;
  }

  void ValueConstraint::ApplyConstraint(const Constraint& rOtherConstr, ConstraintOneResult& cResult)
  {
    if (not rOtherConstr.Contains(*this)) {
      cResult.MarkRemove(1);
    }
  }

  void ValueConstraint::GetValues(std::vector<uint64>& valueVec) const
  {
    valueVec.push_back(mValue);
  }

  uint64 RangeConstraint::ChosenValue(uint64 offset) const
  {
    uint64 chosen_value = mLower + offset;
    if ((chosen_value > mUpper) || (chosen_value < mLower)) {
      LOG(fail) << "{RangeConstraint::ChosenValue} offset value 0x" << hex << offset << " out of range.  Constraint size=0x" << Size() << endl;
      FAIL("incorrect-chosen-offset-range-constraint");
    }
    return chosen_value;
  }

  void RangeConstraint::PrintSimpleString(char* print_buffer, uint32 size) const
  {
    snprintf(print_buffer, size, "0x%llx-0x%llx", mLower, mUpper);
  }

  /*!
    Caller should ensure the passed in Constraint object overlaps with or is right next to the RangeConstraint object.
  */
  void RangeConstraint::MergeConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult)
  {
    switch (pOtherConstr->Type()) {
    case EConstraintType::Value:
      {
        uint64 other_value = pOtherConstr->LowerBound(); // Lower bound same as constraint value.
        rResult.mType = EConstraintResultType::Consumed;
        if (other_value < mLower) {
          mLower = other_value;
          rResult.mSizeChange = 1;
        } else if (other_value > mUpper) {
          mUpper = other_value;
          rResult.mSizeChange = 1;
        }
        DELETE_CONSTRAINT(pOtherConstr);
      }
      break;
    case EConstraintType::Range:
      {
        rResult.mType = EConstraintResultType::Consumed;
        uint64 size_saved = Size();
        if (pOtherConstr->LowerBound() < mLower) {
          mLower = pOtherConstr->LowerBound();
        }
        if (pOtherConstr->UpperBound() > mUpper) {
          mUpper = pOtherConstr->UpperBound();
        }
        rResult.mSizeChange = Size() - size_saved;
        DELETE_CONSTRAINT(pOtherConstr);
      }
      break;
    default:
      UnexpectedConstraintType(pOtherConstr->Type(), "RangeConstraint::MergeConstraint");
    }
  }

  static inline bool merge_into_range_constraint(uint64 sizeChange, Constraint* pOtherConstr, ConstraintOneResult& rResult)
  {
    rResult.mType = EConstraintResultType::Consumed;
    rResult.mSizeChange = sizeChange;
    DELETE_CONSTRAINT(pOtherConstr);
    return true;
  }

  /*!
    This method is used to process the next Constraint objects in a ConstraintSet after some enflating operation.
    Applicable case will need to make sure this is true: lower bound of pOtherConstr is larger or equals to mUpper.
    With this assumption, the merge condition checking is simpler than other more general case.
    This method will check if merge will actually happen.
  */
  bool RangeConstraint::MergeNextConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult)
  {
    uint64 other_lower = pOtherConstr->LowerBound();
    if ((other_lower - mUpper) > 1) return false;

    switch (pOtherConstr->Type()) {
    case EConstraintType::Value:
      {
        // Lower bound same as constraint value.
        if (other_lower == mUpper) {
          merge_into_range_constraint(0, pOtherConstr, rResult); // no size change.
        }
        else {
          mUpper = other_lower;
          merge_into_range_constraint(1, pOtherConstr, rResult);
        }
      }
      break;
    case EConstraintType::Range:
      {
        uint64 size_saved = Size();
        mUpper = pOtherConstr->UpperBound();
        merge_into_range_constraint(Size() - size_saved, pOtherConstr, rResult);
      }
      break;
    default:
      UnexpectedConstraintType(pOtherConstr->Type(), "RangeConstraint::MergeNextConstraint");
    }
    return true;
  }

  /*!
    Caller should ensure the passed in Constraint object overlaps with the RangeConstraint object.
   */
  void RangeConstraint::SubConstraint(const Constraint& rOtherConstr, ConstraintTwoResult& cResult)
  {
    switch (rOtherConstr.Type()) {
    case EConstraintType::Value:
      {
        cResult.mSizeChange = 1;
        uint64 other_value = rOtherConstr.LowerBound(); // Lower bound same as constraint value
        if (other_value == mLower) {
          // peel off the lower bound value.
          ++ mLower;
        }
        else if (other_value == mUpper) {
          // peel off the upper bound value.
          -- mUpper;
        }
        else {
          // slice the range into two parts
          cResult.mpTailConstraint = GetTailConstraint(other_value + 1);
          mUpper = other_value - 1;
        }
      }
      break;
    case EConstraintType::Range:
      {
        uint64 other_lower = rOtherConstr.LowerBound();
        uint64 other_upper = rOtherConstr.UpperBound();
        uint64 saved_size = Size();
        if (other_lower <= mLower) {
          if (other_upper < mUpper) {
            mLower = other_upper + 1;
            cResult.mSizeChange = saved_size - Size();
          } else {
            cResult.MarkRemove(saved_size);
          }
        }
        else {
          uint64 tail_size = 0;
          if (other_upper < mUpper) {
            cResult.mpTailConstraint = GetTailConstraint(other_upper + 1);
            tail_size = cResult.mpTailConstraint->Size();
          }
          mUpper = other_lower - 1;
          cResult.mSizeChange =  saved_size - Size() - tail_size;
        }
      }
      break;
    default:
      UnexpectedConstraintType(rOtherConstr.Type(), "RangeConstraint::SubConstraint");
    }

    if (mLower == mUpper) {
      cResult.mpHeadConstraint = new ValueConstraint(mLower);
      cResult.mType = EConstraintResultType::Replace;
    }
  }

  bool RangeConstraint::Contains(uint64 value) const
  {
    return (value >= mLower) && (value <= mUpper);
  }

  bool RangeConstraint::Contains(const Constraint& rOtherConstr) const
  {
    switch (rOtherConstr.Type()) {
    case EConstraintType::Value:
      {
        auto value = rOtherConstr.LowerBound();
        return (value >= mLower) && (value <= mUpper);
      }
      break;
    case EConstraintType::Range:
      {
        return (rOtherConstr.LowerBound() >= mLower) && (rOtherConstr.UpperBound() <= mUpper);
      }
      break;
    default:
      UnexpectedConstraintType(rOtherConstr.Type(), "RangeConstraint::Contains");
    }

    return false;
  }

  bool RangeConstraint::Intersects(const Constraint& rOtherConstr) const
  {
    switch (rOtherConstr.Type()) {
    case EConstraintType::Value:
      {
        const ValueConstraint* other_value_constr = static_cast<const ValueConstraint* >(&rOtherConstr);
        return ((mLower <= other_value_constr->Value()) and (other_value_constr->Value() <= mUpper));
      }
      break;
    case EConstraintType::Range:
      {
        const RangeConstraint* other_range_constr = static_cast<const RangeConstraint* >(&rOtherConstr);
        return ((mLower <= other_range_constr->Upper()) and (other_range_constr->Lower() <= mUpper));
      }
      break;
    default:
      UnexpectedConstraintType(rOtherConstr.Type(), "RangeConstraint::Intersects");
    }
    return false;
  }

  /*!
    Make the new mLower and mUpper both aligned using the alignMask.  Also ensure the new mUpper is at least
    (alignSize - 1) less than the old mUpper.
   */
  void RangeConstraint::AlignWithSize(uint64 alignMask, uint64 alignSize, ConstraintOneResult& cResult)
  {
    uint64 saved_size = Size();
    uint64 new_lower = mLower & alignMask;
    if (new_lower < mLower) {
      new_lower += (~alignMask + 1);
    }
    if (new_lower > mUpper || new_lower < mLower) { //check new_lower < mLower again for overflow condition
      cResult.MarkRemove(saved_size);
      return;
    }
    uint64 check_size = (mUpper - new_lower) + 1;
    if (check_size < alignSize) {
      cResult.MarkRemove(saved_size);
      return;
    }
    mLower = new_lower;
    mUpper = (mUpper - (alignSize - 1)) & alignMask;
    cResult.mSizeChange = saved_size - Size();
    if (mLower == mUpper) {
      cResult.mpConstraint = new ValueConstraint(mLower);
      cResult.mType = EConstraintResultType::Replace;
    }
  }

  /*!
    Preconditions:
      alignOffset <= ~alignMask

    Invariants:
      mLower <= mUpper

    Postconditions:
      updated mLower >= original mLower
      updated mUpper + alignSize - 1 <= original mUpper
      updated mLower & ~alignSize == alignOffset
      updated mUpper & ~alignSize == alignOffset

    In the event the postconditions cannot be satisfied, the Constraint will be marked for removal.
   */
  void RangeConstraint::AlignOffsetWithSize(uint64 alignMask, uint64 alignOffset, uint64 alignSize, ConstraintOneResult& cResult)
  {
    uint64 new_lower = (mLower & alignMask) + alignOffset;
    uint64 align_increment = ~alignMask + 1;
    if (new_lower < mLower) {
      new_lower += align_increment;
    }

    if (new_lower < mLower) { //check for overflow case
      cResult.MarkRemove(Size());
      return;
    }

    uint64 new_upper = (mUpper & alignMask) + alignOffset;

    // Minimum amount by which new_upper must be reduced to conform with alignSize
    uint64 excess = new_upper - mUpper + (alignSize - 1);

    if (excess > 0) {
      // aligned_excess is the smallest multiple of align_increment greater than or equal to excess
      uint64 aligned_excess = excess & alignMask;

      if (aligned_excess != excess) {
        aligned_excess += align_increment;
      }

      if (aligned_excess <= new_upper) {
        new_upper -= aligned_excess;
      } else {
        cResult.MarkRemove(Size());
        return;
      }
    }

    if (new_lower == new_upper) {
      cResult.mpConstraint = new ValueConstraint(new_lower);
      cResult.mSizeChange = Size() - 1;
      cResult.mType = EConstraintResultType::Replace;
    } else if (new_lower > new_upper) {
      cResult.MarkRemove(Size());
    } else {
      uint64 old_size = Size();
      mLower = new_lower;
      mUpper = new_upper;
      cResult.mSizeChange = old_size - Size();
    }
  }

  void RangeConstraint::AlignMulDataWithSize(uint64 mulData, uint64 baseRemainder, uint64 alignSize, ConstraintOneResult& cResult)
  {
    uint64 new_lower = (mLower/mulData)*mulData + baseRemainder;
    uint64 align_increment = mulData;
    if (new_lower < mLower){
      new_lower += align_increment;
    }
    if (new_lower < mLower) { // check for overflow case
      cResult.MarkRemove(Size());
      return;
    }

    uint64 new_upper = (mUpper/mulData)*mulData + baseRemainder;
    uint64 excess = new_upper - mUpper + (alignSize - 1);
    if (excess > 0){
      uint64 aligned_excess = (excess/mulData)*mulData;
      if (aligned_excess != excess){
        aligned_excess += align_increment;
      }

      if (aligned_excess <= new_upper){
        new_upper -= aligned_excess;
      } else {
        cResult.MarkRemove(Size());
        return;
      }
    }

    if (new_lower == new_upper) {
      cResult.mpConstraint = new ValueConstraint(new_lower);
      cResult.mSizeChange = Size() - 1;
      cResult.mType = EConstraintResultType::Replace;
    } else if (new_lower > new_upper) {
      cResult.MarkRemove(Size());
    } else {
      uint64 old_size = Size();
      mLower = new_lower;
      mUpper = new_upper;
      cResult.mSizeChange = old_size - Size();
    }
  }

  uint64 RangeConstraint::AlignWithPage(const PageAlignTraits& rAlignTraits)
  {
    uint64 old_size = RangeConstraint::Size();
    // inflate lower and upper to page boundaries, shift bounds by page size
    mLower = mLower & rAlignTraits.mAlignMask;
    mUpper = (mUpper & rAlignTraits.mAlignMask) | rAlignTraits.mOffsetMask;
    RangeConstraint::ShiftRight(rAlignTraits.mShiftAmount);
    // old size should be bigger or equal after shift
    return old_size - RangeConstraint::Size();
  }

  void RangeConstraint::Translate(uint64 pageMask, uint64 pageFrame)
  {
    mLower = (mLower & pageMask) | pageFrame;
    mUpper = (mUpper & pageMask) | pageFrame;
  }

  uint64 RangeConstraint::GetAlignedSizeFromBottom(uint64 alignMask, uint64 alignSize, bool &hasMatch) const
  {
    hasMatch = false;
    if (Size() < alignSize) {
      return 0;
    }

    uint64 new_lower = mLower & alignMask;
    if (new_lower < mLower) {
      new_lower += (~alignMask + 1);
    }
    if (new_lower > mUpper) {
      return 0;
    }

    uint64 check_size = (mUpper - new_lower) + 1;
    if (check_size < alignSize) {
      return 0;
    }

    // passed the checks,
    hasMatch = true;
    return new_lower;
  }

  uint64 RangeConstraint::GetAlignedSizeFromTop(uint64 alignMask, uint64 alignSize, bool &hasMatch) const
  {
    hasMatch = false;

    if (Size() < alignSize) {
      return 0;
    }

    uint64 test_lower = mUpper - (alignSize - 1);
    uint64 aligned_lower = test_lower & alignMask;
    if (aligned_lower < mLower) {
      return 0;
    }

    // passed the checks,
    hasMatch = true;
    return aligned_lower;
  }

  Constraint* RangeConstraint::GetIntersection(const Constraint& rOtherConstr) const
  {
    switch (rOtherConstr.Type()) {
    case EConstraintType::Value:
      {
        auto value = rOtherConstr.LowerBound();
        if ((value >= mLower) && (value <= mUpper)) {
          return rOtherConstr.Clone();
        }
      }
      break;
    case EConstraintType::Range:
      {
        uint64 new_lower = mLower;
        uint64 new_upper = mUpper;
        if (rOtherConstr.LowerBound() > new_lower) new_lower = rOtherConstr.LowerBound();
        if (rOtherConstr.UpperBound() < new_upper) new_upper = rOtherConstr.UpperBound();
        if (new_upper > new_lower) {
          return new RangeConstraint(new_lower, new_upper);
        }
        else if (new_upper == new_lower) {
          return new ValueConstraint(new_lower);
        }

      }
      break;
    default:
      UnexpectedConstraintType(rOtherConstr.Type(), "RangeConstraint::GetIntersection");
    }

    LOG(fail) << "{RangeConstraint::GetIntersection} no intersection found." << endl;
    FAIL("no-intersection-found");
    return nullptr;
  }

  void RangeConstraint::ApplyConstraint(const Constraint& rOtherConstr, ConstraintOneResult& cResult)
  {
    uint64 saved_size = Size();
    switch (rOtherConstr.Type()) {
    case EConstraintType::Value:
      {
        auto value = rOtherConstr.LowerBound();
        if ((value >= mLower) && (value <= mUpper)) {
          cResult.mpConstraint = new ValueConstraint(value);
          cResult.mType = EConstraintResultType::Replace;
          cResult.mSizeChange = saved_size - 1;
        }
        else {
          cResult.MarkRemove(saved_size);
        }
      }
      break;
    case EConstraintType::Range:
      {
        if (rOtherConstr.LowerBound() > mLower) mLower = rOtherConstr.LowerBound();
        if (rOtherConstr.UpperBound() < mUpper) mUpper = rOtherConstr.UpperBound();
        if (mLower > mUpper) {
          cResult.MarkRemove(saved_size);
        }
        else if (mLower == mUpper) {
          cResult.mpConstraint = new ValueConstraint(mLower);
          cResult.mSizeChange = saved_size - 1;
          cResult.mType = EConstraintResultType::Replace;
        }
        else {
          // consumed, record size change.
          cResult.mSizeChange = saved_size - Size();
        }
      }
      break;
    default:
      UnexpectedConstraintType(rOtherConstr.Type(), "RangeConstraint::ApplyConstraint");
    }
  }

  void RangeConstraint::GetValues(std::vector<uint64>& valueVec) const
  {
    for (uint64 i = mLower; i <= mUpper; ++ i) {
      valueVec.push_back(i);
    }
  }

  bool compare_constraints(const Constraint* a, const Constraint* b)
  {
    return (a->UpperBound() < b->LowerBound());
  }

#ifdef UNIT_TEST
  uint32 ConstraintSet::msConstraintDeleteCount = 0;
#endif

  ConstraintSet::ConstraintSet(uint64 lower, uint64 upper)
    : mSize(0), mConstraints()
  {
    AddRange(lower, upper);
  }

  ConstraintSet::ConstraintSet(uint64 value)
    : mSize(0), mConstraints()
  {
    AddValue(value);
  }

  ConstraintSet::ConstraintSet(std::vector<Constraint*>& rConstraints)
    : mSize(0), mConstraints()
  {
    if (rConstraints.size() == 0)
    {
      mSize = 0;
    }
    else
    {
      mConstraints.swap(rConstraints);
      mSize = CalculateSize();
    }
  }

  ConstraintSet::ConstraintSet(const std::string& constrStr)
    : mSize(0), mConstraints()
  {
    StringSplitter ss(constrStr, ',');
    while (!ss.EndOfString()) {
      string sub_str = ss.NextSubString();
      uint64 range_low = 0, range_high = 0;
      Constraint* new_constr = nullptr;
      if (parse_range64(sub_str, range_low, range_high)) {
        // actual range
        new_constr = new RangeConstraint(range_low, range_high);
      } else {
        new_constr = new ValueConstraint(range_low);
      }
      MergeConstraint(new_constr);
    }
  }

  ConstraintSet::ConstraintSet(const ConstraintSet& rOther)
    : mSize(rOther.mSize), mConstraints()
  {
    mConstraints.reserve(rOther.mConstraints.size());
    transform(rOther.mConstraints.begin(), rOther.mConstraints.end(), back_inserter(mConstraints), [](Constraint* constr_item) { return constr_item->Clone(); });
  }

  ConstraintSet::~ConstraintSet()
  {
    for (auto constr_item : mConstraints) {
      DELETE_CONSTRAINT(constr_item);
    }
  }

  ConstraintSet* ConstraintSet::Clone() const
  {
    return new ConstraintSet(*this);
  }

  /*!
    Do not use this much in production code, mostly intended for unit testing usage.
   */
  ConstraintSet& ConstraintSet::operator=(const ConstraintSet& rOther)
  {
#ifdef UNIT_TEST
    if (&rOther != this) {
      Clear();
      mSize = rOther.mSize;
      mConstraints.reserve(rOther.mConstraints.size());
      transform(rOther.mConstraints.begin(), rOther.mConstraints.end(), back_inserter(mConstraints), [](Constraint* constr_item) { return constr_item->Clone(); });
    }
#else
    LOG(fail) << "{ConstraintSet::operator=} not meant to be used in production code." << endl;
    FAIL("do-not-use-constraint-setcopy-assignment-operator");
#endif

    return *this;
  }

  bool ConstraintSet::operator==(const ConstraintSet& rOther) const
  {
    if ((this->mSize == rOther.mSize) && (this->mConstraints.size() == rOther.mConstraints.size())) {
      bool equal = true;

      for (size_t i = 0; i < this->mConstraints.size(); i++) {
        if ((*(this->mConstraints[i])) != (*rOther.mConstraints[i])) {
          equal = false;
          break;
        }
      }

      return equal;
    }

    return false;
  }

  bool ConstraintSet::operator!=(const ConstraintSet& rOther) const
  {
    return not ((*this) == rOther);
  }

  void ConstraintSet::Clear()
  {
    for (auto constr_item : mConstraints) {
      DELETE_CONSTRAINT(constr_item);
    }
    mConstraints.clear();
    mSize = 0;
  }

  uint64 ConstraintSet::CalculateSize() const
  {
    return accumulate(mConstraints.cbegin(), mConstraints.cend(), uint64(0),
      [](cuint64 size, const Constraint* constr_item) { return size + constr_item->Size(); });
  }

  uint64 ConstraintSet::LowerBound() const
  {
    if (IsEmpty()) {
      LOG(fail) << "{ConstraintSet::LowerBound} empty ConstraintSet no lower bound." << endl;
      FAIL("empty-constraint-set-no-bound");
    }
    return mConstraints.front()->LowerBound();
  }

  uint64 ConstraintSet::UpperBound() const
  {
    if (IsEmpty()) {
      LOG(fail) << "{ConstraintSet::UpperBound} empty ConstraintSet no upper bound." << endl;
      FAIL("empty-constraint-set-no-bound");
    }
    return mConstraints.back()->UpperBound();
  }

  uint64 ConstraintSet::ChooseValue() const
  {
    uint64 total_size = CalculateSize(); // mSize; TODO make it possible to avoid calculating all weights every time

    // Use IsEmpty() here rather than comparing the size to 0. There's a corner case in which the ConstraintSet
    // encompasses all possible 64-bit values; such a ConstraintSet will have a size of 0 even though it is not empty.
    if (IsEmpty()) {
      stringstream err_stream;
      err_stream << "ConstraintSet is empty.";
      throw ConstraintError(err_stream.str());
    }

    uint64 picked_value = Random::Instance()->Random64(0, total_size - 1);
    uint64 half_size = total_size >> 1;
    if (picked_value > half_size) {
      return ChosenValueFromBack(picked_value, total_size); // TODO, just use mSize without passing in repeatedly calculated size.
    } else {
      return ChosenValueFromFront(picked_value);
    }
  }

  void ConstraintSet::FailedChoosingValue(uint64 offset, const std::string& additionalMsg) const
  {
    LOG(fail) << "Failed to choose a value with randomly picked offset : 0x" << hex << offset << " calling from \"" << additionalMsg << "\"." << endl;
    FAIL("failed-choosing-value");
  }

  void ConstraintSet::AddRange(uint64 lower, uint64 upper)
  {
    Constraint* new_constr = nullptr;
    if (upper < lower) {
      new_constr = new RangeConstraint(upper, lower);
    }
    else if (upper == lower) {
      new_constr = new ValueConstraint(upper);
    }
    else {
      new_constr = new RangeConstraint(lower, upper);
    }

    MergeConstraint(new_constr);
  }

  void ConstraintSet::AddValue(uint64 value)
  {
    auto new_constr = new ValueConstraint(value);
    MergeConstraint(new_constr);
  }

  void ConstraintSet::MergeConstraint(Constraint* new_constr)
  {
    if (IsEmpty()) {
      mConstraints.push_back(new_constr);
      mSize += new_constr->Size();
      return;
    }

    uint64 search_lower = new_constr->LowerBound();
    uint64 search_upper = new_constr->UpperBound();
    auto range_pair = search_for_merge(mConstraints.begin(), mConstraints.end(), search_lower, search_upper);
    auto range_dist = std::distance(range_pair.first, range_pair.second);

    switch (range_dist) {
    case 0: // no overlap, just insert
      mConstraints.insert(range_pair.first, new_constr);
      mSize += new_constr->Size();
      break;
    case 1: // overlap with one Constraint object.
      mSize += merge_into_constraint(range_pair.first, new_constr);
      break;
    default: // overlap with multiple Constraint object.
      {
        if (new_constr->Type() == EConstraintType::Value) {
          // the case of a single value fit in between two ranges.
          DELETE_CONSTRAINT(new_constr);
          new_constr = new RangeConstraint(search_lower, search_upper);
        }

        mSize += merge_with_contiguous_constraints(range_pair.first, range_pair.second, new_constr);
        // replacing the constraints in range with a new RangeConstraint.
        *(range_pair.first) = new_constr;
        mConstraints.erase(range_pair.first + 1, range_pair.second);
      }
    }
  }

  /*!
    \class ConstraintSetMerger
    \brief The class managing subtracting on ConstraintSet from another.
  */
  class ConstraintSetMerger : public ConstraintSetBinaryProcessor {
  public:
    ConstraintSetMerger(ConstraintSet& rTargetConstrSet, const ConstraintYielder& rConstrYielder) : ConstraintSetBinaryProcessor(rTargetConstrSet, rConstrYielder) { } //!< Constructor.

    DESTRUCTOR_DEFAULT(ConstraintSetMerger);
    ASSIGNMENT_OPERATOR_ABSENT(ConstraintSetMerger);
    COPY_CONSTRUCTOR_ABSENT(ConstraintSetMerger);
    DEFAULT_CONSTRUCTOR_ABSENT(ConstraintSetMerger);
  private:
    /*!
      Process a Constraint to be merged.
    */
    inline void ProcessConstraint() override //!< Process one or a block of Constraints.
    {
      const Constraint* merge_constr = mrYielder.NextItem();
      uint64 search_lower = merge_constr->LowerBound();
      uint64 search_upper = merge_constr->UpperBound();
      auto range_pair = search_for_merge(mSearchStartIterator, mrConstraintVector.end(), search_lower, search_upper);
      auto range_dist = std::distance(range_pair.first, range_pair.second);
      switch (range_dist) {
      case 0:
        // no overlap, need to insert the to be merged Constraint.
        AddCloneConstraintBlock(range_pair.first, *merge_constr);
        break;
      case 1:
        // overlap with one Constraint object.  Go ahead and mearge into it.
        MergeIntoOneConstraint(range_pair.first, *merge_constr);
        break;
      default:
        {
          Constraint* clone_constr = nullptr;
          if (merge_constr->Type() == EConstraintType::Value) {
            // the case of a single value fit in between two ranges.
            clone_constr = new RangeConstraint(search_lower, search_upper);
          }
          else {
            clone_constr = merge_constr->Clone();
          }
          MergeWithContiguousConstraints(range_pair.first, range_pair.second, clone_constr);
        }
      }
    }

    /*!
      When the to-be-merged Constraint does not overlap with anything in the target ConstraintSet,
      look into the Yielder to see if multiple next Constraint objects can be included in the same CloneConstraintBlock.
    */
    inline void AddCloneConstraintBlock(ConstraintIterator nextIter, const Constraint& rMergeConstr)
    {
      CommitConstraintBlock(nextIter);

      ConstConstraintIterator last_iter = mrYielder.LastIterator();
      ConstConstraintIterator end_iter;
      if (nextIter == mrConstraintVector.end()) {
        end_iter = mrYielder.EndIterator();
      }
      else {
        auto next_constr = *nextIter;
        // Construct a ValueConstraint with the upper boundary Constraint's lower bound - 1.
        ValueConstraint search_constr(next_constr->LowerBound() - 1);
        end_iter = mrYielder.GetLowerBound(search_constr);
      }
      auto clone_block = mrYielder.YieldCloneConstraintBlock(last_iter, end_iter);
      mBlocks.push_back(clone_block);
      mSizeChange += clone_block->SizeChange(); // add to ConstraintSet size.
      mVectorSize += clone_block->Size(); // add to vector size.
      mSearchStartIterator = nextIter; // move search start iterator to the upper bound of last search.
    }

    /*!
      When the to-be-merged Constraint overlaps with only one target Constraint, it can be merged into it.  Furthermore,
      in the case when target Constraint's upper bound is larger than merged Constraint, we will check with the yielder to see if there
      are more Constraints that can be merged to the target Constraint.
    */
    inline void MergeIntoOneConstraint(ConstraintIterator targetIter, const Constraint& rMergeConstr)
    {
      Constraint* clone_to_merge = rMergeConstr.Clone();
      mSizeChange += merge_into_constraint(targetIter, clone_to_merge);

      auto target_constr = *targetIter;
      uint64 target_upper = target_constr->UpperBound();

      if (target_upper > rMergeConstr.UpperBound()) {
        ConstConstraintIterator yielder_current = mrYielder.CurrentIterator();
        ConstConstraintIterator upper_iter = mrYielder.GetUpperBound(*target_constr);

        if (std::distance(yielder_current, upper_iter) > 0) {
          auto last_merge_iter = upper_iter - 1;
          for (; last_merge_iter != yielder_current; -- last_merge_iter) {
            auto last_merge_constr = *last_merge_iter;
            if (last_merge_constr->UpperBound() <= target_upper) {
              // Set yielder iterator to one past the last merged Constraint.
              mrYielder.SetIterator(last_merge_iter + 1);
              break;
            }
          }
        }
      }
    }

    /*!
      Use merge_with_contiguous_constraints function to merge with multiple Constraints.
    */
    inline void MergeWithContiguousConstraints(ConstraintIterator startIter, ConstraintIterator endIter, Constraint* pMergeConstr)
    {
      mSizeChange += merge_with_contiguous_constraints(startIter, endIter, pMergeConstr);

      ConstraintIterator new_start = endIter - 1;
      *(new_start) = pMergeConstr;

      // last block end is startIter. Start a new block at endIter - 1.
      CommitConstraintBlockWithGap(startIter, new_start);
      // new search starts with the new_start position as well.
      mSearchStartIterator = new_start;
    }

  };

  void ConstraintSet::MergeConstraintSet(const ConstraintSet& rConstrSet)
  {
    if (IsEmpty()) {
      mConstraints.reserve(rConstrSet.mConstraints.size());
      for (auto constr_item: rConstrSet.mConstraints) {
        auto new_constr = constr_item->Clone();
        mConstraints.push_back(new_constr);
        mSize += new_constr->Size();
      }
      return;
    }

    if (rConstrSet.IsEmpty()) {
      // merging with empty ConstraintSet, nothing to do.
      return;
    }

    ConstraintYielder merge_yielder(rConstrSet);
    auto range_dist = merge_yielder.YieldRange();

    switch (range_dist) {
    case 0:
      // if we got here, there must be an error, since there was an empty check above.
      LOG(fail) << "{ConstraintSet::MergeConstraintSet} unexpected empty range." << endl;
      FAIL("unexpected-empty-range");
      return;
    case 1:
      // just use MergeConstraint.
      MergeConstraint(merge_yielder.FrontItem()->Clone());
      return;
    default: // go on to next section code.
      ;
    }

    ConstraintSetMerger merge_processor(*this, merge_yielder);
    merge_processor.Process();
    mSize += merge_processor.SizeChange();
  }

  void ConstraintSet::SubRange(uint64 lower, uint64 upper)
  {
    if (upper < lower) {
      RangeConstraint new_constr(upper, lower);
      SubConstraint(new_constr);
    }
    else if (upper == lower) {
      ValueConstraint new_constr(upper);
      SubConstraint(new_constr);
    }
    else {
      RangeConstraint new_constr(lower, upper);
      SubConstraint(new_constr);
    }
  }

  void ConstraintSet::SubValue(uint64 value)
  {
    ValueConstraint new_constr(value);
    SubConstraint(new_constr);
  }

  void ConstraintSet::SubConstraint(const Constraint& constrSub)
  {
    if (IsEmpty()) {
      // ConstraintSet already empty.
      return;
    }

    auto range_pair = std::equal_range(mConstraints.begin(), mConstraints.end(), &constrSub, &compare_constraints);
    auto range_dist = std::distance(range_pair.first, range_pair.second);

    if (range_dist == 0) {
      // no intersection
      return;
    }

    Constraint* target_constr = *(range_pair.first);
    ConstraintTwoResult cs_result;
    target_constr->SubConstraint(constrSub, cs_result);

    if (range_dist == 1) {
      // intersect with one Constraint object.
      switch (cs_result.mType) {
      case EConstraintResultType::Remove:
        DELETE_CONSTRAINT(target_constr);
        mConstraints.erase(range_pair.first); // iterators become invalid.
        break;
      case EConstraintResultType::Replace:
        DELETE_CONSTRAINT(target_constr);
        *(range_pair.first) = cs_result.GiveHeadConstraint();
        // fall through
      case EConstraintResultType::Consumed:
        if (cs_result.mpTailConstraint) {
          mConstraints.insert(range_pair.first + 1, cs_result.GiveTailConstraint()); // iterators become invalid.
        }
        break;
      default:
        LOG(fail) << "Unexpected constraint result type: \"" << EConstraintResultType_to_string(cs_result.mType) << endl;
        FAIL("unhandled-constraint-result-type");
      }
      mSize -= cs_result.mSizeChange;
      return;
    }

    ConstraintDistanceType delete_start_dist = 0;
    ConstraintDistanceType delete_end_dist = range_dist;
    switch (cs_result.mType) {
    case EConstraintResultType::Remove:
      // remove item start from dist=0
      break;
    case EConstraintResultType::Replace:
      DELETE_CONSTRAINT(target_constr);
      *(range_pair.first) = cs_result.GiveHeadConstraint();
      // fall through
    case EConstraintResultType::Consumed:
      mSize -= cs_result.mSizeChange;
      ++ delete_start_dist; // deleting range start from dist=1
      break;
    default:
      LOG(fail) << "While handling sub constraint range start, unexpected constraint result type: \"" << EConstraintResultType_to_string(cs_result.mType) << endl;
      FAIL("unexpected-sub-constraint-result-type");
    }

    auto last_constr_iter = range_pair.second - 1;
    Constraint* last_constr = *(last_constr_iter);
    ConstraintTwoResult cs_result_last;
    last_constr->SubConstraint(constrSub, cs_result_last);
    switch (cs_result_last.mType) {
    case EConstraintResultType::Remove:
      // remove item to range_pair.second.
      break;
    case EConstraintResultType::Replace:
      DELETE_CONSTRAINT(last_constr);
      *(last_constr_iter) = cs_result_last.GiveHeadConstraint();
      // fall through
    case EConstraintResultType::Consumed:
      mSize -= cs_result_last.mSizeChange;
      -- delete_end_dist; // deleting range shrink by 1
      break;
    default:
      LOG(fail) << "While handling sub constraint range end, unexpected constraint result type: \"" << EConstraintResultType_to_string(cs_result.mType) << endl;
      FAIL("unexpected-sub-constraint-result-type");
    }

    if (delete_start_dist < delete_end_dist) {
      ConstraintIterator del_iter = range_pair.first + delete_start_dist;
      ConstraintIterator end_iter = range_pair.first + delete_end_dist;
      DeleteRange(del_iter, end_iter);
    }
  }

  void ConstraintSet::DeleteRange(ConstraintIterator startIter, ConstraintIterator endIter)
  {
    ConstraintIterator delIter = startIter;
    uint64 deleted_size = 0;
    for (; delIter != endIter; ++ delIter) {
      deleted_size += (*delIter)->Size();
      DELETE_CONSTRAINT((*delIter));
    }
    mConstraints.erase(startIter, endIter);
    mSize -= deleted_size;
  }

  /*!
    \class ConstraintSetSubtractor
    \brief The class managing subtracting on ConstraintSet from another.
  */
  class ConstraintSetSubtractor : public ConstraintSetBinaryProcessor {
  public:
    ConstraintSetSubtractor(ConstraintSet& rTargetConstrSet, const ConstraintYielder& rConstrYielder) : ConstraintSetBinaryProcessor(rTargetConstrSet, rConstrYielder) { } //!< Constructor.

    DESTRUCTOR_DEFAULT(ConstraintSetSubtractor);
    ASSIGNMENT_OPERATOR_ABSENT(ConstraintSetSubtractor);
    COPY_CONSTRUCTOR_ABSENT(ConstraintSetSubtractor);
    DEFAULT_CONSTRUCTOR_ABSENT(ConstraintSetSubtractor);
  private:
    /*!
      Putting the tail Constraint into a InsertConstraintBlock.  Will check if it cover any Constraint objects in the subtrahend.
    */
    void ProcessTailInsertion(Constraint* pConstr)
    {
      auto range_pair = mrYielder.GetMatchingRange(*pConstr);
      uint64 constr_upper = pConstr->UpperBound();
      uint64 more_size_change = 0;
      std::vector<Constraint* > constr_vec;
      Constraint* current_constr = pConstr;
      // << "starting out: " << current_constr->ToSimpleString() << endl;
      for (auto sub_iter = range_pair.first; sub_iter != range_pair.second; ++ sub_iter) {
        const Constraint* sub_constr = *sub_iter;
        // << "subtracting: " << sub_constr->ToSimpleString() << endl;
        uint64 size_change = 0;
        auto sub_pair = subtract_from_constraint(current_constr, sub_constr, size_change);
        more_size_change += size_change;
        auto head_constr = sub_pair.first;
        if (head_constr != current_constr) DELETE_CONSTRAINT(current_constr); // avoid memory leak.
        if (nullptr != head_constr) {
          // << "adding constraint to insert: " << head_constr->ToSimpleString() << endl;
          constr_vec.push_back(head_constr);
        }
        current_constr = sub_pair.second;
        if (nullptr == current_constr) break;
        // << "current constr now: " << current_constr->ToSimpleString() << endl;
      }

      if (nullptr != current_constr) { // check if there is a residual item.
        // << "adding last constraint to insert: " << current_constr->ToSimpleString() << flush << endl;
        constr_vec.push_back(current_constr);
      }

      if (constr_vec.size()) {
        auto insert_block = new InsertConstraintBlock(constr_vec);
        mBlocks.push_back(insert_block);
        mVectorSize += insert_block->Size();
      }
      else {
        LOG(fail) << "{ConstraintSetSubtractor::ProcessTailInsertion} expect the vector to be NOT empty." << endl;
        FAIL("unexpected-empty-vector");
      }

      if (std::distance(range_pair.first, range_pair.second) > 0) {
        // adjust mrYielder's current iterator.
        auto new_yielder_iter = range_pair.second;
        auto check_iter = new_yielder_iter;
        -- check_iter;
        Constraint* last_item = *check_iter;
        if (last_item->UpperBound() > constr_upper) {
          // still need to keep this item for next step.
          new_yielder_iter = check_iter;
        }
        mrYielder.SetIterator(new_yielder_iter);
      }

      mSizeChange += more_size_change;
    }

    /*!
      Handle subtracting one Constraint from one Constraint.
    */
    inline ConstraintIterator SubFromOneConstraint(ConstraintIterator constrIter, ConstraintIterator nextIter, const Constraint& rSubConstr)
    {
      ConstraintIterator ret_iter = nextIter;
      Constraint* target_constr = *(constrIter);
      ConstraintTwoResult cs_result;
      target_constr->SubConstraint(rSubConstr, cs_result);

      switch (cs_result.mType) {
      case EConstraintResultType::Remove:
        DELETE_CONSTRAINT(target_constr);
        (*constrIter) = nullptr;
        CommitConstraintBlockWithGap(constrIter, nextIter);
        break;
      case EConstraintResultType::Replace:
        DELETE_CONSTRAINT(target_constr);
        *(constrIter) = cs_result.GiveHeadConstraint();
        if (cs_result.mpTailConstraint) {
          CommitConstraintBlock(constrIter + 1); // end iterator is the next iterator.
          ProcessTailInsertion(cs_result.GiveTailConstraint());
        }
        break;
      case EConstraintResultType::Consumed:
        if (cs_result.mpTailConstraint) {
          CommitConstraintBlock(constrIter + 1); // end iterator is the next iterator.
          ProcessTailInsertion(cs_result.GiveTailConstraint());
        }
        else if (target_constr->UpperBound() > rSubConstr.UpperBound()) {
          ret_iter = constrIter; // need to keep the iterator on the current item since it could match later subtrahend Constraints.
        }
        break;
      default:
        LOG(fail) << "Unexpected constraint result type: \"" << EConstraintResultType_to_string(cs_result.mType) << endl;
        FAIL("unhandled-constraint-result-type");
      }
      mSizeChange += cs_result.mSizeChange;
      return ret_iter;
    }

    /*!
      Subtranct a Constraint from a contiguous range of Constraints.  It has been determined that the subtrahend Constraint will intersect with
      the first and last Constraint in the range, and everything in the middle should be deleted.
    */
    inline ConstraintIterator SubFromContiguousConstraints(ConstraintIterator startIter, ConstraintIterator endIter, const Constraint& rSubConstr)
    {
      uint64 size_change = 0;
      ConstraintIterator next_start = endIter;

      auto dist_pair = subtract_from_contiguous_constraints(startIter, endIter, rSubConstr, size_change);
      auto delete_start_dist = dist_pair.first;
      auto delete_end_dist = dist_pair.second;

      if (delete_start_dist < delete_end_dist) {
        ConstraintIterator del_iter = startIter + delete_start_dist;
        ConstraintIterator end_iter = startIter + delete_end_dist;
        next_start = end_iter;
        CommitConstraintBlockWithGap(del_iter, end_iter);

        for (; del_iter != end_iter; ++ del_iter) {
          size_change += (*del_iter)->Size();
          DELETE_CONSTRAINT((*del_iter));
        }
      }
      else {
        // check if last item in range could still have space left that could overlap with in coming Constraint to be subtraced
        auto last_constr_iter = endIter - 1;
        Constraint* last_constr = *(last_constr_iter);
        if ((last_constr->UpperBound() > rSubConstr.UpperBound()) and (last_constr->Size() > 1)) {
          // possibly further overlaps.
          next_start = last_constr_iter;
        }
      }
      mSizeChange += size_change;

      return next_start;
    }

    /*!
      Process a subtrahend Constraint.
    */
    inline void ProcessConstraint() override //!< Process one or a block of Constraints.
    {
      const Constraint* sub_constr = mrYielder.NextItem();
      auto range_pair = std::equal_range(mSearchStartIterator, mrConstraintVector.end(), sub_constr, &compare_constraints);
      auto match_size = std::distance(range_pair.first, range_pair.second);

      switch (match_size) {
      case 0:
        mSearchStartIterator = range_pair.second; // No match, update search start.
        return;
      case 1:
        mSearchStartIterator = SubFromOneConstraint(range_pair.first, range_pair.second, *sub_constr); // handle one Consraint case.
        break;
      default:
        mSearchStartIterator = SubFromContiguousConstraints(range_pair.first, range_pair.second, *sub_constr); // subtracting from a range of contiguous Constraints.
      }
    }
  };

  void ConstraintSet::SubConstraintSet(const ConstraintSet& rConstrSet)
  {
    if (IsEmpty()) return; // already empty

    ConstraintYielder sub_yielder(rConstrSet, LowerBound(), UpperBound()); // subtrahend
    auto range_dist = sub_yielder.YieldRange();

    switch (range_dist) {
    case 0:
      // no intersection.
      return;
    case 1:
      // just use SubConstraint.
      SubConstraint(*sub_yielder.FrontItem());
      return;
    default: // go on to next section code.
      ;
    }

    ConstraintSetSubtractor sub_processor(*this, sub_yielder);
    sub_processor.Process();
    mSize -= sub_processor.SizeChange();
  }

  void ConstraintSet::AddDividedElementConstraint(cuint64 lowerBound, cuint64 upperBound, cuint64 divisor, cuint64 factor, cuint64 overflowQuotient)
  {
    uint64 new_lowerBound = (lowerBound / divisor) + overflowQuotient * factor;

    if ((lowerBound % divisor) != 0) {
      new_lowerBound += 1;
    }

    uint64 new_upperBound = (upperBound / divisor) + overflowQuotient * factor;

    if (new_lowerBound < new_upperBound) {
      AddRange(new_lowerBound, new_upperBound);
    }
    else if (new_lowerBound == new_upperBound) {
      AddValue(new_lowerBound);
    }
    // else there are no valid values corresponding to this RangeConstraint for this factor
  }

  /*!
    Used in ApplyConstraintSet/ApplyConstraint etc.  Delete/erase the range from eraseureIter to the end of the Constraint vector..
   */
  static void delete_constraint_vector_tail(vector<Constraint* >& constrVec, const ConstraintIterator& erasureIter, ConstraintOneResult& cResult)
  {
#ifdef UNIT_TEST
    LOG(notice) << "{delete_constraint_vector_tail} delete iterator distance to end: " << dec << std::distance(erasureIter, constrVec.end()) << endl;
#endif
    cResult.Clear();
    bool has_non_null = false;
    auto through_iter = erasureIter;
    for (; through_iter != constrVec.end(); ++ through_iter) {
      if (nullptr != (*through_iter)) {
        if (not has_non_null) {
          // first non-null
          has_non_null = true;
        }
        cResult.mSizeChange += (*through_iter)->Size();
        DELETE_CONSTRAINT((*through_iter));
        (*through_iter) = nullptr;
      }
      else if (has_non_null) {
        LOG(fail) << "{delete_constraint_vector_tail} not expecting nullptr after non-null Constraint object." << endl;
        FAIL("inconsistent-constraint-vector-tail");
      }
    }
  }

  /*!
    Used in ApplyConstraintSet/ApplyConstraint etc.  Apply applyConstr Constraint to the Constraint object pointed to by applierIter.
   */
  static void apply_constraint_in_place(ConstraintIterator& applierIter, const Constraint& applyConstr, ConstraintOneResult& cResult)
  {
    cResult.Clear();
    (*applierIter)->ApplyConstraint(applyConstr, cResult);
    switch (cResult.mType) {
    case EConstraintResultType::Replace:
      DELETE_CONSTRAINT((*applierIter));
      *(applierIter) = cResult.GiveConstraint();
      break;
    case EConstraintResultType::Consumed:
      // does nothing.
      break;
    default:
      LOG(fail) << "{apply_constraint_in_place} unexpected constraint result type: \"" << EConstraintResultType_to_string(cResult.mType) << "\"" << endl;
      FAIL("unexpected-apply-constraint-in-place-result-type");
    }
  }

  /*!
    Move a range of Constraint objects in reverse order before inserting delayed insertion items.
   */
  static inline void finalize_reverse_move_range(ConstraintIterator& copySrcIterator, ConstraintIterator& copyDestIterator, const ConstraintIterator insertLocation, const ConstraintIterator beginIterator)
  {
    for (; copySrcIterator != insertLocation; -- copySrcIterator) {
      (*copyDestIterator) = (*copySrcIterator);
      (*copySrcIterator) = nullptr;
      -- copyDestIterator;
    }

    // copy last item (reverse order)
    if ((nullptr != (*copySrcIterator)) and (copyDestIterator != copySrcIterator)) {
      (*copyDestIterator) = (*copySrcIterator);
      (*copySrcIterator) = nullptr;
      if (copySrcIterator != beginIterator) {
        -- copySrcIterator;
      }
      -- copyDestIterator;
    }
#ifdef UNIT_TEST
    else if (nullptr != (*copySrcIterator)) {
      LOG(fail) << "{finalize_reverse_move_range} expecting copySrcIterator to point to nullptr." << endl;
      FAIL("finalize_reverse_move_range-dangling-pointer-at-copy-source");
    }
#endif
  }

  /*!
    \class ConstraintApplier
    \brief A class used in ConstraintSet::ApplyConstraintSet to organize related code more nicely.
  */
  class ConstraintApplier {
  public:
    explicit ConstraintApplier(vector<Constraint* >& constrVec) //!< Constructor with Constraint vector reference given.
      : mrConstraintVector(constrVec), mSearchStartIterator(), mInsertIterator(), mMatchStart(), mMatchEnd(), mCopyEnd(), mLastInRange(), mpApplyConstraint(nullptr), mpTailItem(nullptr), mpLastTailItem(nullptr), mHasOverlap(false), mRangeDistance(0), mSizeChange(0), mKeepTail(false), mResult(), mDelayedInsertions()
    {
      mSearchStartIterator = mrConstraintVector.begin();
      mInsertIterator = mSearchStartIterator;
      mMatchStart = mSearchStartIterator;
      mMatchEnd = mSearchStartIterator;
      mCopyEnd = mSearchStartIterator;
      mLastInRange = mSearchStartIterator;
    }

    ASSIGNMENT_OPERATOR_ABSENT(ConstraintApplier);
    COPY_CONSTRUCTOR_DEFAULT(ConstraintApplier);
  private:
    void Search() //!< Searching for equal_range matches in the Constraint object vector.
    {
      auto range_pair = std::equal_range(mSearchStartIterator, mrConstraintVector.end(), mpApplyConstraint, &compare_constraints);
      mMatchStart = range_pair.first;
      mMatchEnd = range_pair.second;
      mRangeDistance = std::distance(mMatchStart, mMatchEnd);
      mKeepTail = false;
      // << "search range distance " << dec << mRangeDistance << flush << " match end pos " << IterPos(mMatchEnd) << flush << endl;
    }

    bool HasMatch() const { return (mRangeDistance > 0); } //!< Return true if there is any match.

    void CleanUpLeadingRange() //!< Cleanup the Constraint object in the range leading to the match start
    {
      // << "{CleanUpLeadingRange} current Constraint: " << mpApplyConstraint->ToString() << " search start dist " << dec << IterPos(mSearchStartIterator) << " match start dist " << IterPos(mMatchStart) << flush << endl;
      auto clean_gap = std::distance(mSearchStartIterator, mMatchStart);
      if (TryInsertLastTailItem(clean_gap, false)) {
        if (std::distance(mInsertIterator, mMatchStart) < clean_gap) { // insert iterator advanced past search-start-iterator
          ++ mSearchStartIterator;
        }
      }

      if (mSearchStartIterator == mMatchStart) {
        // << "nothing to clean up." << flush << endl;
        return; // Nothing to clean up.
      }

      // << "before cleaned up leading " << to_simple_string_debug(mrConstraintVector) << endl;
      for (ConstraintIterator del_iter = mSearchStartIterator; del_iter != mMatchStart; ++ del_iter) {
        mSizeChange += (*del_iter)->Size();
        DELETE_CONSTRAINT((*del_iter));
        (*del_iter) = nullptr;
      }

      // << "after cleaned up leading " << to_simple_string_debug(mrConstraintVector) << " leading distance " << endl;
    }

    bool TryInsertLastTailItem(ConstraintDistanceType insertGap, bool isEnd) //!< Try to insert last tail item, return true if inserted.
    {
      bool ret_result = false;
      if (nullptr != mpLastTailItem) {
        if (insertGap > 0) {
          // last tail item can be inserted
          if (nullptr != (*mInsertIterator)) {
            mSizeChange += (*mInsertIterator)->Size();
          }
          DELETE_CONSTRAINT((*mInsertIterator));
          (*mInsertIterator) = nullptr;
          // << "inserting tail item " << mpLastTailItem->ToString() << " is end ? " << isEnd << endl;
          InsertItem(mpLastTailItem);
          mSizeChange -= mpLastTailItem->Size();
          ret_result = true;
        }
        else {
          // nowhere to insert right now, save it up
          // << "saving up item" << flush << endl;
          mDelayedInsertions.push_back(std::make_pair(std::distance(mrConstraintVector.begin(), mInsertIterator), mpLastTailItem));
        }
        mpLastTailItem = nullptr;
      }
      return ret_result;
    }

    void InsertItem(Constraint* constr)
    {
#ifdef UNIT_TEST
      LOG(notice) << "{ConstraintApplier::InsertItem} insertion iterator distance from beginning: " << dec << IterPos(mInsertIterator) << endl;
      if (nullptr != (*mInsertIterator)) {
        LOG(fail) << "{ConstraintApplier::InsertItem} dangling Constraint object pointer, inserting: " << constr->ToString() << " existing: " << (*mInsertIterator)->ToString() << endl;
        FAIL("dangling-constraint-pointer");
      }
#endif
      (*mInsertIterator) = constr;
      ++ mInsertIterator;
      // << "incremented insert iter, now " << IterPos(mInsertIterator) << flush << endl;
    }

    void ProcessTailItem() //!< Process the last item in the search-matching range.
    {
      mLastInRange = mMatchStart + (mRangeDistance - 1);
      mCopyEnd = mMatchEnd;

      // process last item in the resulting range of the search
      const Constraint* last_in_range = *(mLastInRange);
      // << "last in range: " << last_in_range->ToSimpleString() << flush << endl;
      if ((last_in_range->UpperBound() > mpApplyConstraint->UpperBound()) && ((last_in_range->UpperBound() - mpApplyConstraint->UpperBound()) > 1)) {
        // last item in range need to be kept around since next item could overlap it.
        mpTailItem = last_in_range->GetIntersection(*mpApplyConstraint);
        mSearchStartIterator = mLastInRange; // next search still need to search from last item in range.
        mCopyEnd = mLastInRange;
        mKeepTail = true;
      } else {
        apply_constraint_in_place(mLastInRange, *mpApplyConstraint, mResult);
        mSizeChange += mResult.mSizeChange;
        mSearchStartIterator = mMatchEnd;
      }
    }

    void ProcessHeadItem() //!< Process the first item in the search-matching range
    {
      // process the first item in the resulting range of the search
      if (mRangeDistance > 1) {
        apply_constraint_in_place(mMatchStart, *mpApplyConstraint, mResult);
        mSizeChange += mResult.mSizeChange;
      }
      else if ((nullptr != mpTailItem) && (std::distance(mInsertIterator, mMatchStart) > 0)) {
        // head and tail item is the same, if possible, insert the mpTailItem now.
        // << "inserting tail item when head=tail: " << mpTailItem->ToString() << endl;
        InsertItem(mpTailItem);
        mSizeChange -= mpTailItem->Size();
        mpTailItem = nullptr;
      }
    }

    inline ConstraintDistanceType IterPos(ConstraintIterator applierIter) const //!< Return iterator position in terms of distance from beginning of vector
    {
      return std::distance(mrConstraintVector.begin(), applierIter);
    }

    void MoveMatchRange() //!< Move Constraint items in the search-matching range if necessary
    {
      ConstraintDistanceType insert_gap = std::distance(mInsertIterator, mMatchStart);
      // << "insert dist " << dec << IterPos(mInsertIterator) << " match start dist " << IterPos(mMatchStart) << " match end dist " << IterPos(mMatchEnd) << " copy end " << IterPos(mCopyEnd) << " insert gap " << insert_gap << endl;

      // move the items in range if necessary.
      if (insert_gap == 0) {
        // no need to move, check if there the tail of the match range is kept around.
        if (mKeepTail) {
          mInsertIterator = mLastInRange;
          // << "assigned last in range to insert iter, now " << IterPos(mInsertIterator) << flush << endl;
        }
        else {
          mInsertIterator = mMatchEnd;
          // << "assigned match end to insert iter, now " << IterPos(mInsertIterator) << flush << endl;
        }
      } else {
        // << "before moving items " << to_simple_string_debug(mrConstraintVector) << endl;
        // move items
        for (ConstraintIterator move_iter = mMatchStart; move_iter != mCopyEnd; ++ move_iter) {
          Constraint* move_item = (*move_iter);
          (*move_iter) = nullptr;
          // << "move match range item " << move_item->ToString() << endl;
          InsertItem(move_item);
        }
        // << "after moving items " << to_simple_string_debug(mrConstraintVector) << endl;
      }

      if (nullptr != mpTailItem) {
        mpLastTailItem = mpTailItem;
        mpTailItem = nullptr;
      }
    }

  private:
    vector<Constraint* >& mrConstraintVector; //!< Reference to a ConstraintSet object's Constraint vector.
    ConstraintIterator mSearchStartIterator; //!< Iterator pointing the start location of the next search.
    ConstraintIterator mInsertIterator; //!< Iterator pointing to the location of the next insertion.
    ConstraintIterator mMatchStart; //!< Start iterator of the search match.
    ConstraintIterator mMatchEnd; //!< End iterator of the search match.
    ConstraintIterator mCopyEnd; //!< End iterator of copying.
    ConstraintIterator mLastInRange; //!< Iterator pointing to the last item in search-matching range.
    const Constraint* mpApplyConstraint; //!< Pointer to the Constraint object being applied.
    Constraint* mpTailItem; //!< Tail item of each search if any.
    Constraint* mpLastTailItem; //!< Tail item of last search if any.
    bool mHasOverlap; //!< Indicate if there is any overlap.
    ConstraintDistanceType mRangeDistance; //!< Range distance of the search result.
    uint64 mSizeChange; //!< Total size decreased.
    bool mKeepTail; //!< Indicate whether the tail of the match range should be kept around.
    ConstraintOneResult mResult; //!< Result of each individual Constraint application.
    vector<std::pair<ConstraintDistanceType, Constraint*> > mDelayedInsertions; //!< Saved off tail Constraint object to be inserted.
  public:

    void ApplyConstraint(const Constraint* otherConstr) //!< Apply a Constraint, make changes on the Constraint vector reference.
    {
      mpApplyConstraint = otherConstr;
      Search();
      if (not HasMatch()) return; // no overlap

      // << "has match." << flush << endl;
      mHasOverlap = true; // has overlap
      CleanUpLeadingRange();

      ProcessTailItem();
      ProcessHeadItem();
      MoveMatchRange();
    }

    void Finalize() //!< Finalize the ConstraintSet object target.
    {
      auto tail_gap = std::distance(mInsertIterator, mrConstraintVector.end());
      if (TryInsertLastTailItem(tail_gap, true)) {
        -- tail_gap;
      }

      ConstraintSizeType insert_size = mDelayedInsertions.size();

      if (tail_gap) {
        // delete to end
        delete_constraint_vector_tail(mrConstraintVector, mInsertIterator, mResult);
        mSizeChange += mResult.mSizeChange;
        if (insert_size == 0) {
          erase_constraint_vector_tail(mrConstraintVector, mInsertIterator);
          return;
        }
      }

      if (insert_size > 0) {
        ConstraintDistanceType insert_dist = std::distance(mrConstraintVector.begin(), mInsertIterator);
        ConstraintSizeType new_size = insert_dist + insert_size;
        mrConstraintVector.resize(new_size);
        // exsiting iterators might become invalid after resizing.
        ConstraintIterator copy_start = mrConstraintVector.begin() + (insert_dist - 1);
        ConstraintIterator copy_target_start = mrConstraintVector.end() - ConstraintDistanceType(1);
        ConstraintDistanceType copy_end_dist = mDelayedInsertions.front().first;
        ConstraintIterator copy_end = mrConstraintVector.begin() + copy_end_dist;
        while (mDelayedInsertions.size()) {
          ConstraintDistanceType insert_loc = mDelayedInsertions.back().first;
          ConstraintIterator insert_loc_iter = mrConstraintVector.begin() + insert_loc;
          ConstraintDistanceType copy_start_dist = std::distance(mrConstraintVector.begin(), copy_start);
          if (copy_start_dist < insert_loc) {
            LOG(fail) << "{ConstraintApplier::Finalize} copy starting distance " << dec << copy_start_dist << " less than insert location distance " << insert_loc << endl;
            FAIL("final-insertion-error");
          }
          ConstraintDistanceType copy_target_dist = std::distance(mrConstraintVector.begin(), copy_target_start);
          if (copy_target_dist < insert_loc) {
            LOG(fail) << "{ConstraintApplier::Finalize} copy target distance " << dec << copy_target_dist << " less than insert location distance " << insert_loc << endl;
            FAIL("final-insertion-error");
          }
          finalize_reverse_move_range(copy_start, copy_target_start, insert_loc_iter, copy_end);
          FinalizeReverseInsertConstraints(copy_target_start);
        }
      }
    }

    /*!
      Called by FinalizeReserveInsertConstraints to insert one item.
    */
    inline void FinalizeInsertOneItem(ConstraintIterator& rInsertionIterator)
    {
      Constraint* insert_item = mDelayedInsertions.back().second;
      // insert the delayed insertion item
      (*rInsertionIterator) = insert_item;
      -- rInsertionIterator;
      mSizeChange -= insert_item->Size();
      mDelayedInsertions.pop_back();
    }

    /*!
      Insert all items in the list with the same insert location.  When this method is called, there is at least one item in the mDelayedInsertions vector.
    */
    inline void FinalizeReverseInsertConstraints(ConstraintIterator& rInsertionIterator)
    {
      ConstraintDistanceType current_insert_loc = mDelayedInsertions.back().first;
      FinalizeInsertOneItem(rInsertionIterator);

      while (mDelayedInsertions.size()) {
        ConstraintDistanceType next_insert_loc = mDelayedInsertions.back().first; // last item has been popped out.
        if (next_insert_loc != current_insert_loc) {
          // now dealing with a different insert location.
          break;
        }
        FinalizeInsertOneItem(rInsertionIterator);
      }
    }


    uint64 SizeChange() const { return mSizeChange; } //!< Return total size decreased.

    inline bool Done() const //!< Return if the ConstraintSet application should be done by now.
    {
      return (mSearchStartIterator == mrConstraintVector.end());
    }

    bool HasOverlap() const { return mHasOverlap; } //!< Indicates if there is any overlap.
  };

  void ConstraintSet::ApplyConstraintSet(const ConstraintSet& constrSet)
  {
    if (IsEmpty()) return; // already empty
    if ((&constrSet) == this) return; // applying self
    if (constrSet.IsEmpty()) {
      LOG(fail) << "{ConstraintSet::ApplyConstraintSet} applying empty ConstraintSet." << endl;
      FAIL("applying-empty-constraint-set");
    }
    if (constrSet.VectorSize() == 1) {
      const Constraint* apply_constr = constrSet.mConstraints[0];
      ApplyConstraint(*apply_constr);
      return;
    }

    ConstraintYielder other_yielder(constrSet, LowerBound(), UpperBound());
    ConstraintApplier constr_applier(mConstraints);

    while (not other_yielder.Done()) {
      const Constraint* other_constr = other_yielder.NextItem();
      // << "applying constraint: " << other_constr->ToSimpleString() << flush << endl;
      constr_applier.ApplyConstraint(other_constr);
      // << "after applying: " << ToSimpleStringDebug() << flush << endl;
      if (constr_applier.Done()) {
        break;
      }
    }

    if (not constr_applier.HasOverlap()) {
      // no overlap, clear constraint.
      Clear();
      return;
    }

    constr_applier.Finalize();
    mSize -= constr_applier.SizeChange();
  }

  void ConstraintSet::ApplyLargeConstraintSet(const ConstraintSet& constrSet)
  {
    if (IsEmpty()) return; // already empty
    if ((&constrSet) == this) return; // applying self
    if (constrSet.IsEmpty()) {
      LOG(fail) << "{ConstraintSet::ApplyLargeConstraintSet} applying empty ConstraintSet." << endl;
      FAIL("applying-empty-constraint-set");
    }

    vector<Constraint *> new_constraints;

    // There isn't any good way to know it advance how many entries will be needed. Performance was measured with
    // different reserve sizes ranging from constrSet.mConstraints.size()/1000 to constrSet.mConstraints.size(). These
    // measurements were also compared against not calling reserve() at all. constrSet.mConstraints.size()/100 had the
    // best performance of the sizes measured.
    size_t reserve_size = constrSet.mConstraints.size()/100;
    if (reserve_size > 0) {
      new_constraints.reserve(reserve_size);
    }

    ConstConstraintIterator this_constr_iter = mConstraints.cbegin();

    // Set apply_constr_iter to first intersecting constraint
    ConstConstraintIterator apply_constr_iter = std::lower_bound(constrSet.mConstraints.cbegin(), constrSet.mConstraints.cend(), *this_constr_iter, &compare_constraints);

    while ((this_constr_iter != mConstraints.cend()) && (apply_constr_iter != constrSet.mConstraints.cend())) {
      // Compute intersection of constraints, if any
      uint64 max_lower_bound = max((*this_constr_iter)->LowerBound(), (*apply_constr_iter)->LowerBound());
      uint64 this_upper_bound = (*this_constr_iter)->UpperBound();
      uint64 apply_upper_bound = (*apply_constr_iter)->UpperBound();
      uint64 min_upper_bound = min(this_upper_bound, apply_upper_bound);

      if (max_lower_bound < min_upper_bound) {
        new_constraints.push_back(new RangeConstraint(max_lower_bound, min_upper_bound));
      }
      else if (max_lower_bound == min_upper_bound) {
        new_constraints.push_back(new ValueConstraint(max_lower_bound));
      }
      // else the constraints do not intersect

      if (this_upper_bound < apply_upper_bound) {
        ++this_constr_iter;
      }
      else {
        ++apply_constr_iter;
      }
    }

    mConstraints.swap(new_constraints);
    mConstraints.shrink_to_fit();

    // Clean up the old constraints, which are referenced by the new_constraints vector following the swap() call
    for (Constraint* constr_item : new_constraints) {
      DELETE_CONSTRAINT(constr_item);
    }

    mSize = CalculateSize();
  }

  void ConstraintSet::SubtractFromElements(cuint64 subtrahend)
  {
    if (IsEmpty()) return; // already empty

    Constraint* split_constr_upper = nullptr;
    for (Constraint* constr : mConstraints) {
      if (constr->Type() == EConstraintType::Value) {
        auto val_constr = dynamic_cast<ValueConstraint*>(constr);

        // Allow overflows and re-sort the vector at the end
        val_constr->SetValue(val_constr->Value() - subtrahend);
      }
      else if (constr->Type() == EConstraintType::Range) {
        auto range_constr = dynamic_cast<RangeConstraint*>(constr);

        // Allow overflows and re-sort the vector at the end
        uint64 new_lower_bound = range_constr->LowerBound() - subtrahend;
        range_constr->SetLowerBound(new_lower_bound);
        uint64 new_upper_bound = range_constr->UpperBound() - subtrahend;
        range_constr->SetUpperBound(new_upper_bound);

        // If the range partially overflowed, we need to split it
        if (new_upper_bound < new_lower_bound) {
          // TODO(Noah): Insert check that fails if a prior constraint has been split, as this should not be possible,
          // before submitting these changes.

          range_constr->SetLowerBound(0);
          split_constr_upper = new RangeConstraint(new_lower_bound, MAX_UINT64);
        }
      }
    }

    // Add new constraint if needed
    if (split_constr_upper != nullptr) {
      mConstraints.push_back(split_constr_upper);
    }

    // TODO(Noah): Evaluate the performance of keeping track of the new minimum constraint and using the vector rotate()
    // method instead of sort() when there is time to do so.
    sort(mConstraints.begin(), mConstraints.end(), &compare_constraints);
  }

  void ConstraintSet::DivideElementsWithFactor(cuint64 divisor, cuint64 factor)
  {
    DivideElementsWithFactorRange(divisor, factor, factor);
  }

  // DivideElementsWithFactorRange() works by beginning with a value or range and sliding it forward by a set increment,
  // represented by overflow_remainder, for each iteration. It the divides the value or the endpoints of the range by
  // the divisor and adds a multiple of another constaint, represented by overflow_quotient, in order to adjust the
  // result to account for the sliding of the original value or range. Using this algorithm, we can compute values that,
  // when multiplied by the divisor, will overflow and still result in one of the values contained in the original
  // ConstraintSet.
  void ConstraintSet::DivideElementsWithFactorRange(cuint64 divisor, cuint64 startFactor, cuint64 endFactor)
  {
    if (IsEmpty()) return; // already empty

    if (divisor == 0) {
      LOG(fail) << "{ConstraintSet::DivideElements} attempt to divide by 0" << endl;
      FAIL("divide-by-zero");
    }

    // overflow_quotient = 2^64/divisor. We can't represent 2^64, so we do a little manipulation here to make the
    // calculation feasible.
    uint64 overflow_quotient = ((MAX_UINT64 - (divisor - 1)) / divisor) + 1;

    // overflow_remainder = 2^64 mod divisor. We can't represent 2^64, so we do a little manipulation here to make the
    // calculation feasible.
    uint64 overflow_remainder = (MAX_UINT64 - (divisor - 1)) % divisor;

    vector<Constraint *> old_constraints;
    old_constraints.swap(mConstraints);

    // Overlap of constraints between iterations is possible, so use the AddRange() and AddValue() methods to do to the
    // appropriate merging.
    for (uint64 factor = startFactor; factor <= endFactor; factor++) {
      for (const Constraint* constr : old_constraints) {
        if (constr->Type() == EConstraintType::Value) {
          auto val_constr = dynamic_cast<const ValueConstraint*>(constr);
          uint64 test_value = val_constr->Value() + overflow_remainder * factor;

          if (test_value % divisor == 0) {
            uint64 new_value = (test_value / divisor) + overflow_quotient * factor;
            AddValue(new_value);
          }
        }
        else if (constr->Type() == EConstraintType::Range) {
          auto range_constr = dynamic_cast<const RangeConstraint*>(constr);
          uint64 test_lower_bound = range_constr->LowerBound() + overflow_remainder * factor;
          uint64 test_upper_bound = range_constr->UpperBound() + overflow_remainder * factor;

          // If the range partially overflowed, we need to split it
          if (test_upper_bound < test_lower_bound) {
            AddDividedElementConstraint(0, test_upper_bound, divisor, factor, overflow_quotient);
            AddDividedElementConstraint(test_lower_bound, MAX_UINT64, divisor, factor, overflow_quotient);
          }
          else {
            AddDividedElementConstraint(test_lower_bound, test_upper_bound, divisor, factor, overflow_quotient);
          }
        }
      }
    }

    // Clean up the old constraints
    for (Constraint* constr : old_constraints) {
      DELETE_CONSTRAINT(constr);
    }

    mSize = CalculateSize();
  }

  // DivideElementsWithFactorRange() works by beginning with a value or range and sliding it forward by a set increment,
  // represented by overflow_remainder, for each iteration. It the divides the value or the endpoints of the range by
  // the divisor and adds a multiple of another constaint, represented by overflow_quotient, in order to adjust the
  // result to account for the sliding of the original value or range. Using this algorithm, we can compute values that,
  // when multiplied by the divisor, will overflow and still result in one of the values contained in the original
  // ConstraintSet.
  void ConstraintSet::DivideElementsWithFactorRangeUnionedWithZero(cuint64 divisor, cuint64 startFactor, cuint64 endFactor)
  {
    if (IsEmpty()) return; // already empty

    if (divisor == 0) {
      LOG(fail) << "{ConstraintSet::DivideElements} attempt to divide by 0" << endl;
      FAIL("divide-by-zero");
    }

    // overflow_quotient = 2^64/divisor. We can't represent 2^64, so we do a little manipulation here to make the
    // calculation feasible.
    uint64 overflow_quotient = ((MAX_UINT64 - (divisor - 1)) / divisor) + 1;

    // overflow_remainder = 2^64 mod divisor. We can't represent 2^64, so we do a little manipulation here to make the
    // calculation feasible.
    uint64 overflow_remainder = (MAX_UINT64 - (divisor - 1)) % divisor;

    vector<Constraint *> old_constraints;
    old_constraints.swap(mConstraints);

    // Overlap of constraints between iterations is possible, so use the AddRange() and AddValue() methods to do to the
    // appropriate merging.
    vector<uint64> factorVec;
    factorVec.reserve(2 + endFactor - startFactor);
    factorVec.push_back(0);
    for (uint64 factor = startFactor; factor <= endFactor; factor++) {
      factorVec.push_back(factor);
    }
    for (uint64 factor : factorVec) {
      for (const Constraint* constr : old_constraints) {
        if (constr->Type() == EConstraintType::Value) {
          auto val_constr = dynamic_cast<const ValueConstraint*>(constr);
          uint64 test_value = val_constr->Value() + overflow_remainder * factor;

          if (test_value % divisor == 0) {
            uint64 new_value = (test_value / divisor) + overflow_quotient * factor;
            AddValue(new_value);
          }
        }
        else if (constr->Type() == EConstraintType::Range) {
          auto range_constr = dynamic_cast<const RangeConstraint*>(constr);
          uint64 test_lower_bound = range_constr->LowerBound() + overflow_remainder * factor;
          uint64 test_upper_bound = range_constr->UpperBound() + overflow_remainder * factor;

          // If the range partially overflowed, we need to split it
          if (test_upper_bound < test_lower_bound) {
            AddDividedElementConstraint(0, test_upper_bound, divisor, factor, overflow_quotient);
            AddDividedElementConstraint(test_lower_bound, MAX_UINT64, divisor, factor, overflow_quotient);
          }
          else {
            AddDividedElementConstraint(test_lower_bound, test_upper_bound, divisor, factor, overflow_quotient);
          }
        }
      }
    }

    // Clean up the old constraints
    for (Constraint* constr : old_constraints) {
      DELETE_CONSTRAINT(constr);
    }

    mSize = CalculateSize();
  }

  void ConstraintSet::NotElements()
  {
    if (IsEmpty()) return; // already empty

    for (Constraint* constr : mConstraints) {
      if (constr->Type() == EConstraintType::Value) {
        auto val_constr = dynamic_cast<ValueConstraint*>(constr);
        val_constr->SetValue(~(val_constr->Value()));
      }
      else if (constr->Type() == EConstraintType::Range) {
        auto range_constr = dynamic_cast<RangeConstraint*>(constr);

        // Lower and upper bounds will flip after applying not operator
        uint64 lower_bound = range_constr->LowerBound();
        uint64 upper_bound = range_constr->UpperBound();
        range_constr->SetLowerBound(~upper_bound);
        range_constr->SetUpperBound(~lower_bound);
      }
    }

    // Not operator reverses order of elements
    reverse(mConstraints.begin(), mConstraints.end());
  }

  void ConstraintSet::ApplyConstraint(const Constraint& applyConstr)
  {
    if (IsEmpty()) return; // already empty

    auto range_pair = std::equal_range(mConstraints.begin(), mConstraints.end(), &applyConstr, &compare_constraints);
    auto range_dist = std::distance(range_pair.first, range_pair.second);
    if (range_dist == 0) {
      // no intersection
      Clear();
      return;
    }

    if (range_pair.second != mConstraints.end()) {
      DeleteRange(range_pair.second, mConstraints.end());
    }

    ConstraintOneResult constr_result;
    apply_constraint_in_place(range_pair.first, applyConstr, constr_result);
    mSize -= constr_result.mSizeChange;

    if (range_dist > 1) {
      ConstraintIterator last_in_range_iter = range_pair.first + (range_dist - 1);
      apply_constraint_in_place(last_in_range_iter, applyConstr, constr_result);
      mSize -= constr_result.mSizeChange;
    }

    if (range_pair.first != mConstraints.begin()) {
      DeleteRange(mConstraints.begin(), range_pair.first);
    }
  }

  // TODO These contains methods need some optimizations.
  bool ConstraintSet::ContainsValue(uint64 value) const
  {
    if (IsEmpty()) return false;

    ValueConstraint search_constr(value);
    auto range_pair = std::equal_range(mConstraints.begin(), mConstraints.end(), &search_constr, &compare_constraints);
    return (range_pair.first != range_pair.second);
  }

  bool ConstraintSet::ContainsRange(uint64 lower, uint64 upper) const
  {
    if (upper == lower) {
      return ContainsValue(lower);
    } else {
      if (IsEmpty()) return false;

      if (upper < lower) {
        RangeConstraint new_constr(upper, lower);
        return ContainsConstraint(new_constr);
      }
      else {
        RangeConstraint new_constr(lower, upper);
        return ContainsConstraint(new_constr);
      }
    }
  }

  bool ConstraintSet::ContainsConstraint(const Constraint& constr) const
  {
    auto range_pair = std::equal_range(mConstraints.begin(), mConstraints.end(), &constr, &compare_constraints);
    if (range_pair.first == range_pair.second)
      return false;

    return (*range_pair.first)->Contains(constr);
  }

  bool ConstraintSet::ContainsConstraintSet(const ConstraintSet& rConstrSet) const
  {
    auto my_start_iter = mConstraints.begin();
    for (auto other_iter = rConstrSet.mConstraints.begin(); other_iter != rConstrSet.mConstraints.end(); ++ other_iter) {
      auto current_constr = *other_iter;
      auto find_iter = std::lower_bound(my_start_iter, mConstraints.end(), current_constr, &compare_constraints);
      if (find_iter == mConstraints.end()) {
        return false;
      }

      if (not (*find_iter)->Contains(*current_constr)) {
        return false;
      }

      my_start_iter = find_iter;
    }

    return true;
  }

  /*!
    \class CsIntersectionSeeker
    \brief A class used in ConstraintSet::Intersects to organize related intersection seeking code.
  */
  class CsIntersectionSeeker {
  public:
    explicit CsIntersectionSeeker(const vector<Constraint* >& constrVec) //!< Constructor with Constraint vector reference given.
      : mCurrentIterator(constrVec.begin()), mEndIterator(constrVec.end())
    {
    }

    inline const Constraint* CurrentConstraint() const { return (*mCurrentIterator); } //!< Return pointer to the current Constraint object.
    inline ConstConstraintIterator CurrentIterator() const { return mCurrentIterator; } //!< Return current iterator.
    inline ConstConstraintIterator EndIterator() const { return mEndIterator; } //!< Return end iterator.
    inline void SetCurrentIterator(const ConstConstraintIterator& rSetIter) { mCurrentIterator = rSetIter; } //!< Set current iterator.
    inline bool AdvanceIterator() //!< Advance iterator, detect if reaches end of vector.
    {
      ++ mCurrentIterator;
      return (mCurrentIterator != mEndIterator);
    }

    ASSIGNMENT_OPERATOR_ABSENT(CsIntersectionSeeker);
    COPY_CONSTRUCTOR_ABSENT(CsIntersectionSeeker);
  private:
    ConstConstraintIterator mCurrentIterator;
    ConstConstraintIterator mEndIterator;
  };

  bool ConstraintSet::Intersects(const ConstraintSet& rConstrSet) const
  {
    if (IsEmpty()) return false;
    if (rConstrSet.IsEmpty()) return false;

    CsIntersectionSeeker my_seeker(mConstraints);
    CsIntersectionSeeker other_seeker(rConstrSet.mConstraints);

    CsIntersectionSeeker * first_seeker = &my_seeker;
    CsIntersectionSeeker * second_seeker = &other_seeker;
    CsIntersectionSeeker * swap_seeker = nullptr;
    do {
      const Constraint* current_constr = first_seeker->CurrentConstraint();
      ConstConstraintIterator seek_iter = std::lower_bound(second_seeker->CurrentIterator(), second_seeker->EndIterator(), current_constr, &compare_constraints);
      if (seek_iter == second_seeker->EndIterator()) {
        // no lower-bound found, return false;
        return false;
      }

      second_seeker->SetCurrentIterator(seek_iter);
      const Constraint* other_constr = second_seeker->CurrentConstraint();
      if (current_constr->Intersects(*other_constr)) {
        // intersection found.
        return true;
      }

      if (not first_seeker->AdvanceIterator()) {
        // advance to end, return false;
        return false;
      }

      // now second_seeker has the larger current constraint, switch the seeker and continue
      swap_seeker = first_seeker;
      first_seeker = second_seeker;
      second_seeker = swap_seeker;
    }
    while (1);

    return false;
  }

  /*!
    Used in picking value with alignment.
    Due to the typical use cases, usually don't need to merge Constraints that become adjacent or reduced from range to value, etc.
   */
  void ConstraintSet::ShiftRight(uint32 shiftAmount)
  {
    if (shiftAmount >= 64) {
      Clear();
    }
    if (shiftAmount == 0)
      return;

    for (auto constr_item : mConstraints) {
      uint64 old_size = constr_item->Size();
      constr_item->ShiftRight(shiftAmount);
      mSize -= (old_size - constr_item->Size());
    }
  }

  void ConstraintSet::AlignOffsetWithSize(uint64 alignMask, uint64 alignOffset, uint64 alignSize)
  {
    if (alignSize == 0) {
      LOG(fail) << "{ConstraintSet::AlignOffsetWithSize} invalid alignSize = 0" << endl;
      FAIL("invalid-zero-size");
    }

    if (alignOffset > ~alignMask) {
      LOG(fail) << "{ConstraintSet::AlignOffsetWithSize} invalid alignOffset > ~alignMask" << endl;
      FAIL("invalid-align-offset");
    }

    AlignOffsetWithSizeTrimmer align_offset_trimmer(*this, alignMask, alignOffset, alignSize);
    align_offset_trimmer.Trim();
    mSize = align_offset_trimmer.ConstraintSetSize();
  }

  /*!
    Prepare all Constraint objects so that they all satisfy the alignment and size requirements.
   */
  void ConstraintSet::AlignWithSize(uint64 alignMask, uint64 alignSize)
  {
    if (alignSize == 0) {
      LOG(fail) << "{ConstraintSet::AlignWithSize} invalid alignSize = 0" << endl;
      FAIL("invalid-zero-size");
    }

    AlignWithSizeTrimmer align_with_size_trimmer(*this, alignMask, alignSize);
    align_with_size_trimmer.Trim();
    mSize = align_with_size_trimmer.ConstraintSetSize();
  }

  void ConstraintSet::AlignMulDataWithSize(uint64 mulData, uint64 baseRemainder, uint64 alignSize)
  {
    if ((alignSize == 0) || (mulData == 0)) {
      LOG(fail) << "{ConstraintSet::AlignMulDataWithSize} invalid alignSize or mulData: alignSize=" << alignSize << ", mulData=" << mulData << endl;
      FAIL("invalid-zero-size");
    }

    AlignMulDataWithSizeTrimmer align_muldata_trimmer(*this, mulData, baseRemainder, alignSize);
    align_muldata_trimmer.Trim();
    mSize = align_muldata_trimmer.ConstraintSetSize();
  }

  /*!
    \class AlignWithPageAdjustor
    \brief Constraint adjustor class using Constraint::AlignWithPage interface.  This adjustor might need to merge subsequent constraints, therefore cannot use the Trimmer base class.
  */
  class AlignWithPageAdjustor {
  public:
    AlignWithPageAdjustor(ConstraintSet& rConstrSet, uint64 alignMask, uint32 shiftAmount) //!< Constructor with align mask and shift amount parameters.
      : mConstraintSetSize(rConstrSet.Size()), mThroughIterator(), mInsertIterator(), mLastIterator(), mAdjustResult(), mrConstraintVector(rConstrSet.GetConstraintsMutable()), mAlignTraits(alignMask, shiftAmount)
    {
      mThroughIterator = mrConstraintVector.begin();
      mInsertIterator = mThroughIterator;
      mLastIterator = mThroughIterator;
    }

    inline void Adjust() //!< Adjust the Constraint objects.
    {
      AdjustFirstConstraint();
      if (mrConstraintVector.size() == 1) {
        return; // already done.
      }

      for (; mThroughIterator != mrConstraintVector.end(); ++ mThroughIterator) {
        AdjustConstraint();
      }

      if (mInsertIterator != mrConstraintVector.end()) {
        erase_constraint_vector_tail(mrConstraintVector, mInsertIterator);
      }
    }

    inline uint64 ConstraintSetSize() const { return mConstraintSetSize; } //!< Return the updated constraint set size.

    DESTRUCTOR_DEFAULT(AlignWithPageAdjustor);
    ASSIGNMENT_OPERATOR_ABSENT(AlignWithPageAdjustor);
    COPY_CONSTRUCTOR_ABSENT(AlignWithPageAdjustor);
    DEFAULT_CONSTRUCTOR_ABSENT(AlignWithPageAdjustor);
  protected:
    inline void AdjustFirstConstraint() //!< Adjust the first Constraint object to start the process.
    {
      Constraint* the_constr = (*mThroughIterator);
      uint64 size_change = the_constr->AlignWithPage(mAlignTraits);
      mConstraintSetSize -= size_change;
      ++ mThroughIterator;
      ++ mInsertIterator;
    }

    inline void AdjustConstraint() //!< Adjust current constraint with AlignWithPage.
    {
      // Do the adjustment on the item first.
      Constraint* the_constr = (*mThroughIterator);
      uint64 size_change = the_constr->AlignWithPage(mAlignTraits);
      mConstraintSetSize -= size_change;
      Constraint* last_constr = (*mLastIterator);
      uint64 constr_size = the_constr->Size();

      // << "merging " << the_constr->ToSimpleString() << endl;
      if (last_constr->MergeNextConstraint(the_constr, mAdjustResult)) {
        // Merge happend.
        if (mAdjustResult.mType == EConstraintResultType::Replace) {
          DELETE_CONSTRAINT(last_constr);
          *(mLastIterator) = mAdjustResult.GiveConstraint();
        }
        mConstraintSetSize += mAdjustResult.mSizeChange;
        mConstraintSetSize -= constr_size;
        (*mThroughIterator) = nullptr;
        mAdjustResult.Clear();
      }
      else {
        // No merge, move the item if necessary.
        if (mThroughIterator != mInsertIterator) {
          // need to move the Constraint object pointer to the location pointed to by mInsertIterator
          (*mThroughIterator) = nullptr;
          (*mInsertIterator) = the_constr;
        }
        mLastIterator = mInsertIterator;
        ++ mInsertIterator; // move the insertion iterator to next position.
      }
    }

  protected:
    uint64 mConstraintSetSize; //!< Size of the constraint set.
    ConstraintIterator mThroughIterator; //!< Iterator going through items sequentially.
    ConstraintIterator mInsertIterator; //!< Iterator pointing to the current inserting location.
    ConstraintIterator mLastIterator; //!< Iterator pointing to the last Constraint object.
    ConstraintOneResult mAdjustResult; //!< Result of each adjust operation.
    vector<Constraint* >& mrConstraintVector; //!< Reference to a ConstraintSet object's Constraint vector.
    PageAlignTraits mAlignTraits; //!< Align traits object.
  };

  /*!
    Align constraints and inflate to range of given mask size (page size)
  */
  void ConstraintSet::AlignWithPage(uint64 pageMask)
  {
    // << "New version." << endl;
    if (pageMask == 0) {
      LOG(fail) << "{ConstraintSet::AlignWithPage} invalid pageMask = 0" << endl;
      FAIL("invalid-zero-size");
    }

    if (IsEmpty()) {
      LOG(warn) << "{ConstraintSet::AlignWithPage} called on an empty constraint set" << endl;
      return;
    }

    uint32 shift_amount = get_mask64_size(~pageMask);
    AlignWithPageAdjustor align_page_adjustor(*this, pageMask, shift_amount);
    align_page_adjustor.Adjust();
    mSize = align_page_adjustor.ConstraintSetSize();
  }

  void ConstraintSet::Translate(uint64 pageMask, uint64 pageFrame)
  {
    if (pageMask == 0)
    {
      LOG(fail) << "{ConstraintSet::Translate} invalid pageMask = 0" << endl;
      FAIL("invalid-zero-size");
    }
    if ((pageMask & pageFrame) != 0)
    {
      LOG(fail) << "{ConstraintSet::Translate} pageFrame exceeds pageMask boundary" << endl;
      FAIL("invalid-pageframe-value");
    }

    for (auto constr_item : mConstraints) {
      constr_item->Translate(pageMask, pageFrame);
    }
  }

  uint64 ConstraintSet::GetAlignedSizeFromBottom(uint64 alignMask, uint64 alignSize) const
  {
    if (mConstraints.size() == 0) {
      stringstream err_stream;
      err_stream << "{GetAlignedSizeFromBottom}empty constraint container.";
      throw ConstraintError(err_stream.str());
    }

    uint64 ret_addr = 0;
    ConstConstraintIterator through_iter = mConstraints.begin();
    bool has_match = false;
    for (; through_iter != mConstraints.end(); ++ through_iter) {
      ret_addr = (*through_iter)->GetAlignedSizeFromBottom(alignMask, alignSize, has_match);
      if (has_match) {
        break;
      }
    }

    if (not has_match) {
      stringstream err_stream;
      err_stream << "{Constraint::GetAlignedSizeFromBottom} no match found.";
      throw ConstraintError(err_stream.str());
    }

    return ret_addr;
  }

  uint64 ConstraintSet::GetAlignedSizeFromTop(uint64 alignMask, uint64 alignSize) const
  {
    if (mConstraints.size() == 0) {
      stringstream err_stream;
      err_stream << "{GetAlignedSizeFromTop}empty constraint container.";
      throw ConstraintError(err_stream.str());
    }

    uint64 ret_addr = 0;
    ConstConstraintIterator through_iter = mConstraints.end();
    bool has_match = false;
    do {
      -- through_iter;
      ret_addr = (*through_iter)->GetAlignedSizeFromTop(alignMask, alignSize, has_match);
      if (has_match) {
        break;
      }
    }
    while (through_iter != mConstraints.begin());

    if (not has_match) {
      stringstream err_stream;
      err_stream << "{Constraint::GetAlignedSizeFromTop} no match found.";
      throw ConstraintError(err_stream.str());
    }

    return ret_addr;
  }

  uint64 ConstraintSet::OnlyValue() const
  {
    if (mConstraints.size() != 1) {
      stringstream err_stream;
      err_stream << "expecting only one entry in the ConstraintSet object.";
      throw ConstraintError(err_stream.str());
    }

    auto only_constr = mConstraints.front();
    if (only_constr->Size() != 1) {
      stringstream err_stream;
      err_stream << "expecting size=1 in the Constraint object.";
      throw ConstraintError(err_stream.str());
    }
    return only_constr->LowerBound();
  }

  void ConstraintSet::GetValues(std::vector<uint64>& valueVec) const
  {
    // put a limit so that we don't call this on a ConstraintSet object with huge number of values.
    if (mSize > 1024) {
      LOG(fail) << "{ConstraintSet::GetValues} not expecting to be called when size is larger than 1024" << endl;
      FAIL("constraint-value-number-too-large");
    }

    for (auto constr_item : mConstraints) {
      constr_item->GetValues(valueVec);
    }
  }

  bool ConstraintSet::CopyInRange(uint64 startVal, uint64 endVal, ConstraintSet& rCopyConstrs) const
  {
    RangeConstraint range_constr(startVal, endVal);

    auto range_pair = std::equal_range(mConstraints.begin(), mConstraints.end(), &range_constr, &compare_constraints);
    auto range_dist = std::distance(range_pair.first, range_pair.second);

    if (range_dist == 0) return false; // no match.

    uint64 copy_size = 0;
    vector<Constraint* > & copy_vec = rCopyConstrs.GetConstraintsMutable();
    copy_vec.reserve(range_dist);

    // Copy all Constraint objects in the equal-range, then we need to apply the range_constr to adjust the first and last Constraint objects in the range.
    for (ConstConstraintIterator copy_iter = range_pair.first; copy_iter != range_pair.second; ++ copy_iter) {
      const Constraint* src_constr = (*copy_iter);
      copy_vec.push_back(src_constr->Clone());
      copy_size += src_constr->Size();
    }
    rCopyConstrs.mSize = copy_size;

    // Apply the range_constr to adjust the first and last Constraint object.  Take advantage of the existing ConstraintApplier facility.
    ConstraintApplier constr_applier(copy_vec);
    constr_applier.ApplyConstraint(&range_constr);

    if (not constr_applier.HasOverlap()) {
      // no overlap, something is wrong.
      LOG(fail) << "[ConstraintSet::CopyInRange] overlap with range constraint expected." << endl;
      FAIL("overlap-with-range-expected");
    }

    constr_applier.Finalize();
    rCopyConstrs.mSize -= constr_applier.SizeChange();
    return true;
  }

  /*!
    \class ConstraintRangeReplacer
    \brfief A local class to oranize code for ConstraintSet::ReplaceInRange better.
  */
  class ConstraintRangeReplacer {
  public:
    ConstraintRangeReplacer(ConstraintSet& rTargetConstrs, uint64 lower, uint64 upper, ConstraintSet& rReplaceConstrs) //!< Constructor.
      : mLower(lower), mUpper(upper), mDeletedSize(0), mAddedSize(0), mRangeDist(0), mLeadingDist(0), mTrailingDist(0), mReplaceRange(lower, upper), mrTargetConstraints(rTargetConstrs), mrReplaceConstraints(rReplaceConstrs)
    {
    }

    ASSIGNMENT_OPERATOR_ABSENT(ConstraintRangeReplacer);
    COPY_CONSTRUCTOR_ABSENT(ConstraintRangeReplacer);
    DEFAULT_CONSTRUCTOR_ABSENT(ConstraintRangeReplacer);

    void Validate()
    {
      if (mrTargetConstraints.IsEmpty()) {
        LOG(fail) << "[ConstraintRangeReplacer::Validate] ConstraintSet is empty." << endl;
        FAIL("unexpected-empty-constraint-set");
      }

      if (mrReplaceConstraints.IsEmpty()) {
        LOG(fail) << "[ConstraintRangeReplacer::Validate] replacement ConstraintSet is empty." << endl;
        FAIL("unexpected-empty-replace-constraint-set");
      }

      if ((mrReplaceConstraints.LowerBound() < mLower) or (mrReplaceConstraints.UpperBound() > mUpper)) {
        LOG(fail) << "[ConstraintRangeReplacer::Validate] replacement ConstraintSet out of bound. " << endl;
        FAIL("replacement-constraint-set-out-of-bound");
      }

      // << "target constr: " << mrTargetConstraints.ToSimpleString() << endl << "lower-upper : 0x" << hex << mLower << "-0x" << mUpper << endl << " replacement: " << mrReplaceConstraints.ToSimpleString() << endl;
    }

    /*!
      Process lower/upper bound of the search constraint with the replace range then merge remainder to the replacement ConstraintSet.
      The passed in pLowerConstr object is cloned.
    */
    Constraint* ProcessBoundConstraint(Constraint* pBoundConstr)
    {
      ConstraintTwoResult cs_result;
      pBoundConstr->SubConstraint(mReplaceRange, cs_result);

      switch (cs_result.mType) {
      case EConstraintResultType::Remove:
        // item will be removed by subtracting the lower-upper range.
        delete pBoundConstr;
        break;
      case EConstraintResultType::Replace:
        mrReplaceConstraints.MergeConstraint(cs_result.GiveHeadConstraint());
        delete pBoundConstr;
        break;
      case EConstraintResultType::Consumed:
        mrReplaceConstraints.MergeConstraint(pBoundConstr);
        break;
      default:
        LOG(fail) << "While handling sub constraint range end, unexpected constraint result type: \"" << EConstraintResultType_to_string(cs_result.mType) << endl;
        FAIL("unexpected-sub-constraint-result-type");
      }

      return cs_result.GiveTailConstraint();
    }

    /*!
      Deleting the Constraint object in the range to be replaced.
    */
    void DeleteEqualRange(ConstraintIterator start, ConstraintIterator end)
    {
      // delete the Constraint objects intersects with the lower-upper range
      for (auto del_iter = start; del_iter != end; ++ del_iter) {
        mDeletedSize += (*del_iter)->Size();
        DELETE_CONSTRAINT((*del_iter));
        (*del_iter) = nullptr;
      }
    }

    /*!
      Process the lower bound Constraint and the upper bound Constraint in the matches betwen the specified lower-upper range.
      This is necessary since these constraints after cutting off the range could still merge with the in coming replacement ConstraintSet.
    */
    void ProcessBoundary()
    {
      // << "target constraints=" << mrTargetConstraints.ToSimpleString() << endl;
      auto & target_vec = mrTargetConstraints.GetConstraintsMutable();
      auto range_pair = std::equal_range(target_vec.begin(), target_vec.end(), &mReplaceRange, &compare_constraints);
      mRangeDist = std::distance(range_pair.first, range_pair.second);
      if (mRangeDist == 0) {
        LOG(fail) << "[ConstraintRangeReplacer::ProcessBoundary] expecting to find some constraints in the range to replace." << endl;
        FAIL("expecting-constraints-in-range");
      }

      mLeadingDist = std::distance(target_vec.begin(), range_pair.first);
      mTrailingDist = std::distance(range_pair.second, target_vec.end());

      Constraint* first_in_range = (*range_pair.first);
      Constraint* trailing_constr = ProcessBoundConstraint(first_in_range->Clone());
      if (nullptr != trailing_constr) {
        // this should be the case when mRangeDist == 1
        if (mRangeDist != 1) {
          LOG(fail) << "[ConstraintRangeReplacer::ProcessBoundary] expecting range distance to be 1 in this case." << endl;
          FAIL("range-distance-should-be-1");
        }
        mrReplaceConstraints.MergeConstraint(trailing_constr);
        return DeleteEqualRange(range_pair.first, range_pair.second);
      }

      if (mRangeDist < 2) // no more boundary processing needed.
        return DeleteEqualRange(range_pair.first, range_pair.second);

      // processing upper boundary constraint.
      auto last_constr_iter = range_pair.second - 1;
      Constraint* last_constr = *(last_constr_iter);
      if (nullptr != ProcessBoundConstraint(last_constr->Clone())) {
        LOG(fail) << "[ConstraintRangeReplacer::ProcessBoundary] not expecting tail constraint after processing upper bound constraint." << endl;
        FAIL("unexpected-tail-constraint");
      }

      DeleteEqualRange(range_pair.first, range_pair.second);
    }

    /*!
      Copy the necessary replacement constraints and tail constraints to the target vector.
    */
    void CopyBlocksToTarget()
    {
      auto & target_vec = mrTargetConstraints.GetConstraintsMutable();
      auto current_size = target_vec.size();
      auto & replace_vec = mrReplaceConstraints.GetConstraintsMutable();
      auto replacement_size = replace_vec.size();
      ConstraintDistanceType target_size = mLeadingDist + mTrailingDist + replacement_size;

      // << "range dist is : " << mRangeDist << " leading dist is : " << mLeadingDist << " trailing dist is: " << mTrailingDist << " target constr current size: " << dec << current_size << " capacity: " << target_vec.capacity() << " new size is: " << target_size << endl;
      if (target_size <= ConstraintDistanceType(target_vec.capacity())) {
        if (mTrailingDist == 0) {
          if (target_size > ConstraintDistanceType(current_size)) {
            // need to resize first, or the copy at COPY-REPLACEMENT won't work.
            target_vec.resize(target_size);
          }
        }
        else if (mRangeDist != ConstraintDistanceType(replacement_size)) { // trailing dist > 0
          if (target_size < ConstraintDistanceType(current_size)) {
            // copying is necessary, shrinking
            shrinking_move_block(mLeadingDist + mRangeDist, mLeadingDist + replacement_size, mTrailingDist, target_vec);
            target_vec.resize(target_size); // Now we can shrink the actual size.
          }
          else {
            // copy is necessary, expanding.
            target_vec.resize(target_size); // Expand the vector first.
            expanding_move_block(mLeadingDist + mRangeDist, mLeadingDist + replacement_size, mTrailingDist, target_vec);
          }
        }
        // copy replacement constraints in. COPY-REPLACEMENT.
        std::memcpy(&(target_vec[mLeadingDist]), &(replace_vec[0]), replacement_size * sizeof(Constraint *));
      }
      else {
        // need to reserve to increase capacity first, will use a temporary vector then do swap.
        vector<Constraint* > temp_vec;
        temp_vec.reserve(target_size);

        // copy constraints
        // insert the Constraints in leading part to temp_vec.
        temp_vec.insert(temp_vec.end(), target_vec.begin(), target_vec.begin() + mLeadingDist);
        temp_vec.insert(temp_vec.end(), replace_vec.begin(), replace_vec.end());
        temp_vec.insert(temp_vec.end(), target_vec.begin() + mLeadingDist + mRangeDist, target_vec.end());
        target_vec.swap(temp_vec);
        temp_vec.clear(); // Objects has been copied or deleted, clearing is fine.
      }
      mAddedSize = mrReplaceConstraints.Size();
      replace_vec.clear(); // clear this first to avoid double deleting.
      mrReplaceConstraints.Clear();
    }

    void Replace()
    {
      // Do some basic validation on the parameters provided.
      Validate();
      // Process the constraints at the range boundary.
      ProcessBoundary();
      // Copy the necessary replacement constraints and tail constraints to target vector.
      CopyBlocksToTarget();
    }

    uint64 DeletedSize() const { return mDeletedSize; } //!< Return deleted size.
    uint64 AddedSize() const { return mAddedSize; } //!< Return added size.
  private:
    uint64 mLower; //!< Lower bound of replacement range.
    uint64 mUpper; //!< Upper bound of replacement range.
    uint64 mDeletedSize; //!< Size deleted due to deleting Constraint objects intersects with the lower-upper range.
    uint64 mAddedSize; //!< Added size from the size of the mrReplaceConstraints after adjusting boundary constraints.
    ConstraintDistanceType mRangeDist; //!< Distance between the search lower bound and upper bound.
    ConstraintDistanceType mLeadingDist; //!< Distance from beginning of original vector to the search lower bound.
    ConstraintDistanceType mTrailingDist; //!< Distance from search upper bound to the end of the original vector.
    RangeConstraint mReplaceRange; //!< Constraint representing the replace range.
    ConstraintSet& mrTargetConstraints; //!< Reference to target ConstraintSet.
    ConstraintSet& mrReplaceConstraints; //!< Reference to replacement ConstraintSet.
  };

  void ConstraintSet::ReplaceInRange(uint64 lower, uint64 upper, ConstraintSet& rReplaceConstr)
  {
    ConstraintRangeReplacer range_replacer(*this, lower, upper, rReplaceConstr);

    //auto cset_s_replace = ConstraintSetSerializer(rReplaceConstr, FORCE_CSET_DEFAULT_PERLINE);
    // << "constraint range replacer lower=0x" << hex << lower << " upper=0x" << upper << "replace_constr: " << cset_s_replace.ToDebugString() << endl;

    range_replacer.Replace();
    mSize += range_replacer.AddedSize();
    mSize -= range_replacer.DeletedSize();
  }

  uint64 ConstraintSet::LeadingIntersectingRange(uint64 interStart, uint64 interEnd, uint64& interSize) const
  {
    uint64 start_addr = 0;
    interSize = 0;

    RangeConstraint check_constr(interStart, interEnd);
    auto find_iter = std::lower_bound(mConstraints.begin(), mConstraints.end(), &check_constr, &compare_constraints);
    if (find_iter != mConstraints.end()) {
      // has match.
      const Constraint* match_constr = (*find_iter);
      if (match_constr->LowerBound() <= interEnd) {
        // at least some overlap
        auto inter_constr = match_constr->GetIntersection(check_constr);
        if (nullptr != inter_constr) {
          start_addr = inter_constr->LowerBound();
          interSize = inter_constr->Size();
          delete inter_constr;
        }
      }
    }
    return start_addr;
  }

  string ConstraintSet::ToSimpleString() const
  {
    stringstream out_stream;

    char print_buffer[64];
    bool first_item = true;
    for (auto constr_item : mConstraints) {
      constr_item->PrintSimpleString(print_buffer, 64);
      if (first_item) {
        first_item = false;
      } else {
        out_stream << ",";
      }
      out_stream << print_buffer;
    }

    return out_stream.str();
  }

  void ConstraintSet::Serialize(const std::string& rName, uint32 serialNumber, uint32 itemPerLine) const
  {
    ConstraintSetSerializer serializer(*this, itemPerLine);
    serializer.Serialize(rName, serialNumber);
  }

  void ConstraintSet::Deserialize(const std::string& rName, uint32 serialNumber)
  {
    ConstraintSetDeserializer deserializer(*this);
    deserializer.Deserialize(rName, serialNumber);
  }

  string ConstraintSet::ToString() const
  {
    return string("ConstraintSet: ") + ToSimpleString();
  }

  void ConstraintSet::DivideMulData(uint64 mulData, uint64 baseRemainder, uint64 alignShift)
  {
    //constraintset / mulData
    DivideElementsWithFactor(mulData,0);
  }

}
