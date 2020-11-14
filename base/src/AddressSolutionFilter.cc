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
#include <AddressSolutionFilter.h>
#include <Generator.h>
#include <AddressSolver.h>
#include <Operand.h>
#include <ResourceDependence.h>
#include <InstructionStructure.h>
#include <Instruction.h>
#include <InstructionConstraint.h>
#include <Constraint.h>
#include <Register.h>
#include <Log.h>
#include <ChoicesModerator.h>
#include <Choices.h>
#include <memory>

using namespace std;

namespace Force {

  const string AddressSolutionFilter::ToString() const
  {
    return Type();
  }

  void AddressSolutionFilter::Setup(const Generator* pGen)
  {
    mpGenerator = pGen;
  }

  void BaseDependencyFilter::Setup(const Generator* pGen)
  {
    AddressSolutionFilter::Setup(pGen);
  }

  bool BaseDependencyFilter::FilterSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const
  {
    vector<AddressingMode* >& ref_solutions = rAddrSolver.GetSolutionChoicesForFiltering();
    rRemainder = ref_solutions.size();
    // << "solution size: " << dec << rRemainder << endl;
    if (1 == rRemainder) {
      return false; // only one choice, no need to continue.
    }

    const RegisterOperand* reg_opr_ptr = rAddrSolver.GetBaseOperand();
    auto opr_struct = reg_opr_ptr->GetOperandStructure();
    if (EOperandType::VECREG ==opr_struct->mType) { //TBD: handle vector Base-imm dependence
      return false;
    }

    const ConstraintSet* dep_constr = nullptr;
    EResourceType res_type = EResourceType(0);
    if (mpGenerator->OperandTypeToResourceType(opr_struct->mType, res_type)) {
      dep_constr = mpGenerator->GetDependenceInstance()->GetDependenceConstraint(opr_struct->mAccess, res_type, rInstr.GetInstructionConstraint()->GetHotResource());
    }

    if (nullptr == dep_constr) {
      return false; // no filtering, no change.
    }

    if (dep_constr->IsEmpty()) {
      LOG(fail) << "{BaseDependencyFilter::FilterSolutions} empty constraint returned from dependence module." << endl;
      FAIL("empty-dependence-constraint");
    }

    vector<AddressingMode* >& ref_filtered = rAddrSolver.GetFilteredChoicesForFiltering();

    // << "dependence constraint: " << dep_constr->ToSimpleString() << " sol size: " << dec << ref_solutions.size() << " filtered size: " << ref_filtered.size() << endl;
    //uint32 original_size = ref_solutions.size() + ref_filtered.size();
    for (vector<AddressingMode* >::iterator s_iter = ref_solutions.begin(); s_iter != ref_solutions.end();) {
      auto addr_item = (*s_iter);
      if (dep_constr->ContainsValue(addr_item->GetRegister()->IndexValue())) {
        ++ s_iter;
      }
      else {
        s_iter = ref_solutions.erase(s_iter);
        ref_filtered.push_back(addr_item); // put filtered items into this vector.
        -- rRemainder;
      }
    }
    // << "num_solution remaining " << dec << rRemainder << endl;
    // uint32 new_size =  ref_solutions.size() + ref_filtered.size();
    //if (original_size != new_size) {
    //  LOG(fail) << "{BaseDependencyFilter::FilterSolutions} new size (" << dec << new_size << ") != orginal zie (" << original_size << ")." << endl;
    //  FAIL("filter-size-mismatch");
    //}

    return true;
  }

  void IndexDependencyFilter::Setup(const Generator* pGen)
  {
    AddressSolutionFilter::Setup(pGen);
  }

  bool IndexDependencyFilter::FilterSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const
  {
    const ConstraintSet* dep_constr = GetIndexDependenceConstraint(rAddrSolver, rInstr);
    if (nullptr == dep_constr) {
      return false; // No filtering, no change.
    }

    if (dep_constr->IsEmpty()) {
      LOG(fail) << "{IndexDependencyFilter::FilterSolutions} empty constraint returned from dependence module." << endl;
      FAIL("empty-dependence-constraint");
    }

    const vector<AddressingMode* >& ref_solutions = rAddrSolver.GetSolutionChoicesForFiltering();
    rRemainder = 0;
    for (AddressingMode* addr_item : ref_solutions) {
      auto base_index_mode = dynamic_cast<BaseIndexMode*>(addr_item);
      vector<IndexSolution* >& ref_index_solutions = base_index_mode->GetSolutionChoicesForFiltering();
      rRemainder += ref_index_solutions.size();

      if (1 == ref_index_solutions.size()) {
        continue; // Only one choice, no need to filter further.
      }

      vector<IndexSolution* >& ref_filtered = base_index_mode->GetFilteredChoicesForFiltering();
      for (vector<IndexSolution* >::iterator s_iter = ref_index_solutions.begin(); s_iter != ref_index_solutions.end();) {
        auto index_solution = (*s_iter);

        if (dep_constr->ContainsValue(index_solution->GetRegister()->IndexValue())) {
          ++s_iter;
        }
        else {
          s_iter = ref_index_solutions.erase(s_iter);
          ref_filtered.push_back(index_solution);
          --rRemainder;
        }
      }
    }

    return true;
  }

  const ConstraintSet* IndexDependencyFilter::GetIndexDependenceConstraint(const AddressSolver& rAddrSolver, const Instruction& rInstr) const
  {
    const AddressingOperand* reg_opr_ptr = rAddrSolver.GetAddressingOperand();
    const OperandStructure* opr_struct = reg_opr_ptr->GetOperandStructure();
    const auto lsop_struct = opr_struct->CastOperandStructure<LoadStoreOperandStructure>();

    const Operand* index_opr_ptr = reg_opr_ptr->MatchOperand(lsop_struct->Index());
    const OperandStructure* index_opr_struct = index_opr_ptr->GetOperandStructure();

    EResourceType res_type = EResourceType(0);
    const ConstraintSet* dep_constr = nullptr;
    if (mpGenerator->OperandTypeToResourceType(index_opr_struct->mType, res_type)) {
      dep_constr = mpGenerator->GetDependenceInstance()->GetDependenceConstraint(index_opr_struct->mAccess, res_type, rInstr.GetInstructionConstraint()->GetHotResource());
    }

    return dep_constr;
  }

  void SpAlignmentFilter::Setup(const Generator* pGen)
  {
    AddressSolutionFilter::Setup(pGen);
    auto choices_tree = mpGenerator->GetChoicesModerator(EChoicesType::OperandChoices)->CloneChoiceTree("SP alignment");
    std::unique_ptr<ChoiceTree> choices_tree_storage(choices_tree);

    auto choice_ptr = choices_tree->Choose();
    uint32 available_choices = choices_tree->AvailableChoices();

    mPreventHard = available_choices == 1;
    mUnalign = choice_ptr->Value() == 0;
  }

  static bool check_sp_alignment(uint64 base_value, bool ref_unalign)
  {
    bool unalign = base_value & 0xF;
    return unalign == ref_unalign;
  }

  bool SpAlignmentFilter::FilterIndexSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const
  {
    vector<AddressingMode* >& ref_solutions = rAddrSolver.GetSolutionChoicesForFiltering();
    bool any_filtered = false;
    rRemainder = 0;
    for (auto s_iter = ref_solutions.begin(); s_iter != ref_solutions.end(); ++ s_iter)
    {
      if (!(dynamic_cast<BaseIndexMode*>(*s_iter))) { return false; } // TBD: BaseVectorIndexExtendMode
      auto base_index_mode = dynamic_cast<BaseIndexMode*>(*s_iter);
      vector<IndexSolution* >& ref_index_solutions = base_index_mode->GetSolutionChoicesForFiltering();
      rRemainder += ref_index_solutions.size();
      const Register* base_reg = base_index_mode->Base();
      if (base_reg->RegisterType() != ERegisterType::SP) continue; // next solution.

      vector<IndexSolution* >& ref_index_filtered = base_index_mode->GetFilteredChoicesForFiltering();
      for (auto i_iter = ref_index_solutions.begin(); i_iter != ref_index_solutions.end();)
      {
        auto index_solution = (*i_iter);
        if (check_sp_alignment(index_solution->BaseValue(), mUnalign))
        {
          LOG(info) << "{SpAlignmentFilter::FilterSolutions} Filter the SP index solutions " << index_solution->ToString() << endl;
          any_filtered = true;
          ref_index_filtered.push_back(index_solution); // put filtered items into this vector.
          i_iter = ref_index_solutions.erase(i_iter);
          -- rRemainder;
        }
        else // next index solution
        {
          ++ i_iter;
        }
      }
    }

    vector<AddressingMode* >& ref_filtered = rAddrSolver.GetFilteredChoicesForFiltering();
    for (auto s_iter = ref_solutions.begin(); s_iter != ref_solutions.end();)
    {
      auto base_index_mode = dynamic_cast<BaseIndexMode*>(*s_iter);
      vector<IndexSolution* >& ref_index_solutions = base_index_mode->GetSolutionChoicesForFiltering();
      if (0 == ref_index_solutions.size())
      {
        LOG(warn) << "{SpAlignmentFilter::FilterSolutions} Filter the SP solutions " << base_index_mode->ToString() << endl;
        ref_filtered.push_back(base_index_mode); // put filtered items into this vector.
        s_iter = ref_solutions.erase(s_iter);
      }
      else // next solution.
      {
        ++ s_iter;
      }
    }
    return any_filtered;
  }

  bool SpAlignmentFilter::FilterOffsetSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const
  {
    vector<AddressingMode* >& ref_solutions = rAddrSolver.GetSolutionChoicesForFiltering();
    rRemainder = ref_solutions.size();
    bool any_filtered = false;

    vector<AddressingMode* >& ref_filtered = rAddrSolver.GetFilteredChoicesForFiltering();
    for (vector<AddressingMode* >::iterator s_iter = ref_solutions.begin(); s_iter != ref_solutions.end();)
    {
      AddressingMode* addr_item = *s_iter;
      const Register* base_reg = addr_item->Base();
      if ((base_reg->RegisterType() == ERegisterType::SP) and check_sp_alignment(addr_item->BaseValue(), mUnalign))
      {
        LOG(warn) << "{SpAlignmentFilter::FilterSolutions} Filter the SP solutions " << addr_item->ToString() << endl;
        any_filtered = true;
        ref_filtered.push_back(addr_item);
        s_iter = ref_solutions.erase(s_iter);
        -- rRemainder;
      }
      else // next solution.
      {
        ++ s_iter;
      }
    }
    return any_filtered;
  }

  bool SpAlignmentFilter::FilterSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const
  {
    vector<AddressingMode* >& ref_solutions = rAddrSolver.GetSolutionChoicesForFiltering();
    rRemainder = ref_solutions.size();

    if (not mPreventHard) {  // Instruction has hard constraint.
      mPreventHard = rInstr.AlignedSP();
      mUnalign = false;
    }
    if (0 == rRemainder or not mPreventHard) { // Only it is work when hard prevent is true.
      return false;
    }

    if (mUnalign and rInstr.AlignedSP()) {
      LOG(fail) << "instruction and operand constraints collision" << endl;
      FAIL("instruction-and-operand-constraints-collision");
    }

    bool filtered = false;
    AddressingMode* address_tmplate = ref_solutions[0];
    if (address_tmplate->ShouldApplyIndexFilters()) {
      filtered = FilterIndexSolutions(rAddrSolver, rInstr, rRemainder);
    }
    else {
      filtered = FilterOffsetSolutions(rAddrSolver, rInstr, rRemainder);
    }
    return filtered;
  }
}
