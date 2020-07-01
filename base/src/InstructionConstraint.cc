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
#include <InstructionConstraint.h>
#include <Choices.h>
#include <InstructionStructure.h>
#include <Generator.h>
#include <ChoicesModerator.h>
#include <GenException.h>
#include <Instruction.h>
#include <Constraint.h>
#include <Operand.h>
#include <GenRequest.h>
#include <ResourceDependence.h>
#include <Log.h>
#include <VectorLayout.h>

using namespace std;

namespace Force {

  InstructionConstraint::InstructionConstraint(const InstructionConstraint& rOther)
    : mpInstructionRequest(rOther.mpInstructionRequest), mpHotResource(rOther.mpHotResource)
  {
    LOG(fail) << "InstructionConstraint copy constructor not meant to be called." << endl;
    FAIL("calling-instruction-constraint-copy-constructor-unexpectedly");
  }

  InstructionConstraint::~InstructionConstraint()
  {
    mpInstructionRequest = nullptr;
    delete mpHotResource; // delete if not null
  }

  void InstructionConstraint::Setup(const Generator& gen, const Instruction& instr, const InstructionStructure& instructionStruct)
  {
    mpHotResource = gen.GetDependenceInstance()->CreateHotResource();
  }

  BranchInstructionConstraint::~BranchInstructionConstraint()
  {
    mpBranchOperand = nullptr;
  }

  void BranchInstructionConstraint::Setup(const Generator& gen, const Instruction& instr, const InstructionStructure& instructionStruct)
  {
    InstructionConstraint::Setup(gen, instr, instructionStruct);
    mpBranchOperand = instr.FindOperandType<BranchOperand>();
    if (nullptr == mpBranchOperand) {
      LOG(fail) << "{BranchInstructionConstraint::Setup} BranchOperand not found." << endl;
      FAIL("branch-operand-not-found");
    }
  }

  BranchInstructionConstraint::BranchInstructionConstraint(const BranchInstructionConstraint& rOther)
    : InstructionConstraint(rOther), mpBranchOperand(nullptr)
  {

  }

  LoadStoreInstructionConstraint::~LoadStoreInstructionConstraint()
  {
    mpLoadStoreOperand = nullptr;
  }

  void LoadStoreInstructionConstraint::Setup(const Generator& gen, const Instruction& instr, const InstructionStructure& instructionStruct)
  {
    InstructionConstraint::Setup(gen, instr, instructionStruct);
    mpLoadStoreOperand = instr.FindOperandType<LoadStoreOperand>();
    if (nullptr == mpLoadStoreOperand) {
      LOG(fail) << "{LoadStoreInstructionConstraint::Setup} LoadStoreOperand not found." << endl;
      FAIL("load-store-operand-not-found");
    }
  }

  LoadStoreInstructionConstraint::LoadStoreInstructionConstraint(const LoadStoreInstructionConstraint& rOther)
    : InstructionConstraint(rOther), mpLoadStoreOperand(nullptr)
  {

  }

  VectorInstructionConstraint::VectorInstructionConstraint()
    : InstructionConstraint(), mpVectorLayout(new VectorLayout())
  {
  }

  VectorInstructionConstraint::~VectorInstructionConstraint()
  {
    delete mpVectorLayout;
  }

  void VectorInstructionConstraint::SetVectorLayout(const VectorLayout& rVectorLayout)
  {
    (*mpVectorLayout) = rVectorLayout;
  }

}
