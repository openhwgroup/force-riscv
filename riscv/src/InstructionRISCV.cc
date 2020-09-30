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
#include <InstructionRISCV.h>
#include <InstructionConstraintRISCV.h>
#include <OperandRISCV.h>
#include <RetOperandRISCV.h>
#include <VectorLayoutSetupRISCV.h>
#include <Choices.h>
#include <ChoicesModerator.h>
#include <Generator.h>
#include <GenRequest.h>
#include <Register.h>
#include <VectorLayout.h>
#include <Log.h>

#include <memory>
#include <sstream>

/*!
  \file InstructionRISCV.cc
  \brief Code supporting RISCV specific instruction generation
*/

using namespace std;

namespace Force {

  void RetInstruction::Setup(const GenInstructionRequest& instrReq, Generator& gen)
  {
    Instruction::Setup(instrReq, gen);
  }

  bool RetInstruction::GetPrePostAmbleRequests(Generator& gen) const
  {
    if (NoRestriction())
    {
      LOG(notice) << "{RetInstruction::GetPrePostAmbleRequests} No restriction, bypassing..." << endl;
      return false;
    }

    LOG(notice) << "{RetInstruction::GetPrePostAmbleRequests}" << endl;
    auto opr = FindOperandType<RetOperand>();
    if (nullptr != opr)
    {
      return opr->GetPrePostAmbleRequests(gen);
    }
    else
    {
      LOG(notice) << "{RetInstruction::GetPrePostAmbleRequests} - can't find RetOperand" << endl;
      return false;
    }
  }

  InstructionConstraint* RetInstruction::InstantiateInstructionConstraint() const
  {
    return new RetInstructionConstraint();
  }

  bool VectorAMOInstructionRISCV::Validate(const Generator& gen, string& error) const
  {
    VectorLayoutSetupRISCV vec_layout_setup(gen.GetRegisterFile());
    VectorLayout data_vec_layout;
    vec_layout_setup.SetUpVectorLayoutVtype(data_vec_layout);
    if (data_vec_layout.mElemSize < 32) {
      ChoicesModerator* choices_moderator = gen.GetChoicesModerator(EChoicesType::GeneralChoices);
      unique_ptr<Choice> choices_tree(choices_moderator->CloneChoiceTree("Skip Generation - Vector AMO"));
      auto chosen = choices_tree->Choose();
      if (chosen->Name() == "DoSkip") {
        stringstream err_stream;
        err_stream << "Instruction \"" << Name() << "\" failed because Handcar does not currently support VSEW < 32 (VSEW = " << data_vec_layout.mElemSize << ")";
        error = err_stream.str();
        return false;
      }
    }
    return true;
  }

}
