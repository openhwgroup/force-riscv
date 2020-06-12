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
#include <PagingChoicesAdapter.h>
#include <Choices.h>
#include <ChoicesModerator.h>
#include <Variable.h>
#include <Constraint.h>
#include <ChoicesFilter.h>
#include <Log.h>

#include <memory>

using namespace std;

/*!
  \file PagingChoicesAdapter.cc
  \brief Code managing paging choices via the adapter class.
*/

namespace Force {

  PagingChoicesAdapter::PagingChoicesAdapter(const string& rRegimeStr, uint32 stage)
    : mPagingChoicesSuffix(), mpPagingChoices(nullptr), mpVariables(nullptr)
  {
    mPagingChoicesSuffix += "#";
    mPagingChoicesSuffix += rRegimeStr;
    mPagingChoicesSuffix += "#stage ";
    char print_buffer[32];
    snprintf(print_buffer, 32, "%d", stage);
    mPagingChoicesSuffix += print_buffer;
  }

  PagingChoicesAdapter::PagingChoicesAdapter()
    : mPagingChoicesSuffix(), mpPagingChoices(nullptr), mpVariables(nullptr)
  {

  }

  PagingChoicesAdapter::PagingChoicesAdapter(const PagingChoicesAdapter& rOther)
    : mPagingChoicesSuffix(rOther.mPagingChoicesSuffix), mpPagingChoices(nullptr), mpVariables(nullptr)
  {
  }

  void PagingChoicesAdapter::Setup(const ChoicesModerator* pChoicesMod, const VariableModerator* pVar)
  {
    mpPagingChoices = pChoicesMod;
    mpVariables = pVar;
  }

  uint64 PagingChoicesAdapter::GetPlainPagingChoice(const string& rChoiceName) const
  {
    auto choices_raw = mpPagingChoices->CloneChoiceTree(rChoiceName);
    std::unique_ptr<Choice> choices_tree(choices_raw);
    auto chosen_ptr = choices_tree->Choose();
    return chosen_ptr->Value();
  }

  uint64 PagingChoicesAdapter::GetPagingChoice(const string& rChoiceName) const
  {
    string full_name = PagingChoicesName(rChoiceName);
    auto choices_raw = mpPagingChoices->CloneChoiceTree(full_name);
    std::unique_ptr<Choice> choices_tree(choices_raw);
    auto chosen_ptr = choices_tree->Choose();
    return chosen_ptr->Value();
  }

  ChoiceTree* PagingChoicesAdapter::GetPagingChoiceTree(const string& rChoiceName) const
  {
    string full_name = PagingChoicesName(rChoiceName);
    return mpPagingChoices->CloneChoiceTree(full_name);
  }

  ChoiceTree* PagingChoicesAdapter::GetPagingChoiceTreeWithLevel(const string& rChoiceName, uint32 level) const
  {
    string name = rChoiceName + "#level " + to_string(level);
    UpdatePagingChoicesFullName(name);
    return mpPagingChoices->CloneChoiceTree(name);
  }

  uint64 PagingChoicesAdapter::GetVariableValue(const std::string& rVarName) const
  {
    auto var_ptr = dynamic_cast<const ValueVariable*>(mpVariables->GetVariableSet()->GetVariable(rVarName));
    return var_ptr->Value();
  }

  ChoiceTree* PagingChoicesAdapter::GetPagingChoiceTreeWithGranule(const std::string& rChoiceName, const std::string& rSuffixGranule) const
  {
    string name = rChoiceName + "#" + rSuffixGranule;
    UpdatePagingChoicesFullName(name);
    return mpPagingChoices->CloneChoiceTree(name);
  }

}
