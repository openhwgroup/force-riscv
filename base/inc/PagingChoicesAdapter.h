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
#ifndef Force_PagingChoicesAdapter_H
#define Force_PagingChoicesAdapter_H

#include <string>

#include "Defines.h"

namespace Force {

  class ChoicesModerator;
  class ChoiceTree;
  class VariableModerator;
  class ConstraintSet;

  /*!
    \class PagingChoicesAdapter
    \brief An adapter for handling requests for paging choices.
  */
  class PagingChoicesAdapter {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(PagingChoicesAdapter);
    PagingChoicesAdapter(const std::string& rRegimeStr, uint32 stage); //!< Constructor.
    virtual ~PagingChoicesAdapter() { } //!< Destructor.

    void Setup(const ChoicesModerator* pChoicesMod, const VariableModerator* pVar); //!< Setup paging choices moderator.
    uint64 GetPlainPagingChoice(const std::string& rChoiceName) const; //!< Return a randomly picked value from the specified paging choice, without applying suffix to the choice name.
    uint64 GetPagingChoice(const std::string& rChoiceName) const; //!< Return a randomly picked value from the specified paging choice.
    ChoiceTree* GetPagingChoiceTree(const std::string& rChoiceName) const; //!< Return a ChoiceTree object based on the name provided.
    ChoiceTree* GetPagingChoiceTreeWithGranule(const std::string& rChoiceName, const std::string& rSuffixGranule) const; //!< Return a ChoiceTree object based on the name provided.
    inline ChoiceTree* GetPageSizeChoiceTree(const std::string& rSuffixGranule) const //!< Return a cloned ChoiceTree object pointing to page size choices.
    {
      return GetPagingChoiceTreeWithGranule("Page size", rSuffixGranule);
    }
    inline ChoiceTree* GetSystemPageSizeChoiceTree(const std::string& rSuffixGranule) const //!< Return a cloned ChoiceTree object pointing to page size choices.
    {
      return GetPagingChoiceTreeWithGranule("System Page size", rSuffixGranule);
    }
    ChoiceTree* GetPagingChoiceTreeWithLevel(const std::string& rChoiceName, uint32 level) const; //!< Return a ChoiceTree object based on the name and page/table level provided.
    uint64 GetVariableValue(const std::string& rVarName) const; //!< Get variable value.
    inline const std::string PagingChoicesName(const std::string& rBaseName) const //!< Return full choices name without granule differentiation.
    {
      return rBaseName + mPagingChoicesSuffix;
    }
  protected:
    PagingChoicesAdapter(); //!< Constructor.
    PagingChoicesAdapter(const PagingChoicesAdapter& rOther); //!< Copy constructor.
    inline void UpdatePagingChoicesFullName(std::string& rBaseName) const //!< Return full choices name without granule differentiation.
    {
      rBaseName += mPagingChoicesSuffix;
    }
  protected:
    std::string mPagingChoicesSuffix;        //!< Paging choices suffix.
    const ChoicesModerator* mpPagingChoices; //!< Const pointer to paging choices moderator.
    const VariableModerator* mpVariables; //!< Const pointer to value type variables.
  };
}

#endif
