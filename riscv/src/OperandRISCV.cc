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
#include <VaGenerator.h>
#include <VmMapper.h>

#include <InstructionConstraintRISCV.h>
#include <OperandConstraintRISCV.h>

#include <memory>
#include <sstream>

using namespace std;

/*!
  \file OperandRISCV.cc
  \brief Code supporting RISCV specific operand generation
*/

namespace Force {

  void BaseOffsetBranchOperand::Generate(Generator& gen, Instruction& instr)
  {
    if (instr.NoRestriction()) {
      // Front end has set up the details; no need to generate through constraint solving
      BaseGenerate(gen, instr, true);
      return;
    }

    auto addr_opr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    if (not addr_opr_constr->UsePreamble()) {
      if (GenerateNoPreamble(gen, instr)) {
        LOG(info) << "{BaseOffsetBranchOperand::Generate} generated without preamble" << endl;
        return;
      }
      else if (addr_opr_constr->NoPreamble()) {
        stringstream err_stream;
        err_stream << "Operand \"" << Name() << "\" failed to generate no-preamble";
        throw OperandError(err_stream.str());
      }
      else {
        LOG(info) << "{BaseOffsetBranchOperand::Generate} switch from no-preamble to preamble" << endl;
      }
    }

    addr_opr_constr->SetUsePreamble(gen);
    GenerateWithPreamble(gen, instr);

    LOG(info) << "{BaseOffsetBranchOperand::Generate} generated with preamble" << endl;
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

  void BaseOffsetBranchOperand::UpdateNoRestrictionTarget(const Instruction& instr)
  {
    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    const ConstraintSet* target_constr = addr_constr->TargetConstraint();
    if ((target_constr != nullptr) and (target_constr->Size() == 1)) {
      mTargetAddress = target_constr->OnlyValue();
    }
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

  OperandConstraint* VectorDataTypeOperand::InstantiateOperandConstraint() const
  {
    return new VectorDataTypeOperandConstraint();
  }

  void VectorDataTypeOperand::SetDataTraits(VectorDataTraits& dataTraits) const
  {
    dataTraits.SetTraits(mChoiceText);
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

  static void calculate_data_element_size(const LoadStoreOperandStructure* lsop_struct, const string& data_type, uint32 num_registers)
  {
    if (data_type == "8B") {
      lsop_struct->SetDataSize(8 * num_registers);
      lsop_struct->SetAlignment(1);
      lsop_struct->SetElementSize(1);
    }
    else if (data_type == "4H") {
      lsop_struct->SetDataSize(8 * num_registers);
      lsop_struct->SetAlignment(2);
      lsop_struct->SetElementSize(2);
    }
    else if (data_type == "2S") {
      lsop_struct->SetDataSize(8 * num_registers);
      lsop_struct->SetAlignment(4);
      lsop_struct->SetElementSize(4);
    }
    else if (data_type == "1D") {
      lsop_struct->SetDataSize(8 * num_registers);
      lsop_struct->SetAlignment(8);
      lsop_struct->SetElementSize(8);
    }
    else if (data_type == "16B" || data_type == "B") {
      lsop_struct->SetDataSize(16 * num_registers);
      lsop_struct->SetAlignment(1);
      lsop_struct->SetElementSize(1);
    }
    else if (data_type == "8H" || data_type == "H") {
      lsop_struct->SetDataSize(16 * num_registers);
      lsop_struct->SetAlignment(2);
      lsop_struct->SetElementSize(2);
    }
    else if (data_type == "4S" || data_type == "S") {
      lsop_struct->SetDataSize(16 * num_registers);
      lsop_struct->SetAlignment(4);
      lsop_struct->SetElementSize(4);
    }
    else if (data_type == "2D" || data_type == "D") {
      lsop_struct->SetDataSize(16 * num_registers);
      lsop_struct->SetAlignment(8);
      lsop_struct->SetElementSize(8);
    }
    else {
      LOG(fail) << "unknown vector data type " << data_type << endl;
      FAIL("unknown-vector-data-type");
    }
  }

  void VectorLoadStoreOperand::Generate(Generator& gen, Instruction& instr)
  {
    auto vls_constr = mpOperandConstraint->CastInstance<VectorLoadStoreOperandConstraint>();
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();

    //TODO: if instruction contains ConstDataTypeOperand then calculate_data_size()
    calculate_data_element_size(lsop_struct, vls_constr->VectorDataTypeOperand()->ChoiceText(), vls_constr->GetMultiRegisterOperand()->NumberRegisters());
    //TODO: else just pull info from vtype register

    BaseOffsetLoadStoreOperand::Generate(gen, instr);
  }

  OperandConstraint* VectorLoadStoreOperand::InstantiateOperandConstraint() const
  {
    return new VectorLoadStoreOperandConstraint();
  }

  void RISCMultiVectorRegisterOperand::GetRegisterIndices(uint32 regIndex, ConstraintSet& rRegIndices) const
  {
    uint32 end_index = regIndex + NumberRegisters() - 1;
    if (end_index > 31) {
      rRegIndices.AddRange(regIndex, 31);
      rRegIndices.AddRange(0, end_index - 32);
    } else {
      rRegIndices.AddRange(regIndex, end_index);
    }
  }

  uint32 RISCMultiVectorRegisterOperand::NumberRegisters() const
  {
    //TODO
    //get vtype register
    //extract vlmul and compare to table in spec
    //return value in table (but for now just use 1)
    return 1;
  }

  const std::string RISCMultiVectorRegisterOperand::AssemblyText() const
  {
    stringstream out_str;

    out_str << "{ " << mChoiceText << "." << mDataType;
    for (auto extra_reg : mExtraRegisters)
      out_str << ", " << extra_reg << "." << mDataType;
    out_str << " }";

    return out_str.str();
  }

  void RISCMultiVectorRegisterOperand::SetupDataTraits(Generator& gen, Instruction& instr)
  {
    auto opr_constr_ptr = static_cast<RISCVectorRegisterOperandConstraint*>(mpOperandConstraint);
    auto instr_constr_ptr = dynamic_cast<const VectorInstructionConstraint*>(instr.GetInstructionConstraint());

    if (nullptr == instr_constr_ptr) {
      LOG(fail) << "{SetupDataTraits} expecting instruction \"" << instr.FullName() << "\" to be \"VectorInstruction\" type." << endl;
      FAIL("incorrect-instruction-type");
    }
    opr_constr_ptr->SetDataTraits(instr_constr_ptr->DataTraits());
    
    mDataType = opr_constr_ptr->DataTraits()->mTypeName;
  }

  OperandConstraint* RISCMultiVectorRegisterOperand::InstantiateOperandConstraint() const
  {
    return new VectorRegisterOperandConstraint();
  }

  const std::string RISCMultiVectorRegisterOperand::GetNextRegisterName(uint32& indexVar) const
  {
    ++indexVar;
    if (indexVar > 31) indexVar = 0;
    return "V" + indexVar;
  }

  ChoicesFilter* RISCMultiVectorRegisterOperand::GetChoicesFilter(const ConstraintSet* pConstrSet) const
  {
    return new ConstraintChoicesFilter(pConstrSet);
  }

}
