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
#include <OperandSolutionMap.h>
#include <OperandSolution.h>

using namespace std;

namespace Force {

    OperandSolutionMap::OperandSolutionMap()
      : mStorage()
    {
    }

    vector<pair<string, OperandSolution> >::iterator OperandSolutionMap::find(const string& k)
    {
        return find_if(mStorage.begin(), mStorage.end(),[k](const pair<string, OperandSolution>& entry)->bool{return k==entry.first;});
    }

    vector<pair<string, OperandSolution> >::iterator OperandSolutionMap::end()
    {
        vector<pair<string, OperandSolution> >::iterator it = mStorage.end();
        return it;
    }

    vector<pair<string, OperandSolution> >::const_iterator OperandSolutionMap::end() const
    {
        vector<pair<string, OperandSolution> >::const_iterator it = mStorage.end();
        return it;
    }

    vector<pair<string, OperandSolution> >::iterator OperandSolutionMap::begin()
    {
        vector<pair<string, OperandSolution> >::iterator it = mStorage.begin();
        return it;
    }

    vector<pair<string, OperandSolution> >::const_iterator OperandSolutionMap::begin() const
    {
        vector<pair<string, OperandSolution> >::const_iterator it = mStorage.begin();
        return it;
    }

    size_t OperandSolutionMap::size() const {

      return mStorage.size();
    }

    void OperandSolutionMap::emplace(const string& k, OperandSolution&& ropr_sol)
    {
        mStorage.emplace(mStorage.end(), make_pair(k, move(ropr_sol)));
    }
}

