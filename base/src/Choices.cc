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
#include "Choices.h"

#include <algorithm>
#include <numeric>  // C++UP accumulate defined in numeric
#include <sstream>

#include "ChoicesFilter.h"
#include "Constraint.h"
#include "GenException.h"
#include "Log.h"
#include "Random.h"
#include "StringUtils.h"

using namespace std;

namespace Force {

  Choice::Choice(const Choice& rOther)
    : Object(rOther), mName(rOther.mName), mValue(rOther.mValue), mWeight(rOther.mWeight)
  {
  }

  Choice::~Choice()
  {
  }

  Object* Choice::Clone() const
  {
    return new Choice(*this);
  }

  uint32 Choice::ApplyFilter(const ChoicesFilter& filter)
  {
    if (not filter.Usable(this)) {
      mWeight = 0;
    }

    return mWeight;
  }

  void Choice::GetAvailableChoices(const ChoicesFilter& filter, vector<const Choice*>& rChoicesList) const
  {
    if ((mWeight > 0) and filter.Usable(this)) {
      rChoicesList.push_back(this);
    }
  }

  const string Choice::ToString() const
  {
    stringstream out_stream;

    out_stream << Type() << ":";
    if (mName.size()) {
      out_stream << " name=\"" << mName << "\"";
    }
    out_stream << " value=0x" << hex << mValue << " weight=" << dec << mWeight;

    return out_stream.str();
  }

  Object* RangeChoice::Clone() const
  {
    return new RangeChoice(*this);
  }

  const string RangeChoice::ToString() const
  {
    stringstream out_stream;
    out_stream << mLower << "-" << mHigh << ":" << mWeight;
    return out_stream.str();
  }

  RangeChoice::RangeChoice(const string& name, const string& range, uint32 weight) : Choice(name, 0, weight), mLower(0), mHigh(0)
  {
    parse_range32(range, mLower, mHigh);
  }

  uint32 RangeChoice::Value() const
  {
    LOG(fail) << "RangeChoice does not support Value() operation" << endl;
    FAIL("Not-supported-operation");
    return 0;
  }

  RangeChoice::RangeChoice(const RangeChoice& rOther) : Choice(rOther), mLower(rOther.mLower),mHigh(rOther.mHigh)
  {

  }

  void RangeChoice::LimitRange(uint32 limitValue)
  {
    if (mLower > limitValue) mLower = limitValue;
    if (mHigh > limitValue) mHigh = limitValue;
  }

  ChoiceTree::ChoiceTree(const ChoiceTree& rOther)
    : Choice(rOther), mChoices()
  {
    transform(rOther.mChoices.cbegin(), rOther.mChoices.cend(), back_inserter(mChoices),
      [](const Choice* pChoice) { return dynamic_cast<Choice*>(pChoice->Clone()); });
  }

  ChoiceTree::~ChoiceTree()
  {
    for (auto choice_item: mChoices) {
      delete choice_item;
    }
  }

  Object* ChoiceTree::Clone() const
  {
    return new ChoiceTree(*this);
  }

  const string ChoiceTree::ToString() const
  {
    stringstream out_stream;

    out_stream << Type() << ":";
    if (mName.size()) {
      out_stream << " name=\"" << mName << "\"";
    }
    out_stream << " weight=" << dec << mWeight << endl;
    for (auto const choice_item : mChoices) {
      out_stream << choice_item->ToString() << endl;
    }

    return out_stream.str();
  }

  void ChoiceTree::AddChoice(Choice* choice)
  {
    mChoices.push_back(choice);
  }

  const Choice* ChoiceTree::Choose() const
  {
    uint32 all_weights = SumChoiceWeights();
    if (all_weights == 0) {
      stringstream err_stream;
      err_stream << "total weight of ChoiceTree: \"" << mName << "\" equals 0.";
      throw ChoicesError(err_stream.str());
    }

    uint32 picked_value = Random::Instance()->Random32(0, all_weights - 1);
    const Choice* chosen_one = Chosen(picked_value);
    if (nullptr == chosen_one) {
      LOG(fail) << "Failed to choose any choice from ChoiceTree \"" << mName << "\"." << endl;
      FAIL("fail-choose-from-tree");
    }

    return chosen_one->Choose();
  }

  Choice* ChoiceTree::ChooseMutable()
  {
    uint32 all_weights = SumChoiceWeights();
    if (all_weights == 0) {
      stringstream err_stream;
      err_stream << "total weight of ChoiceTree: \"" << mName << "\" equals 0.";
      throw ChoicesError(err_stream.str());
    }

    uint32 picked_value = Random::Instance()->Random32(0, all_weights - 1);
    Choice* chosen_one = Chosen(picked_value);
    if (nullptr == chosen_one) {
      LOG(fail) << "Failed to choose any choice from ChoiceTree \"" << mName << "\"." << endl;
      FAIL("fail-choose-from-tree");
    }

    return chosen_one->ChooseMutable();
  }


  const Choice* ChoiceTree::CyclicChoose()
  {
    uint32 all_weights = SumChoiceWeights();
    if (all_weights == 0) {
      stringstream err_stream;
      err_stream << "total weight of cyclic ChoiceTree: \"" << mName << "\" equals 0.";
      throw ChoicesError(err_stream.str());
    }

    uint32 picked_value = Random::Instance()->Random32(0, all_weights - 1);
    Choice* chosen_one = Chosen(picked_value);
    if (nullptr == chosen_one) {
      LOG(fail) << "Failed to choose any cyclic choice from ChoiceTree \"" << mName << "\"." << endl;
      FAIL("fail-choose-from-tree");
    }

    uint32 child_weight = chosen_one->Weight();
    const Choice* ret_choice = chosen_one->CyclicChoose();
    if (chosen_one->Weight() == 0) {
      // child weight updated to 0, check if need to update my weight to 0
      if (all_weights == child_weight) {
        mWeight = 0;
      }
    }
    return ret_choice;
  }

  uint64 ChoiceTree::ChooseValueWithHardConstraint(const ConstraintSet& rHardConstr)
  {
    ConstraintChoicesFilter choices_filter(&rHardConstr);
    ApplyFilter(choices_filter);

    if (HasChoice()) {
      auto chosen_ptr = Choose();
      return chosen_ptr->Value();
    }
    return rHardConstr.ChooseValue();
  }

  uint64 ChoiceTree::ChooseValueWithConstraint(const ConstraintSet& rConstr, bool& rHasChoice, uint64& rFallBackValue)
  {
    // Obtain the fall back value first.
    auto chosen_ptr = Choose();
    rFallBackValue = chosen_ptr->Value();

    rHasChoice = false;
    ConstraintChoicesFilter choices_filter(&rConstr);
    ApplyFilter(choices_filter);
    if (HasChoice()) {
      chosen_ptr = Choose();
      rHasChoice = true;
      return chosen_ptr->Value();
    }
    return 0;
  }

  uint32 ChoiceTree::ApplyFilter(const ChoicesFilter& filter)
  {
    uint32 all_weights = accumulate(mChoices.begin(), mChoices.end(), uint32(0),
      [&filter](cuint32 partialSum, Choice* choice_item) { return (partialSum + choice_item->ApplyFilter(filter)); });

    if (all_weights == 0) {
      mWeight = 0;
    }

    return mWeight;
  }

  const Choice* ChoiceTree::FindChoiceByValue(uint32 value) const
  {
    Choice* choice_item = nullptr;

    auto itr = find_if(mChoices.cbegin(), mChoices.cend(),
      [value](const Choice* pChoiceItem) { return (pChoiceItem->Value() == value); });

    if (itr != mChoices.end()) {
      choice_item = *itr;
    }
    else {
      stringstream err_stream;
      err_stream <<  "failed to find any choice with value " << dec << value << " from ChoiceTree << \"" << mName << "\".";
      throw ChoicesError(err_stream.str());
    }

    return choice_item;
  }

  Choice* ChoiceTree::FindChoiceByValue(uint32 value)
  {
    Choice* choice_item = nullptr;

    auto itr = find_if(mChoices.cbegin(), mChoices.cend(),
      [value](const Choice* pChoiceItem) { return (pChoiceItem->Value() == value); });

    if (itr != mChoices.end()) {
      choice_item = *itr;
    }
    else {
      LOG(fail) << "{ChoiceTree::FindChoiceByValue} failed to find any choice with value " << dec << value << " from ChoiceTree << \"" << mName << "\"." << endl;
      FAIL("failed-find-choice-by-value");
    }

    return choice_item;
  }

  const Choice* ChoiceTree::FindChoiceByName(const string& rChoiceName) const
  {
    Choice* choice_item = nullptr;

    auto itr = find_if(mChoices.cbegin(), mChoices.cend(),
      [&rChoiceName](const Choice* pChoiceItem) { return (pChoiceItem->Name() == rChoiceName); });

    if (itr != mChoices.end()) {
      choice_item = *itr;
    }
    else {
      stringstream err_stream;
      err_stream <<  "failed to find any choice with name \"" << rChoiceName << "\" from ChoiceTree \"" << mName << "\".";
      throw ChoicesError(err_stream.str());
    }

    return choice_item;
  }

  void ChoiceTree::RestoreWeight(const Choice* pRefChoice)
  {
    const ChoiceTree* ref_cast = dynamic_cast<const ChoiceTree* > (pRefChoice);
    if (nullptr == ref_cast) {
      LOG(fail) << "{ChoiceTree::RestoreWeight} expecting incoming object to be of \"ChoiceTree\" type." << endl;
      FAIL("mismatching-object-type");
    }

    if (mChoices.size() != ref_cast->mChoices.size()) {
      LOG(fail) << "{ChoiceTree::RestoreWeight} choices vector size mismatch: " << dec << mChoices.size() << " and " << ref_cast->mChoices.size() << endl;
      FAIL("mismatching-choices-vector-size");
    }

    mWeight = pRefChoice->Weight();

    auto my_choice_iter = mChoices.begin();
    auto ref_choice_iter = ref_cast->mChoices.begin();

    for (; my_choice_iter != mChoices.end(); ) {
      (*my_choice_iter)->RestoreWeight(*ref_choice_iter);
      ++ my_choice_iter;
      ++ ref_choice_iter;
    }
  }

  bool ChoiceTree::HasChoice() const
  {
    bool has_choice = any_of(mChoices.cbegin(), mChoices.cend(),
      [](const Choice* pChoiceItem) { return pChoiceItem->HasChoice(); });

    return has_choice;
  }

  bool ChoiceTree::OnlyChoice() const
  {
    uint32 available_choices = 0;
    for (auto choice_item : mChoices) {
      available_choices += choice_item->AvailableChoices();
      if (available_choices > 1) return false;
    }

    return (available_choices == 1);
  }

  uint32 ChoiceTree::AvailableChoices() const
  {
    return accumulate(mChoices.cbegin(), mChoices.cend(), uint32(0),
      [](cuint32 partialSum, const Choice* choice_item) { return (partialSum + choice_item->AvailableChoices()); });
  }

  void ChoiceTree::GetAvailableChoices(vector<const Choice*>& rChoicesList) const
  {
    for (auto choice_item : mChoices) {
      choice_item->GetAvailableChoices(rChoicesList);
    }
  }

  void ChoiceTree::GetAvailableChoices(const ChoicesFilter& filter, vector<const Choice*>& rChoicesList) const
  {
    for (auto choice_item : mChoices) {
      choice_item->GetAvailableChoices(filter, rChoicesList);
    }
  }

  uint32 ChoiceTree::SumChoiceWeights() const
  {
    return accumulate(mChoices.cbegin(), mChoices.cend(), uint32(0),
      [](cuint32 partialSum, const Choice* choice_item) { return (partialSum + choice_item->Weight()); });
  }

  CyclicChoiceTree::CyclicChoiceTree(ChoiceTree* pBaseChoiceTree)
    : ChoiceTree(pBaseChoiceTree->Name(), pBaseChoiceTree->Value(), pBaseChoiceTree->Weight()), mpBaseChoiceTree(pBaseChoiceTree)
  {
    auto base_choices = pBaseChoiceTree->GetChoices();
    if (base_choices.size()) {
      for (auto choice_item : base_choices) {
        mChoices.push_back(dynamic_cast<Choice* >(choice_item->Clone()));
      }
    }
  }

  CyclicChoiceTree::CyclicChoiceTree(const CyclicChoiceTree& rOther)
    : ChoiceTree(rOther), mpBaseChoiceTree(nullptr)
  {
    if (nullptr != rOther.mpBaseChoiceTree) {
      mpBaseChoiceTree = dynamic_cast<ChoiceTree* > (rOther.mpBaseChoiceTree->Clone());
    }
  }

  CyclicChoiceTree::~CyclicChoiceTree()
  {
    delete mpBaseChoiceTree;
  }

  Object* CyclicChoiceTree::Clone() const
  {
    return new CyclicChoiceTree(*this);
  }

  const Choice* CyclicChoiceTree::CyclicChoose()
  {
    const Choice* my_choice = ChoiceTree::CyclicChoose();
    // << "picked " << my_choice->Name() << " value " << dec << my_choice->Value() << endl;
    if (mWeight == 0) {
      // Restore weights since all items has been picked.
      // << "restore weight called " << endl;
      RestoreWeight(mpBaseChoiceTree);
    }
    return my_choice;
  }

  ChoicesSet::~ChoicesSet()
  {
    for (auto & map_iter : mChoiceTrees) {
      delete map_iter.second;
    }
  }

  const ChoiceTree* ChoicesSet::FindChoiceTree(const string& treeName) const
  {
    auto find_iter = mChoiceTrees.find(treeName);
    if (find_iter == mChoiceTrees.end()) {
      stringstream err_stream;
      err_stream << "cannot find ChoiceTree \"" << treeName << "\" in \"" << Name() << "\" set.";
      throw ChoicesError(err_stream.str());
    }

    return find_iter->second;
  }

  const ChoiceTree* ChoicesSet::TryFindChoiceTree(const string& treeName) const
  {
    auto find_iter = mChoiceTrees.find(treeName);
    if (find_iter == mChoiceTrees.end()) {
      return nullptr;
    }

    return find_iter->second;
  }

  void ChoicesSet::AddChoiceTree(ChoiceTree* choiceTree)
  {
    auto find_iter = mChoiceTrees.find(choiceTree->Name());
    if (find_iter != mChoiceTrees.end()) {
      delete choiceTree;
      LOG(fail) << "Duplicated ChoiceTree \"" << choiceTree->Name() << "\" being added to \"" << Name() << "\" set." << endl;
      FAIL("duplicated-choices-tree");
    }
    if (choiceTree->Weight() == 0) {
      choiceTree->SetWeight(10);
    }
    mChoiceTrees[choiceTree->Name()] = choiceTree;
  }

  const string ChoicesSet::ToString() const
  {
    stringstream out_stream;

    out_stream << "ChoicesSet: " << Name() << endl;
    for (auto & map_item : mChoiceTrees) {
      out_stream << map_item.second->ToString();
    }

    return out_stream.str();
  }

}
