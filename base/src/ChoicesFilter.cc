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
#include <ChoicesFilter.h>
#include <Constraint.h>
#include <Choices.h>
#include <Operand.h>
#include <Log.h>

using namespace std;

namespace Force {

  bool ConstraintChoicesFilter::Usable(const Choice* choice) const
  {
    return mpConstraint->ContainsValue(choice->Value());
  }

  bool MultiRegisterChoicesFilter::Usable(const Choice* choice) const
  {
    ConstraintSet my_indices;
    mpOperand->GetRegisterIndices(choice->Value(), my_indices);
    // << "register index : " << dec << choice->Value() << " indices " << my_indices.ToSimpleString() << endl;
    return mpConstraint->ContainsConstraintSet(my_indices);
  }

}
