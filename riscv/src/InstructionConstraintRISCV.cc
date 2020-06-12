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
#include <InstructionConstraintRISCV.h>
#include <OperandRISCV.h>
#include <OperandConstraintRISCV.h>
#include <Generator.h>
#include <InstructionStructure.h>
#include <GenException.h>
#include <Log.h>

using namespace std;

/*!
  \file InstructionConstraintRISCV.cc
  \brief Code supporting RISCV specific instruction constraint
*/

namespace Force {

  VectorInstructionConstraint::~VectorInstructionConstraint()
  {
    delete mpDataTraits;
  }

  const VectorDataTraits* VectorInstructionConstraint::DataTraits() const
  {
    if (nullptr == mpDataTraits) {
      mpDataTraits = new VectorDataTraits();
      if (nullptr == mpDataTypeOperand) {
        LOG(fail) << "{VectorInstructionConstraint::DataTraits} pointer to data type operand not set." << endl;
        FAIL("vector-data-type-operand-not-set");
      }

      mpDataTypeOperand->SetDataTraits(*mpDataTraits);
    }

    return mpDataTraits;
  }

}
