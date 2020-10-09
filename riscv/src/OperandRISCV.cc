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
#include <OperandRISCV.h>

#include <AddressSolver.h>
#include <BntNode.h>
#include <ChoicesFilter.h>
#include <Constraint.h>
#include <GenException.h>
#include <GenRequest.h>
#include <Generator.h>
#include <Instruction.h>
#include <InstructionStructure.h>
#include <Log.h>
#include <Random.h>
#include <Register.h>
#include <VaGenerator.h>
#include <VectorLayout.h>
#include <VmMapper.h>

#include <InstructionConstraintRISCV.h>
#include <OperandConstraintRISCV.h>
#include <VectorLayoutSetupRISCV.h>

#include <cmath>
#include <memory>
#include <sstream>

using namespace std;

/*!
  \file OperandRISCV.cc
  \brief Code supporting RISCV specific operand generation
*/

namespace Force {

  OperandConstraint* VsetvlVtypeImmediateOperand::InstantiateOperandConstraint() const
  {
    return new VsetvlVtypeImmediateOperandConstraint();
  }

  void VectorMaskOperand::Generate(Generator& gen, Instruction& instr)
  {
    mpOperandConstraint->SubDifferOperandValues(instr, *mpStructure);

    ChoicesOperand::Generate(gen, instr);
  }

  void VectorMaskOperand::Commit(Generator& gen, Instruction& instr)
  {
    if (mValue == 0) {
      gen.RandomInitializeRegister("v0", "");
    }
  }

  OperandConstraint* VectorMaskOperand::InstantiateOperandConstraint() const
  {
    return new VectorMaskOperandConstraint();
  }

  bool BaseOffsetBranchOperand::GetPrePostAmbleRequests(Generator& gen) const
  {
    auto branch_opr_constr = mpOperandConstraint->CastInstance<BaseOffsetBranchOperandConstraint>();
    if (branch_opr_constr->UsePreamble()) {
      RegisterOperand* base_opr = branch_opr_constr->BaseOperand();
      gen.AddLoadRegisterAmbleRequests(base_opr->ChoiceText(), branch_opr_constr->BaseValue());
      return true;
    }

    return false;
  }

  AddressingMode* BaseOffsetBranchOperand::GetAddressingMode(uint64 alignment) const
  {
    return new BaseOffsetMode();
  }

  OperandConstraint* BaseOffsetBranchOperand::InstantiateOperandConstraint() const
  {
    return new BaseOffsetBranchOperandConstraint();
  }

  void BaseOffsetBranchOperand::GenerateWithPreamble(Generator& gen, Instruction& instr)
  {
    auto branch_opr_constr = mpOperandConstraint->CastInstance<BaseOffsetBranchOperandConstraint>();

    // We can't load x0, so remove it from the list of choices for preamble generation.
    RegisterOperand* base_opr = branch_opr_constr->BaseOperand();
    auto base_opr_constr = dynamic_cast<RegisterOperandConstraint*>(base_opr->GetOperandConstraint());
    base_opr_constr->SubConstraintValue(0, *mpStructure);

    VmMapper* vm_mapper = branch_opr_constr->GetVmMapper();
    if (BaseGenerate(gen, instr)) {
      vm_mapper->MapAddressRange(mTargetAddress, gen.InstructionSpace(), false);
      return;
    }

    const GenPageRequest* page_req = branch_opr_constr->GetPageRequest();
    VaGenerator va_gen(vm_mapper, page_req, branch_opr_constr->TargetConstraint());
    mTargetAddress = va_gen.GenerateAddress(gen.InstructionAlignment(), gen.InstructionSpace(), true, page_req->MemoryAccessType());

    CalculateBaseValueForPreamble(branch_opr_constr);

    LOG(notice) << "{BaseOffsetBranchOperand::GenerateWithPreamble} generated target address 0x" << hex << mTargetAddress << " base value 0x" << branch_opr_constr->BaseValue() << endl;
  }

  bool BaseOffsetBranchOperand::GenerateNoPreamble(Generator& gen, Instruction& instr)
  {
    AddressSolver* addr_solver = GetAddressSolver(GetAddressingMode(), gen.InstructionAlignment());
    unique_ptr<AddressSolver> addr_solver_storage(addr_solver);

    auto branch_opr_struct = mpStructure->CastOperandStructure<BranchOperandStructure>();
    const AddressingMode* addr_mode = addr_solver->Solve(gen, instr, gen.InstructionSpace(), true, branch_opr_struct->MemAccessType());
    if (addr_mode == nullptr) {
      return false;
    }

    auto branch_opr_constr = mpOperandConstraint->CastInstance<BaseOffsetBranchOperandConstraint>();
    branch_opr_constr->SetBaseValue(addr_mode->BaseValue());
    mTargetAddress = addr_mode->TargetAddress();

    LOG(notice) << "{BaseOffsetBranchOperand::GenerateNoPreamble} instruction: " << instr.FullName() << " addressing-mode: " << addr_mode->Type() << " target address: 0x" << hex << mTargetAddress << endl;

    addr_solver->SetOperandResults();

    return true;
  }

  void BaseOffsetBranchOperand::CalculateBaseValueForPreamble(BaseOffsetBranchOperandConstraint* pBranchOprConstr)
  {
    ImmediateOperand* offset_opr = pBranchOprConstr->OffsetOperand();
    uint64 offset_value = offset_opr->Value();
    if (offset_opr->IsSigned()) {
      offset_value = sign_extend64(offset_value, offset_opr->Size());
    }

    // The JALR instruction clears the last bit of the computed address, so we can randomly assign
    // it to 0 or 1
    Random* random = Random::Instance();
    uint64 base_value = mTargetAddress - offset_value + random->Random32(0, 1);

    pBranchOprConstr->SetBaseValue(base_value);
  }

  OperandConstraint* ConditionalBranchOperandRISCV::InstantiateOperandConstraint() const
  {
    return new FullsizeConditionalBranchOperandConstraint();
  }

  OperandConstraint* CompressedConditionalBranchOperandRISCV::InstantiateOperandConstraint() const
  {
    return new CompressedConditionalBranchOperandConstraint();
  }

  bool ConditionalBranchOperandRISCV::IsBranchTaken(const Instruction& instr) const
  {
    if (instr.NoRestriction()) {
      auto taken_constr = instr.ConditionTakenConstraint();
      if (taken_constr && taken_constr->ChooseValue())
        return true;
      else
        return false;
    }
    auto opr_constr = dynamic_cast<const ConditionalBranchOperandRISCVConstraint* >(mpOperandConstraint);
    return opr_constr->BranchTaken();
  }

  BntNode* ConditionalBranchOperandRISCV::GetBntNode(const Instruction& instr) const
  {
    auto branch_constr = mpOperandConstraint->CastInstance<BranchOperandConstraint>();
    bool br_taken = IsBranchTaken(instr) && !mEscapeTaken;
    if (instr.SpeculativeBnt() and branch_constr->SimulationEnabled())
      return new SpeculativeBntNode(mTargetAddress, br_taken, true);
    else
      return new BntNode(mTargetAddress, br_taken, true); // true => conditional branch.
  }

  void ConditionalBranchOperandRISCV::Commit(Generator& gen, Instruction& instr)
  {
    // Determine whether or not the branch will be taken first, so that this information is
    // available to the superclass's Commit() method
    auto opr_constr = dynamic_cast<ConditionalBranchOperandRISCVConstraint* >(mpOperandConstraint);
    opr_constr->SetConditionalBranchTaken(gen, instr, *mpStructure);

    PcRelativeBranchOperand::Commit(gen, instr);
  }

  OperandConstraint* CompressedRegisterOperandRISCV::InstantiateOperandConstraint() const
  {
    return new CompressedRegisterOperandRISCVConstraint();
  }

  VsetvlAvlRegisterOperand::VsetvlAvlRegisterOperand()
    : RegisterOperand(), mAvlRegVal(0)
  {
  }

  VsetvlAvlRegisterOperand::VsetvlAvlRegisterOperand(const VsetvlAvlRegisterOperand& rOther)
    : RegisterOperand(rOther), mAvlRegVal(rOther.mAvlRegVal)
  {
  }

  void VsetvlAvlRegisterOperand::Generate(Generator& gen, Instruction& instr)
  {
    RegisterOperand::Generate(gen, instr);

    // We want to maintain the same vl value by default
    const RegisterFile* reg_file = gen.GetRegisterFile();
    Register* vl_reg = reg_file->RegisterLookup("vl");
    mAvlRegVal = vl_reg->Value();
  }

  bool VsetvlAvlRegisterOperand::GetPrePostAmbleRequests(Generator& gen) const
  {
    if (not mpOperandConstraint->ConstraintForced()) {
      gen.AddLoadRegisterAmbleRequests(mChoiceText, mAvlRegVal);
      return true;
    }

    return false;
  }

  OperandConstraint* VsetvlAvlRegisterOperand::InstantiateOperandConstraint() const
  {
    return new VsetvlRegisterOperandConstraint();
  }

  VsetvlVtypeRegisterOperand::VsetvlVtypeRegisterOperand()
    : RegisterOperand(), mVtypeRegVal(0)
  {
  }

  VsetvlVtypeRegisterOperand::VsetvlVtypeRegisterOperand(const VsetvlVtypeRegisterOperand& rOther)
    : RegisterOperand(rOther), mVtypeRegVal(rOther.mVtypeRegVal)
  {
  }

  void VsetvlVtypeRegisterOperand::Generate(Generator& gen, Instruction& instr)
  {
    mpOperandConstraint->SubDifferOperandValues(instr, *mpStructure);

    RegisterOperand::Generate(gen, instr);

    // We want to maintain the same vtype value by default
    const RegisterFile* reg_file = gen.GetRegisterFile();
    Register* vtype_reg = reg_file->RegisterLookup("vtype");
    mVtypeRegVal = vtype_reg->Value();
  }

  bool VsetvlVtypeRegisterOperand::GetPrePostAmbleRequests(Generator& gen) const
  {
    if (not mpOperandConstraint->ConstraintForced()) {
      gen.AddLoadRegisterAmbleRequests(mChoiceText, mVtypeRegVal);
      return true;
    }

    return false;
  }

  OperandConstraint* VsetvlVtypeRegisterOperand::InstantiateOperandConstraint() const
  {
    return new VsetvlRegisterOperandConstraint();
  }

  void VtypeLayoutOperand::SetupVectorLayout(const Generator& rGen, const Instruction& rInstr)
  {
    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    VectorLayout vec_layout(*(instr_constr->GetVectorLayout()));

    VectorLayoutSetupRISCV vec_layout_setup(rGen.GetRegisterFile());
    vec_layout_setup.SetUpVectorLayoutVtype(vec_layout);

    instr_constr->SetVectorLayout(vec_layout);
  }

  void CustomLayoutOperand::SetupVectorLayout(const Generator& rGen, const Instruction& rInstr)
  {
    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    VectorLayout vec_layout(*(instr_constr->GetVectorLayout()));

    VectorLayoutSetupRISCV vec_layout_setup(rGen.GetRegisterFile());
    vec_layout_setup.SetUpVectorLayoutFixedElementSize(*(mpStructure->CastOperandStructure<VectorLayoutOperandStructure>()), vec_layout);

    instr_constr->SetVectorLayout(vec_layout);
  }

  void WholeRegisterLayoutOperand::SetupVectorLayout(const Generator& rGen, const Instruction& rInstr)
  {
    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    VectorLayout vec_layout(*(instr_constr->GetVectorLayout()));

    VectorLayoutSetupRISCV vec_layout_setup(rGen.GetRegisterFile());
    vec_layout_setup.SetUpVectorLayoutWholeRegister(*(mpStructure->CastOperandStructure<VectorLayoutOperandStructure>()), vec_layout);

    instr_constr->SetVectorLayout(vec_layout);
  }

  void VectorIndexedLoadStoreOperandRISCV::AdjustMemoryElementLayout(const Generator& rGen, const Instruction& rInstr)
  {
    Operand* data_opr = GetDataOperand(rInstr);
    OperandConstraint* data_opr_constr = data_opr->GetOperandConstraint();
    auto vec_reg_opr_constr = data_opr_constr->CastInstance<VectorRegisterOperandConstraintRISCV>();
    float data_layout_multiple = vec_reg_opr_constr->GetLayoutMultiple();

    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    const VectorLayout* index_vec_layout = instr_constr->GetVectorLayout();

    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    uint32 data_elem_byte_size = lround(index_vec_layout->mElemSize * data_layout_multiple) / 8;
    lsop_struct->SetElementSize(data_elem_byte_size);
    lsop_struct->SetAlignment(data_elem_byte_size);
    lsop_struct->SetDataSize(data_elem_byte_size * index_vec_layout->mFieldCount);
  }

  void VectorIndexedLoadStoreOperandRISCV::GetIndexRegisterNames(vector<string>& rIndexRegNames) const
  {
    auto indexed_opr_constr = mpOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    auto index_opr = dynamic_cast<MultiRegisterOperand*>(indexed_opr_constr->IndexOperand());
    rIndexRegNames.push_back(index_opr->ChoiceText());
    index_opr->GetExtraRegisterNames(index_opr->Value(), rIndexRegNames);
  }

  Operand* VectorIndexedLoadStoreOperandRISCV::GetDataOperand(const Instruction& rInstr) const
  {
    Operand* data_opr = nullptr;

    auto indexed_opr_constr = mpOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    RegisterOperand* index_opr = indexed_opr_constr->IndexOperand();
    for (Operand* opr : rInstr.GetOperands()) {
      if ((opr->OperandType() == EOperandType::VECREG) and (opr != index_opr)) {
        data_opr = opr;
        break;
      }
    }

    return data_opr;
  }

  void MultiVectorRegisterOperandRISCV::Setup(Generator& gen, Instruction& instr)
  {
    MultiVectorRegisterOperand::Setup(gen, instr);

    AdjustRegisterCount(instr);
  }

  void MultiVectorRegisterOperandRISCV::Generate(Generator& gen, Instruction& instr)
  {
    mpOperandConstraint->SubDifferOperandValues(instr, *mpStructure);

    MultiVectorRegisterOperand::Generate(gen, instr);
  }

  void MultiVectorRegisterOperandRISCV::GetRegisterIndices(uint32 regIndex, ConstraintSet& rRegIndices) const
  {
    uint32 end_index = regIndex + mRegCount - 1;
    if (end_index < 32) {
      rRegIndices.AddRange(regIndex, end_index);
    }
    else {
      LOG(fail) << "{MultiVectorRegisterOperandRISCV::GetRegisterIndices} ending register index " << dec << end_index << " is not valid" << endl;
      FAIL("invalid-register-index");
    }
  }

  void MultiVectorRegisterOperandRISCV::GetChosenRegisterIndices(const Generator& gen, ConstraintSet& rRegIndices) const
  {
    ConstraintSet reg_indices;
    MultiVectorRegisterOperand::GetChosenRegisterIndices(gen, reg_indices);

    GetRegisterIndices(reg_indices.LowerBound(), rRegIndices);
  }

  uint32 MultiVectorRegisterOperandRISCV::NumberRegisters() const
  {
    if (mRegCount == 0) {
      LOG(fail) << "{MultiVectorRegisterOperandRISCV::NumberRegisters} invalid register count " << dec << mRegCount << endl;
      FAIL("invalid-register-count");
    }

    return mRegCount;
  }

  OperandConstraint* MultiVectorRegisterOperandRISCV::InstantiateOperandConstraint() const
  {
    return new VectorRegisterOperandConstraintRISCV();
  }

  const std::string MultiVectorRegisterOperandRISCV::GetNextRegisterName(uint32& indexVar) const
  {
    ++indexVar;
    if (indexVar > 31) indexVar = 0;
    return "v" + to_string(indexVar);
  }

  ChoicesFilter* MultiVectorRegisterOperandRISCV::GetChoicesFilter(const ConstraintSet* pConstrSet) const
  {
    return new ConstraintChoicesFilter(pConstrSet);
  }

  void MultiVectorRegisterOperandRISCV::AdjustRegisterCount(const Instruction& rInstr)
  {
    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();
    auto vec_reg_opr_constr = mpOperandConstraint->CastInstance<VectorRegisterOperandConstraintRISCV>();
    mRegCount = lround(vec_layout->GetRegisterCount() * vec_reg_opr_constr->GetLayoutMultiple());
    if (mRegCount == 0) {
      mRegCount = 1;
    }
  }

  OperandConstraint* VectorIndexedDataRegisterOperand::InstantiateOperandConstraint() const
  {
    return new VectorIndexedDataRegisterOperandConstraint();
  }

}
