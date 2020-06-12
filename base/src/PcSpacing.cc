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
#include <PcSpacing.h>
#include <Constraint.h>
#include <VmMapper.h>
#include <Generator.h>
#include <GenPC.h>
#include <Log.h>

using namespace std;

/*!
  \file PcSpacing.cc
  \brief Code managing PC vicinity spacing constraint
*/

namespace Force {

  PcSpacing* PcSpacing::mspPcSpacing = nullptr;

  void PcSpacing::Initialize()
  {
    if (nullptr == mspPcSpacing) {
      mspPcSpacing = new PcSpacing();
    }
  }

  void PcSpacing::Destroy()
  {
    delete mspPcSpacing;
    mspPcSpacing = nullptr;
  }

  PcSpacing::PcSpacing()
    : mpPcConstraint(nullptr), mpBranchPcConstraint(nullptr), mGenerators()
  {
    mpPcConstraint = new ConstraintSet();
    mpBranchPcConstraint = new ConstraintSet();
  }

  PcSpacing::~PcSpacing()
  {
    delete mpPcConstraint;
    delete mpBranchPcConstraint;
  }

  void PcSpacing::SignUp(const Generator* pGen)
  {
    mGenerators.push_back(pGen);
  }

  const ConstraintSet* PcSpacing::GetPcSpaceConstraint()
  {
    mpPcConstraint->Clear();
    for (const Generator* gen : mGenerators) {
      GenPC* gen_pc = gen->GetGenPC();
      AugmentConstraint(gen_pc->Value(), gen_pc->InstructionSpace(), mpPcConstraint);
    }

    return mpPcConstraint;
  }

  const ConstraintSet* PcSpacing::GetBranchPcSpaceConstraint(const VmMapper* pVmMapper, uint32 instrSize)
  {
    mpBranchPcConstraint->Clear();
    const Generator* vm_gen = pVmMapper->GetGenerator();
    for (const Generator* gen : mGenerators) {
      GenPC* gen_pc = gen->GetGenPC();
      uint64 pc_value = gen_pc->Value();

      if (gen == vm_gen) {
        AugmentConstraint(pc_value, instrSize, mpBranchPcConstraint);
      }
      else {
        AugmentConstraint(pc_value, gen_pc->InstructionSpace(), mpBranchPcConstraint);
      }
    }

    return mpBranchPcConstraint;
  }

  void PcSpacing::AugmentConstraint(cuint64 pcValue, cuint32 instrSpace, ConstraintSet* constr)
  {
    uint64 end_pc = pcValue + (instrSpace - 1);

    if (end_pc >= pcValue) {
      constr->AddRange(pcValue, end_pc);
    }
    else { // address wrap around
      constr->AddRange(pcValue, MAX_UINT64);
      constr->AddRange(0, end_pc);
    }
  }

}
