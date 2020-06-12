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
#ifndef Force_ChoicesParser_H
#define Force_ChoicesParser_H

#include <vector>

namespace Force {

  class ChoicesSet;
  class ArchInfo;

  /*!
    \class ChoicesParser
    \brief Parser class for choices files.

    ChoicesParser handles parsing of all choices files.
  */
  class ChoicesParser {
  public:
    /*!
      Constructor, pass in pointer to ChoicesSet object.
    */
    explicit ChoicesParser(std::vector<ChoicesSet* >& setsVector)
      : mChoicesSets(setsVector)
    {
    }

    ~ChoicesParser() //!< Destructor.
    {
    }

    void Setup(const ArchInfo& archInfo);
  protected:
    std::vector<ChoicesSet *>& mChoicesSets;

    friend class ChoicesFileParser;
  };

}

#endif
