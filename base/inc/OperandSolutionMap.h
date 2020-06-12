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
#ifndef Force_OperandSolutionMap_H
#define Force_OperandSolutionMap_H

#include <Defines.h>
#include <vector>
#include <algorithm>
#include <utility>
#include <Log.h>

using namespace std;

namespace Force {

  class OperandSolution;

  /*!
   *\class OperandSolutionMap
   *\brief Wrapper around an stl vector to (string, OperandSolution) pairs that preserves insertion order and emulates two functions from std::map.
   *
   */
  class OperandSolutionMap {
  public:
    OperandSolutionMap();
    COPY_CONSTRUCTOR_DEFAULT(OperandSolutionMap);
    DESTRUCTOR_DEFAULT(OperandSolutionMap);
    ASSIGNMENT_OPERATOR_DEFAULT(OperandSolutionMap);

    vector<std::pair<std::string, OperandSolution> >::iterator find(const std::string& k);

    vector<std::pair<std::string, OperandSolution> >::iterator end();

    vector<pair<std::string, OperandSolution> >::const_iterator end() const;

    vector<pair<std::string, OperandSolution> >::iterator begin();

    vector<pair<std::string, OperandSolution> >::const_iterator begin() const;

    size_t size() const;

    void emplace(const std::string& k, OperandSolution&& ropr_sol);

  private:
    std::vector<std::pair<std::string, OperandSolution> > mStorage;
  };
}

#endif
