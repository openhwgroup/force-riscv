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
#include "AluImmediateConstraint.h"

#include "Constraint.h"
#include "Log.h"
#include "UtilityFunctions.h"

/*!
  \file AluImmediateConstraint.cc
  \brief Code handling ALU immediate constraint building.
*/

using namespace std;

namespace Force {

  AluImmediateConstraint::AluImmediateConstraint(const EAluOperationType operationType, cuint32 offsetSize, cuint32 offsetShift)
    : mOperationType(operationType),  mMaxOffset(0)
  {
    mMaxOffset = get_mask64(offsetSize, offsetShift);
  }

  void AluImmediateConstraint::GetConstraint(uint64 baseValue, uint32 accessSize, ConstraintSet* pResultConstr) const
  {
    uint64 range_lower = 0;
    uint64 range_upper = 0;
    switch (mOperationType) {
    case EAluOperationType::ADD:
      range_lower = baseValue;
      range_upper = baseValue + mMaxOffset + (accessSize - 1);
      break;
    case EAluOperationType::SUB:
      range_lower = baseValue - mMaxOffset;
      range_upper = baseValue + (accessSize - 1);
      break;
    default:
      LOG(fail) << "{AluImmediateConstraint::GetConstraint} unhandled ALU operation type: " << EAluOperationType_to_string(mOperationType) << endl;
      FAIL("unhandled-alu-operation-type");
    }

    if (range_lower <= range_upper) {
      pResultConstr->AddRange(range_lower, range_upper);
    } else {
      // Overflowed.
      pResultConstr->AddRange(range_lower, MAX_UINT64);
      pResultConstr->AddRange(0, range_upper);
    }
  }

}
