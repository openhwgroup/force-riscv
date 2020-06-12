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
#include <ChoicesModerator.h>
#include <Choices.h>
#include <Log.h>
#include <GenException.h>

#include <sstream>

using namespace std;

namespace Force {

  ChoicesModerator::ChoicesModerator(const ChoicesSet* choicesSet)
    : Object(), Sender(), mpChoicesSet(choicesSet), mCurrentModificationSet(), mNewModificationSets(), mModificationSetStack()
  {

  }

  ChoicesModerator::ChoicesModerator(const ChoicesModerator& rOther)
    : Object(rOther), Sender(rOther), mpChoicesSet(rOther.mpChoicesSet), mCurrentModificationSet(), mNewModificationSets(), mModificationSetStack()
  {

  }

  ChoicesModerator::~ChoicesModerator()
  {
    mpChoicesSet = nullptr;

    for (auto mod_set : mNewModificationSets)
      delete mod_set;

    for (auto mod_set : mModificationSetStack)
      delete mod_set;
  }

  Object* ChoicesModerator::Clone() const
  {
    return new ChoicesModerator(*this);
  }

  const string ChoicesModerator::ToString() const
  {
    stringstream out_stream;

    out_stream << "ChoicesModerator: ";

    if (nullptr != mpChoicesSet) {
      out_stream << mpChoicesSet->ToString();
    }

    return out_stream.str();
  }

  ChoiceTree* ChoicesModerator::CloneChoiceTree(const std::string& treeName) const
  {
    auto choices_tree_const = mpChoicesSet->FindChoiceTree(treeName);
    auto return_tree = dynamic_cast<ChoiceTree* >(choices_tree_const->Clone());
    mCurrentModificationSet.ApplyModifications(return_tree);
    return return_tree;
  }

  ChoiceTree* ChoicesModerator::TryCloneChoiceTree(const std::string& treeName) const
  {
    auto choices_tree_const = mpChoicesSet->TryFindChoiceTree(treeName);

    if (choices_tree_const == nullptr) { return nullptr; }

    auto return_tree = dynamic_cast<ChoiceTree* >(choices_tree_const->Clone());
    mCurrentModificationSet.ApplyModifications(return_tree);
    return return_tree;
  }

  void ChoicesModerator::AddChoicesModification(const std::string& treeName, const std::map<std::string, uint32>& modifications, uint32 setId)
  {
    ValidateModifications(treeName, modifications);

    ChoiceModificationSet *mod_set = new ChoiceModificationSet(setId);
    mod_set->AddChoicesModification(treeName, modifications);
    mNewModificationSets.push_back(mod_set);
  }

  uint32 ChoicesModerator::DoChoicesModification(const std::string& treeName,  const std::map<std::string, uint32>& modifications)
  {
    ValidateModifications(treeName, modifications);

    auto choice_tree = mpChoicesSet->FindChoiceTree(treeName);
    const std::vector<Choice* >& choices = choice_tree->GetChoices();
    for (auto choice : choices) {
      auto it = modifications.find(choice->Name());
      if (it != modifications.end())
        choice->SetWeight(it->second);
    }

    return 0;
  }

  void ChoicesModerator::CommitModificationSet(uint32 setId)
  {
    bool found = false;

    auto it = mNewModificationSets.begin();
    while (it != mNewModificationSets.end())
    {
      if ((*it)->Id() == setId) {
        found = true;
        break;
      }
      it ++;
    }
    if (!found) {
      LOG(fail) << "{ChoicesModerator::CommitModificationSet} Can't find modification set with ID " << setId << endl;
      FAIL("Can't find modification set ID");
    }

    mModificationSetStack.push_front(*it);
    mCurrentModificationSet.Merge(**it);
    mNewModificationSets.erase(it);

    Sender::SendNotification(ENotificationType::ChoiceUpdate);
  }

  void ChoicesModerator::RevertModificationSet(uint32 setId)
  {
    bool found = false;

    auto it = mModificationSetStack.begin();
    while (it != mModificationSetStack.end())
    {
      if ((*it)->Id() == setId) {
        found = true;
        break;
      }
      it ++;
    }
    if (!found) {
      LOG(fail) << "{ChoicesModerator::RevertModificationSet} Can't find modification set with ID " << setId << endl;
      FAIL("Can't find modification set ID");
    }
    delete *it;
    mModificationSetStack.erase(it);
    mCurrentModificationSet.Clear();
    for (auto rit = mModificationSetStack.rbegin(); rit != mModificationSetStack.rend(); rit ++)
      mCurrentModificationSet.Merge(**rit);

    Sender::SendNotification(ENotificationType::ChoiceUpdate);
  }

  void ChoicesModerator::ValidateModifications(const string& rTreeName, const map<string, uint32>& rModifications)
  {
    try {
      const ChoiceTree* choice_tree = mpChoicesSet->FindChoiceTree(rTreeName);

      for (const auto& modification : rModifications) {
        choice_tree->FindChoiceByName(modification.first);
      }
    }
    catch(const ChoicesError& error) {
      LOG(fail) << "Invalid choices modifications: " << error.what() << endl;
      FAIL("Invalid choices modifications");
    }
  }

  void ChoiceModificationSet::Clear()
  {
    mModificationSet.clear();
  }

  void ChoiceModificationSet::ApplyModifications(ChoiceTree* choiceTree) const
  {
    const std::string& treeName = choiceTree->Name();

    auto it = mModificationSet.find(treeName);
    if (it == mModificationSet.end()) {
      //LOG(info) << "{ChoiceModificationSet::ApplyModifications} No modification for choice tree name " << treeName << " in the modification set" << endl;
      return;
    }
    it->second.ApplyModifications(choiceTree);
  }

  void ChoiceModificationSet::Merge(const ChoiceModificationSet& rOther)
  {
    for (auto& modificationSet : rOther.GetModificationSet()) {
      auto it = mModificationSet.find(modificationSet.first);
      if (it == mModificationSet.end()) {
        mModificationSet[modificationSet.first] = modificationSet.second;
        //LOG(notice) << "{ChoiceModificationSet::Merge} merge choice name " << modificationSet.first << endl;
      }
      else
        it->second.Merge(modificationSet.second);
    }
    mId = rOther.Id();
  }

  uint32 ChoiceModificationSet::AddChoicesModification(const std::string& choiceTreeName, const std::map<std::string, uint32>& modifications)
  {
    ChoiceTreeModifications choiceTreeModifications(modifications);
    auto it = mModificationSet.find(choiceTreeName);
    if (it == mModificationSet.end()) {
      mModificationSet[choiceTreeName] = choiceTreeModifications;
    }
    else {
      it->second.Merge(choiceTreeModifications);
    }

    return mId;
  }

  const std::string ChoiceModificationSet::ToString() const
  {
    stringstream out_stream;

    out_stream << "ChoicesModificationSet: ";

    for (auto& modification_item : mModificationSet)
      out_stream << modification_item.first << " ";

    return out_stream.str();

  }

  void ChoiceTreeModifications::ApplyModifications(ChoiceTree * choiceTree) const
  {
    for (auto& modification : mModifications )
      ApplyOneModification(choiceTree, modification.first, modification.second);

  }

  void ChoiceTreeModifications::ApplyOneModification(ChoiceTree* choiceTree, const std::string& name, uint32 weight) const
  {
    bool applied = false;
    const std::vector<Choice* >& choices = choiceTree->GetChoices();

    for (auto choice : choices)
      if (choice->Name() == name) {
        choice->SetWeight(weight);
        applied = true;
        LOG(debug) << "choice tree name: " << choiceTree->Name() << " choice name: " << name << " set weight: " << weight << endl;
      }
    if (!applied)
     LOG(info) << "Ignore the unkown the choice " << name << endl;

  }

  void ChoiceTreeModifications::Merge(const ChoiceTreeModifications& rOther)
  {
    for (auto& modification : rOther.GetModifications()) {
      mModifications[modification.first] = modification.second;
      //LOG(notice) << "{ChoicesModerator::Merge} choice item =" << modification.first << ", weight =" << hex << modification.second << endl;
    }
  }

  ChoicesModerators::ChoicesModerators(const ChoicesModerators& rOther)
    : mChoicesModerators(), mId(rOther.mId)
  {
    transform(rOther.mChoicesModerators.cbegin(), rOther.mChoicesModerators.cend(), back_inserter(mChoicesModerators),
      [](const ChoicesModerator* pChoicesModerator) { return dynamic_cast<ChoicesModerator*>(pChoicesModerator->Clone()); });
  }

  ChoicesModerators::~ChoicesModerators()
  {
    for (auto choices_moderator : mChoicesModerators) {
      delete choices_moderator;
    }
  }

  void ChoicesModerators::Setup(const std::vector<ChoicesSet*>& mChoicesSets)
  {
    for (auto choices_set : mChoicesSets) {
      auto choices_moderator = new ChoicesModerator(choices_set);
      mChoicesModerators.push_back(choices_moderator);
    }
  }

  uint32 ChoicesModerators::AddChoicesModification(EChoicesType choicesType, const std::string& treeName, const std::map<std::string, uint32>& modifications)
  {
    auto choices_moderator = GetChoicesModerator(choicesType);
    auto set_id = GetId();
    choices_moderator->AddChoicesModification(treeName, modifications, set_id);
    return set_id;
  }

  void ChoicesModerators::CommitModificationSet(EChoicesType choicesType, uint32 set_id)
  {
    auto choices_moderator = GetChoicesModerator(choicesType);
    choices_moderator->CommitModificationSet(set_id);
  }

  void ChoicesModerators::RevertModificationSet(EChoicesType choicesType, uint32 set_id)
  {
    GetChoicesModerator(choicesType)->RevertModificationSet(set_id);
  }

  Object* ChoicesModerators::Clone() const
  {
    return new ChoicesModerators(*this);
  }

  const std::string ChoicesModerators::ToString() const
  {
    stringstream out_stream;
    for (auto choices_moderator : mChoicesModerators) {
      out_stream << choices_moderator->ToString() << endl;
    }

    return out_stream.str();
  }

}
