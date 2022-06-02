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
#include "ConstraintUtils.h"

#include <fstream>
#include <numeric>  // C++UP accumulate defined in numeric
#include <sstream>

#include "Constraint.h"
#include "Log.h"

using namespace std;

/*!
  \file ConstraintUtils.cc
  \brief Code containing various Constraint/ConstraintSet module utilities.
*/

namespace Force {

  pair<ConstConstraintIterator, ConstConstraintIterator> ConstraintYielder::GetMatchingRange(const Constraint& rSearchConstr) const
  {
    return equal_range(mItemIterator, mrConstraintVector.end(), &rSearchConstr, &compare_constraints);
  }

  void ConstraintYielder::SetIterator(ConstConstraintIterator newIter) const
  {
    auto new_dist = distance(mItemIterator, newIter);
    auto whole_dist = distance(mItemIterator, mEndIterator);
    if ((new_dist < 0) || (new_dist > whole_dist)) {
      LOG(fail) << "{ConstraintYielder::SetIterator} new iterator out of range." << endl;
      FAIL("iterator-out-of-range");
    }

    mItemIterator = newIter;
  }

  CloneConstraintBlock* ConstraintYielder::YieldCloneConstraintBlock(ConstConstraintIterator startIter, ConstConstraintIterator endIter) const
  {
    auto start_dist = distance(mrConstraintVector.begin(), startIter);
    if (start_dist < 0) {
      LOG(fail) << "{ConstraintYielder::YieldCloneBlock} starting distance less than 0: " << dec << start_dist << endl;
      FAIL("starting-distance-less-than-zero");
    }
    auto end_dist = distance(mrConstraintVector.begin(), endIter);
    if (end_dist <= start_dist) {
      LOG(fail) << "{ConstraintYielder::YieldCloneBlock} ending distance: " << dec << end_dist << " should be larger than starting distance: " << start_dist << endl;
      FAIL("ending-dist-not-larger-than-starting-dist");
    }
    if (end_dist > ConstraintDistanceType(mrConstraintVector.size())) {
      LOG(fail) << "{ConstraintYielder::YieldCloneBlock} ending distance : " << dec << end_dist << " larger than vector size: " << mrConstraintVector.size() << endl;
      FAIL("ending-dist-out-of-bound");
    }

    auto ret_block = new CloneConstraintBlock(start_dist, &mrConstraintSet);
    ret_block->SetEnd(end_dist);

    // set size change.
    uint64 size_change = accumulate(startIter, endIter, uint64(0),
      [](cuint64 size, const Constraint* pConstr) { return size + pConstr->Size(); });
    ret_block->SetSizeChange(size_change);

    mItemIterator = endIter; // update iterator.
    return ret_block;
  }

  void check_move_target(ConstConstraintIterator beginIter, ConstConstraintIterator targetIter, const Constraint* pConstr)
  {
    LOG(notice) << "{check_move_target} insertion iterator distance from beginning: " << dec << distance(beginIter, targetIter) << endl;
    if (nullptr == pConstr) {
      LOG(fail) << "{check_move_target} moving in a nullptr." << endl;
      FAIL("nullptr-being-moved");
    }
    if (nullptr != (*targetIter)) {
      LOG(fail) << "{check_move_target} dangling Constraint object pointer, inserting: " << pConstr->ToString() << endl;
      LOG(fail) << "{check_move_target} existing object at target location: " << (*targetIter)->ToString() << endl;
      FAIL("dangling-constraint-pointer-moving-item");
    }
  }

  void ConstraintBlock::VerifyCapacity(const ConstraintSet& rConstrSet, uint32 vectorSize) const
  {
    const vector<Constraint* >& constr_vec = rConstrSet.GetConstraints();
    if (vectorSize > constr_vec.capacity()) {
      LOG(fail) << "{ConstraintBlock::VerifyCapacity} intended vector size (" << dec << vectorSize << ") shouldn't exceed target vector's capacity (" << constr_vec.capacity() << ") for good performance." << endl;
      FAIL("constraint-set-assembling-issue");
    }
  }

  ConstraintDistanceType ConstraintBlock::InPlaceStart() const
  {
    LOG(fail) << "{ConstraintBlock::InPlaceStart} called unexpectedly on class that don't implement it." << endl;
    FAIL("in-place-start-not-implemented");
    return 0;
  }

  ConstraintDistanceType ConstraintBlock::InPlaceEnd() const
  {
    LOG(fail) << "{ConstraintBlock::InPlaceEnd} called unexpectedly on class that don't implement it." << endl;
    FAIL("in-place-end-not-implemented");
    return 0;
  }

  const string ExistingConstraintBlock::ToString() const
  {
    stringstream out_str;

    out_str << Type() << ": ConstraintSet(InPlace=" << InPlace() << ") [" << mStart << "-" << mEnd << "]";

    return out_str.str();
  }

  void MoveConstraintBlock::AppendTo(ConstraintSet& rConstrSet) const
  {
    vector<Constraint* >& target_vec = rConstrSet.GetConstraintsMutable();

#ifdef UNIT_TEST
    // verify capacity is okay
    ConstraintSizeType new_size = target_vec.size() + Size();
    VerifyCapacity(rConstrSet, new_size);
#endif

    vector<Constraint* >& source_vec = mpConstraintSet->GetConstraintsMutable();
    ConstraintIterator block_start_iter = source_vec.begin() + mStart;
    ConstraintIterator block_end_iter = source_vec.begin() + mEnd;
    target_vec.insert(target_vec.end(), block_start_iter, block_end_iter);

#ifdef UNIT_TEST
    for (auto test_iter = block_start_iter; test_iter != block_end_iter; ++ test_iter) {
      if (nullptr == (*test_iter)) {
        LOG(fail) << "{MoveConstraintBlock::AppendTo} appending nullptr, block range: " << dec << mStart << "-" << mEnd << " nullptr at: " << distance(source_vec.begin(), test_iter) << endl;
        // don't call FAIL here, will segfault due to the ConstraintSet is in a fluke state.
      }
    }
    // clean up the pointers for UNIT_TEST checking.
    fill(block_start_iter, block_end_iter, nullptr);
#endif
  }

  bool MoveConstraintBlock::ShrinkTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const
  {
#ifdef UNIT_TEST
    if (mpConstraintSet != (&rConstrSet)) {
      LOG(fail) << "{MoveConstraintBlock::ShrinkTo} should be shrinking to the same ConstraintSet object." << endl;
      FAIL("shrink-not-to-the-same-obejct");
    }
    if (targetLoc > mStart) {
      LOG(fail) << "{MoveConstraintBlock::ShrinkTo} target location: " << dec << targetLoc << " shouldn't be larger than original start location: " << mStart << endl;
      FAIL("shrinking-to-larger-location");
    }
#endif

    if (targetLoc == mStart) return true; // nothing to do, stay put.

    shrinking_move_block(mStart, targetLoc, Size(), rConstrSet.GetConstraintsMutable());
    return true;
  }

  bool MoveConstraintBlock::ExpandTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const
  {
    auto end_loc = mEnd - 1;

#ifdef UNIT_TEST
    if (mpConstraintSet != (&rConstrSet)) {
      LOG(fail) << "{MoveConstraintBlock::ExpandTo} should be expanding to the same ConstraintSet object." << endl;
      FAIL("expand-not-to-the-same-obejct");
    }
    if (targetLoc < end_loc) {
      LOG(fail) << "{MoveConstraintBlock::ExpandTo} target location: " << dec << targetLoc << " shouldn't be less than original end location: " << end_loc << endl;
      FAIL("expanding-to-lesser-location");
    }
#endif

    if (targetLoc == end_loc) return true; // nothing to do, stay put.

    auto block_size = Size();
    expanding_move_block(mStart, targetLoc - (block_size - 1), block_size, rConstrSet.GetConstraintsMutable());
    return true;
  }

  void CloneConstraintBlock::AppendTo(ConstraintSet& rConstrSet) const
  {
    vector<Constraint* >& target_vec = rConstrSet.GetConstraintsMutable();

#ifdef UNIT_TEST
    // verify capacity is okay
    ConstraintSizeType new_size = target_vec.size() + Size();
    VerifyCapacity(rConstrSet, new_size);
#endif

    const vector<Constraint* >& source_vec = mpConstraintSet->GetConstraints();
    ConstConstraintIterator block_start_iter = source_vec.begin() + mStart;
    ConstConstraintIterator block_end_iter = source_vec.begin() + mEnd;

    for (auto block_iter = block_start_iter; block_iter != block_end_iter; ++ block_iter) {
      target_vec.push_back((*block_iter)->Clone());
    }
  }

  bool CloneConstraintBlock::ShrinkTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const
  {
    vector<Constraint* >& target_vec = rConstrSet.GetConstraintsMutable();
    if (not ShrinkInsertionOkay(targetLoc, rBlockYielder, target_vec.size()))
      return false;

    const vector<Constraint* > &clone_vec = mpConstraintSet->GetConstraints();
    ConstConstraintIterator clone_iter = clone_vec.begin() + mStart;
    ConstConstraintIterator end_iter = clone_vec.begin() + mEnd;

    ConstraintIterator target_iter = target_vec.begin() + targetLoc;

    for (; clone_iter != end_iter; ++ clone_iter) {
      auto clone_item = (*clone_iter)->Clone();

#ifdef UNIT_TEST
      check_move_target(target_vec.begin(), target_iter, clone_item);
#endif

      (*target_iter) = clone_item;
      ++ target_iter;
    }

    return true;
  }

  bool CloneConstraintBlock::ExpandTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const
  {
    vector<Constraint* >& target_vec = rConstrSet.GetConstraintsMutable();
    if (not ExpandInsertionOkay(targetLoc, rBlockYielder, target_vec.size()))
      return false;

    const vector<Constraint* > &clone_vec = mpConstraintSet->GetConstraints();
    ConstConstraintIterator clone_iter = clone_vec.begin() + mEnd - 1;
    ConstConstraintIterator end_iter = clone_vec.begin() + mStart - 1;

    ConstraintIterator target_iter = target_vec.begin() + targetLoc;

    for (; clone_iter != end_iter; -- clone_iter) {
      auto clone_item = (*clone_iter)->Clone();

#ifdef UNIT_TEST
      check_move_target(target_vec.begin(), target_iter, clone_item);
#endif

      (*target_iter) = clone_item;
      -- target_iter;
    }

    return true;
  }

  void InsertConstraintBlock::AppendTo(ConstraintSet& rConstrSet) const
  {
    vector<Constraint* >& target_vec = rConstrSet.GetConstraintsMutable();

#ifdef UNIT_TEST
    // verify capacity is okay
    ConstraintSizeType new_size = target_vec.size() + Size();
    VerifyCapacity(rConstrSet, new_size);
#endif

    target_vec.insert(target_vec.end(), mConstraints.begin(), mConstraints.end());
    mConstraints.clear();
  }

  bool InsertConstraintBlock::ShrinkTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const
  {
    vector<Constraint* >& target_vec = rConstrSet.GetConstraintsMutable();
    if (not ShrinkInsertionOkay(targetLoc, rBlockYielder, target_vec.size()))
      return false;

#ifdef UNIT_TEST
    ConstraintIterator test_target_iter = target_vec.begin() + targetLoc;
    for (auto constr_item : mConstraints) {
      check_move_target(target_vec.begin(), test_target_iter, constr_item);
      ++ test_target_iter;
    }
#endif

    memcpy(&(target_vec[targetLoc]), &(mConstraints[0]), Size() * sizeof(Constraint *));
    mConstraints.clear(); // items have been moved away.
    return true;
  }

  bool InsertConstraintBlock::ExpandTo(ConstraintSet& rConstrSet, ConstraintDistanceType targetLoc, const ConstraintBlockYielder& rBlockYielder) const
  {
    vector<Constraint* >& target_vec = rConstrSet.GetConstraintsMutable();
    if (not ExpandInsertionOkay(targetLoc, rBlockYielder, target_vec.size()))
      return false;

#ifdef UNIT_TEST
    ConstraintIterator test_target_iter = target_vec.begin() + targetLoc;
    for (vector<Constraint* >::reverse_iterator reverse_iter = mConstraints.rbegin(); reverse_iter != mConstraints.rend(); ++ reverse_iter) {
      auto constr_item = (*reverse_iter);
      check_move_target(target_vec.begin(), test_target_iter, constr_item);
      -- test_target_iter;
    }
#endif

    auto block_size = Size();
    auto target_start = targetLoc - (block_size - 1);
    memcpy(&(target_vec[target_start]), &(mConstraints[0]), block_size * sizeof(Constraint *));
    mConstraints.clear(); // items have been moved away.
    return true;
  }

  const string InsertConstraintBlock::ToString() const
  {
    stringstream out_str;

    // Use the vector size of the ConstraintSet object as sort of a ID.
    out_str << Type() << ": ConstraintSet(vector-size=" << dec << mConstraints.size() << ") [";

    bool first_done = false;
    for (auto constr_item : mConstraints) {
      if (first_done)
        out_str << ",";
      else
        first_done = true;

      out_str << constr_item->ToSimpleString();
    }

    out_str << "]";

    return out_str.str();
  }

  ConstraintBlockYielder::ConstraintBlockYielder(const ConstraintBlocksPlacement* pPlacement, const vector<ConstraintBlock* >& rBlockVec)
    : mCurrentIndex(0), mEndIndex(0), mIncrement(0), mpBlocksPlacement(pPlacement), mrBlockVector(rBlockVec)
  {
    if (mpBlocksPlacement->mLowerIndex > mpBlocksPlacement->mUpperIndex) {
      LOG(fail) << "{ConstraintBlockYielder::ConstraintBlockYielder} lower index: " << dec << mpBlocksPlacement->mLowerIndex << " larger than upper index: " << mpBlocksPlacement->mUpperIndex << endl;
      FAIL("lower-larger-than-upper");
    }

    switch (mpBlocksPlacement->mDirectionType) {
    case EConstraintMoveDirection::Shrink:
      {
        mIncrement = 1;
        mCurrentIndex = mpBlocksPlacement->mLowerIndex;
        mEndIndex = mpBlocksPlacement->mUpperIndex + 1;
      }
      break;
    case EConstraintMoveDirection::Expand:
      {
        mIncrement = -1;
        mCurrentIndex = mpBlocksPlacement->mUpperIndex;
        mEndIndex = mpBlocksPlacement->mLowerIndex - 1;
      }
      break;
    default:
      LOG(fail) << "{ConstraintBlockYielder::ConstraintBlockYielder} unsupported EConstraintMoveDirection: " << EConstraintMoveDirection_to_string(mpBlocksPlacement->mDirectionType) << endl;
      FAIL("unsuppored-constraint-move-direction");
    }
  }

  ConstraintDistanceType ConstraintBlockYielder::NextShrinkingBoundary() const
  {
#ifdef UNIT_TEST
    if (mCurrentIndex > mrBlockVector.size()) {
      LOG(fail) << "{ConstraintBlockYielder::NextShrinkingItem} index out of bound." << endl;
      FAIL("shrinking-next-index-out-of-bound");
    }
#endif

    if (mCurrentIndex == mEndIndex) { // reached end already
      return mpBlocksPlacement->mUpperLocation + 1;
    }
    else {
      return mrBlockVector[mCurrentIndex]->InPlaceStart();
    }
  }

  ConstraintDistanceType ConstraintBlockYielder::NextExpandingBoundary() const
  {
#ifdef UNIT_TEST
    if ((mCurrentIndex != uint32(-1)) && (mCurrentIndex > mrBlockVector.size())) {
      LOG(fail) << "{ConstraintBlockYielder::NextExpandingItem} index out of bound." << endl;
      FAIL("expanding-next-index-out-of-bound");
    }
#endif

    if (mCurrentIndex == mEndIndex) { // reached end aleady
      return mpBlocksPlacement->mLowerLocation;
    }
    else {
      return mrBlockVector[mCurrentIndex]->InPlaceEnd();
    }
  }

  ConstraintDistanceType ConstraintBlockYielder::SeekShrinkingSpace(ConstraintDistanceType targetLoc, uint32 blockSize, uint32& rUpperIndex) const
  {
    uint32 search_index = mCurrentIndex;
    // << "seek, target loc" << dec << targetLoc << " block size: " << blockSize << " search index: " << search_index << endl;
    uint32 remaining_size = blockSize;
    ConstraintDistanceType last_start = targetLoc;
    for (; search_index != mEndIndex; search_index += mIncrement) {
      auto next_item = mrBlockVector[search_index];

      if (next_item->InPlace()) {
        // In place block is the only possibility to provide space.
        auto next_start = next_item->InPlaceStart();
        if (next_start >= last_start) {
          auto space_found = next_start - last_start;
          if (space_found >= remaining_size) {
            // Search done, enough space found.
            rUpperIndex = search_index - mIncrement;
            ConstraintDistanceType upper_loc = last_start + (remaining_size - 1);
            return upper_loc;
          }
          else {
            // subtract newly found space from remaining_size
            remaining_size -= space_found;
            last_start = next_item->InPlaceEnd();
          }
        }
        else { // sanity check
          LOG(fail) << "{ConstraintBlockYielder::SeekShrinkingSpace} unexpected next item starting location: " << dec << next_start << " less than last start: " << last_start << endl;
          FAIL("next-start-less-than-last-start");
        }
      }
      else {
        // more space is needed to allow for this new insertion.
        remaining_size += next_item->Size();
      }
    }

    // handle the case when the last item (or last few items) in the original ConstraintSet were deleted
    auto last_size = mpBlocksPlacement->mUpperLocation - last_start + 1;
    if (last_size == remaining_size) {
      rUpperIndex = mEndIndex - 1;
      ConstraintDistanceType upper_loc = mpBlocksPlacement->mUpperLocation;
      return upper_loc;
    }

    LOG(fail) << "{ConstraintBlockYielder::SeekShrinkingSpace} failed to find space to insert block of size: " << dec << blockSize <<  " last start: " << dec << last_start << " remaining size: " << remaining_size << endl;
    FAIL("no-space-to-insert-while-shrinking");
    return 0;
  }

  ConstraintDistanceType ConstraintBlockYielder::SeekExpandingSpace(ConstraintDistanceType targetLoc, uint32 blockSize, uint32& rLowerIndex) const
  {
    uint32 search_index = mCurrentIndex;

    uint32 remaining_size = blockSize;
    ConstraintDistanceType last_end = targetLoc;
    // << "target loc: " << dec << targetLoc << " block size: " << blockSize << endl;
    for (; search_index != mEndIndex; search_index += mIncrement) {
      auto next_item = mrBlockVector[search_index];
      // << "next item index: " << dec << search_index << " " << next_item->ToString() << endl;
      if (next_item->InPlace()) {
        // In place block is the only possibility to provide space.
        auto next_end = next_item->InPlaceEnd() - 1;
        if (next_end <= last_end) {
          auto space_found = last_end - next_end;
          // << " space found: " << dec << space_found << " remaining size: " << remaining_size << " last end: " << last_end << endl;
          if (space_found >= remaining_size) {
            // Search done, enough space found.
            rLowerIndex = search_index - mIncrement;
            ConstraintDistanceType lower_loc = last_end - (remaining_size - 1);
            return lower_loc;
          }
          else {
            // subtract newly found space from remaining_size
            remaining_size -= space_found;
            last_end = next_item->InPlaceStart() - 1;
          }
        }
        else { // sanity check
          LOG(fail) << "{ConstraintBlockYielder::SeekExpandingSpace} unexpected next item ending location: " << dec << next_end << " larger than last end: " << last_end << endl;
          FAIL("next-end-larger-than-last-end");
        }
      }
      else {
        // more space is needed to allow for this new insertion.
        remaining_size += next_item->Size();
      }
    }

    // handle the case when the first item (or first few items) in the original ConstraintSet were deleted
    auto first_size = last_end - mpBlocksPlacement->mLowerLocation + 1;
    if (first_size == remaining_size) {
      rLowerIndex = mpBlocksPlacement->mLowerIndex;
      ConstraintDistanceType lower_loc = mpBlocksPlacement->mLowerLocation;
      return lower_loc;
    }

    LOG(fail) << "{ConstraintBlockYielder::SeekExpandingSpace} failed to find space to insert block of size: " << dec << blockSize << " last end: " << dec << last_end << " remaining size: " << remaining_size << endl;
    FAIL("no-space-to-insert-while-expanding");
    return 0;
  }

  const string ConstraintBlockYielder::ToString() const
  {
    stringstream out_str;
    out_str << "ConstraintBlockYielder (current-index=" << dec << mCurrentIndex << ", end-index=" << mEndIndex << ", increment=" << mIncrement << ", block vector size: " << mrBlockVector.size() << endl;

    for (auto index = mCurrentIndex; index != mEndIndex; index += mIncrement) {
      out_str << mrBlockVector[index]->ToString() << endl;
    }

    return out_str.str();
  }

  const string ConstraintBlocksPlacement::ToString() const
  {
    stringstream out_str;

    out_str << "ConstraintBlocksPlacement: indices [" << dec << mLowerIndex << "-" << mUpperIndex << "], target location [" << mLowerLocation << "-" << mUpperLocation << "] direction: " << EConstraintMoveDirection_to_string(mDirectionType);

    return out_str.str();
  }

  ConstraintSetAssembler::~ConstraintSetAssembler()
  {
    // Nothing to do, for efficiency, the vector of ConstraintBlock object would have been deleting all the ConstraintBlock object when iterating through it.
  }

  const string ConstraintSetAssembler::ToString() const
  {
    // mostly we only care about the vector of ConstraintBlock.
    stringstream out_str;

    out_str << "ConstraintBlockAssembler: " << endl;
    for (auto block_item : mrConstraintBlocks) {
      out_str << block_item->ToString() << endl;
    }

    return out_str.str();
  }

  uint32 ConstraintSetAssembler::CalculateSize() const
  {
    return accumulate(mrConstraintBlocks.cbegin(), mrConstraintBlocks.cend(), uint32(0),
      [](cuint32 size, const ConstraintBlock* pBlockItem) { return size + pBlockItem->Size(); });
  }

  /*!
    When doing shrinking placement, go from low index to high index.
  */
  void ConstraintSetAssembler::ShrinkPlaceConstraints(const ConstraintBlocksPlacement& rPlacement)
  {
    auto block_loc = rPlacement.mLowerLocation;
    ConstraintBlockYielder block_yielder(&rPlacement, mrConstraintBlocks);

    // << "Block yielder: " << block_yielder.ToString() << endl;

    while (not block_yielder.Done()) {
      auto constr_block = block_yielder.Yield();
      uint32 block_size = constr_block->Size(); // get size first since it could change after the ShrinkTo call.
      // << "Shrinking " << constr_block->ToString() << " block loc: " << dec << block_loc << " size: " << block_size << endl;
      if (constr_block->ShrinkTo(mrConstraintSet, block_loc, block_yielder)) {
        block_loc += block_size;
      }
      else {
        uint32 upper_index = 0;
        auto upper_loc = block_yielder.SeekShrinkingSpace(block_loc, constr_block->Size(), upper_index);
        // << "Found upper loc: " << dec << upper_loc << " upper index: " << upper_index << endl;
        // Push the remaing part onto stack so it can be processed after the insertion-expansion is done, if there is remainder.
        if (upper_loc < rPlacement.mUpperLocation) {
          if (upper_index >= rPlacement.mUpperIndex) {
            LOG(fail) << "{ConstraintSetAssembler::ShrinkPlaceConstraints} since there is remainder, the upper index (" << dec << upper_index << ") of last pushed item should be less than placement uppder index (" << rPlacement.mUpperIndex << ")." << endl;
            FAIL("upper-index-should-be-less-than-original-placement");
          }
          mPlacementStack.emplace(upper_index + 1, rPlacement.mUpperIndex, upper_loc + 1, rPlacement.mUpperLocation, EConstraintMoveDirection::Shrink);
        }
        // Now push the insertion-expansion placement to top of stack.
        mPlacementStack.emplace(block_yielder.LastIndex(), upper_index, block_loc, upper_loc, EConstraintMoveDirection::Expand);
        break;
      }
    }
  }

  /*!
    When doing expanding placement, go from high index to low index.
   */
  void ConstraintSetAssembler::ExpandPlaceConstraints(const ConstraintBlocksPlacement& rPlacement)
  {
    auto block_loc = rPlacement.mUpperLocation;
    ConstraintBlockYielder block_yielder(&rPlacement, mrConstraintBlocks);

    // << "Block yielder: " << block_yielder.ToString() << endl;

    while (not block_yielder.Done()) {
      auto constr_block = block_yielder.Yield();
      uint32 block_size = constr_block->Size(); // get size first since it could change after the ExpandTo call.
      // << "Expanding " << constr_block->ToString() << " block loc: " << dec << block_loc << " size: " << block_size << endl;
      if (constr_block->ExpandTo(mrConstraintSet, block_loc, block_yielder)) {
        block_loc -= block_size;
      }
      else {
        uint32 lower_index = 0;
        auto lower_loc = block_yielder.SeekExpandingSpace(block_loc, constr_block->Size(), lower_index);
        // << "Found lower loc: " << dec << lower_loc << " lower index: " << lower_index << endl;
        // Push the remaining part onto stack so it can be processed after the shrink and insertion is done, if there is a remainder.
        if (lower_loc > rPlacement.mLowerLocation) {
          if (lower_index <= rPlacement.mLowerIndex) {
            LOG(fail) << "{ConstraintSetAssembler::ExpandPlaceConstraints} since there is remainder, the lower index (" << dec << lower_index << ") of last pushed item should be larger than placement lower index (" << rPlacement.mLowerIndex << ")." << endl;
            FAIL("lower-index-should-be-larger-than-original-placement");
          }
          mPlacementStack.emplace(rPlacement.mLowerIndex, lower_index - 1, rPlacement.mLowerLocation, lower_loc - 1, EConstraintMoveDirection::Expand);
        }
        // Now push the shrinking-insertion placement to top of stack.
        mPlacementStack.emplace(lower_index, block_yielder.LastIndex(), lower_loc, block_loc, EConstraintMoveDirection::Shrink);
        break;
      }
    }
  }

  void ConstraintSetAssembler::PlaceConstraints()
  {
    // << " start placing constraints." << endl;
    while (not mPlacementStack.empty()) {
      ConstraintBlocksPlacement top_placement = mPlacementStack.top();
      mPlacementStack.pop();
      // << "placement " << top_placement.ToString() << endl;

      switch (top_placement.mDirectionType) {
      case EConstraintMoveDirection::Shrink:
        ShrinkPlaceConstraints(top_placement);
        break;
      case EConstraintMoveDirection::Expand:
        ExpandPlaceConstraints(top_placement);
        break;
      default:
        LOG(fail) << "{ConstraintSetAssembler::PlaceConstraints} unsupported EConstraintMoveDirection: " << EConstraintMoveDirection_to_string(top_placement.mDirectionType) << endl;
        FAIL("unsuppored-constraint-move-direction");
      }
    }
  }

  void ConstraintSetAssembler::Assemble()
  {
#ifdef UNIT_TEST
    uint32 test_size = CalculateSize();
    if (test_size != mNewSize) {
      LOG(fail) << "{ConstraintSetAssembler::Assemble} calculated size: " << dec << test_size << " not equal to expected size: " << mNewSize << endl;
      FAIL("size-check-failed");
    }
#endif

    vector<Constraint* >& target_vec = mrConstraintSet.GetConstraintsMutable();

    auto orig_size = mrConstraintSet.VectorSize();
    if (mNewSize <= orig_size) {
      if (mNewSize == 0) {
        clear_constraint_vector(target_vec);
        if (mrConstraintBlocks.size()) {
          LOG(fail) << "{ConstraintSetAssembler::Assemble} new result is empty but ConstraintBlock vector size (should be zero) is: " << dec << mrConstraintBlocks.size() << endl;
          FAIL("blocks-exist-for-empty-result-vector");
        }
        return;
      }
      // << "new size: " << dec << mNewSize << " smaller than original size: " << orig_size << ", shrinking." << endl;
      mPlacementStack.emplace(0, mrConstraintBlocks.size() - 1, 0, ConstraintDistanceType(mNewSize - 1), EConstraintMoveDirection::Shrink);
      PlaceConstraints();
      target_vec.resize(mNewSize); // Now we can shrink the actual size.
    }
    else {
      auto capacity = target_vec.capacity();
      // << "new size: " << dec << mNewSize << " larger than original size: " << orig_size << ", expanding, capacity: " << capacity << endl;
      if (mNewSize <= capacity) {
        target_vec.resize(mNewSize); // Expand the vector first.
        mPlacementStack.emplace(0, mrConstraintBlocks.size() - 1, 0, ConstraintDistanceType(mNewSize - 1), EConstraintMoveDirection::Expand);
        PlaceConstraints();
      }
      else {
        ConstraintSet temp_constr_set;
        vector<Constraint* >& temp_vec = temp_constr_set.GetConstraintsMutable();
        // need to reserve to increase capacity first, will use a temporary vector then do swap.
        temp_vec.reserve(mNewSize);

        // Move all Constraint objects here.
        AppendTo(temp_constr_set);

        target_vec.swap(temp_vec);
        temp_vec.clear(); // Objects has been copied or deleted, clearing is fine.
      }
    }

#ifdef UNIT_TEST
    check_nullptr_entry(target_vec);
#endif

  }

  void check_shrinking_move_block(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, ConstraintIterator targetVectorBegin)
  {
    if (targetStart >= sourceStart) {
      LOG(fail) << "{check_shrinking_move_block} target start should be less than source start." << endl;
      FAIL("target-start-not-less-than-source-start");
    }

    ConstraintIterator src_start_iter = targetVectorBegin + sourceStart;
    ConstraintIterator src_end_iter = src_start_iter + blockSize;

    auto test_tgt_iter = targetVectorBegin + targetStart;
    for (auto test_src_iter = src_start_iter; test_src_iter != src_end_iter; ++ test_src_iter) {
      Constraint* move_item = (*test_src_iter);
      check_move_target(targetVectorBegin, test_tgt_iter, move_item);
      ++ test_tgt_iter;
      if (test_tgt_iter == src_start_iter) {
        // start to overlap.
        break;
      }
    }
  }

  void check_expanding_move_block(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, ConstraintIterator targetVectorBegin)
  {
    if (targetStart <= sourceStart) {
      LOG(fail) << "{check_expanding_move_block} target start should be larger than source start." << endl;
      FAIL("target-start-not-larger-than-source-start");
    }

    ConstraintIterator test_src_start_iter = targetVectorBegin + sourceStart + (blockSize - 1);
    ConstraintIterator test_src_end_iter = targetVectorBegin + sourceStart - 1;
    auto test_tgt_iter = targetVectorBegin + targetStart + (blockSize - 1);

    for (auto test_src_iter = test_src_start_iter; test_src_iter != test_src_end_iter; -- test_src_iter) {
      Constraint* move_item = (*test_src_iter);

      // << "expanding move item " << move_item->ToString() << endl;
      check_move_target(targetVectorBegin, test_tgt_iter, move_item);
      -- test_tgt_iter;
      if (test_tgt_iter == test_src_start_iter) {
        // start to overlap
        break;
      }
    }
  }

  void shrinking_fill_blank(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, ConstraintIterator targetVectorBegin)
  {
    // fill the left behind spots with nullptr
    ConstraintDistanceType target_end = targetStart + blockSize;
    auto fill_start = sourceStart;
    if (sourceStart < target_end) {
      // has overlap
      fill_start = target_end;
    }
    ConstraintIterator fill_start_iter = targetVectorBegin + fill_start;
    ConstraintIterator src_end_iter = targetVectorBegin + sourceStart + blockSize;
    fill(fill_start_iter, src_end_iter, nullptr);
  }

  void expanding_fill_blank(ConstraintDistanceType sourceStart, ConstraintDistanceType targetStart, ConstraintDistanceType blockSize, ConstraintIterator targetVectorBegin)
  {
    // fill the left behind spots with nullptr
    auto fill_end = sourceStart + blockSize;
    if (targetStart < fill_end) {
      // has overlap
      fill_end = targetStart;
    }
    ConstraintIterator src_start_iter = targetVectorBegin + sourceStart;
    ConstraintIterator fill_end_iter = targetVectorBegin + fill_end;
    fill(src_start_iter, fill_end_iter, nullptr);
  }

  ConstraintTrimmer::ConstraintTrimmer(ConstraintSet& rConstrSet)
    : mConstraintSetSize(), mThroughIterator(), mInsertIterator(), mTrimResult(), mrConstraintVector(rConstrSet.GetConstraintsMutable())
  {
    mThroughIterator = mrConstraintVector.begin();
    mInsertIterator = mThroughIterator;
    mConstraintSetSize = rConstrSet.Size();
  }

  void ConstraintTrimmer::Trim()
  {
    for (; mThroughIterator != mrConstraintVector.end(); ++ mThroughIterator) {
      TrimConstraint();
      switch (mTrimResult.mType) {
      case EConstraintResultType::Remove:
        DELETE_CONSTRAINT((*mThroughIterator));
        (*mThroughIterator) = nullptr;
        break;
      case EConstraintResultType::Replace:
        DELETE_CONSTRAINT((*mThroughIterator));
        (*mThroughIterator) = nullptr;
        (*mInsertIterator) = mTrimResult.GiveConstraint();
        ++ mInsertIterator; // move the insertion iterator to next position.
        break;
      case EConstraintResultType::Consumed:
        if (mThroughIterator != mInsertIterator) {
          // need to move the Constraint object pointer to the location pointed to by mInsertIterator
          auto constr_ptr = (*mThroughIterator);
          (*mThroughIterator) = nullptr;
          (*mInsertIterator) = constr_ptr;
        }
        ++ mInsertIterator; // move the insertion iterator to next position.
        break;
      default:
        LOG(fail) << "{" << TrimmerName() << "} unexpected constraint result type: \"" << EConstraintResultType_to_string(mTrimResult.mType) << endl;
        FAIL("unexpected-trim-result-type");
      }
      mConstraintSetSize -= mTrimResult.mSizeChange;
      mTrimResult.Clear();
    }

    if (mInsertIterator != mrConstraintVector.end()) {
      erase_constraint_vector_tail(mrConstraintVector, mInsertIterator);
    }
  }

  AlignWithSizeTrimmer::AlignWithSizeTrimmer(ConstraintSet& rConstrSet, uint64 alignMask, uint64 alignSize)
    : ConstraintTrimmer(rConstrSet), mAlignMask(alignMask), mAlignSize(alignSize)
  {

  }

  void AlignWithSizeTrimmer::TrimConstraint()
  {
    (*mThroughIterator)->AlignWithSize(mAlignMask, mAlignSize, mTrimResult);
  }

  AlignOffsetWithSizeTrimmer::AlignOffsetWithSizeTrimmer(ConstraintSet& rConstrSet, uint64 alignMask, uint64 alignOffset, uint64 alignSize)
    : AlignWithSizeTrimmer(rConstrSet, alignMask, alignSize), mAlignOffset(alignOffset)
  {

  }

  void AlignOffsetWithSizeTrimmer::TrimConstraint()
  {
    (*mThroughIterator)->AlignOffsetWithSize(mAlignMask, mAlignOffset, mAlignSize, mTrimResult);
  }

  AlignMulDataWithSizeTrimmer::AlignMulDataWithSizeTrimmer(ConstraintSet& rConstrSet, uint64 mulData, uint64 baseRemainder, uint64 alignSize)
    : AlignWithSizeTrimmer(rConstrSet, 0, alignSize), mMulData(mulData), mBaseRemainder(baseRemainder)
  {}

  void AlignMulDataWithSizeTrimmer::TrimConstraint()
  {
    (*mThroughIterator)->AlignMulDataWithSize(mMulData, mBaseRemainder, mAlignSize, mTrimResult);
  }

  IndexMaskOperator::IndexMaskOperator(uint64 index, uint64 mask)
    : mIndex(index), mMask(mask)
  {

  }

  IndexMaskOperator::IndexMaskOperator()
    : mIndex(0), mMask(0)
  {

  }

  IndexMaskOperator::IndexMaskOperator(const IndexMaskOperator& rOther)
    : mIndex(rOther.mIndex), mMask(rOther.mMask)
  {

  }

  IndexMaskOperator::~IndexMaskOperator()
  {

  }

  void IndexMaskOperator::Process(Constraint* pConstr, ConstraintOneResult& rResult) const
  {
    switch (pConstr->Type()) {
    case EConstraintType::Value:
      {
        ValueConstraint* pValueConstr = static_cast<ValueConstraint* >(pConstr);
        if ((pValueConstr->mValue & mMask) == mIndex) {
          // matches with the index/mask
          TakeOutMask(pValueConstr->mValue);
        }
        else {
          rResult.MarkRemove(1);
          return;
        }
      }
      break;
    case EConstraintType::Range:
      pConstr->UnexpectedConstraintType(pConstr->Type(), "IndexMaskOperator::Process");
      break;
    default:
      pConstr->UnexpectedConstraintType(pConstr->Type(), "IndexMaskOperator::Process");
    }
  }

  void IndexMaskOperator::TakeOutMask(uint64& rValue) const
  {
  }

  IndexMaskTrimmer::IndexMaskTrimmer(ConstraintSet& rConstrSet, const IndexMaskOperator* pOperator)
    : ConstraintTrimmer(rConstrSet), mpOperator(pOperator)
  {

  }

  void IndexMaskTrimmer::TrimConstraint()
  {
    mpOperator->Process((*mThroughIterator), mTrimResult);
  }

  string ConstraintSetSerializer::ToSimpleString() const
  {
    stringstream out_str;

    while (not Done()) {
      OutputLine(out_str);
    }

    return out_str.str();
  }

  string ConstraintSetSerializer::ToDebugString() const
  {
    stringstream out_str;

    while (not Done()) {
      OutputLineDebug(out_str);
    }

    return out_str.str();
  }

  /*
  static string to_simple_string_debug(const vector<Constraint* >& constr_vec)
  {
    stringstream out_stream;

    char print_buffer[64];
    bool first_item = true;
    for (auto constr_item : constr_vec) {
      if (first_item) {
        first_item = false;
      } else {
        out_stream << ",";
      }

      if (nullptr == constr_item) {
        out_stream << "[nullptr]";
      }
      else {
        out_stream << "[0x" << hex << (uint64)(constr_item) << "]={";
        constr_item->PrintSimpleString(print_buffer, 64);
        out_stream << print_buffer << "}";
      }
    }

    return out_stream.str();
  }
  */

  void ConstraintSetSerializer::OutputLine(ostream& rOutputStream) const
  {
    uint32 count = 0;
    for (; mConstraintIterator != mEndIterator; ++ mConstraintIterator) {
      if (count > 0) rOutputStream << ",";
      rOutputStream << (*mConstraintIterator)->ToSimpleString();
      ++ count;
      if (count >= mNumberPerLine) {
        rOutputStream << endl; // new line.
        break;
      }
    }
  }

  void ConstraintSetSerializer::OutputLineDebug(ostream& rOutputStream) const
  {
    uint32 count = 0;
    char print_buffer[64];
    for (; mConstraintIterator != mEndIterator; ++ mConstraintIterator) {
      if (count > 0) rOutputStream << ",";
      auto constr_item = (*mConstraintIterator);

      if (nullptr == constr_item) {
        rOutputStream << "[nullptr]";
      }
      else {
        rOutputStream << "[0x" << hex << (uint64)(constr_item) << "]={";
        constr_item->PrintSimpleString(print_buffer, 64);
        rOutputStream << print_buffer << "}";
      }

      ++ count;
      if (count >= mNumberPerLine) {
        rOutputStream << endl; // new line.
        break;
      }
    }
  }

  static inline string constraint_set_data_file_name(const string& rName, uint32 serialNumber)
  {
    return rName + "_" + to_string(serialNumber) + ".ConstraintSet";
  }

  void ConstraintSetSerializer::Serialize(const string& rName, uint32 serialNumber) const
  {
    string file_name = constraint_set_data_file_name(rName, serialNumber);

    ofstream serialize_file;
    serialize_file.open(file_name);
    if (serialize_file.bad()) {
      LOG(fail) << "{ConstraintSetSerializer::Serialize} failed to open \"" << file_name << "\" file to write." << endl;
      FAIL("failed-to-open-file-to-write");
    }

    while (not Done()) {
      OutputLine(serialize_file);
    }
  }

  void ConstraintSetDeserializer::Deserialize(const string& rName, uint32 serialNumber)
  {
    string file_name = constraint_set_data_file_name(rName, serialNumber);
    ifstream serialized_file;
    serialized_file.open(file_name);
    if (serialized_file.bad()) {
      LOG(fail) << "{ConstraintSetDeserializer::Deserialize} failed to open \"" << file_name << "\" file to read." << endl;
      FAIL("failed-to-open-file-to-read");
    }

    string line;
    while (getline(serialized_file, line)) {
      ConstraintSet line_constr(line);
      mrConstraintSet.MergeConstraintSet(line_constr);
    }
  }

}
