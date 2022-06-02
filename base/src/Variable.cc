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
#include "Variable.h"

#include <sstream>

#include "Choices.h"
#include "GenException.h"
#include "Log.h"
#include "StringUtils.h"

using namespace std;

/*!
  \file Variable.cc
  \brief Code for variable
*/

namespace Force {

  Object* Variable::Clone() const
  {
    return new Variable(*this);
  }

  const std::string Variable::ToString() const
  {
    return Name() + " = " + mStrValue;
  }

  void Variable::SetUp(const std::string& value)
  {
    mStrValue = value;
    Sender::SendNotification(ENotificationType::VariableUpdate);
  }

  uint64 Variable::Value() const
  {
    LOG(fail) << "{ Variable::Value} The API not intended to be used " << endl;
    FAIL("unsupported-API");
    return 0;
  }

  Variable::Variable(const Variable& rOther) : Sender(rOther), Object(rOther), mName(rOther.mName), mStrValue(rOther.mStrValue)
  {

  }

  Object* ValueVariable::Clone() const
  {
    return new ValueVariable(*this);
  }

  ValueVariable::ValueVariable(const ValueVariable& rOther) : Variable(rOther), mIntValue(rOther.mIntValue)
  {

  }

  void ValueVariable::SetUp(const std::string& value)
  {
    mIntValue = parse_uint64(value);
    Variable::SetUp(value);
  }

  ChoiceVariable::~ChoiceVariable()
  {
    delete mpChoiceTree;
  }

  Object* ChoiceVariable::Clone() const
  {
    return new ChoiceVariable(*this);
  }

  void ChoiceVariable::SetUp(const std::string& value)
  {
    if (mpChoiceTree != nullptr) {
      LOG(fail) << "Dangling choice tree pointer" << endl;
      FAIL("dangling-choice-tree-pointer");
    }
    mpChoiceTree = new ChoiceTree("Variable Choice Tree", 0, 10);

    vector<string> range_weights;
    StringSplitter splitter(value, ',');
    while (not splitter.EndOfString())
      range_weights.push_back(splitter.NextSubString());

    for (auto const& range_weight : range_weights) {
      string::size_type colon_pos = range_weight.find(':');
      if (colon_pos == string::npos) {
        LOG(fail) << "Invalid choice variable: \"" << value << "\""<< endl;
        FAIL("invalid-choice-variable");
      }

      unsigned i = 0;
      stringstream choice_stream;
      choice_stream << "variable choice " << i ++;
      string range = range_weight.substr(0, colon_pos);
      string weight = range_weight.substr(colon_pos + 1);
      auto range_choice = new RangeChoice(choice_stream.str(), range, parse_uint32(weight));
      range_choice->LimitRange(mRangeLimit);
      mpChoiceTree->AddChoice(range_choice);
    }

    Variable::SetUp(value);
  }

  void ChoiceVariable::CleanUp()
  {
    Variable::CleanUp();
    delete mpChoiceTree;
    mpChoiceTree = nullptr;
  }

  void ChoiceVariable::LimitRange(uint32 limit) const
  {
    mRangeLimit = limit;
    auto tree_choices = mpChoiceTree->GetChoicesMutable();
    for (auto choice_item : tree_choices) {
      RangeChoice * range_choice = dynamic_cast<RangeChoice* >(choice_item);
      if (nullptr != range_choice) {
        range_choice->LimitRange(limit);
      }
    }
  }

  ChoiceVariable::ChoiceVariable(const ChoiceVariable& rOther): Variable(rOther), mpChoiceTree(nullptr), mRangeLimit(rOther.mRangeLimit)
  {
    mpChoiceTree = dynamic_cast<ChoiceTree* >(rOther.mpChoiceTree->Clone());
  }

  void VariableSet::AddVariable(const std::string& name, const std::string& value)
  {
    for (auto& var : mVariables) {
      if (var.first == name) {
        LOG(fail) << "duplicate variable name: \"" << name << "\"" << endl;
        FAIL("duplicate-variable-name");
      }
    }

    mVariables[name] = InstantiateVariable();
    mVariables[name]->SetName(name);
    mVariables[name]->SetUp(value);
  }

  const Variable* VariableSet::FindVariable(const std::string& rName) const
  {
    auto find_iter = mVariables.find(rName);
    if (find_iter != mVariables.end()) {
      return find_iter->second;
    }

    return nullptr;
  }

  Variable* VariableSet::FindVariable(const std::string& rName)
  {
    auto find_iter = mVariables.find(rName);
    if (find_iter != mVariables.end()) {
      return find_iter->second;
    }

    return nullptr;
  }

  const Variable* VariableSet::GetVariable(const std::string& rName) const
  {
    auto var_ptr = FindVariable(rName);
    if (nullptr == var_ptr) {
      LOG(fail) << "{VariableSet::GetVariable} variable not found: \"" << rName << "\"." << endl;
      FAIL("varible-not-found");
    }
    return var_ptr;
  }

  void VariableSet::ModifyVariable(const std::string& name, const std::string& value)
  {
    auto var = FindVariable(name);
    if (nullptr == var) {
      LOG(fail) << "Failed to modify variable: \"" << name << "\"" << endl;
      FAIL("failed-to-modify-variable");
    }
    var->CleanUp();
    var->SetUp(value);
  }

  Variable* VariableSet::InstantiateVariable()
  {
    switch (mType) {
    case EVariableType::String:
      return (new Variable());
    case EVariableType::Value:
      return (new ValueVariable());
    case  EVariableType::Choice:
      return (new ChoiceVariable());
    default:
      LOG(fail) << "Unknown variable type";
      FAIL("unknown-variable-type");
    }
    return nullptr;
  }

  Object* VariableModerator::Clone() const
  {
    return new VariableModerator(*this);
  }

  const std::string VariableModerator::ToString() const
  {
    stringstream out_stream;
    out_stream << "VariableModerator for the type : " << EVariableType_to_string(mpVariableSet->VariableType());
    return out_stream.str();
  }

  VariableModerator::VariableModerator(const VariableModerator& rOther) : Object(rOther), mpVariableSet(rOther.mpVariableSet)
  {

  }

 }
