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
#ifndef Force_Constraint_H
#define Force_Constraint_H

#include <Defines.h>
#include <vector>
#include <string>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  struct ConstraintOneResult;
  struct ConstraintTwoResult;
  class IndexMaskOperator;
  struct PageAlignTraits;

  /*!
    \class Constraint
    \brief Base class for constraints
  */
  class Constraint {
  public:
    Constraint() { } //!< Constructor.
    virtual ~Constraint(); //!< Destuctor.

    bool operator==(const Constraint& rOther) const;
    bool operator!=(const Constraint& rOther) const;
    virtual EConstraintType Type() const = 0; //!< Return the type of the constraint.
    virtual Constraint* Clone() const = 0; //!< Clone the Constraint object.
    virtual uint64 LowerBound() const = 0; //!< Lower bound value of the constraint.
    virtual uint64 UpperBound() const = 0; //!< Upper bound value of the constraint.
    virtual uint64 Size() const = 0; //!< Size of the constraint.
    virtual uint64 ChosenValue(uint64 offset) const = 0; //!< Given offset, return a value contained in the constraint.
    virtual void PrintSimpleString(char* print_buffer, uint32 size) const = 0; //!< Print the constraint string representation into the buffer.
    virtual void MergeConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult) = 0; //!< Merge another constraint.
    virtual bool MergeNextConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult) = 0; //!< Try to merge next Constraint object in the set.
    virtual void SubConstraint(const Constraint& rOtherConstr, ConstraintTwoResult& cResult) = 0; //!< Subtract another constraint.
    virtual bool Contains(uint64 value) const = 0; //!< Return whether the Constraint object contains the value.
    virtual bool Contains(const Constraint& rOtherConstr) const = 0; //!< Return whether the Constraint object contains the other constraint.
    virtual bool Intersects(const Constraint& rOtherConstr) const = 0; //!< Return whether the two Constraint object intersects each other.
    virtual void ShiftRight(uint32 shiftAmount) = 0; //!< Shift the Constraint object to the right by shiftAmount.
    virtual void AlignWithSize(uint64 alignMask, uint64 alignSize, ConstraintOneResult& cResult) = 0; //!< Align the constraint object with size.
    virtual void AlignOffsetWithSize(uint64 alignMask, uint64 alignOffset, uint64 alignSize, ConstraintOneResult& cResult) = 0; //!< Align Constraint boundaries to the specified offset from zero while considering required size.
    virtual void AlignMulDataWithSize(uint64 mulData, uint64 baseRemainder, uint64 alignSize, ConstraintOneResult& cResult) = 0; //!< Align Constraint boundaries to the specified offset from zero while considering required size.
    virtual uint64 AlignWithPage(const PageAlignTraits& rAlignTraits) = 0; //!< Align and inflate constraint object to nearest boundaries based on mask, then shift right based on page size.  Return size change.
    virtual void Translate(uint64 pageMask, uint64 pageFrame) = 0; //!< Translate constraint based on provided page mask and frame value
    virtual Constraint* GetIntersection(const Constraint& rOtherConstr) const = 0; //!< Return intersection with the passed in Constraint object.
    virtual void ApplyConstraint(const Constraint& rOtherConstr, ConstraintOneResult& cResult) = 0; //!< Apply a Constraint on this Constraint.
    virtual void GetValues(std::vector<uint64>& valueVec) const = 0; //!< Return the values contained in the Constraint.
    virtual uint64 GetAlignedSizeFromBottom(uint64 alignMask, uint64 alignSize, bool &hasMatch) const = 0; //!< Get aligned size from bottom of the range if available.
    virtual uint64 GetAlignedSizeFromTop(uint64 alignMask, uint64 alignSize, bool &hasMatch) const = 0; //!< Get aligned size from top of the range if available.
    std::string ToSimpleString() const; //!< Return the constraint printed as simple string.
    std::string ToString() const; //!< Return a string representation of the Constraint object.
    Constraint(const Constraint& rOther) { } //!< Copy constructor.
  protected:
    void UnexpectedConstraintType(EConstraintType constrType, const std::string& callerName) const; //!< Common function to report unhandled constraint type.

    friend class IndexMaskOperator;
  };

  /*!
    \class ValueConstraint
    \brief Simple constraint that only contains a value.
  */
  class ValueConstraint : public Constraint {
  public:
    explicit ValueConstraint(uint64 value) : Constraint(), mValue(value) { } //!< Constructor with value parameter given.
    ValueConstraint() : Constraint(), mValue(0) { } //!< Default onstructor.
    ~ValueConstraint() { } //!< Destuctor.

    EConstraintType Type() const override { return EConstraintType::Value; } //!< Return the type of the ValueConstraint.
    Constraint* Clone() const override { return new ValueConstraint(*this); } //!< Return a clone of the ValueConstraint object.
    uint64 LowerBound() const override { return mValue; } //!< Lower bound value of the ValueConstraint.
    uint64 UpperBound() const override { return mValue; } //!< Upper bound value of the ValueConstraint.
    inline uint64 Size() const override { return 1; }//!< Size of the ValueConstraint.
    uint64 ChosenValue(uint64 offset) const override; //!< Given offset, return a value contained in the ValueConstraint.
    void PrintSimpleString(char* print_buffer, uint32 size) const override; //!< Print the ValueConstraint string representation into the buffer.
    void MergeConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult) override; //!< Merge another constraint.
    bool MergeNextConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult) override; //!< Try to merge next Constraint object in the set.
    void SubConstraint(const Constraint& rOtherConstr, ConstraintTwoResult& cResult) override; //!< Subtract another constraint.
    bool Contains(uint64 value) const override; //!< Return whether the Constraint object contains the value.
    bool Contains(const Constraint& rOtherConstr) const override; //!< Return whether the Constraint object contains the other constraint.
    bool Intersects(const Constraint& rOtherConstr) const override; //!< Return wehther the ValueConstraint object intersects with the passed in Constraint.

    inline void ShiftRight(uint32 shiftAmount) override //!< Shift the Constraint object to the right by shiftAmount.
    {
      mValue >>= shiftAmount;
    }

    void AlignWithSize(uint64 alignMask, uint64 alignSize, ConstraintOneResult& cResult) override; //!< Align the constraint object with size.
    void AlignOffsetWithSize(uint64 alignMask, uint64 alignOffset, uint64 alignSize, ConstraintOneResult& cResult) override; //!< Align Constraint boundaries to the specified offset from zero while considering required size.
    uint64 AlignWithPage(const PageAlignTraits& rAlignTraits) override; //!< Align and inflate constraint object to nearest boundaries based on mask, then shift right based on page size.  Return size change.
    void Translate(uint64 pageMask, uint64 pageFrame) override; //!< Translate constraint based on provided page mask and frame value
    Constraint* GetIntersection(const Constraint& rOtherConstr) const override; //!< Return intersection with the passed in Constraint object.
    void ApplyConstraint(const Constraint& rOtherConstr, ConstraintOneResult& cResult) override; //!< Apply a Constraint on this ValueConstraint.
    void GetValues(std::vector<uint64>& valueVec) const override; //!< Return the value contained in the ValueConstraint.
    uint64 GetAlignedSizeFromBottom(uint64 alignMask, uint64 alignSize, bool &hasMatch) const override; //!< Get aligned size from bottom of the range if available.
    uint64 GetAlignedSizeFromTop(uint64 alignMask, uint64 alignSize, bool &hasMatch) const override; //!< Get aligned size from top of the range if available.
    inline uint64 Value() const { return mValue; } //!< Get value.
    void SetValue(cuint64 value) { mValue = value; } //!< Modify value.
    void AlignMulDataWithSize(uint64 mulData, uint64 baseRemainder, uint64 alignSize, ConstraintOneResult& cResult) override;//!< Align Constraint boundaries to the specified offset from zero while considering required size.

    ValueConstraint(const ValueConstraint& rOther) : Constraint(rOther), mValue(rOther.mValue) { } //!< Copy constructor.
  private:
    uint64 mValue; //!< The single value contained in the ValueConstraint object.

    friend class IndexMaskOperator;
    friend class RangeConstraint;
  };

  /*!
    \class RangeConstraint
    \brief A range constraint that represent a contiguous range of values.
  */
  class RangeConstraint : public Constraint {
  public:
    RangeConstraint(uint64 lower, uint64 upper) : Constraint(), mLower(lower), mUpper(upper) { } //!< Constructor with lower and upper parameters given.
    RangeConstraint() : Constraint(), mLower(0), mUpper(0) { } //!< Default onstructor.
    ~RangeConstraint() { } //!< Destuctor.

    EConstraintType Type() const override { return EConstraintType::Range; } //!< Return the type of the ValueConstraint.
    Constraint* Clone() const override { return new RangeConstraint(*this); } //!< Return a clone of the RangeConstraint object.
    uint64 LowerBound() const override { return mLower; } //!< Lower bound value of the RangeConstraint.
    uint64 UpperBound() const override { return mUpper; } //!< Upper bound value of the RangeConstraint.
    inline uint64 Size() const override { return (mUpper - mLower + 1); } //!< Size of the RangeConstraint.
    uint64 ChosenValue(uint64 offset) const override; //!< Given offset, return a value contained in the RangeConstraint.
    void PrintSimpleString(char* print_buffer, uint32 size) const override; //!< Print the RangeConstraint string representation into the buffer.
    void MergeConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult) override; //!< Merge another constraint.
    bool MergeNextConstraint(Constraint* pOtherConstr, ConstraintOneResult& rResult) override; //!< Try to merge next Constraint object in the set.
    void SubConstraint(const Constraint& rOtherConstr, ConstraintTwoResult& cResult) override; //!< Subtract another constraint.
    bool Contains(uint64 value) const override; //!< Return whether the Constraint object contains the value.
    bool Contains(const Constraint& rOtherConstr) const override; //!< Return whether the Constraint object contains the other constraint.
    bool Intersects(const Constraint& rOtherConstr) const override; //!< Check if the RangeConstraintS intersects with the Constraint object passed in.

    inline void ShiftRight(uint32 shiftAmount) override //!< Shift the Constraint object to the right by shiftAmount.
    {
      mLower >>= shiftAmount;
      mUpper >>= shiftAmount;
    }

    void AlignWithSize(uint64 alignMask, uint64 alignSize, ConstraintOneResult& cResult) override; //!< Align the constraint object with size.
    void AlignOffsetWithSize(uint64 alignMask, uint64 alignOffset, uint64 alignSize, ConstraintOneResult& cResult) override; //!< Align Constraint boundaries to the specified offset from zero while considering required size.
    uint64 AlignWithPage(const PageAlignTraits& rAlignTraits) override; //!< Align and inflate constraint object to nearest boundaries based on mask, then shift right based on page size.  Return size change.
    void Translate(uint64 pageMask, uint64 pageFrame) override; //!< Translate constraint based on provided page mask and frame value
    Constraint* GetIntersection(const Constraint& rOtherConstr) const override; //!< Return intersection with the passed in Constraint object.
    void ApplyConstraint(const Constraint& rOtherConstr, ConstraintOneResult& cResult) override; //!< Apply a Constraint on this RangeConstraint.
    void GetValues(std::vector<uint64>& valueVec) const override; //!< Return the value contained in the RangeConstraint.
    RangeConstraint(const RangeConstraint& rOther) : Constraint(rOther), mLower(rOther.mLower), mUpper(rOther.mUpper) { } //!< Copy constructor.

    void SetLowerBound(uint64 lower) { mLower = lower; } //!< Modify lower bound value.
    void SetUpperBound(uint64 upper) { mUpper = upper; } //!< Modify upper bound value.
    uint64 GetAlignedSizeFromBottom(uint64 alignMask, uint64 alignSize, bool &hasMatch) const override; //!< Get aligned size from bottom of the range if available.
    uint64 GetAlignedSizeFromTop(uint64 alignMask, uint64 alignSize, bool &hasMatch) const override; //!< Get aligned size from top of the range if available.
    inline uint64 Lower() const { return mLower; } //!< Return range lower value.
    inline uint64 Upper() const { return mUpper; } //!< Return range upper value.
    void AlignMulDataWithSize(uint64 mulData, uint64 baseRemainder, uint64 alignSize, ConstraintOneResult& cResult) override;//!< Align Constraint boundaries to the specified offset from zero while considering required size.
  private:

    inline Constraint* GetTailConstraint(uint64 tailLower) const { //!< Return tail part of the constraint in a new Constraint object.
      if (tailLower == mUpper) return new ValueConstraint(tailLower);
      else return new RangeConstraint(tailLower, mUpper);
    }

  private:
    uint64 mLower; //!< The lower bound of the RangeConstraint object.
    uint64 mUpper; //!< The upper bound of the RangeConstraint object.

    friend class IndexMaskOperator;
  };

  /*!
    Iterator type defines.
  */
  typedef std::vector<Constraint* >::iterator ConstraintIterator;
  typedef std::vector<Constraint* >::const_iterator ConstConstraintIterator;
  typedef std::vector<Constraint* >::size_type ConstraintSizeType;
  typedef std::vector<Constraint* >::difference_type ConstraintDistanceType;

  /*!
    \class ConstraintSet
    \brief Container of a set of constraints.
  */
  class ConstraintSet {
  public:
    ConstraintSet(uint64 lower, uint64 upper); //!< Constructor with initial range given.
    explicit ConstraintSet(uint64 value); //!< Constructor with a single value.
    explicit ConstraintSet(const std::string& constrStr); //!< Constructor with constraint string given.
    explicit ConstraintSet(std::vector<Constraint*>& rConstraints); //!< Constructor with constraint vector specified.
    ConstraintSet() : mSize(0), mConstraints() { } //!< Default constructor.
    ConstraintSet(const ConstraintSet& rOther); //!< Copy constructor.
    ~ConstraintSet(); //!< Destructor.
    ConstraintSet& operator=(const ConstraintSet& rOther); //!< Copy assignment operator.
    bool operator==(const ConstraintSet& rOther) const;
    bool operator!=(const ConstraintSet& rOther) const;
    ConstraintSet* Clone() const; //!< Clone the ConstraintSet object.
    uint64 CalculateSize() const; //!< Calculate possible value choices in the constraint-set.
    bool IsEmpty() const { return mConstraints.empty(); } //!< Check if the constraint set is empty.
    void Clear(); //!< Clear the ConstraintSet, make it empty.
    uint64 Size() const { return mSize; } //!< Return the constraint set size.
    uint32 VectorSize() const { return mConstraints.size(); } //!< Return the Constraint object vector size.
    uint64 LowerBound() const; //!< Return lower bound of the ConstraintSet, call with care, ensure the set is not empty.
    uint64 UpperBound() const; //!< Return upper bound of the ConstraintSet, call with care, ensure the set is not empty.
    uint64 ChooseValue() const; //!< Choose a value from the ConstraintSet.
    bool Intersects(const ConstraintSet& rConstrSet) const; //!< Check if the two ConstraintSet intersects each other.
    void AddRange(uint64 lower, uint64 upper); //!< Add a value range to the constraint set.
    void AddValue(uint64 value); //!< Add a single value to the constraint set.
    void SubRange(uint64 lower, uint64 upper); //!< Subtract a value range from the constraint set
    void SubValue(uint64 value); //!< Subtract a single value from the constraint set.
    void SubConstraint(const Constraint& constr); //!< Subtract a constraint from the constraint set.
    void SubConstraintSet(const ConstraintSet& rConstrSet); //!< Subtract a ConstraintSet from the ConstraintSet.
    void ApplyConstraintSet(const ConstraintSet& constrSet); //!< Apply additional constraint set to this ConstraintSet object.
    void ApplyLargeConstraintSet(const ConstraintSet& constrSet); //!< Apply additional constraint set to this ConstraintSet object. The result should be identical to ApplyConstraintSet(); however, this implementation is expected to be faster when the constrSet argument contains many more Constraints than this ConstraintSet.
    void ApplyConstraint(const Constraint& constr); //!< Apply additional Constraint object to this ConstraintSet object.
    void MergeConstraintSet(const ConstraintSet& rConstrSet); //!< Merge a ConstraintSet object.
    bool ContainsValue(uint64 value) const; //!< Check if a value is part of the ConstraintSet.
    bool ContainsRange(uint64 lower, uint64 upper) const; //!< Check if a range is part of the ConstraintSet.
    bool ContainsConstraint(const Constraint& constr) const; //!< Check if a Constraint is part of the ConstraintSet.
    bool ContainsConstraintSet(const ConstraintSet& rConstrSet) const; //!< Check if a ConstraintSet is contained by this ConstraintSet.
    void ShiftRight(uint32 shiftAmount); //!< Shift the ConstraintSet object to the right by shiftAmount.
    void AlignWithSize(uint64 alignMask, uint64 alignSize); //!< Align Constraint object considering required size.
    void AlignOffsetWithSize(uint64 alignMask, uint64 alignOffset, uint64 alignSize); //!< Align Constraint boundaries to the specified offset from zero while considering required size.
    void AlignWithPage(uint64 pageMask); //!< Align and inflate constraint object to nearest boundaries based on mask.
    void Translate(uint64 pageMask, uint64 pageFrame); //!< Translate constraint set based on provided page mask and frame value.
    void SubtractFromElements(cuint64 subtrahend); //!< Subtract subtrahend from each element, allowing elements to overflow. The resulting ConstraintSet will contain all elements E such that E + subtrahend would be in the original ConstraintSet.
    void DivideElementsWithFactor(cuint64 divisor, cuint64 factor); //!< Transform the ConstraintSet such that it contains some subset of the elements E such that E * divisor would be in the original ConstraintSet. DivideElementsWithFactor() is useful to avoid the cost of computing all of the elements that DivideElements() would yield. factor determines which subset results; the larger factor is, the larger the resultant elements will be. 0 <= factor < divisor.
    void DivideElementsWithFactorRange(cuint64 divisor, cuint64 startFactor, cuint64 endFactor); //!< Transform the ConstraintSet such that it contains some subset of the elements E such that E * divisor would be in the original ConstraintSet. The factor range values determine which subset results. 0 <= factor(Low/High) < divisor.
    void DivideElementsWithFactorRangeUnionedWithZero(cuint64 divisor, cuint64 startFactor, cuint64 endFactor); //!< Transform the ConstraintSet such that it contains some subset of the elements E such that E * divisor would be in the original ConstraintSet. The factor range values determine which subset results. 0 <= factor(Low/High) < divisor.
    void NotElements(); //!< Replace each element with its bitwise complement.
    uint64 GetAlignedSizeFromBottom(uint64 alignMask, uint64 alignSize) const; //!< Get an aligned range with size from the bottom of the constraint set.
    uint64 GetAlignedSizeFromTop(uint64 alignMask, uint64 alignSize) const; //!< Get an aligned range with size from the top of the constraint set.
    uint64 OnlyValue() const; //!< Return the only value in the constraint set.
    void GetValues(std::vector<uint64>& valueVec) const; //!< Return the values contained in the ConstraintSet.
    bool CopyInRange(uint64 startVal, uint64 endVal, ConstraintSet& rCopyConstrs) const; //!< Return a copy of all constraints from this set between startVal and endVal.
    uint64 LeadingIntersectingRange(uint64 interStart, uint64 interEnd, uint64& interSize) const; //!< Return the leading continuous range that intersect with [interStart, interEnd], if any.
    std::string ToString() const; //!< Return a string representation of the constraint set.
    std::string ToSimpleString() const; //!< Return a simple string representation of the constraint set.
    void Serialize(const std::string& rName, uint32 serialNumber, uint32 itemPerLine) const; //!< Serialize the ConstraintSet object to a file.
    void Deserialize(const std::string& rName, uint32 serialNumber); //!< Deserialize the ConstraintSet object from a file.
    const std::vector<Constraint* >& GetConstraints() const { return mConstraints; } //!< Return a const reference of the Constraint object vector.
    std::vector<Constraint* >& GetConstraintsMutable() { return mConstraints; } //!< Return a mutable reference of the Constraint object vector.
    inline uint64 ChosenValueFromFront(uint64 offset) const { //!< Find the chosen value starting from the back of the constraint vector.
      for (auto const constr_item: mConstraints) {
        uint64 constr_size = constr_item->Size();
        if (offset < constr_size) {
          return constr_item->ChosenValue(offset);
        }
        offset -= constr_size;
      }
      FailedChoosingValue(offset, "ChosenValueFromFront");
      return 0;
    }

    inline uint64 ChosenValueFromBack(uint64 offset, uint64 total_size) const { //!< Find the chosen value starting from the front of the constraint vector.
      uint64 lookup_size = total_size;
      for (std::vector<Constraint* >::const_reverse_iterator cr_iter = mConstraints.rbegin(); cr_iter != mConstraints.rend(); ++ cr_iter) {
        const Constraint* constr_item = (*cr_iter);
        lookup_size -= constr_item->Size();
        if (offset >= lookup_size) {
          return constr_item->ChosenValue(offset - lookup_size);
        }
      }
      FailedChoosingValue(offset, "ChosenValueFromBack");
      return 0;
    }
    void AlignMulDataWithSize(uint64 mulData, uint64 baseRemainder, uint64 alignSize);//!< Align Constraint object considering required size.
    void AlignElement(uint64 mulData, uint64 baseRemainder, uint64 alignShift);//!< Remove unalign first address
    void DivideMulData(uint64 mulData, uint64 baseRemainder, uint64 alignShift);//<!Divide Target_address with MulData
    void ReplaceInRange(uint64 lower, uint64 upper, ConstraintSet& rReplaceConstr); //!< Replace the specified range with the specified replacement constraint.
    void MergeConstraint(Constraint* new_constr); //!< Merge a constraint into the ConstraintSet.
  private:
    void FailedChoosingValue(uint64 offset, const std::string& additionalMsg) const; //!< Return error in choosing value.
    void DeleteRange(ConstraintIterator startIter, ConstraintIterator endIter); //!< Delete constraints in the range.
    void AddDividedElementConstraint(cuint64 lowerBound, cuint64 upperBound, cuint64 divisor, cuint64 factor, cuint64 overflowQuotient); // Add a constraint representing a subset of the elements E such that E * divisor is between lowerBound and upperBound.
  private:
    uint64 mSize; //!< Size of the constraint set.
    std::vector<Constraint* > mConstraints; //!< container of the sorted constraint objects
#ifdef UNIT_TEST
  public:
    static uint32 msConstraintDeleteCount; //!< Variable to assist unit-testing.
#endif
  };

  bool compare_constraints(const Constraint* a, const Constraint* b); //!< Compare function used in sorting and searching operations.
}

#endif
