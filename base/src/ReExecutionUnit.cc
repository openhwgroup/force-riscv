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
#include <ReExecutionUnit.h>
#include <Constraint.h>
#include <Log.h>

using namespace std;

/*!
  \file ReExecutionUnit.cc
  \brief Code managing various re-execution units, such as loop, subroutine, linear block.
*/


namespace Force {

  ReExecutionUnit::ReExecutionUnit()
    : mId(0), mBeginAddress(0), mEndAddress(0), mpVaRanges(nullptr), mpPaRanges()
  {
    mpVaRanges = new ConstraintSet();
    mpPaRanges = new ConstraintSet();
  }

  ReExecutionUnit::~ReExecutionUnit()
  {
    delete mpVaRanges;
    delete mpPaRanges;
  }

}
