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
#ifndef Force_ConstraintUtils_H
#define Force_ConstraintUtils_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>
#include <stack>
#include <algorithm>
#include <cstring>
#include <Log.h>

namespace Force {

  /*!
    \macro DELETE_CONSTRAINT
    \brief When DEBUG_CONSTRAINT_DELETE is defined, print some debug information when DELETE_CONSTRAINT is used in the place of simply "delete constr;"
  */
#ifdef DEBUG_CONSTRAINT_DELETE
#define DELETE_CONSTRAINT(constr)                \
  if (nullptr == constr) { \
    LOG(notice) << "Deleting : nullptr." << endl; \
  } \
  else { \
    LOG(notice) << "Deleting : " << constr->ToString() << endl; \
    delete constr; \
  }
#else
#define DELETE_CONSTRAINT(constr) delete constr
#endif

  class Constraint;
  class ConstraintSet;

  /*!
    \struct ConstraintOneResult
    \brief Return information regarding constraint operations that has at the most one remaining Constraint object to handle.
  */
  struct ConstraintOneResult {
    EConstraintResultType mType; //!< Result type of constraint operation.
    uint64 mSizeChange; //!< Modification value to the overall constraint size.
    Constraint* mpConstraint; //!< Any remaining Constraint object to handle.

    ConstraintOneResult() : mType(EConstraintResultType(0)), mSizeChange(0), mpConstraint(nullptr) { } //!< Constructor.

    ~ConstraintOneResult() //!< Destructor.
    {
      DELETE_CONSTRAINT(mpConstraint);
    }

    Constraint* GiveConstraint()
    {
      Constraint* ret_constr = mpConstraint;
      mpConstraint = nullptr;
      return ret_constr;
    }

    ASSIGNMENT_OPERATOR_ABSENT(ConstraintOneResult);
    COPY_CONSTRUCTOR_DEFAULT(ConstraintOneResult);

    void Clear()
    {
      mType = EConstraintResultType(0);
      mSizeChange = 0;
      if (nullptr != mpConstraint) {
        LOG(fail) << "{ConstraintOneResult::Clear} dangling Constraint pointer." << std::endl;
        FAIL("dangle-constraint-pointer");
      }
      mpConstraint = nullptr;
    }

    inline void MarkRemove(uint64 sizeChange)
    {
      mType = EConstraintResultType::Remove;
      mSizeChange = sizeChange;
    }

  };

  /*!
    \struct ConstraintTwoResult
    \brief Return information regarding constraint operations that has at the most two remaining Constraint objects to handle.
  */
  struct ConstraintTwoResult {
    EConstraintResultType mType; //!< Result type of constraint operation.
    uint64 mSizeChange; //!< Modification value to the overall constraint size.
    Constraint* mpHeadConstraint; //!< Constraint object to replace existing constraint.
    Constraint* mpTailConstraint; //!< Trailing constraint to be inserted.

    ConstraintTwoResult() : mType(EConstraintResultType(0)), mSizeChange(0), mpHeadConstraint(nullptr), mpTailConstraint(nullptr) { }

    ~ConstraintTwoResult()
    {
      DELETE_CONSTRAINT(mpHeadConstraint);
      DELETE_CONSTRAINT(mpTailConstraint);
    }

    Constraint* GiveHeadConstraint()
    {
      Constraint* ret_constr = mpHeadConstraint;
      mpHeadConstraint = nullptr;
      return ret_constr;
    }

    Constraint* GiveTailConstraint()
    {
      Constraint* ret_constr = mpTailConstraint;
      mpTailConstraint = nullptr;
      return ret_constr;
    }

    ASSIGNMENT_OPERATOR_ABSENT(ConstraintTwoResult);
    COPY_CONSTRUCTOR_DEFAULT(ConstraintTwoResult);

    inline void MarkRemove(uint64 sizeChange)
    {
      mType = EConstraintResultType::Remove;
      mSizeChange = sizeChange;
    }

  };

  class CloneConstraintBlock;

  /*!
    \class ConstraintYielder
    \brief A class used to yield const pointer to Constraint objects in a const ConstraintSet object
  */
  class ConstraintYielder {
  public:
    explicit ConstraintYielder(const ConstraintSet& rConstrSet) //!< Constructor with const ConstraintSet reference given.
      : ConstraintYielder(rConstrSet, rConstrSet.LowerBound(), rConstrSet.UpperBound())
    {
    }

    ConstraintYielder(const ConstraintSet& rConstrSet, uint64 lowerBound, uint64 upperBound) //!< Constructor with const ConstraintSet reference given, as well as yield range.
      : mrConstraintSet(rConstrSet), mrConstraintVector(rConstrSet.GetConstraints()), mItemIterator(), mEndIterator()
    {
      RangeConstraint search_constr(lowerBound, upperBound);
      auto range_pair = std::equal_range(mrConstraintVector.begin(), mrConstraintVector.end(), &search_constr, &compare_constraints);
      mItemIterator = range_pair.first;
      mEndIterator = range_pair.second;
    }

    bool Done() const { return (mItemIterator == mEndIterator); } //!< Indicates whether the end of the yield range has been reached.
    const Constraint* NextItem() const { return (*mItemIterator ++); } //!< Return the next Constraint object, increment the iterator.
    const Constraint* FrontItem() const { return (*mItemIterator); } //!< Return a const pointer to the item in the front of the yield range.
    ConstraintSizeType YieldRange() const { return std::distance(mItemIterator, mEndIterator); } //!< Return size of the yield range.
    std::pair<ConstConstraintIterator, ConstConstraintIterator> GetMatchingRange(const Constraint& rSearchConstr) const; //!< Return a pair of lower bound, upper bound iterators searching from the current iterator location.
    void SetIterator(ConstConstraintIterator newIter) const; //!< Set new iterator.
    inline ConstConstraintIterator CurrentIterator() const { return mItemIterator; } //!< Return current iterator position.
    inline ConstConstraintIterator LastIterator() const { return (mItemIterator - 1); } //!< Return iterator pointing to the last item.
    inline ConstConstraintIterator EndIterator() const { return mrConstraintVector.end(); } //!< Return iterator pointing to the end of the vector.

    inline ConstConstraintIterator GetLowerBound(const Constraint& rConstr) const //!< Return lower bound for the passed in Constraint object.
    {
      return std::lower_bound(mItemIterator, mrConstraintVector.end(), &rConstr, &compare_constraints);
    }

    inline ConstConstraintIterator GetUpperBound(const Constraint& rConstr) const //!< Return upper bound for the passed in Constraint object.
    {
      return std::upper_bound(mItemIterator, mrConstraintVector.end(), &rConstr, &compare_constraints);
    }

    CloneConstraintBlock* YieldCloneConstraintBlock(ConstConstraintIterator startIter, ConstConstraintIterator endIter) const; //!< Yield a CloneConstraintBlock.
  private:
    const ConstraintSet& mrConstraintSet; //!< Const reference to the ConstraintSet object being yield.
    const std::vector<Constraint* >& mrConstraintVector; //!< Const reference to a ConstraintSet object's Constraint vector.
    mutable ConstConstraintIterator mItemIterator; //!< Iterator pointing to the current item in the Constraint object vector.
    ConstConstraintIterator mEndIterator; //!< Iterator pointing to the end of iterating range.
  };

  /*!
    Used in ApplyConstraintSet/ApplyConstraint etc.  Erase the range from eraseureIter to the end of the Constraint vector.  All the items in the range should already be nullptr.
   */
  inline static void erase_constraint_vector_tail(std::vector<Constraint* >& rConstrVec, ConstraintIterator& rErasureIter)
  {
#ifdef UNIT_TEST
    LOG(debug) << "{erase_constraint_vector_tail} erasure iterator distance to end: " << std::dec << std::distance(rErasureIter, rConstrVec.end()) << std::endl;
    auto test_iter = rErasureIter;
    for (; test_iter != rConstrVec.end(); ++ test_iter) {
      if (nullptr != (*test_iter)) {
        LOG(fail) << "{erase_constraint_vector_tail} dangling Constraint object pointer." << std::endl;
        FAIL("erasure-dangling-constraint-pointer");
      }
    }
#endif
    rConstrVec.erase(rErasureIter, rConstrVec.end());
  }

  /*!
    Clear Constraint vector, in unit testing case, will check if all entries are nullptrs.
  */
  inline static void clear_constraint_vector(std::vector<Constraint* >& rConstrVec)
  {
#ifdef UNIT_TEST
    LOG(debug) << "{clear_constraint_vector} clearing vector." <<std::endl;
    for (auto constr_item : rConstrVec) {
      if (nullptr != constr_item) {
        LOG(fail) << "{clear_constraint_vector} expecting all entries to be nullptr at this point." << std::endl;
        FAIL("dangling-points-in-constraint-vector");
      }
    }
#endif
    rConstrVec.clear();
  }

  /*!
    Check if the Constraint vector has nullptr entry, for debugging purpose.
  */
  inline static void check_nullptr_entry(const std::vector<Constraint* >& rConstrVec)
  {
    for (auto constr_item : rConstrVec) {
      if (nullptr == constr_item) {
        LOG(fail) << "{check_nullptr_entry} nullptr entry found." << std::endl;
        FAIL("nullptr-entry-in-final-constraint-vector");
      }
    }
  }

  /*!
    Used in Constraint subtraction scenarios.
  */
  inline std::pair<Constraint*, Constraint* > subtract_from_constraint(Constraint* pMinuend, const Constraint* pSubtrahend, uint64& rSizeChange)
  {
    Constraint* ret_ptr1 = pMinuend;
    Constraint* ret_ptr2 = nullptr;

    ConstraintTwoResult cs_result;
    pMinuend->SubConstraint(*pSubtrahend, cs_result);

    switch (cs_result.mType) {
    case EConstraintResultType::Remove:
      ret_ptr1 = nullptr;
      break;
    case EConstraintResultType::Replace:
      ret_ptr1 = cs_result.GiveHeadConstraint();
      // fall through
    case EConstraintResultType::Consumed:
      if (cs_result.mpTailConstraint) {
        ret_ptr2 = cs_result.GiveTailConstraint();
      }
      break;
    default:
      LOG(fail) << "Unexpected constraint result type: \"" << EConstraintResultType_to_string(cs_result.mType) << std::endl;
      FAIL("unhandled-constraint-result-type");
    }

    rSizeChange = cs_result.mSizeChange;
    return std::make_pair(ret_ptr1, ret_ptr2);
  }

  /*!
    Used in Constraint subtraction scenario when the subtrahend intersects multiple contiguous Constraints in the target ConstraintSet.

    The returned pair of ConstraintDistanceType values are distance from the passed startIter parameter.  The caller will then decide how to handle the Constraint object located within that range, this part differ by different scenarios.
  */
  inline std::pair<ConstraintDistanceType, ConstraintDistanceType> subtract_from_contiguous_constraints(ConstraintIterator startIter, ConstraintIterator endIter, const Constraint& rSubConstr, uint64& rSizeChange)
  {
    Constraint* first_constr = *(startIter);
    ConstraintTwoResult cs_result;
    first_constr->SubConstraint(rSubConstr, cs_result);

    ConstraintDistanceType delete_start_dist = 0;
    ConstraintDistanceType delete_end_dist = std::distance(startIter, endIter);

    switch (cs_result.mType) {
    case EConstraintResultType::Remove:
      // remove item start from dist=0
      break;
    case EConstraintResultType::Replace:
      DELETE_CONSTRAINT(first_constr);
      *startIter = cs_result.GiveHeadConstraint();
      // fall through
    case EConstraintResultType::Consumed:
      rSizeChange += cs_result.mSizeChange;
      ++ delete_start_dist; // deleting range start from dist=1
      break;
    default:
      LOG(fail) << "While handling sub constraint range start, unexpected constraint result type: \"" << EConstraintResultType_to_string(cs_result.mType) << std::endl;
      FAIL("unexpected-sub-constraint-result-type");
    }

    auto last_constr_iter = endIter - 1;
    Constraint* last_constr = *(last_constr_iter);
    ConstraintTwoResult cs_result_last;
    last_constr->SubConstraint(rSubConstr, cs_result_last);
    switch (cs_result_last.mType) {
    case EConstraintResultType::Remove:
      // remove item to range_pair.second.
      break;
    case EConstraintResultType::Replace:
      DELETE_CONSTRAINT(last_constr);
      *(last_constr_iter) = cs_result_last.GiveHeadConstraint();
      // fall through
    case EConstraintResultType::Consumed:
      rSizeChange += cs_result_last.mSizeChange;
      -- delete_end_dist; // deleting range shrink by 1
      break;
    default:
      LOG(fail) << "While handling sub constraint range end, unexpected constraint result type: \"" << EConstraintResultType_to_string(cs_result.mType) << std::endl;
      FAIL("unexpected-sub-constraint-result-type");
    }

    return std::make_pair(delete_start_dist, delete_end_dist);
  }

  /*!
    Search for Constraint objects between the startIter-endIter range that can be merged with the passed in rSearchConstr
  */
  inline std::pair<ConstraintIterator, ConstraintIterator> search_for_merge(ConstraintIterator startIter, ConstraintIterator endIter, uint64& rLowerBound, uint64& rUpperBound)
  {
    if (rLowerBound > 0) -- rLowerBound; // so that we can get the constraints that are one off
    if (rUpperBound < MAX_UINT64) ++ rUpperBound; // so that we can get the constraints that are one off
    RangeConstraint search_constr(rLowerBound, rUpperBound);

    return std::equal_range(startIter, endIter, &search_constr, &compare_constraints);
  }

  /*!
    Merge one Constraint into the other Constraint pointed to by the iterator parameter.
  */
  inline uint64 merge_into_constraint(ConstraintIterator targetIter, Constraint* pToBeMerged)
  {
    Constraint* target_constr = *(targetIter);
    ConstraintOneResult cm_result;

    target_constr->MergeConstraint(pToBeMerged, cm_result);
    if (cm_result.mType == EConstraintResultType::Replace) {
      DELETE_CONSTRAINT(target_constr);
      *(targetIter) = cm_result.GiveConstraint();
    }

    return cm_result.mSizeChange;
  }

  /*!
    Merge one Constraint into a contiguous block of Constraint objects that all intersect with the to-be-merged Constraint.
  */
  inline uint64 merge_with_contiguous_constraints(ConstraintIterator startIter, ConstraintIterator endIter, Constraint* pMergeConstr)
  {
    uint64 merge_lower = (*startIter)->LowerBound();
    auto last_item_iter = endIter - 1;
    uint64 merge_upper = (*last_item_iter)->UpperBound();

    RangeConstraint* cast_constr = dynamic_cast<RangeConstraint* >(pMergeConstr);
    if (nullptr == cast_constr) {
      LOG(fail) << "Expect straddling constraint to be RangeConstraint." << std::endl;
      FAIL("unexpected-non-range-constraint");
    }
    if (merge_lower < cast_constr->LowerBound()) cast_constr->SetLowerBound(merge_lower);
    if (merge_upper > cast_constr->UpperBound()) cast_constr->SetUpperBound(merge_upper);

    uint64 deleted_size = 0;
    for (auto del_iter = startIter; del_iter != endIter; ++ del_iter) {
      deleted_size += (*del_iter)->Size();
      DELETE_CONSTRAINT((*del_iter));
      (*del_iter) = nullptr;
    }

    // return size change.
    return pMergeConstr->Size() - deleted_size;
  }

  void check_move_target(ConstConstraintIterator beginIter, ConstConstraintIterator targetIter, const Constraint* pConstr); //!< For unit testing, check if target of a move operation is clear.
  void check_shrinking_move_block(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, ConstraintIterator targetVectorBegin); //!< Check if copy target is clean in testing mode.
  void check_expanding_move_block(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, ConstraintIterator targetVectorBegin); //!< Check if copy target is clean in testing mode.
  void shrinking_fill_blank(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, ConstraintIterator targetVectorBegin); //!< Fill in left over space with nullptr in testing mode.
  void expanding_fill_blank(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, ConstraintIterator targetVectorBegin); //!< Fill in left over space with nullptr in testing mode.

  inline void shrinking_move_block(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, std::vector<Constraint* >& rConstrVec) //!< Move a contiguous block of Constraint objects towards begining of vector.
  {
#ifdef UNIT_TEST
    check_shrinking_move_block(sourceStart, targetStart, blockSize, rConstrVec.begin());
#endif

    std::memmove(&(rConstrVec[targetStart]), &(rConstrVec[sourceStart]), blockSize * sizeof(Constraint *));

#ifdef UNIT_TEST
    shrinking_fill_blank(sourceStart, targetStart, blockSize, rConstrVec.begin());
#endif
  }

  inline void expanding_move_block(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, std::vector<Constraint* >& rConstrVec) //!< MOve a contiguous block of Constraint objects towards end of vector.
  {
#ifdef UNIT_TEST
    check_expanding_move_block(sourceStart, targetStart, blockSize, rConstrVec.begin());
#endif

    std::memmove(&(rConstrVec[targetStart]), &(rConstrVec[sourceStart]), blockSize * sizeof(Constraint *));

#ifdef UNIT_TEST
    expanding_fill_blank(sourceStart, targetStart, blockSize, rConstrVec.begin());
#endif
  }

  class ConstraintBlock;
  class ConstraintBlocksPlacement;

  /*!
    \class ConstraintBlockYielder
    \brief A class used to yield ConstraintBlock objects from a vector of those objects.
  */
  class ConstraintBlockYielder {
  public:
    ConstraintBlockYielder(const ConstraintBlocksPlacement* pPlacement, const std::vector<ConstraintBlock* >& rBlockVec); //!< Constructor.

    DESTRUCTOR_DEFAULT(ConstraintBlockYielder);

    inline bool Done() const { return mCurrentIndex == mEndIndex; } //!< Return whether the yielding is done.

    inline const ConstraintBlock* Yield() const //!< Return the current item to be yielded.
    {
      auto next_item = mrBlockVector[mCurrentIndex];
      mCurrentIndex += mIncrement;
      return next_item;
    }

    inline uint32 LastIndex() const { return (mCurrentIndex - mIncrement); } //!< Return last index value.
    ConstraintDistanceType NextShrinkingBoundary() const; //!< Return the next boundary in shrinking mode.
    ConstraintDistanceType NextExpandingBoundary() const; //!< Return the next boundary in expanding mode.
    uint32 VectorSize() const { return mrBlockVector.size(); } //!< Return block vector size.
    ConstraintDistanceType SeekShrinkingSpace(ConstraintDistanceType targetLoc, uint32 blockSize, uint32& rUpperIndex) const; //!< Seek space when needing to insert block in Shrinking mode.
    ConstraintDistanceType SeekExpandingSpace(ConstraintDistanceType targetLoc, uint32 blockSize, uint32& rLowerIndex) const; //!< Seek space when needing to insert block in Expanding mode.
    const std::string ToString() const; //!< Print out details of all ConstraintBlocks.

    ASSIGNMENT_OPERATOR_ABSENT(ConstraintBlockYielder);
    COPY_CONSTRUCTOR_ABSENT(ConstraintBlockYielder);
  private:
    mutable uint32 mCurrentIndex; //!< Current index of the yielding.
    uint32 mEndIndex; //!< Ending index of the yielding.
    int32 mIncrement; //!< Increment of the yielding, +1 or -1.
    const ConstraintBlocksPlacement* mpBlocksPlacement; //!< Parameters for the target placement.
    const std::vector<ConstraintBlock* >& mrBlockVector; //!< Const reference to the ConstraintBlock vector.
  };

  /*!
    \class ConstraintBlock
    \brief Base class for containers for a continuous block of Constraint objects.
  */
  class ConstraintBlock {
  public:
    ConstraintBlock() { } //!< Constructor.
    virtual ~ConstraintBlock() { } //!< Destructor.

    virtual uint32 Size() const = 0; //!< Return the size of the block.
    virtual const std::string ToString() const = 0; //!< Return the string presentation of the ConstraintBlock object.
    virtual const char* Type() const = 0; //!< Return the class type in C string.
    virtual void AppendTo(ConstraintSet& rConstrSet) const = 0; //!< Append Constraint objects in the block to the passed-in ConstraintSet object.
    virtual bool ShrinkTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const = 0; //!< Move Constraint objects in the block to target location in the passed in ConstraintSet object, shrinking move.
    virtual bool ExpandTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const = 0; //!< Move Constraint objects in the block to target location in the passed in ConstraintSet object, expanding mode.
    virtual bool InPlace() const { return false; } //!< Return a boolean indicating whether the maneuver is in place (intra-ConstraintSet).
    virtual ConstraintDistanceType InPlaceStart() const; //!< Return in-place ConstraintBlock starting location.
    virtual ConstraintDistanceType InPlaceEnd() const; //!< Return in-place ConstraintBlock ending location.
  protected:
    void VerifyCapacity(const ConstraintSet& rConstrSet, uint32 vectorSize) const; //!< Verify capacity is sufficient for the vector size, for unit testing purpose.

    inline bool ShrinkInsertionOkay(ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder, uint32 vecSize) const //!< Return if inserting current ConstraintBlock in shrinking mode would be okay, i.e. there is enough space.
    {
      auto end_loc = targetLoc + (Size() - 1);
      auto next_boundary = rBlockYielder.NextShrinkingBoundary();
      // << "shrink insertion, target loc: " << std::dec << targetLoc << " size: " << Size() << " end loc: " << end_loc << "next shrinking boundary: " << next_boundary << std::endl;
      return (end_loc < next_boundary);
    }

    inline bool ExpandInsertionOkay(ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder, uint32 vecSize) const //!< Return if inserting current ConstraintBlock in expanding mode would be okay, i.e. there is enough space.
    {
      auto end_loc = targetLoc - (Size() - 1);
      auto next_boundary = rBlockYielder.NextExpandingBoundary();
      return (end_loc >= next_boundary);
    }

    ASSIGNMENT_OPERATOR_ABSENT(ConstraintBlock);
    COPY_CONSTRUCTOR_ABSENT(ConstraintBlock);
  };

  /*!
    \class ExistingConstraintBlock
    \brief An existing ConstraintBlock reside in a ConstraintSet object.
  */
  class ExistingConstraintBlock : public ConstraintBlock {
  public:
    explicit ExistingConstraintBlock(ConstraintDistanceType start) //!< Constructor.
      : ConstraintBlock(), mStart(start), mEnd(0)
    {
    }

    inline uint32 Size() const override { return (mEnd - mStart); } //!< Return block size.
    const std::string ToString() const override; //!< Return the string presentation of the ConstraintBlock object.
    const char* Type() const override { return "ExistingConstraintBlock"; } //!< Return the class type in C string.
    inline ConstraintDistanceType Start() const { return mStart; } //!< Return start location of the block.
    void SetEnd(ConstraintDistanceType end) { mEnd = end; } //!< Set end of block.
    inline bool IsEmpty() const { return (mStart == mEnd); } //!< Return whether the existing Constraint block is empty.

    ASSIGNMENT_OPERATOR_ABSENT(ExistingConstraintBlock);
    COPY_CONSTRUCTOR_ABSENT(ExistingConstraintBlock);
    DEFAULT_CONSTRUCTOR_ABSENT(ExistingConstraintBlock);
    DESTRUCTOR_DEFAULT(ExistingConstraintBlock);
  protected:
    ConstraintDistanceType mStart; //!< Starting location of the Constraint block inside the ConstraintSet.
    ConstraintDistanceType mEnd; //!< Ending location of the constraint block inside the ConstraintSet.
  };

  /*!
    \class MoveConstraintBlock
    \brief A ConstraintBlock to be moved into target position.
  */
  class MoveConstraintBlock : public ExistingConstraintBlock {
  public:
    MoveConstraintBlock(ConstraintDistanceType start, ConstraintSet* pConstrSet)
      : ExistingConstraintBlock(start), mpConstraintSet(pConstrSet)
    {
    }

    const char* Type() const override { return "MoveConstraintBlock"; } //!< Return the class type in C string.
    void AppendTo(ConstraintSet& rConstrSet) const override; //!< Append Constraint objects in the block to the passed-in ConstraintSet object.
    bool ShrinkTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const override; //!< Move Constraint objects in the block to target location in the passed in ConstraintSet object, shrinking move.
    bool ExpandTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const override; //!< Move Constraint objects in the block to target location in the passed in ConstraintSet object, expanding mode.
    ConstraintDistanceType InPlaceStart() const override { return mStart; } //!< Return starting address for the InPlaceStart call.
    ConstraintDistanceType InPlaceEnd() const override { return mEnd; } //!< Return starting address for the InPlaceStart call.
    bool InPlace() const override { return true; } //!< Return true indicating the maneuver is in place (intra-ConstraintSet).

    ASSIGNMENT_OPERATOR_ABSENT(MoveConstraintBlock);
    COPY_CONSTRUCTOR_ABSENT(MoveConstraintBlock);
    DEFAULT_CONSTRUCTOR_ABSENT(MoveConstraintBlock);
    DESTRUCTOR_DEFAULT(MoveConstraintBlock);
  private:
    ConstraintSet* mpConstraintSet; //!< Pointer to the ConstraintSet object the Constraint block reside in.
  };

  /*!
    \class CloneConstraintBlock
    \brief A ConstraintBlock to be cloned into target position.
  */
  class CloneConstraintBlock : public ExistingConstraintBlock {
  public:
    CloneConstraintBlock(ConstraintDistanceType start, const ConstraintSet* pConstrSet)
      : ExistingConstraintBlock(start), mpConstraintSet(pConstrSet), mSizeChange(0)
    {
    }

    const char* Type() const override { return "CloneConstraintBlock"; } //!< Return the class type in C string.
    void AppendTo(ConstraintSet& rConstrSet) const override; //!< Append Constraint objects in the block to the passedin ConstraintSet object.
    bool ShrinkTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const override; //!< Move Constraint objects in the block to target location in the passed in ConstraintSet object, shrinking mode.
    bool ExpandTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const override; //!< Move Constraint objects in the block to target location in the passed in ConstraintSet object, expanding mode.
    bool InPlace() const override { return false; } //!< Return false indicating the maneuver is NOT in place (inter-ConstraintSet).
    void SetSizeChange(uint64 sizeChange) { mSizeChange = sizeChange; } //!< Set size change.
    uint64 SizeChange() const { return mSizeChange; } //!< Return size change.

    ASSIGNMENT_OPERATOR_ABSENT(CloneConstraintBlock);
    COPY_CONSTRUCTOR_ABSENT(CloneConstraintBlock);
    DEFAULT_CONSTRUCTOR_ABSENT(CloneConstraintBlock);
    DESTRUCTOR_DEFAULT(CloneConstraintBlock);
  private:
    const ConstraintSet* mpConstraintSet; //!< Const pointer to the ConstraintSet object the Constraint block reside in.
    uint64 mSizeChange; //!< Change in size.
  };

  /*!
    \class InsertConstraintBlock
    \brief A container for a continuous block of Constraint objects to be inserted into target ConstraintSet.
  */
  class InsertConstraintBlock : public ConstraintBlock {
  public:
    explicit InsertConstraintBlock(std::vector<Constraint* >& rConstrVec) //!< Constructor with vector parameter.
      : ConstraintBlock(), mConstraints()
    {
      mConstraints.swap(rConstrVec);
    }

    explicit InsertConstraintBlock(const Constraint& rConstr) //!< Constructor with a Constraint parameter.
      : ConstraintBlock(), mConstraints()
    {
      mConstraints.push_back(rConstr.Clone());
    }

    InsertConstraintBlock() : ConstraintBlock(), mConstraints() { } //!< Constructor.
    ~InsertConstraintBlock() { } //!< Destructor.

    uint32 Size() const override { return mConstraints.size(); } //!< Return vector size.
    const std::string ToString() const override; //!< Return the string presentation of the ConstraintBlock object.
    const char* Type() const override { return "InsertConstraintBlock"; } //!< Return the class type in C string.
    void AppendTo(ConstraintSet& rConstrSet) const override; //!< Append Constraint objects in the block to the passed-in ConstraintSet object.
    bool ShrinkTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const override; //!< Move Constraint objects in the block to target location in the passed in ConstraintSet object, shrinking mode.
    bool ExpandTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const override; //!< Move Constraint objects in the block to target location in the passed in ConstraintSet object, expanding mode.

    ASSIGNMENT_OPERATOR_ABSENT(InsertConstraintBlock);
    COPY_CONSTRUCTOR_ABSENT(InsertConstraintBlock);
  private:
    mutable std::vector<Constraint* > mConstraints; //!< Constraints to be inserted.
  };

  /*!
    \struct ConstraintBlocksPlacement
    \brief Internal struct keep track of ConstraintBlocks placement information.
  */
  struct ConstraintBlocksPlacement {
  public:
    ConstraintBlocksPlacement(uint32 lower, uint32 upper, ConstraintDistanceType lowerLoc, ConstraintDistanceType upperLoc, EConstraintMoveDirection directionType)
      : mLowerIndex(lower), mUpperIndex(upper), mLowerLocation(lowerLoc), mUpperLocation(upperLoc), mDirectionType(directionType)
    {
    }

    DESTRUCTOR_DEFAULT(ConstraintBlocksPlacement);

    uint32 mLowerIndex; //!< Lower index of the ConstraintBlock object in mrConstraintBlocks for this placement.
    uint32 mUpperIndex; //!< Upper index of the ConstraintBlock object in mrConstraintBlocks for this placement.
    ConstraintDistanceType mLowerLocation; //!< Lower target location for the placement.
    ConstraintDistanceType mUpperLocation; //!< Upper target locstion for the placement.
    EConstraintMoveDirection mDirectionType; //!< Move direction type, shrink or expand.

    const std::string ToString() const; //!< Print out a string presentation of the object.
  };

  /*!
    \class ConstraintSetAssembler
    \brief This class is responsible of putting togather a new/renewed ConstraintSet object based on information provided by a vector of ConstraintBlock objects.
  */
  class ConstraintSetAssembler {
  public:
    ConstraintSetAssembler(ConstraintSet& rConstrSet, std::vector<ConstraintBlock* >& rConstrBlocks, uint32 newSize) //!< Constructor.
      : mrConstraintSet(rConstrSet), mrConstraintBlocks(rConstrBlocks), mNewSize(newSize), mPlacementStack()
    {

    }

    ~ConstraintSetAssembler(); //!< Destructor.
    const std::string ToString() const; //!< Print out the string representation of the ConstraintSetAssembler.
    void Assemble(); //!< Assemble the target ConstraintSet.

    ASSIGNMENT_OPERATOR_ABSENT(ConstraintSetAssembler);
    COPY_CONSTRUCTOR_ABSENT(ConstraintSetAssembler);
    DEFAULT_CONSTRUCTOR_ABSENT(ConstraintSetAssembler);
  private:
    uint32 CalculateSize() const; //!< Calculate the new vector size, for testing purpose.

    inline void AppendTo(ConstraintSet& rConstrSet) const //!< Append ConstraintBlocks to passed in ConstraintSet object.
    {
      for (auto constr_block : mrConstraintBlocks) {
        // << "Appending block: " << constr_block->ToString() << endl;
        constr_block->AppendTo(rConstrSet);
      }
    }

    void PlaceConstraints(); //!< Place ConstraintBlocks as specified by the passed in parameter.
    void ShrinkPlaceConstraints(const ConstraintBlocksPlacement& rPlacement); //!< Place ConstraintBlocks in shrinking direction.
    void ExpandPlaceConstraints(const ConstraintBlocksPlacement& rPlacement); //!< Place ConstraintBlocks in expanding direction.
  private:
    ConstraintSet& mrConstraintSet; //!< A reference holding the target ConstraintSet.
    std::vector<ConstraintBlock* > mrConstraintBlocks; //!< A refence holding the vector containing all the ContraintBlock objects.
    uint32 mNewSize; //!< New size of the ConstraintSet internal vector.
    std::stack<ConstraintBlocksPlacement> mPlacementStack; //!< A stack holding ConstraintBlocksPlacement objects.
  };

  /*!
    \class ConstraintSetBinaryProcessor
    \brief This class is the base class for supporting binary operations with two ConstraintSet objects, such as SubConstraintSet and MergeConstraintSet.
  */
  class ConstraintSetBinaryProcessor {
  public:
    ConstraintSetBinaryProcessor(ConstraintSet& rTargetConstrSet, const ConstraintYielder& rConstrYielder) //!< Constructor.
      : mrConstraintSet(rTargetConstrSet), mrConstraintVector(rTargetConstrSet.GetConstraintsMutable()), mrYielder(rConstrYielder), mSearchStartIterator(), mSizeChange(0), mVectorSize(0), mpCurrentBlock(nullptr), mBlocks()
    {
      mSearchStartIterator = mrConstraintVector.begin();
    }

    virtual ~ConstraintSetBinaryProcessor() //!< Destructor.
    {
      for (auto block : mBlocks) {
        delete block;
      }
      mBlocks.clear();

      if (nullptr != mpCurrentBlock) {
        LOG(fail) << "{ConstraintSetBinaryProcessor::~ConstraintSetSubtractor} dangling mpCurrentBlock pointer." << std::endl;
        FAIL("dangling-current-block-pointer");
      }
    }

    ASSIGNMENT_OPERATOR_ABSENT(ConstraintSetBinaryProcessor);
    COPY_CONSTRUCTOR_ABSENT(ConstraintSetBinaryProcessor);
    DEFAULT_CONSTRUCTOR_ABSENT(ConstraintSetBinaryProcessor);

    void Process() //!< Main processing loop.
    {
      mpCurrentBlock = new MoveConstraintBlock(ItemLocation(mrConstraintVector.begin()), &mrConstraintSet);

      while (not mrYielder.Done()) {
        ProcessConstraint();
        if (Done()) {
          break;
        }
      }

      // Commit last ConstraintBlock.
      CommitLastConstraintBlock();

      ConstraintSetAssembler constr_set_assembler(mrConstraintSet, mBlocks, mVectorSize);
      // << "Block assembler: " << constr_set_assembler.ToString() << endl;
      constr_set_assembler.Assemble();
    }

    uint64 SizeChange() const { return mSizeChange; } //!< Return the change of size after the subtraction.
  protected:
    virtual void ProcessConstraint() = 0; //!< Process one or a block of Constraints.

    inline bool Done() const //!< Return if the ConstraintSet processing should be done by now.
    {
      return (mSearchStartIterator == mrConstraintVector.end());
    }

    /*!
      Return item displacement from the beginning of the Constraint vector.
    */
    inline ConstraintDistanceType ItemLocation(ConstraintIterator constrIter) const
    {
      return std::distance(mrConstraintVector.begin(), constrIter);
    }

    /*!
      Shared simple function of actually committing the current ConstraintBlock object.
    */
    inline void CommitConstraintBlock(ConstraintDistanceType endLoc)
    {
      mpCurrentBlock->SetEnd(endLoc);
      if (not mpCurrentBlock->IsEmpty()) {
        mBlocks.push_back(mpCurrentBlock);
        mVectorSize += mpCurrentBlock->Size();
      }
      else {
        delete mpCurrentBlock;
      }
      // << "Committing block now: " << mpCurrentBlock->ToString() << endl;
      mpCurrentBlock = nullptr;
    }

    /*!
      Commit current ConstraintBlock and start a new one.
    */
    inline void CommitConstraintBlock(ConstraintIterator constrIter)
    {
      auto end_loc = ItemLocation(constrIter);
      CommitConstraintBlock(end_loc);
      // << "New block starting at displacement: " << dec << end_loc << endl;
      mpCurrentBlock = new MoveConstraintBlock(end_loc, &mrConstraintSet);
    }

    /*!
      Commit current ConstraintBlock and start a new one.  There is a gap between the two passed in iterators.
    */
    inline void CommitConstraintBlockWithGap(ConstraintIterator constrIter, ConstraintIterator newStartIter)
    {
      auto end_loc = ItemLocation(constrIter);
      CommitConstraintBlock(end_loc);
      auto new_start = ItemLocation(newStartIter);
      mpCurrentBlock = new MoveConstraintBlock(new_start, &mrConstraintSet);
    }

    /*!
      Commit the last ConstraintBlock item.
    */
    inline void CommitLastConstraintBlock()
    {
      auto end_loc = ItemLocation(mrConstraintVector.end());
      auto last_start = mpCurrentBlock->Start();
      if (end_loc < last_start) {
        LOG(fail) << "{ConstraintBinaryProcessor::CommitLastConstraintBlock} last item end loc: " << std::dec << end_loc << " less than start loc: " << last_start << std::endl;
        FAIL("incorrect-boundary-on-last-item");
      }
      else if (end_loc > last_start) {
        CommitConstraintBlock(end_loc);
      }
      else {
        delete mpCurrentBlock;
        mpCurrentBlock = nullptr;
      }
    }

  protected:
    ConstraintSet& mrConstraintSet; //!< Reference to target ConstraintSet.
    std::vector<Constraint* >& mrConstraintVector; //!< Reference to the vector holding ConstraintSet of the target ConstraintSet.
    const ConstraintYielder& mrYielder; //!< Constraint yielder yielding all the Constraint items to subtract.
    ConstraintIterator mSearchStartIterator; //!< Iterator pointing to the start of the searching range.
    uint64 mSizeChange; //!< Size change.
    uint32 mVectorSize; //!< New vector size.
    MoveConstraintBlock* mpCurrentBlock; //!< Pointer to the current MoveConstraintBlock object.
    std::vector<ConstraintBlock* > mBlocks; //!< Blocks of Constraints to be put into the final ConstraintSet.
  };

  /*!
    \struct PageAlignTraits
    \brief Struct used in AlignWithPage calculation.
  */
  struct PageAlignTraits
  {
    uint64 mAlignMask; //!< Page align mask.
    uint64 mOffsetMask; //!< Mask to extract offset.
    uint32 mShiftAmount; //!< Shift amount based on page size.

    PageAlignTraits(uint64 alignMask, uint32 shiftAmount) //!< Constructor.
      : mAlignMask(alignMask), mOffsetMask(~alignMask), mShiftAmount(shiftAmount)
    {
    }
  };

  /*!
    \class ConstraintTrimmer
    \brief Base class for reducing operations on a ConstraintSet object.
  */
  class ConstraintTrimmer {
  public:
    explicit ConstraintTrimmer(ConstraintSet& rConstrSet); //!< Constructor.
    void Trim(); //!< Trim Constraint entries in the vector.
    virtual const char* TrimmerName() const { return "ConstraintTrimmer"; } //!< Return constraint trimmer name.
    uint64 ConstraintSetSize() const { return mConstraintSetSize; } //!< Return the updated constraint set size.
    virtual DESTRUCTOR_DEFAULT(ConstraintTrimmer);
  protected:
    virtual void TrimConstraint() = 0; //!< Pure virtual method to be implemented by children classes.
  protected:
    uint64 mConstraintSetSize; //!< Size of the constraint set.
    ConstraintIterator mThroughIterator; //!< Iterator going through items sequentially.
    ConstraintIterator mInsertIterator; //!< Iterator pointing to the current inserting location.
    ConstraintOneResult mTrimResult; //!< Result of each trimming operation.
    std::vector<Constraint* >& mrConstraintVector; //!< Reference to a ConstraintSet object's Constraint vector.
  };

  /*!
    \class AlignWithSizeTrimmer
    \brief ConstraintTrimmer based class using Constraint::AlignWithSize interface.
  */
  class AlignWithSizeTrimmer : public ConstraintTrimmer {
  public:
    AlignWithSizeTrimmer(ConstraintSet& rConstrSet, uint64 alignMask, uint64 alignSize); //!< Constructor with align mask and align size parameters.
    const char* TrimmerName() const override { return "AlignWithSizeTrimmer"; } //!< Return AlignWithSizeTrimmer name.
    DESTRUCTOR_DEFAULT(AlignWithSizeTrimmer);
  protected:
    void TrimConstraint() override; //!< Trim current constraint with AlignWithSize.
  protected:
    uint64 mAlignMask; //!< Alignment mask.
    uint64 mAlignSize; //!< Aligning size.
  };

  /*!
    \class AlignOffsetWithSizeTrimmer
    \brief ConstraintTrimmer based class using Constraint::AlignOffsetWithSize interface.
  */
  class AlignOffsetWithSizeTrimmer : public AlignWithSizeTrimmer {
  public:
    AlignOffsetWithSizeTrimmer(ConstraintSet& rConstrSet, uint64 alignMask, uint64 alignOffset, uint64 alignSize); //!< Constructor with align mask, align offset and align size parameters.
    const char* TrimmerName() const override { return "AlignOffsetWithSizeTrimmer"; } //!< Return AlignWithSizeTrimmer name.
    DESTRUCTOR_DEFAULT(AlignOffsetWithSizeTrimmer);
  protected:
    void TrimConstraint() override; //!< Trim current constraint with AlignOffsetWithSize.
  protected:
    uint64 mAlignOffset; //!< Alignment offset.
  };
  /*!
    \class AlignMulDataWithSizeTrimmer
    \brief ConstraintTrimmer based class using Constraint::AlignMulDataWithSize interface.
  */
  class AlignMulDataWithSizeTrimmer: public AlignWithSizeTrimmer {
  public:
    AlignMulDataWithSizeTrimmer(ConstraintSet& rConstrSet, uint64 mulData, uint64 baseRemainder, uint64 alignSize); //!< Constructor.
    const char* TrimmerName() const override { return "AlignMulDataWithSizeTrimmer"; } //!< Return AlignWithSizeTrimmer name.
    DESTRUCTOR_DEFAULT(AlignMulDataWithSizeTrimmer);
  protected:
    void TrimConstraint() override; //!< Trim current constraint with AlignMulDataWithSize.
  protected:
    uint64 mMulData; //!< multiplier of offset.
    uint64 mBaseRemainder; //!< Value of basevalue mod muldata.
  };

  /*!
    \class IndexMaskOperator.h
    \brief Base class for index mask operator.
  */
  class IndexMaskOperator {
  public:
    IndexMaskOperator(uint64 index, uint64 mask); //!< Constructor with index and mask given.
    IndexMaskOperator(); //!< Default constructor.
    virtual ~IndexMaskOperator(); //!< Destructor.

    inline uint64 Index() const { return mIndex; } //!< Return index value.
    inline uint64 Mask() const { return mMask; } //!< Return mask value.
    void Process(Constraint* pConstr, ConstraintOneResult& rResult) const; //!< Process Constraint object.
  protected:
    IndexMaskOperator(const IndexMaskOperator& rOther); //!< Copy constructor.
    virtual void TakeOutMask(uint64& rValue) const; //!< Take out the masked bits.
  protected:
    uint64 mIndex; //!< Index value.
    uint64 mMask; //!< Mask value.
  };

  /*!
    \class IndexMaskTrimmer
    \brief ConstraintTrimmer based class using Constraint::ApplyIndexMask interface.
  */
  class IndexMaskTrimmer : public ConstraintTrimmer {
  public:
    IndexMaskTrimmer(ConstraintSet& rConstrSet, const IndexMaskOperator* pOperator); //!< Constructor with operator parameters.
    const char* TrimmerName() const override { return "IndexMaskTrimmer"; } //!< Return IndexMaskTrimmer name.
    ASSIGNMENT_OPERATOR_ABSENT(IndexMaskTrimmer);
    COPY_CONSTRUCTOR_DEFAULT(IndexMaskTrimmer);
    DESTRUCTOR_DEFAULT(IndexMaskTrimmer);
  protected:
    void TrimConstraint() override; //!< Trim current constraint with IndexMask.
  protected:
    const IndexMaskOperator* mpOperator; //!< Pointer to IndexMaskOperator object.
  };

  /*!
    \class ConstraintSetSerializer
    \brief This class is responsible for serialize a ContraintSet object.
  */
  class ConstraintSetSerializer {
  public:
    ConstraintSetSerializer(const ConstraintSet& rConstrSet, uint32 numPerLine) //!< Constructor.
      : mrConstraintSet(rConstrSet), mConstraintIterator(), mEndIterator(), mNumberPerLine(numPerLine)
    {
      mConstraintIterator = mrConstraintSet.GetConstraints().begin();
      mEndIterator = mrConstraintSet.GetConstraints().end();
    }

    ~ConstraintSetSerializer() { } //!< Destructor.
    std::string ToSimpleString() const; //!< Return a simple string output.
    std::string ToDebugString() const; //!< Return a debug string output.
    void Serialize(const std::string& rName, uint32 serialNumber) const; //!< Write out the representation of the ConstraintSet object.
  private:
    void OutputLine(std::ostream& rOutputStream) const; //!< Return a line of Constraint print out.
    void OutputLineDebug(std::ostream& rOutputStream) const; //!< Return a line of Constraint print out, debug version.
    inline bool Done() const { return (mConstraintIterator == mEndIterator); } //!< Return whether the iteration is done.
  private:
    const ConstraintSet& mrConstraintSet; //!< Const reference to the ConstraintSet object being serialized.
    mutable ConstConstraintIterator mConstraintIterator; //!< Iterator pointing to the current Constraint object.
    ConstConstraintIterator mEndIterator; //!< Iterator pointing to the end of the vector.
    uint32 mNumberPerLine; //!< Number of Constraints per line.
  };

  /*!
    \class ConstraintSetDeserializer
    \brief This class is responsible for deserializing a ConstraintSet object.
  */
  class ConstraintSetDeserializer {
  public:
    explicit ConstraintSetDeserializer(ConstraintSet& rConstrSet) //!< Constructor.
      : mrConstraintSet(rConstrSet)
    {

    }

    ~ConstraintSetDeserializer() { } //!< Destructor.

    void Deserialize(const std::string& rName, uint32 serialNumber); //!< Deserialize the referenced ConstraintSet object.
  private:
    ConstraintSet& mrConstraintSet; //!< Reference to the ConstraintSet object being deserialized.
  };

}

#endif
