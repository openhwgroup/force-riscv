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
#include <OperandConstraintRISCV.h>

#include <Instruction.h>
#include <InstructionConstraint.h>
#include <InstructionStructure.h>
#include <Constraint.h>
#include <Random.h>
#include <Log.h>
#include <Operand.h>
#include <VectorLayout.h>
#include <Generator.h>
#include <Register.h>
#include <RegisterReserver.h>

#include <algorithm>

using namespace std;

/*!
  \file OperandConstraintRISCV.cc
  \brief Code supporting RISCV specific operand constraints
*/

namespace Force {

  void VectorMaskOperandConstraint::GetAdjustedDifferValues(const Instruction& rInstr, const OperandStructure& rOperandStruct, const OperandStructure& rDifferOperandStruct, cuint64 differVal, ConstraintSet& rAdjDifferValues) const
  {
    if (differVal == 0) {
      rAdjDifferValues.AddValue(differVal);
    }
  }

  void BaseOffsetBranchOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    BranchOperandConstraint::Setup(rGen, rInstr, rOperandStruct);

    auto branch_opr_struct = dynamic_cast<const BranchOperandStructure*>(&rOperandStruct);
    if (branch_opr_struct == nullptr) {
      LOG(fail) << "{BaseOffsetBranchOperandConstraint::Setup} expecting operand " << rOperandStruct.mName << " to be \"BranchOperandStructure\" type." << endl;
      FAIL("unexpected-operand-structure-type");
    }

    Operand* base_opr = rInstr.FindOperandMutable(branch_opr_struct->Base(), true);
    mpBase = dynamic_cast<RegisterOperand*>(base_opr);
    if (mpBase == nullptr) {
      LOG(fail) << "{BaseOffsetBranchOperandConstraint::Setup} expecting operand " << base_opr->Name() << " to be \"RegisterOperand\" type." << endl;
      FAIL("unexpected-operand-type");
    }

    Operand* offset_opr = rInstr.FindOperandMutable(branch_opr_struct->Offset(), true);
    mpOffset = dynamic_cast<ImmediateOperand*>(offset_opr);
    if (mpOffset == nullptr) {
      LOG(fail) << "{BaseOffsetBranchOperandConstraint::Setup} expecting operand " << offset_opr->Name() << " to be \"ImmediateOperand\" type." << endl;
      FAIL("unexpected-operand-type");
    }
  }


  void CompressedRegisterOperandRISCVConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    ChoicesOperandConstraint::Setup(rGen, rInstr, rOperandStruct);

    if (mConstraintForced) {
      LOG(info) << "constraint already forced, ignore reservation check" << endl;
      return;
    }

    auto reg_file = rGen.GetRegisterFile();
    const ConstraintSet* read_reserv_constr = nullptr;
    const ConstraintSet* write_reserv_constr = nullptr;
    if (not reg_file->GetRegisterReserver()->HasReservations(rOperandStruct.mType, rOperandStruct.mAccess, read_reserv_constr, write_reserv_constr)) {
      // << "no reservation constraints for: " << rOperandStruct.Name() << endl;
      return;
    }

    // << "YES reservation constraints for: " << rOperandStruct.Name() << " read constr? " << (read_reserv_constr != nullptr)
    // << " write constr? " << (write_reserv_constr != nullptr) << endl;
    if (nullptr == mpConstraintSet) {
      mpConstraintSet = DefaultConstraintSet(rOperandStruct);
    }
    //LOG(notice) << " default constraints: " << mpConstraintSet->ToSimpleString() << endl;
    if (nullptr != read_reserv_constr) {
      //LOG(notice) << " read constr " << read_reserv_constr->ToSimpleString() << endl;
      ConstraintSet compressed_read_reserv_constr = *read_reserv_constr;
      compressed_read_reserv_constr.SubtractFromElements(8);
      //LOG(notice) << " 'prime' read constr " << compressed_read_reserv_constr.ToSimpleString() << endl;
      mpConstraintSet->SubConstraintSet(compressed_read_reserv_constr);
    }
    if (nullptr != write_reserv_constr) {
      //LOG(notice) << " write constr " << write_reserv_constr->ToSimpleString() << endl;
      ConstraintSet compressed_write_reserv_constr = *write_reserv_constr;
      compressed_write_reserv_constr.SubtractFromElements(8);
      //LOG(notice) << " 'prime' write constr " << compressed_write_reserv_constr.ToSimpleString() << endl;
      mpConstraintSet->SubConstraintSet(compressed_write_reserv_constr);
    }
    //LOG(notice) << " constraints (with reserved regs removed): " << mpConstraintSet->ToSimpleString() << endl;
  }

  void ConditionalBranchOperandRISCVConstraint::Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    PcRelativeBranchOperandConstraint::Setup(gen, instr, operandStruct);
    auto taken_constr = instr.ConditionTakenConstraint();
    if (taken_constr)
      mTaken = bool(taken_constr->ChooseValue());
    else
      mTaken = Random::Instance()->Random32(0, 1);
    // << "{ConditionalBranchOperandRISCVConstraint::Setup} condition branch is set to " << ((mTaken) ? "taken" : "not taken") << endl;
   }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBEQ(uint64 rs1Val, uint64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBEQ(...)" << endl;
    LOG(info) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BEQ logic here
    mTaken = (rs2Val == rs1Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBNE(uint64 rs1Val, uint64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBNE(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BNE logic here
    mTaken = (rs2Val != rs1Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBLT(int64 rs1Val, int64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBLT(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BLT logic here
    mTaken = (rs1Val < rs2Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBLTU(uint64 rs1Val, uint64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBLTU(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BLTU logic here
    mTaken = (rs1Val < rs2Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBGE(int64 rs1Val, int64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBGE(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BGE logic here
    mTaken = (rs1Val >= rs2Val);
  }

  void FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBGEU(uint64 rs1Val, uint64 rs2Val)
  {
    LOG(info) << "FullsizeConditionalBranchOperandConstraint::SetBranchTakenForBGEU(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << " and the rs2 val is: " << rs2Val << endl;
    //Evaluate the BGEU logic here
    mTaken = (rs1Val >= rs2Val);
  }

  void CompressedConditionalBranchOperandConstraint::SetBranchTakenForCBEQZ(uint64 rs1Val)
  {
    LOG(info) << "CompressedConditionalBranchOperandConstraint::SetBranchTakenForCBEQZ(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << endl;
    //Evaluate the CBEQZ logic here
    mTaken = (rs1Val == 0);
  }

  void CompressedConditionalBranchOperandConstraint::SetBranchTakenForCBNEZ(uint64 rs1Val)
  {
    LOG(info) << "CompressedConditionalBranchOperandConstraint::SetBranchTakenForBNEZ(...)" << endl;
    LOG(debug) << "Rs1 val is: " << hex << rs1Val << endl;
    //Evaluate the CBNEZ logic here
    mTaken = (rs1Val != 0);
  }

  void CompressedConditionalBranchOperandConstraint::SetConditionalBranchTaken(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    if (gen.SimulationEnabled()) 
    {
      // << "Simulation is enabled" << endl;
      string instr_name = instr.Name();
      
      auto branch_opr_struct = dynamic_cast<const BranchOperandStructure*>(&operandStruct);
      if (branch_opr_struct == nullptr) {
        LOG(fail) << "{CompressedConditionalBranchOperandConstraint::Setup} expecting operand " << operandStruct.mName << " to be \"BranchOperandStructure\" type." << endl;
        FAIL("unexpected-operand-structure-type");
      }
      EBranchConditionType branch_type = branch_opr_struct->mCondition;

      //Obtain handles to operands
      bool fail_if_not_found = true;
      const Operand*  src_1_opr = instr.FindOperand("rs1'", fail_if_not_found);
      const RegisterFile* reg_file = gen.GetRegisterFile();
      uint64 rs1_val = 0;

      //The rs1_val is alternatively used for rs1' if it is present.
      auto rs1_ropr = dynamic_cast<const RegisterOperand* >(src_1_opr);
      string rs1_reg_name = rs1_ropr->ChoiceText();
      rs1_val = reg_file->RegisterLookup(rs1_reg_name)->Value();
 
      switch(branch_type)
      {
        case EBranchConditionType::CBEQZ: SetBranchTakenForCBEQZ(rs1_val); break;
        case EBranchConditionType::CBNEZ: SetBranchTakenForCBNEZ(rs1_val); break;
        default:
          LOG(fail) << "{CompressedConditionalBranchOperandConstraint::SetConditionalBranchTaken} unknown branch instruction:" << instr_name << endl;
          FAIL("unknown-branch_instruction");
      } 
    }
    auto taken_constr = instr.ConditionTakenConstraint();
    if (taken_constr && bool(taken_constr->ChooseValue()) != mTaken) {
      LOG(fail) << "{CompressedConditionalBranchOperandConstraint::SetConditionalBranchTaken} not resolved condition taken constraint:" << taken_constr->ChooseValue() << endl;
      FAIL("unresolved-condition-taken-constraint");
    }
    LOG(notice) << "{CompressedConditionalBranchOperandConstraint::SetConditionalBranchTaken} condition branch is set to " << ((mTaken) ? "taken" : "not taken") << endl;
  }

  void FullsizeConditionalBranchOperandConstraint::SetConditionalBranchTaken(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    if (gen.SimulationEnabled()) 
    {
      // << "Simulation is enabled" << endl;
      string instr_name = instr.Name();
      
      auto branch_opr_struct = dynamic_cast<const BranchOperandStructure*>(&operandStruct);
      if (branch_opr_struct == nullptr) {
        LOG(fail) << "{FullsizeConditionalBranchOperandConstraint::Setup} expecting operand " << operandStruct.mName << " to be \"BranchOperandStructure\" type." << endl;
        FAIL("unexpected-operand-structure-type");
      }
      EBranchConditionType branch_type = branch_opr_struct->mCondition;

      //Obtain handles to operands
      bool fail_if_not_found = true;
      const Operand* src_2_opr = instr.FindOperand("rs2", fail_if_not_found);
      const Operand* src_1_opr = instr.FindOperand("rs1", fail_if_not_found);
      const RegisterFile* reg_file = gen.GetRegisterFile();

      uint64 rs2_val = 0;
      uint64 rs1_val = 0;
    
      auto rs2_ropr = dynamic_cast<const RegisterOperand* >(src_2_opr);
      string rs2_reg_name = rs2_ropr->ChoiceText();
      rs2_val = reg_file->RegisterLookup(rs2_reg_name)->Value();

      auto rs1_ropr = dynamic_cast<const RegisterOperand* >(src_1_opr);
      string rs1_reg_name = rs1_ropr->ChoiceText();
      rs1_val = reg_file->RegisterLookup(rs1_reg_name)->Value();
 
      switch(branch_type)
      {
        case EBranchConditionType::BEQ:  SetBranchTakenForBEQ(rs1_val, rs2_val); break; 
        case EBranchConditionType::BNE:  SetBranchTakenForBNE(rs1_val, rs2_val); break;
        case EBranchConditionType::BLTU: SetBranchTakenForBLTU(rs1_val, rs2_val); break; 
        case EBranchConditionType::BLT:  SetBranchTakenForBLT(rs1_val, rs2_val); break; 
        case EBranchConditionType::BGEU: SetBranchTakenForBGEU(rs1_val, rs2_val); break;         
        case EBranchConditionType::BGE:  SetBranchTakenForBGE(rs1_val, rs2_val); break;
        default:
          LOG(fail) << "{FullsizeConditionalBranchOperandConstraint::SetConditionalBranchTaken} unknown branch instruction:" << instr_name << endl;
          FAIL("unknown-branch_instruction");
      } 
    }
    auto taken_constr = instr.ConditionTakenConstraint();
    if (taken_constr && bool(taken_constr->ChooseValue()) != mTaken) {
      LOG(fail) << "{FullsizeConditionalBranchOperandConstraint::SetConditionalBranchTaken} not resolved condition taken constraint:" << taken_constr->ChooseValue() << endl;
      FAIL("unresolved-condition-taken-constraint");
    }
    LOG(notice) << "{FullsizeConditionalBranchOperandConstraint::SetConditionalBranchTaken} condition branch is set to " << ((mTaken) ? "taken" : "not taken") << endl;
  }

  void VectorRegisterOperandConstraintRISCV::Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    VectorRegisterOperandConstraint::Setup(gen, instr, operandStruct);

    if (mConstraintForced) {
      return;
    }

    if (mpConstraintSet == nullptr) {
      mpConstraintSet = DefaultConstraintSet(operandStruct);
    }

    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(instr.GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();
    auto vec_reg_operand_struct = dynamic_cast<const VectorRegisterOperandStructure*>(&operandStruct);
    uint32 reg_index_alignment = vec_layout->mRegIndexAlignment * vec_reg_operand_struct->GetLayoutMultiple();
    if (reg_index_alignment == 0) {
      LOG(fail) << "{VectorRegisterOperandConstraintRISCV::Setup} invalid register index alignment " << dec << reg_index_alignment << endl;
      FAIL("invalid-register-index-alignment");
    }

    // Notification for illegal instruction when EMUL * NFIELDS > 8 (Section 7.8)
    // TODO (Chris): Handle this case when implementing generic generation control option later
    if (vec_layout->mRegCount > 8) {
      LOG(notice) << "{VectorRegisterOperandConstraintRISCV::Setup} EMUL * NFIELDS = " << dec << vec_layout->mRegCount << " > 8" << endl;
    }
    // Removing invalid vector register choices for vd/vs3 (Section 7.8)
    for (uint32 i = 1; i < vec_layout->mRegCount; ++i) {
      SubConstraintValue(32 - i, operandStruct);
    }

    // Unaligned register indices are architecturally illegal choices
    mpConstraintSet->FilterAlignedElements(get_align_mask(reg_index_alignment));
  }

  void VectorRegisterOperandConstraintRISCV::GetAdjustedDifferValues(const Instruction& rInstr, const OperandStructure& rOperandStruct, const OperandStructure& rDifferOperandStruct, cuint64 differVal, ConstraintSet& rAdjDifferValues) const
  {
    auto vec_reg_operand_struct = dynamic_cast<const VectorRegisterOperandStructure*>(&rOperandStruct);
    auto differ_vec_reg_operand_struct = dynamic_cast<const VectorRegisterOperandStructure*>(&rDifferOperandStruct);
    uint32 layout_multiple = vec_reg_operand_struct->GetLayoutMultiple();
    uint32 differ_layout_multiple = differ_vec_reg_operand_struct->GetLayoutMultiple();

    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();
    uint32 reg_index_alignment = vec_layout->mRegIndexAlignment;

    // If the operands have the same layout multiples, they will only conflict if they have the same
    // exact value. However, if the operands have different layouts, we need to ensure that the
    // register ranges used by the operands don't overlap. For example, if this operand has a layout
    // multiple of 2, the differ operand has the value 7 and the register index alignment is 2, this
    // operand will conflict if it uses the value 4 because the register range will include
    // registers 4, 5, 6 and 7.
    if (layout_multiple == differ_layout_multiple) {
      rAdjDifferValues.AddValue(differVal);
    }
    else if (layout_multiple > differ_layout_multiple) {
      uint64 align_mask = get_align_mask(reg_index_alignment * layout_multiple);
      rAdjDifferValues.AddValue(differVal & align_mask);
    }
    else {
      uint32 range_max = differVal + (reg_index_alignment * differ_layout_multiple);
      rAdjDifferValues.AddRange(differVal, range_max);
    }
  }

}
