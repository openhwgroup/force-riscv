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
#include "UtilityAlgorithms.h"

#include "Constraint.h"
#include "Log.h"

using namespace std;

namespace Force {

  RandomSampler::RandomSampler(uint64 min, uint64 max)
    : mpConstraint(nullptr)
  {
    mpConstraint = new ConstraintSet(min, max);

  }

  RandomSampler::RandomSampler(const RandomSampler& rOther)
    : mpConstraint(nullptr)
  {
    LOG(fail) << "{RandomSampler::RandomSampler} copy constructor not expected to be called." << endl;
    FAIL("copy-constructor-not-expected-to-be-called");
  }

  RandomSampler::~RandomSampler()
  {
    delete mpConstraint;
  }

  uint64 RandomSampler::Sample()
  {
    if (mpConstraint->IsEmpty()) {
      return 0;
    }

    uint64 val = mpConstraint->ChooseValue();
    mpConstraint->SubValue(val);
    return val;
  }

}
