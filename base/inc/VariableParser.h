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
#ifndef Force_VariableParser_H
#define Force_VariableParser_H

#include <vector>

namespace Force {

  class VariableSet;
  class ArchInfo;

  /*!
    \class VariableParser
    \brief Parser class for variable files.

    VariableParser handles parsing of all variable files.
  */
  class VariableParser {
  public:
    /*!
      Constructor, pass in pointer to VariableSet object.
    */
    explicit VariableParser(std::vector<VariableSet* >& setsVector)
      : mVariableSets(setsVector)
    {
    }

    ~VariableParser() //!< Destructor.
    {
    }

    void Setup(const ArchInfo& archInfo);
  protected:
    std::vector<VariableSet *>& mVariableSets;

    friend class VariableFileParser;
  };

}

#endif
