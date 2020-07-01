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
#include <Instruction.h>
#include <InstructionStructure.h>
#include <Operand.h>
#include <ObjectRegistry.h>
#include <AsmText.h>
#include <GenRequest.h>
#include <InstructionConstraint.h>
#include <Log.h>
#include <Generator.h>
#include <ResourceDependence.h>
#include <BntNodeManager.h>
#include <BntNode.h>
#include <GenException.h>

#include <algorithm>
#include <string.h>
//#include <stdio.h>
// C++UP accumulate defined in numeric
#include <numeric>

/*!
  \file Instruction.cc
  \brief Code for handling basic instruction generation.
*/

using namespace std;

namespace Force {

  Instruction::Instruction()
    : Object(), mpStructure(nullptr), mpInstructionConstraint(nullptr), mOpcode(0), mOperands(), mUnpredictable(false)
  {

  }

  Instruction::Instruction(const Instruction& rOther)
    : Object(rOther), mpStructure(rOther.mpStructure), mpInstructionConstraint(nullptr), mOpcode(rOther.mOpcode), mOperands(), mUnpredictable(false)
  {
    transform(rOther.mOperands.cbegin(), rOther.mOperands.cend(), back_inserter(mOperands),
      [](const Operand* pOpr) { return dynamic_cast<Operand*>(pOpr->Clone()); });
  }

  Instruction::~Instruction()
  {
    for (auto opr_ptr : mOperands) {
      delete opr_ptr;
    }

    if (nullptr != mpInstructionConstraint) {
      LOG(fail) << "Expect instruction constraint to be delete by now." << endl;
      FAIL("dangling-instruction-constraint-pointer");
    }
  }

  Object* Instruction::Clone() const
  {
    return new Instruction(*this);
  }

  const std::string Instruction::ToString() const
  {
    return "Instruction: " + mpStructure->FullName();
  }

  const string Instruction::FullName() const
  {
    return mpStructure->FullName();
  }

  const string& Instruction::Name() const
  {
    return mpStructure->Name();
  }

  uint32 Instruction::Size() const
  {
    return mpStructure->Size();
  }

  uint32 Instruction::ByteSize() const
  {
    return mpStructure->Size() >> 3; // devide by 8
  }

  uint32 Instruction::ElementSize() const
  {
    return mpStructure->ElementSize();
  }

  void Instruction::Initialize(const InstructionStructure* instrStructure)
  {
    mpStructure = instrStructure;

    const vector<OperandStructure* >& opr_vec = mpStructure->OperandStructures();

    ObjectRegistry* obj_registry = ObjectRegistry::Instance();
    for (auto opr_struct_ptr : opr_vec) {
      Operand* opr = obj_registry->TypeInstance<Operand>(opr_struct_ptr->mClass);
      opr->Initialize(opr_struct_ptr);
      mOperands.push_back(opr);
    }
  }

  void Instruction::Setup(const GenInstructionRequest& instrReq, Generator& gen)
  {
    gen.SetupInstructionGroup(mpStructure->mGroup);

    if (nullptr != mpInstructionConstraint) {
      LOG(fail) << "{Instruction::Setup} expecting mpInstructionConstraint to be nullptr at this point." << endl;
      FAIL("instruction-constraint-not-null");
    }
    mpInstructionConstraint = InstantiateInstructionConstraint();
    mpInstructionConstraint->SetInstructionRequest(&instrReq);
    mpInstructionConstraint->Setup(gen, *this, *(this->mpStructure));

    for (auto opr_ptr : mOperands) {
      opr_ptr->Setup(gen, *this);
    }
  }

  InstructionConstraint* Instruction::InstantiateInstructionConstraint() const
  {
    return new InstructionConstraint();
  }

  void Instruction::Generate(Generator& gen)
  {
    if (gen.InSpeculative()) {
      auto hot_bntNode = gen.GetBntNodeManager()->GetHotSpeculativeBntNode();
      if (hot_bntNode->ExecutionIsOverflow()) {
        stringstream err_stream;
        err_stream << "Instruction \"" << Name() << "\" failed to generate as Bnt Execution is overflow";
        throw InstructionError(err_stream.str());
      }
    }

    for (auto opr_ptr : mOperands) {
      LOG(info)<<" opname="<<opr_ptr->Name()<<endl;
      opr_ptr->Generate(gen, *this);
    }

    Assemble();
  }

  void Instruction::SetOperandDataValue(const string& oprName, uint64 oprValue, uint32 valueSize) const
  {
    auto instr_req = GetInstructionConstraint()->InstructionRequest();
    instr_req->SetOperandDataRequest(oprName, oprValue, valueSize);
  }

  void Instruction::SetOperandDataValue(const std::string& oprName, const std::string& oprValue, uint32 valueSize) const
  {
    auto instr_req = GetInstructionConstraint()->InstructionRequest();
    instr_req->SetOperandDataRequest(oprName, oprValue, valueSize);
  }
  void Instruction::SetOperandDataValue(const std::string& oprName, std::vector<uint64> oprValues, uint32 valueSize) const
  {
    auto instr_req = GetInstructionConstraint()->InstructionRequest();
    instr_req->SetOperandDataRequest(oprName, oprValues, valueSize);
  }

  void Instruction::Assemble()
  {
    mOpcode = accumulate(mOperands.begin(), mOperands.end(), mpStructure->ConstantValue(), [](uint32 opcode, Operand* opr_ptr) { return opcode |= opr_ptr->Encoding(); });
  }

  void Instruction::Commit(Generator& gen)
  {
    for (auto opr_ptr : mOperands) {
      opr_ptr->Commit(gen, *this);
    }
  }

  void Instruction::CleanUp()
  {
    for (auto opr_ptr : mOperands) {
      opr_ptr->CleanUp();
    }

    delete mpInstructionConstraint;
    mpInstructionConstraint = nullptr;
  }

  const std::string Instruction::AssemblyText() const
  {
    return mpStructure->mpAsmText->Text(*this);
  }

  const Operand* Instruction::FindOperand(const string& oprName, bool failNotFound) const
  {
    for (auto opr_ptr : mOperands) {
      auto match_opr = opr_ptr->MatchOperand(oprName);
      if (nullptr != match_opr) {
        return match_opr;
      }
    }

    if (failNotFound) {
      LOG(fail) << "{Instruction::FindOperand} no operand \"" << oprName << "\" found in instruction \"" << FullName() << endl;
      FAIL("operand-lookup-failed");
    }

    return nullptr;
  }

  Operand* Instruction::FindOperandMutable(const string& oprName, bool failNotFound) const
  {
    for (auto opr_ptr : mOperands) {
      auto match_opr = opr_ptr->MatchOperandMutable(oprName);
      if (nullptr != match_opr) {
        return match_opr;
      }
    }

    if (failNotFound) {
      LOG(fail) << "{Instruction::FindOperandMutable} no operand \"" << oprName << "\" found in instruction \"" << FullName() << endl;
      FAIL("operand-mutable-lookup-failed");
    }

    return nullptr;
  }

  bool Instruction::NoRestriction() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::NoRestriction);
  }

  bool Instruction::NoSkip() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::NoSkip);
  }

  bool Instruction::UnalignedPC() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::UnalignedPC);
  }

  bool Instruction::AlignedData() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::AlignedData);
  }

  bool Instruction::AlignedSP() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::AlignedSP);
  }

  bool Instruction::NoBnt() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::NoBnt);
  }

  bool Instruction::NoPreamble() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::NoPreamble);
  }

  bool Instruction::SpeculativeBnt() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::SpeculativeBnt);
  }

  bool Instruction::NoDataAbort() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::NoDataAbort);
  }

  bool Instruction::SharedTarget() const
  {
    return mpInstructionConstraint->InstructionRequest()->BoolAttribute(EInstrBoolAttrType::SharedTarget);
  }

  ResourceAccessStage* Instruction::GiveHotResource()
  {
    return mpInstructionConstraint->GiveHotResource();
  }

  const ConstraintSet* Instruction::BranchTargetConstraint() const
  {
    return mpInstructionConstraint->InstructionRequest()->ConstraintAttribute(EInstrConstraintAttrType::BRTarget);
  }

  const ConstraintSet* Instruction::LoadStoreTargetConstraint() const
  {
    return mpInstructionConstraint->InstructionRequest()->ConstraintAttribute(EInstrConstraintAttrType::LSTarget);
  }

  const vector<ConstraintSet* >& Instruction::LoadStoreDataConstraints() const
  {
    return mpInstructionConstraint->InstructionRequest()->LSDataConstraints();
  }
  const vector<ConstraintSet* >& Instruction::LoadStoreGatherScatterTargetListConstraints() const
  {
    return mpInstructionConstraint->InstructionRequest()->LSTargetListConstraints();
  }

  const ConstraintSet* Instruction::ConditionTakenConstraint() const
  {
    return mpInstructionConstraint->InstructionRequest()->ConstraintAttribute(EInstrConstraintAttrType::CondTaken);
  }

  EInstructionGroupType Instruction::Group() const
  {
    return mpStructure->mGroup;
  }

  InstructionConstraint* BranchInstruction::InstantiateInstructionConstraint() const
  {
    return new BranchInstructionConstraint();
  }

  bool BranchInstruction::GetPrePostAmbleRequests(Generator& gen) const
  {
    auto cast_iconstr = mpInstructionConstraint->CastInstance<BranchInstructionConstraint>();
    return cast_iconstr->GetBranchOperand()->GetPrePostAmbleRequests(gen);
  }

  uint64 BranchInstruction::BranchTarget() const
  {
    auto cast_iconstr = mpInstructionConstraint->CastInstance<BranchInstructionConstraint>();
    return cast_iconstr->GetBranchOperand()->BranchTarget();
  }

  bool BranchInstruction::IsBranchTaken() const
  {
    auto cast_iconstr = mpInstructionConstraint->CastInstance<BranchInstructionConstraint>();
    return cast_iconstr->GetBranchOperand()->IsBranchTaken(*this);
  }

  BntNode* BranchInstruction::GetBntNode() const
  {
    auto cast_iconstr = mpInstructionConstraint->CastInstance<BranchInstructionConstraint>();
    return cast_iconstr->GetBranchOperand()->GetBntNode(*this);
  }

  InstructionConstraint* LoadStoreInstruction::InstantiateInstructionConstraint() const
  {
    return new LoadStoreInstructionConstraint();
  }

  bool LoadStoreInstruction::GetPrePostAmbleRequests(Generator& gen) const
  {
    auto cast_iconstr = mpInstructionConstraint->CastInstance<LoadStoreInstructionConstraint>();
    return cast_iconstr->GetLoadStoreOperand()->GetPrePostAmbleRequests(gen);
  }

  void UnpredictStoreInstruction::Generate(Generator& gen)
  {
    LoadStoreInstruction::Generate(gen);
    SetUnpredictable(true);
    LOG(notice) << "{UnpredictStoreInstruction::Generate} Set instructon to be unpredicted" << endl;
  }

  InstructionConstraint* VectorInstruction::InstantiateInstructionConstraint() const
  {
    return new VectorInstructionConstraint();
  }

  InstructionConstraint* VectorLoadStoreInstruction::InstantiateInstructionConstraint() const
  {
    return new VectorLoadStoreInstructionConstraint();
  }

}
