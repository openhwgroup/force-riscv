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
#include "PaGenerator.h"

#include <memory>

#include "Constraint.h"
#include "Log.h"
#include "UtilityFunctions.h"

using namespace std;

/*!
  \file PaGenerator
  \brief Code for physical address generator.
*/
namespace Force {

  PaGenerator::~PaGenerator()
  {
    mpBaseConstraint = nullptr; // set pointer to nullptr.
  }

  uint64 PaGenerator::GenerateAddress(uint64 align, uint64 size, bool isInstr, const ConstraintSet *rangeConstraintSet)
  {
    uint32 align_shift = get_align_shift(align);
    uint64 align_mask = ~(align - 1);
    uint64 addr = 0;
    std::unique_ptr<ConstraintSet> pa_constr;

    if (nullptr != rangeConstraintSet) {
      pa_constr.reset(rangeConstraintSet->Clone());
      //LOG(notice) << "Generate Address constraints:"<< pa_constr->ToSimpleString() << endl;
      //LOG(notice) << "Generate Address base constraints:"<< mpBaseConstraint->ToSimpleString() << endl;
      pa_constr->ApplyConstraintSet(*mpBaseConstraint);
    }
    else {
      pa_constr.reset(mpBaseConstraint->Clone());
    }

    pa_constr->AlignWithSize(align_mask, size);
    pa_constr->ShiftRight(align_shift);
    addr = pa_constr->ChooseValue() << align_shift;
    // << "PaGenerator::GenerateAddress addr:" << addr << endl;
    return addr;
  }

  uint64 PaGenerator::GetAddressFromBottom(uint64 align, uint64 size, const ConstraintSet *rangeConstraintSet)
  {
    verify_alignment(align);
    uint64 align_mask = ~(align - 1);
    std::unique_ptr<ConstraintSet> pa_constr(mpBaseConstraint->Clone());

    if (nullptr != rangeConstraintSet) {
      pa_constr->SubConstraintSet(*rangeConstraintSet);
    }

    return pa_constr->GetAlignedSizeFromBottom(align_mask, size);
  }

  uint64 PaGenerator::GetAddressFromTop(uint64 align, uint64 size, const ConstraintSet *rangeConstraintSet)
  {
    verify_alignment(align);
    uint64 align_mask = ~(align - 1);
    std::unique_ptr<ConstraintSet> pa_constr(mpBaseConstraint->Clone());

    if (nullptr != rangeConstraintSet) {
      pa_constr->SubConstraintSet(*rangeConstraintSet);
    }

    return pa_constr->GetAlignedSizeFromTop(align_mask, size);
  }

}
