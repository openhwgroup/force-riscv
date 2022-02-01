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
#include <InstructionStructure.h>
#include <Log.h>

#include <memory>
#include <numeric>
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
    const Operand* data_opr = nullptr;
    if (mpStructure->mForm == "Register write") {
      data_opr = FindOperand("vd", true);
    }
    else {
      data_opr = FindOperand("vs3", true);
    }

    OperandConstraint* data_opr_constr = data_opr->GetOperandConstraint();
    auto vec_reg_opr_constr = data_opr_constr->CastInstance<VectorRegisterOperandConstraint>();
    const VectorLayout* data_vec_layout = vec_reg_opr_constr->GetVectorLayout();

    if (data_vec_layout->mElemSize < 32) {
      ChoicesModerator* choices_moderator = gen.GetChoicesModerator(EChoicesType::GeneralChoices);
      unique_ptr<Choice> choices_tree(choices_moderator->CloneChoiceTree("Skip Generation - Vector AMO"));
      auto chosen = choices_tree->Choose();
      if (chosen->Name() == "DoSkip") {
        stringstream err_stream;
        err_stream << "Instruction \"" << Name() << "\" skipped because Handcar does not currently support SEW < 32 (SEW = " << data_vec_layout->mElemSize << "). To enable generation, adjust the \"Skip Generation - Vector AMO\" general choice tree.";
        error = err_stream.str();
        return false;
      }
    }
    return true;
  }

  bool VsetvlInstruction::GetPrePostAmbleRequests(Generator& gen) const
  {
    return accumulate(mOperands.cbegin(), mOperands.cend(), bool(false),
      [&gen](cbool hasRequests, const Operand* pOpr) {
        bool opr_has_requests = pOpr->GetPrePostAmbleRequests(gen);
        return (hasRequests | opr_has_requests);
      });
  }

}
