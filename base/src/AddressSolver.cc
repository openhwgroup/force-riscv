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
#include <Defines.h>
#include <AddressSolver.h>
#include <Generator.h>
#include <Instruction.h>
#include <Operand.h>
#include <OperandConstraint.h>
#include <Register.h>
#include <Choices.h>
#include <VmMapper.h>
#include <VmManager.h>
#include <Constraint.h>
#include <RandomUtils.h>
#include <AddressSolutionFilter.h>
#include <AddressSolvingShared.h>
#include <AddressTagging.h>
#include <BaseOffsetConstraint.h>
#include <AluImmediateConstraint.h>
#include <Random.h>
#include <AddressFilteringRegulator.h>
#include <Log.h>
#include <OperandSolution.h>
#include <AddressSolutionStrategy.h>
#include <InstructionStructure.h>
#include <GenMode.h>

#include <algorithm>
#include <memory>
#include <sstream>

using namespace std;

/*!
  \file AddressSolver.cc
  \brief Code handling various addressing solving.
*/

namespace Force {

  AddressingRegister::AddressingRegister()
    : Object(), mpRegister(nullptr), mRegisterValue(0), mRegisterValues(), mWeight(0), mVmTimeStamp(0), mFree(false)
  {
  }

  AddressingRegister::AddressingRegister(const AddressingRegister& rOther)
    : Object(rOther), mpRegister(rOther.mpRegister), mRegisterValue(rOther.mRegisterValue),mRegisterValues(rOther.mRegisterValues), mWeight(rOther.mWeight), mVmTimeStamp(rOther.mVmTimeStamp), mFree(rOther.mFree)
  {
  }

  Object* AddressingRegister::Clone() const
  {
    return new AddressingRegister(*this);
  }

  const std::string AddressingRegister::ToString() const
  {
    stringstream out_str;
    out_str << "register: " << mpRegister->Name() << " value: 0x" << hex << mRegisterValue << " weight: " << dec << mWeight << " time-stamp: " << mVmTimeStamp << " free? " << mFree;
    return out_str.str();
  }

  bool AddressingRegister::IsNonSystemRegisterOperand(const Operand& rOpr) const
  {
    return (rOpr.IsRegisterOperand() && (rOpr.OperandType() != EOperandType::SysReg));
  }

  AddressingMode::AddressingMode()
    : AddressingRegister(), mTargetAddress(0)
  {
  }

  AddressingMode::AddressingMode(const AddressingMode& rOther)
    : AddressingRegister(rOther), mTargetAddress(0)
  {
  }

  bool AddressingMode::BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const
  {
    return pAddrSolShared->AlignmentOkay(baseValue);
  }

  AddressSolvingShared* AddressingMode::AddressSolvingSharedInstance() const
  {
    return new AddressSolvingShared();
  }

  void AddressingMode::SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const
  {
    Generator* gen_ptr = rShared.GetGenerator();
    Instruction* instr_ptr = rShared.GetInstruction();
    const Register* base_reg = GetRegister();
    rBaseOperand.SetChoiceResultDirect(*gen_ptr, *instr_ptr, base_reg->Name());

    if (IsFree()) {
      instr_ptr->SetOperandDataValue(rBaseOperand.Name(), BaseValue(), base_reg->Size());
    }
  }

  bool AddressingMode::SolveWithValue(uint64 value, const AddressSolvingShared& rShared, const ConstraintSet* pTargetConstr, uint64& rTargetAddr) const
  {
    auto addr_opr_constr = rShared.GetAddressingOperandConstraint();

    ConstraintSet value_constr;
    addr_opr_constr->GetBaseConstraint(value, MAX_UINT64, rShared.Size(), value_constr);
    rShared.ApplyVirtualUsableConstraint(&value_constr, mVmTimeStamp);
    if (value_constr.IsEmpty()) return false;

    value_constr.SubConstraintSet(*(rShared.PcConstraint()));
    if (value_constr.IsEmpty()) return false;

    value_constr.AlignWithSize(rShared.AlignMask(), rShared.Size());
    if (value_constr.IsEmpty()) return false;

    if (pTargetConstr != nullptr) {
      value_constr.ApplyConstraintSet(*pTargetConstr);
    }
    if (value_constr.IsEmpty()) return false;

    uint32 align_shift = rShared.AlignShift();
    value_constr.ShiftRight(align_shift);
    rTargetAddr = value_constr.ChooseValue() << align_shift;
    return true;
  }

  bool AddressingMode::SolveWithBase(cuint64 baseValue, const AddressSolvingShared& rShared, const BaseOffsetConstraint& rBaseOffsetConstr, const ConstraintSet* pTargetConstr, uint64& rTargetAddr) const
  {
    const AddressTagging* addr_tagging = rShared.GetAddressTagging();
    uint64 untagged_base_value = addr_tagging->UntagAddress(baseValue, rShared.IsInstruction());

    ConstraintSet target_addr_constr;
    rBaseOffsetConstr.GetConstraint(untagged_base_value, rShared.Size(), nullptr, target_addr_constr);

    rShared.ApplyVirtualUsableConstraint(&target_addr_constr, mVmTimeStamp);
    if (target_addr_constr.IsEmpty()) return false;

    target_addr_constr.SubConstraintSet(*(rShared.PcConstraint()));
    if (target_addr_constr.IsEmpty()) return false;

    //apply target constraint. Hard constraint also need to be checked.
    if (pTargetConstr != nullptr) {
      target_addr_constr.ApplyConstraintSet(*pTargetConstr);
      if (target_addr_constr.IsEmpty()) return false;
    }

    bool solve_result = ChooseTargetAddress(rShared, target_addr_constr, rTargetAddr);

    uint64 base_value_tag_value = addr_tagging->GetTagValue(baseValue);
    rTargetAddr = addr_tagging->TagAddress(rTargetAddr, base_value_tag_value, rShared.IsInstruction());
    // If the the base register value can be tagged and the target address cannot (or vice versa), this solution won't
    // work, so reject it.
    if (!addr_tagging->AreTaggingCapacitiesEqual(rTargetAddr, baseValue, rShared.IsInstruction())) {
      solve_result = false;
    }
    LOG(info) << "{AddressingMode::SolveWithBase} target=0x" << hex << rTargetAddr << " base=0x" << baseValue << endl;
    return solve_result;
  }

  const string AddressingMode::ToString() const
  {
    stringstream out_str;
    out_str << Type() << ": target-address: 0x" << hex << mTargetAddress << " base " << AddressingRegister::ToString();
    return out_str.str();
  }

  bool AddressingMode::ChooseTargetAddress(const AddressSolvingShared& rShared, ConstraintSet& rBaseOffsetConstr, uint64& rTargetAddr) const
  {
    rBaseOffsetConstr.AlignWithSize(rShared.AlignMask(), rShared.Size());
    if (rBaseOffsetConstr.IsEmpty()) return false;

    uint32 align_shift = rShared.AlignShift();
    rBaseOffsetConstr.ShiftRight(align_shift);

    rTargetAddr = rBaseOffsetConstr.ChooseValue() << align_shift;

    return true;
  }

  Object* BaseOnlyMode::Clone() const
  {
    return new BaseOnlyMode(*this);
  }

  bool BaseOnlyMode::SolveFree(const AddressSolvingShared& rShared)
  {
    bool solve_result = false;
    if (rShared.SolveFree()) {
      mTargetAddress = rShared.FreeTarget();
      mRegisterValue = mTargetAddress;
      solve_result = true;
    }
    return solve_result;
  }

  bool BaseOnlyMode::Solve(const AddressSolvingShared& rShared)
  {
    uint64 base_value = BaseValue();
    const AddressTagging* addr_tagging = rShared.GetAddressTagging();
    uint64 untagged_base_value = addr_tagging->UntagAddress(base_value, rShared.IsInstruction());
    auto target_constr = rShared.TargetConstraint();
    if ((nullptr != target_constr) and (1 == target_constr->Size())) { // target forced
      if (target_constr->OnlyValue() == untagged_base_value) {
        mTargetAddress = base_value;
        return true;
      } else {
        return false;
      }
    }

    bool solve_result = SolveWithValue(untagged_base_value, rShared, target_constr, mTargetAddress);

    uint64 base_value_tag_value = addr_tagging->GetTagValue(base_value);
    mTargetAddress = addr_tagging->TagAddress(mTargetAddress, base_value_tag_value, rShared.IsInstruction());
    return solve_result;
  }

  BaseOnlyAlignedMode::BaseOnlyAlignedMode(uint64 baseAlign)
    : BaseOnlyMode(), mBaseMask(0)
  {
    mBaseMask = ~(baseAlign - 1);
    // << "{BaseOnlyAlignedMode::BaseOnlyAlignedMode} base mask is 0x" << hex << mBaseMask << endl;
  }

  Object* BaseOnlyAlignedMode::Clone() const
  {
    return new BaseOnlyAlignedMode(*this);
  }

  bool BaseOnlyAlignedMode::Solve(const AddressSolvingShared& rShared)
  {
    uint64 base_value = BaseValue();

    const AddressTagging* addr_tagging = rShared.GetAddressTagging();
    uint64 untagged_base_value = addr_tagging->UntagAddress(base_value, rShared.IsInstruction());

    auto target_constr = rShared.TargetConstraint();
    if (nullptr != target_constr) {
      if (target_constr->Size() == 1) {
        // target forced.
        if (target_constr->ContainsValue(untagged_base_value)) {
          mTargetAddress = base_value & rShared.AlignMask();
          return true;
        }
        else
          return false;
      }
    }

    bool solve_result = SolveWithValue(untagged_base_value & rShared.AlignMask(), rShared, target_constr, mTargetAddress);

    uint64 base_value_tag_value = addr_tagging->GetTagValue(base_value);
    mTargetAddress = addr_tagging->TagAddress(mTargetAddress, base_value_tag_value, rShared.IsInstruction());

    return solve_result;
  }

  uint32 BaseOnlyAlignedMode::GetRandomLowerBits(const AddressSolvingShared& rShared) const
  {
    uint32 offset_bits = ~uint32(mBaseMask & rShared.AlignMask());
    uint32 random_bits = Random::Instance()->Random32(0, offset_bits);
    return random_bits;
  }

  bool BaseOnlyAlignedMode::SolveFree(const AddressSolvingShared& rShared)
  {
    bool solve_result = false;
    if (rShared.IsInstruction() and rShared.SolveFree()) {
      mRegisterValue = rShared.FreeTarget();
      mTargetAddress = mRegisterValue & rShared.AlignMask();
      if ((nullptr == rShared.TargetConstraint()) and (mBaseMask != rShared.AlignMask())) {
        // need to randomize lower bits.
        uint32 random_bits = GetRandomLowerBits(rShared);
        mRegisterValue |= random_bits;
        // << "or with random bits: 0x" << hex << random_bits << " register value: 0x" << mRegisterValue << endl;
      }
      solve_result = true;
      // << "register value: 0x" << hex << mRegisterValue << " target address 0x" << mTargetAddress << " base mask 0x" << mBaseMask <<" cannonical mask 0x" << rShared.AlignMask() << endl;
    }
    return solve_result;
  }

  AddressSolvingShared* RegisterBranchMode::AddressSolvingSharedInstance() const
  {
    return new RegisterBranchSolvingShared();
  }

  bool UnalignedRegisterBranchMode::BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const
  {
    uint64 aligned_value = baseValue & pAddrSolShared->AlignMask();
    // << "base value 0x" << hex << baseValue << " align mask 0x" << pAddrSolShared->AlignMask() << " aligned value 0x" << aligned_value << endl;
    return (baseValue != aligned_value); // We want value that is not aligned.
  }

  uint32 UnalignedRegisterBranchMode::GetRandomLowerBits(const AddressSolvingShared& rShared) const
  {
    uint32 offset_bits = ~rShared.AlignMask();
    uint32 random_bits = Random::Instance()->Random32(1, offset_bits);
    return random_bits;
  }

  Object* BaseOffsetMode::Clone() const
  {
    return new BaseOffsetMode(*this);
  }

  bool BaseOffsetMode::Solve(const AddressSolvingShared& rShared)
  {
    //get offset operand
    auto addr_opr_constr = rShared.GetAddressingOperandConstraint();
    auto offset_opr = addr_opr_constr->OffsetOperand();
    auto offset_constr = offset_opr->GetOperandConstraint();

    if (offset_constr->HasConstraint()) {
      return SolveOffsetHasConstraint(&rShared);
    }

    bool isOffsetShift = true;
    if (dynamic_cast<BaseOffsetMulMode*> (this)) {isOffsetShift = false;}
    BaseOffsetConstraint bo_constr_builder(offset_opr->BaseValue(), offset_opr->Size(), OffsetScale(), MAX_UINT64, isOffsetShift);

    return SolveWithBase(BaseValue(), rShared, bo_constr_builder, rShared.TargetConstraint(), mTargetAddress);
  }

  bool BaseOffsetMode::SolveOffsetHasConstraint(const AddressSolvingShared* rShared) {
    auto offset_value = rShared->FreeOffset(this);
    uint64 target = BaseValue() + offset_value;
    const AddressTagging* addr_tagging = rShared->GetAddressTagging();
    uint64 untagged_target = addr_tagging->UntagAddress(target, rShared->IsInstruction());
    if (not rShared->AlignmentOkay(untagged_target)) {
      LOG(trace) << "{BaseOffsetMode::SolveOffsetHasConstraint} target=0x"<<hex<<target<<" is unalign!"<<" base=0x"<<BaseValue() << endl;
      return false;
    }
    bool solve_result = SolveWithValue(untagged_target, *rShared, rShared->TargetConstraint(), mTargetAddress);
    uint64 target_tag_value = addr_tagging->GetTagValue(target);
    mTargetAddress = addr_tagging->TagAddress(mTargetAddress, target_tag_value, rShared->IsInstruction());
    if ((!solve_result) || (!addr_tagging->AreTaggingCapacitiesEqual(mTargetAddress, target, rShared->IsInstruction()))) {
      LOG(trace) << "{BaseOffsetMode::SolveOffsetHasConstraint} target=0x"<<hex<<target<<" untag_target=0x"<<untagged_target<<" solve_result="<<(uint32)solve_result<<" base=0x"<<BaseValue()<< endl;
      return false;
    }
    LOG(info)<<"{BaseOffsetMode::SolveOffsetHasConstraint} target=0x"<<hex<<mTargetAddress<<" base=0x"<<BaseValue()<<" offset=0x"<<offset_value<<endl;
    return true;
  }

  bool BaseOffsetMode::SolveFree(const AddressSolvingShared& rShared)
  {
    //Target constraint has been included to flow of rShared.SolveFree()
    if (not rShared.SolveFree()) {
      return false;
    }
    mTargetAddress = rShared.FreeTarget();
    auto offset_value = rShared.FreeOffset(this);
    mRegisterValue = mTargetAddress - offset_value;
    LOG(info)<<"{BaseOffsetMode::SolveFree}" << " base=0x" << hex << mRegisterValue << " target=0x" << mTargetAddress << " offset=0x" << hex << offset_value << endl;
    return true;
  }

  AddressSolvingShared* BaseOffsetMode::AddressSolvingSharedInstance() const
  {
    return new BaseOffsetSolvingShared();
  }

  void BaseOffsetMode::SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const
  {
    AddressingMode::SetOperandResults(rShared, rBaseOperand);
      LOG(info)<<" offset"<<endl;
    if (IsFree()) {
      return; // If register is free, don't need to call operand set value again here.
    }
    const AddressingOperandConstraint* addr_opr_constr = rShared.GetAddressingOperandConstraint();
    ImmediateOperand* offset_opr = addr_opr_constr->OffsetOperand();
    if (nullptr != offset_opr) {
      uint32 offset_value = calculate_offset_value(TargetAddress() - BaseValue(), OffsetScale(), offset_opr->Mask());
      offset_opr->SetValue(offset_value);
      LOG(info)<<" offset=0x"<<hex<<offset_value<<endl;
    }
  }

  BaseOffsetShiftMode::BaseOffsetShiftMode(uint32 offsetScale)
    : BaseOffsetMode(), mOffsetScale(offsetScale)
  {
  }

  Object* BaseOffsetShiftMode::Clone() const
  {
    return new BaseOffsetShiftMode(*this);
  }

  bool BaseOffsetShiftMode::BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const
  {
    return AddressingMode::BaseValueUsable(baseValue, pAddrSolShared);
  }

  AddressSolvingShared* BaseOffsetShiftMode::AddressSolvingSharedInstance() const
  {
    return new BaseOffsetShiftSolvingShared(mOffsetScale);
  }

  bool BaseOffsetShiftMode::ChooseTargetAddress(const AddressSolvingShared& rShared, ConstraintSet& rBaseOffsetConstr, uint64& rTargetAddr) const
  {
    const auto& base_offset_scale_shared = dynamic_cast<const BaseOffsetShiftSolvingShared&>(rShared);

    uint64 align_offset = mRegisterValue & ~(base_offset_scale_shared.ScaleAlignMask());
    rBaseOffsetConstr.AlignOffsetWithSize(base_offset_scale_shared.ScaleAlignMask(), align_offset, base_offset_scale_shared.Size());
    if (rBaseOffsetConstr.IsEmpty()) return false;

    rBaseOffsetConstr.ShiftRight(base_offset_scale_shared.ScaleAlignShift());
    rTargetAddr = (rBaseOffsetConstr.ChooseValue() << base_offset_scale_shared.ScaleAlignShift()) + align_offset;

    return true;
  }
  uint32 BaseOffsetShiftMode::calculate_offset_value(uint64 offset, uint32 scale, uint32 mask) const
  {
    uint64 offset_shifted = offset >> scale;
    uint64 offset_back = (offset_shifted << scale);
    if (offset_back != offset) {
      LOG(fail) << "{calculate_offset_value} not proper offset: 0x" << hex << offset << " with scale value: 0x" << scale << endl;
      FAIL("not-proper-offset");
    }

    return (offset_shifted & mask);
  }

  Object* BaseOffsetMulMode::Clone() const
  {
    return new BaseOffsetMulMode(*this);
  }
  bool BaseOffsetMulMode::BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const
  {
    return AddressingMode::BaseValueUsable(baseValue, pAddrSolShared);
  }
  AddressSolvingShared* BaseOffsetMulMode::AddressSolvingSharedInstance() const
  {
    return new BaseOffsetMulSolvingShared(mOffsetMulData);
  }

  uint32 BaseOffsetMulMode::calculate_offset_value(uint64 offset, uint32 scale, uint32 mask) const
  {
    uint64 offset_dev= (int64)offset/(int64)scale;
    uint64 offset_back = (offset_dev *scale);
    if (offset_back != offset) {
      LOG(fail) << "{BaseOffsetMulMode::calculate_offset_value} not proper offset: 0x" << hex << offset << " with scale value: 0x" << scale << endl;
      FAIL("not-proper-offset");
    }

    return (offset_dev & mask);
  }

  bool BaseOffsetMulMode::ChooseTargetAddress(const AddressSolvingShared& rShared, ConstraintSet& rBaseOffsetConstr, uint64& rTargetAddr) const
  {
    const auto& base_offset_scale_shared = dynamic_cast<const BaseOffsetMulSolvingShared&>(rShared);

    const AddressTagging* addr_tagging = rShared.GetAddressTagging();
    uint64 untagged_base_value = addr_tagging->UntagAddress(mRegisterValue, rShared.IsInstruction());
    uint64 remainder_base = untagged_base_value % (base_offset_scale_shared.OffsetMulData());
    LOG(trace)<<"{BaseOffsetMulMode::ChooseTargetAddress} constraint="<<rBaseOffsetConstr.ToString()<<" reg_value="<<mRegisterValue<<endl;

    rBaseOffsetConstr.AlignMulDataWithSize(base_offset_scale_shared.OffsetMulData(), remainder_base, base_offset_scale_shared.Size());
    if (rBaseOffsetConstr.IsEmpty()) return false;

    rBaseOffsetConstr.DivideMulData(base_offset_scale_shared.OffsetMulData(),remainder_base,rShared.AlignShift());
    if (rBaseOffsetConstr.IsEmpty()) return false;

    uint64 choosevalue = rBaseOffsetConstr.ChooseValue();
    rTargetAddr = (choosevalue * base_offset_scale_shared.OffsetMulData()) + remainder_base;
    LOG(trace)<<"{BaseOffsetMulMode::ChooseTargetAddress} remainder_base="<<remainder_base<<" choosevalue="<<choosevalue<<" TargetAddress="<<rTargetAddr<<endl;
    return true;
  }

  BaseIndexMode::BaseIndexMode()
    : AddressingMode(), mpChosenIndexSolution(nullptr), mIndexSolutionChoices(), mFilteredChoices()
  {
  }

  BaseIndexMode::BaseIndexMode(const BaseIndexMode& rOther)
    : AddressingMode(rOther), mpChosenIndexSolution(nullptr), mIndexSolutionChoices(), mFilteredChoices()
  {
    for (IndexSolution* index_solution : rOther.mIndexSolutionChoices) {
      auto index_solution_clone = dynamic_cast<IndexSolution*>(index_solution->Clone());
      mIndexSolutionChoices.push_back(index_solution_clone);

      if (index_solution == rOther.mpChosenIndexSolution) {
        mpChosenIndexSolution = index_solution_clone;
      }
    }

    for (IndexSolution* index_solution : rOther.mFilteredChoices) {
      auto index_solution_clone = dynamic_cast<IndexSolution*>(index_solution->Clone());
      mFilteredChoices.push_back(index_solution_clone);
    }
  }

  BaseIndexMode::~BaseIndexMode()
  {
    for (IndexSolution* index_solution : mIndexSolutionChoices) {
      delete index_solution;
    }

    for (IndexSolution* index_solution: mFilteredChoices) {
      delete index_solution;
    }
  }

  const string BaseIndexMode::ToString() const
  {
    stringstream out_str;
    out_str << Type() << ": IndexSolutionChoices size 0x" << hex << mIndexSolutionChoices.size();
    return out_str.str();
  }

  bool BaseIndexMode::ChooseSolution(const AddressSolvingShared& rAddrSolShared)
  {
    IndexSolution* index_solution = nullptr;
    bool choice_usable = false;
    uint64 remaining_choices_count = mIndexSolutionChoices.size();
    while ((not choice_usable) and (remaining_choices_count > 0)) {
      index_solution = choose_weighted_item(mIndexSolutionChoices);

      if (index_solution == nullptr) {
        break;
      }

      if (rAddrSolShared.MapTargetAddressRange(index_solution->TargetAddress(), index_solution->VmTimeStampReference())) {
        choice_usable = true;
      }
      else {
        index_solution->SetWeight(0);
        remaining_choices_count--;
      }
    }

    if (choice_usable) {
      mpChosenIndexSolution = index_solution;
      mRegisterValue = mpChosenIndexSolution->BaseValue();
      mTargetAddress = mpChosenIndexSolution->TargetAddress();
    }

    RemoveUnusableSolutionChoices();

    return choice_usable;
  }

  void BaseIndexMode::SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const
  {
    AddressingMode::SetOperandResults(rShared, rBaseOperand);

    RegisterOperand* index_opr = GetIndexOperand(rShared);
    Instruction* instr = rShared.GetInstruction();
    const Register* index_reg = Index();
    index_opr->SetChoiceResultDirect(*(rShared.GetGenerator()), *instr, index_reg->Name());

    if (mpChosenIndexSolution->IsFree()) {
      instr->SetOperandDataValue(index_opr->Name(), IndexValue(), index_reg->Size());
    }
  }

  void BaseIndexMode::RemoveUnusableSolutionChoices()
  {
    auto delete_if_zero_weight = [this](IndexSolution* index_solution) {
      if (index_solution->Weight() == 0) {
        if (index_solution != mpChosenIndexSolution) {
          delete index_solution;
        }
        else {
          LOG(fail) << "{BaseIndexMode::RemoveUnusableSolutionChoices} about to delete last known good chosen solution: " << index_solution->ToString() << ". This shouldn't happen." << endl;
          FAIL("deleting-will-cause-dangling-pointer");
        }

        return true;
      }

      return false;
    };

    // remove_if will shift the valid elements to the beginning of the vector while erase will
    // remove the invalid elements, which have been shifted to the end of the vector
    mIndexSolutionChoices.erase(remove_if(mIndexSolutionChoices.begin(), mIndexSolutionChoices.end(), delete_if_zero_weight), mIndexSolutionChoices.end());
  }

  bool BaseIndexExtendMode::Solve(const AddressSolvingShared& rShared)
  {
    const auto& rSharedBI= dynamic_cast<const BaseIndexSolvingShared&>(rShared);
    if ((nullptr != rSharedBI.TargetConstraint()) and (1 == rSharedBI.TargetConstraint()->Size())) {
        SolveTargetHasConstraint(rSharedBI, 1);
        return HasIndexSolutions();
    }
    //normal solve
    SolveWithAmountBit(rSharedBI, 1);
    return HasIndexSolutions();
  }

  bool BaseIndexExtendMode::SolveFree(const AddressSolvingShared& rShared)
  {
    //Target constraint has been included to the flow of rShared.SolveFree()
    if (not rShared.SolveFree()) {
      return false;
    }
    const auto& rSharedBI = dynamic_cast<const BaseIndexSolvingShared&>(rShared);
    const vector<AddressingRegister* >& index_choices = rSharedBI.GetIndexChoices();
    for (AddressingRegister* index_choice : index_choices) {
      uint64 index_value = 0;
      if (index_choice->GetRegister()->IndexValue() != GetRegister()->IndexValue()) {
        if (index_choice->IsFree()) {
          const Register* index_ptr = index_choice->GetRegister();
          index_value = index_ptr->ReloadValue();
        } else {
          index_value = index_choice->RegisterValue();
        }
      } else {
        // TODO(Noah): Implement solving for the case when the base and index registers are the same register, as soon
        // as the other cases have been handled and a solution for this case can be devised.
        continue;
      }
      CreateFreeBaseIndexSolution(rSharedBI, *index_choice, index_value);
    }
    return HasIndexSolutions();
  }

  AddressSolvingShared* BaseIndexExtendMode::AddressSolvingSharedInstance() const
  {
    return new BaseIndexSolvingShared();
  }

  void BaseIndexExtendMode::SolveWithAmountBit(const BaseIndexSolvingShared& rSharedBI, uint32 amountBit)
  {
    uint32 extend_amount = rSharedBI.ExtendAmount() * amountBit;
    const vector<AddressingRegister* >& index_choices = rSharedBI.GetIndexChoices();
    for (auto index_choice : index_choices) {
      uint64 index_value = extend_regval(index_choice->RegisterValue(), rSharedBI.ExtendType(), extend_amount);
      uint64 temp_target = BaseValue() + index_value;

      const AddressTagging* addr_tagging = rSharedBI.GetAddressTagging();
      uint64 untagged_temp_target = addr_tagging->UntagAddress(temp_target, rSharedBI.IsInstruction());

      if (not rSharedBI.AlignmentOkay(untagged_temp_target)) {
        // << "alignment NOT okay address 0x" << hex << untagged_temp_target << endl;
        continue;
      }
      // << "alignment okay address 0x" << hex << untagged_temp_target << endl;
      uint64 solved_target = 0;
      bool solve_result = SolveWithValue(untagged_temp_target, rSharedBI, rSharedBI.TargetConstraint(), solved_target);
      uint64 temp_target_tag_value = addr_tagging->GetTagValue(temp_target);
      solved_target = addr_tagging->TagAddress(solved_target, temp_target_tag_value, rSharedBI.IsInstruction());

      // If the temp target address can be tagged and the solved target address cannot (or vice versa), this solution
      // won't work, so reject it.
      if (solve_result && addr_tagging->AreTaggingCapacitiesEqual(solved_target, temp_target, rSharedBI.IsInstruction())) {
        auto index_solution = new IndexSolution(*index_choice, index_choice->RegisterValue(), BaseValue(), solved_target, amountBit);
        AddIndexSolution(index_solution);
        // << "has solution 0x" << hex << solved_target << endl;
      }
    }
  }

  void BaseIndexExtendMode::CreateFreeBaseIndexSolutionWithAmount(const BaseIndexSolvingShared& rBaseIndexShared, const AddressingRegister& rIndexChoice, const uint64 index_value, uint32 amountBit)
  {
    uint32 extend_amount = rBaseIndexShared.ExtendAmount() * amountBit;
    uint64 target_address = rBaseIndexShared.FreeTarget();
    uint64 offset_value = extend_regval(index_value, rBaseIndexShared.ExtendType(), extend_amount);
    uint64 base_value = target_address - offset_value;

    auto index_solution = new IndexSolution(rIndexChoice, index_value, base_value, target_address,amountBit);
    AddIndexSolution(index_solution);
  }

  void BaseIndexExtendMode::CreateFreeBaseIndexSolution(const BaseIndexSolvingShared& rBaseIndexShared, const AddressingRegister& rIndexChoice, const uint64 index_value)
  {
    CreateFreeBaseIndexSolutionWithAmount(rBaseIndexShared, rIndexChoice, index_value, 1);
  }

  void BaseIndexExtendMode::SolveTargetHasConstraint(const BaseIndexSolvingShared& rSharedBI, uint32 amountBit) {
    uint32 extend_amount = rSharedBI.ExtendAmount() * amountBit;
    uint64 target_force = rSharedBI.TargetConstraint()->OnlyValue();

    const AddressTagging* addr_tagging = rSharedBI.GetAddressTagging();
    const vector<AddressingRegister* >& index_choices = rSharedBI.GetIndexChoices();

    for (auto index_choice : index_choices) {
      uint64 index_value = extend_regval(index_choice->RegisterValue(), rSharedBI.ExtendType(), extend_amount);
      uint64 temp_target = BaseValue() + index_value;
      uint64 untagged_target = addr_tagging->UntagAddress(temp_target, rSharedBI.IsInstruction());

      if (target_force == untagged_target) {
        auto index_solution = new IndexSolution(*index_choice, index_choice->RegisterValue(), BaseValue(), temp_target, amountBit);
        AddIndexSolution(index_solution);
      }
    }
  }

  RegisterOperand* BaseIndexExtendMode::GetIndexOperand(const AddressSolvingShared& rShared) const
  {
    const AddressingOperandConstraint* addr_opr_constr = rShared.GetAddressingOperandConstraint();
    return addr_opr_constr->IndexOperand();
  }

  bool VectorStridedMode::Solve(const AddressSolvingShared& rShared)
  {
    auto& strided_shared = dynamic_cast<const VectorStridedSolvingShared&>(rShared);
    if ((strided_shared.TargetConstraint() != nullptr) and (strided_shared.TargetConstraint()->Size() == 1)) {
      SolveFixedTargetConstraintForced(strided_shared);
    }
    else {
      SolveFixed(strided_shared);
    }

    return HasIndexSolutions();
  }

  bool VectorStridedMode::SolveFree(const AddressSolvingShared& rShared)
  {
    // Target constraint is accounted for in rShared.SolveFree()
    if (not rShared.SolveFree()) {
      return false;
    }

    auto& strided_shared = dynamic_cast<const VectorStridedSolvingShared&>(rShared);
    for (AddressingRegister* stride_choice : strided_shared.GetStrideChoices()) {
      const Register* stride_reg = stride_choice->GetRegister();

      if (stride_reg->IndexValue() != mpRegister->IndexValue()) {
        if (stride_choice->IsFree()) {
          SolveFreeStrideFree(strided_shared, *stride_choice);
        } else {
          SolveFreeStrideFixed(strided_shared, *stride_choice);
        }
      }

      // TODO(Noah): Implement solving for the case when the base and index registers are the same
      // register when a solution for this case can be devised.
    }

    return HasIndexSolutions();
  }

  AddressSolvingShared* VectorStridedMode::AddressSolvingSharedInstance() const
  {
    return new VectorStridedSolvingShared();
  }

  RegisterOperand* VectorStridedMode::GetIndexOperand(const AddressSolvingShared& rShared) const
  {
    const AddressingOperandConstraint* addr_opr_constr = rShared.GetAddressingOperandConstraint();
    auto strided_opr_constr = addr_opr_constr->CastInstance<const VectorStridedLoadStoreOperandConstraint>();
    return strided_opr_constr->StrideOperand();
  }

  void VectorStridedMode::SolveFixedTargetConstraintForced(const VectorStridedSolvingShared& rStridedShared) {
    const ConstraintSet* target_constr = rStridedShared.TargetConstraint();
    uint64 forced_target_addr = target_constr->OnlyValue();

    const AddressTagging* addr_tagging = rStridedShared.GetAddressTagging();
    for (AddressingRegister* stride_choice : rStridedShared.GetStrideChoices()) {
      uint64 target_addr = BaseValue() + stride_choice->RegisterValue();
      uint64 untagged_target_addr = addr_tagging->UntagAddress(target_addr, rStridedShared.IsInstruction());

      if (untagged_target_addr == forced_target_addr) {
        auto stride_solution = new IndexSolution(*stride_choice, stride_choice->RegisterValue(), BaseValue(), target_addr, 0);
        AddIndexSolution(stride_solution);
      }
    }
  }

  void VectorStridedMode::SolveFixed(const VectorStridedSolvingShared& rStridedShared)
  {
    for (auto stride_choice : rStridedShared.GetStrideChoices()) {
      uint64 base_val = BaseValue();
      uint64 stride_val = stride_choice->RegisterValue();

      if (AreTargetAddressesUsable(rStridedShared, base_val, stride_val)) {
        auto stride_solution = new IndexSolution(*stride_choice, stride_val, base_val, base_val, 0);
        AddIndexSolution(stride_solution);
      }
    }
  }

  void VectorStridedMode::SolveFreeStrideFree(const VectorStridedSolvingShared& rStridedShared, const AddressingRegister& rStrideChoice)
  {
    // TODO(Noah): Devise a better algorithm for this when there is time to do so. This approach
    // starts with relatively large random value for the stride and makes it successively smaller
    // assuming that smaller strides should be more likely to yield a solution.
    bool solved = false;
    uint64 base_val = rStridedShared.FreeTarget();
    const Register* stride_ptr = rStrideChoice.GetRegister();
    uint64 stride_val = stride_ptr->ReloadValue();

    // The base target address should be valid, so a stride value of 0 should always be a valid
    // solution in case a solution with larger stride value can't be found.
    while ((not solved) and (stride_val > 0)) {
      stride_val &= rStridedShared.AlignMask();

      if (AreTargetAddressesUsable(rStridedShared, base_val, stride_val)) {
        solved = true;
      }
      else {
        uint32 reduction_factor = 6;
        stride_val = static_cast<uint64>(static_cast<int64>(stride_val) >> reduction_factor);
      }
    }

    auto stride_solution = new IndexSolution(rStrideChoice, stride_val, base_val, base_val, 0);
    AddIndexSolution(stride_solution);
  }

  void VectorStridedMode::SolveFreeStrideFixed(const VectorStridedSolvingShared& rStridedShared, const AddressingRegister& rStrideChoice)
  {
    uint64 base_val = rStridedShared.FreeTarget();
    uint64 stride_val = rStrideChoice.RegisterValue();
    if (AreTargetAddressesUsable(rStridedShared, base_val, stride_val)) {
      auto stride_solution = new IndexSolution(rStrideChoice, stride_val, base_val, base_val, 0);
      AddIndexSolution(stride_solution);
    }
  }

  bool VectorStridedMode::AreTargetAddressesUsable(const VectorStridedSolvingShared& rStridedShared, cuint64 baseVal, cuint64 strideVal)
  {
    bool target_addresses_usable = IsTargetAddressUsable(rStridedShared, baseVal, rStridedShared.TargetConstraint());
    for (uint32 elem_index = 1; elem_index < rStridedShared.GetElementCount(); elem_index++) {
      if (not target_addresses_usable) {
        break;
      }

      uint64 elem_target_addr = baseVal + strideVal * elem_index;
      target_addresses_usable = IsTargetAddressUsable(rStridedShared, elem_target_addr, nullptr);
    }

    return target_addresses_usable;
  }

  bool VectorStridedMode::IsTargetAddressUsable(const VectorStridedSolvingShared& rStridedShared, cuint64 targetAddr, const ConstraintSet* pTargetConstr)
  {
    bool target_addr_usable = false;
    const AddressTagging* addr_tagging = rStridedShared.GetAddressTagging();
    uint64 untagged_target_addr = addr_tagging->UntagAddress(targetAddr, rStridedShared.IsInstruction());

    if (rStridedShared.AlignmentOkay(untagged_target_addr)) {
      uint64 solved_target_addr = 0;
      bool solved = SolveWithValue(untagged_target_addr, rStridedShared, pTargetConstr, solved_target_addr);

      if (solved and (solved_target_addr == untagged_target_addr)) {
        target_addr_usable = true;
      }
    }

    return target_addr_usable;
  }

  bool VectorIndexedMode::Solve(const AddressSolvingShared& rShared)
  {
    auto& indexed_shared = dynamic_cast<const VectorIndexedSolvingShared&>(rShared);
    if ((indexed_shared.TargetConstraint() != nullptr) and (indexed_shared.TargetConstraint()->Size() == 1)) {
      SolveFixedTargetConstraintForced(indexed_shared);
    }
    else {
      SolveFixed(indexed_shared);
    }

    return HasIndexSolutions();
  }

  bool VectorIndexedMode::SolveFree(const AddressSolvingShared& rShared)
  {
    // Target constraint is accounted for in rShared.SolveFree()
    if (not rShared.SolveFree()) {
      return false;
    }

    auto& indexed_shared = dynamic_cast<const VectorIndexedSolvingShared&>(rShared);
    for (AddressingRegister* index_choice : indexed_shared.GetIndexChoices()) {
      if (index_choice->IsFree()) {
        SolveFreeIndexFree(indexed_shared, *index_choice);
      } else {
        SolveFreeIndexFixed(indexed_shared, *index_choice);
      }
    }

    return HasIndexSolutions();
  }

  AddressSolvingShared* VectorIndexedMode::AddressSolvingSharedInstance() const
  {
    return new VectorIndexedSolvingShared();
  }

  void VectorIndexedMode::SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const
  {
    AddressingMode::SetOperandResults(rShared, rBaseOperand);

    RegisterOperand* index_opr = GetIndexOperand(rShared);
    Instruction* instr = rShared.GetInstruction();
    const Register* index_reg = Index();
    index_opr->SetChoiceResultDirect(*(rShared.GetGenerator()), *instr, index_reg->Name());

    const IndexSolution* chosen_index_solution = GetChosenIndexSolution();
    if (chosen_index_solution->IsFree()) {
      instr->SetOperandDataValue(index_opr->Name(), chosen_index_solution->RegisterValues(), sizeof_bits<uint64>());
    }
  }

  vector<uint64> VectorIndexedMode::IndexValues() const
  {
    const IndexSolution* chosen_index_solution = GetChosenIndexSolution();
    return chosen_index_solution->RegisterValues();
  }

  RegisterOperand* VectorIndexedMode::GetIndexOperand(const AddressSolvingShared& rShared) const
  {
    const AddressingOperandConstraint* addr_opr_constr = rShared.GetAddressingOperandConstraint();
    auto indexed_opr_constr = addr_opr_constr->CastInstance<const VectorIndexedLoadStoreOperandConstraint>();
    return indexed_opr_constr->IndexOperand();
  }

  void VectorIndexedMode::SolveFixedTargetConstraintForced(const VectorIndexedSolvingShared& rIndexedShared) {
    const ConstraintSet* target_constr = rIndexedShared.TargetConstraint();
    uint64 forced_target_addr = target_constr->OnlyValue();

    const AddressTagging* addr_tagging = rIndexedShared.GetAddressTagging();
    for (AddressingRegister* index_choice : rIndexedShared.GetIndexChoices()) {
      uint64 target_addr = BaseValue() + rIndexedShared.GetVectorElementValue(index_choice->RegisterValues(), 0);
      uint64 untagged_target_addr = addr_tagging->UntagAddress(target_addr, rIndexedShared.IsInstruction());

      if (untagged_target_addr == forced_target_addr) {
        auto index_solution = new IndexSolution(*index_choice, BaseValue(), target_addr);
        AddIndexSolution(index_solution);
      }
    }
  }

  void VectorIndexedMode::SolveFixed(const VectorIndexedSolvingShared& rIndexedShared)
  {
    for (auto index_choice : rIndexedShared.GetIndexChoices()) {
      if (AreTargetAddressesUsable(rIndexedShared, BaseValue(), index_choice->RegisterValues())) {
        uint64 target_addr = BaseValue() + rIndexedShared.GetVectorElementValue(index_choice->RegisterValues(), 0);
        auto index_solution = new IndexSolution(*index_choice, BaseValue(), target_addr);
        AddIndexSolution(index_solution);
      }
    }
  }

  void VectorIndexedMode::SolveFreeIndexFree(const VectorIndexedSolvingShared& rIndexedShared, const AddressingRegister& rIndexChoice)
  {
    vector<uint64> index_elem_values;

    ConstraintSet index_elem_constr(0, get_mask64(rIndexedShared.GetElementSize()));
    uint64 index_elem_val = index_elem_constr.ChooseValue();
    index_elem_values.push_back(index_elem_val);
    uint64 base_val = rIndexedShared.FreeTarget() - index_elem_val;

    bool solved = true;
    uint64 elem_target_addr = 0;
    BaseOffsetConstraint base_offset_constr(0, rIndexedShared.GetElementSize(), 0, MAX_UINT64, true);
    for (uint32 elem_index = 1; elem_index < rIndexedShared.GetElementCount(); elem_index++) {
      solved = SolveWithBase(base_val, rIndexedShared, base_offset_constr, nullptr, elem_target_addr);
      index_elem_values.push_back(elem_target_addr - base_val);

      if (not solved) {
        break;
      }
    }

    if (solved) {
      vector<uint64> reg_values;
      rIndexedShared.GetVectorRegisterValues(index_elem_values, reg_values);

      auto index_solution = new IndexSolution(rIndexChoice, base_val, rIndexedShared.FreeTarget());
      index_solution->SetRegisterValue(reg_values);
      AddIndexSolution(index_solution);
    }
  }

  void VectorIndexedMode::SolveFreeIndexFixed(const VectorIndexedSolvingShared& rIndexedShared, const AddressingRegister& rIndexChoice)
  {
    uint64 base_val = rIndexedShared.FreeTarget() - rIndexedShared.GetVectorElementValue(rIndexChoice.RegisterValues(), 0);
    if (AreTargetAddressesUsable(rIndexedShared, base_val, rIndexChoice.RegisterValues())) {
      auto index_solution = new IndexSolution(rIndexChoice, base_val, rIndexedShared.FreeTarget());
      AddIndexSolution(index_solution);
    }
  }

  bool VectorIndexedMode::AreTargetAddressesUsable(const VectorIndexedSolvingShared& rIndexedShared, cuint64 baseVal, const vector<uint64>& rIndexRegValues)
  {
    uint64 index_elem_val = rIndexedShared.GetVectorElementValue(rIndexRegValues, 0);
    bool target_addresses_usable = IsTargetAddressUsable(rIndexedShared, (baseVal + index_elem_val), rIndexedShared.TargetConstraint());
    for (uint32 elem_index = 1; elem_index < rIndexedShared.GetElementCount(); elem_index++) {
      if (not target_addresses_usable) {
        break;
      }

      index_elem_val = rIndexedShared.GetVectorElementValue(rIndexRegValues, elem_index);
      target_addresses_usable = IsTargetAddressUsable(rIndexedShared, (baseVal + index_elem_val), nullptr);
    }

    return target_addresses_usable;
  }

  bool VectorIndexedMode::IsTargetAddressUsable(const VectorIndexedSolvingShared& rIndexedShared, cuint64 targetAddr, const ConstraintSet* pTargetConstr)
  {
    bool target_addr_usable = false;
    const AddressTagging* addr_tagging = rIndexedShared.GetAddressTagging();
    uint64 untagged_target_addr = addr_tagging->UntagAddress(targetAddr, rIndexedShared.IsInstruction());

    if (rIndexedShared.AlignmentOkay(untagged_target_addr)) {
      uint64 solved_target_addr = 0;
      bool solved = SolveWithValue(untagged_target_addr, rIndexedShared, pTargetConstr, solved_target_addr);

      if (solved and (solved_target_addr == untagged_target_addr)) {
        target_addr_usable = true;
      }
    }

    return target_addr_usable;
  }

  bool BaseIndexAmountBitExtendMode::Solve(const AddressSolvingShared& rShared)
  {
    const auto & rSharedBI= dynamic_cast<const BaseIndexAmountBitSolvingShared&>(rShared);
    if ((nullptr != rSharedBI.TargetConstraint()) and (1 == rSharedBI.TargetConstraint()->Size())) {
      if (rSharedBI.IsExtendAmount0Valid()) {
        SolveTargetHasConstraint(rSharedBI, 0);
      }

      if (rSharedBI.IsExtendAmount1Valid()) {
        SolveTargetHasConstraint(rSharedBI, 1);
      }
      return HasIndexSolutions();
    }
    //normal solve
    if (rSharedBI.IsExtendAmount0Valid()) {
      SolveWithAmountBit(rSharedBI, 0);
    }
    if (rSharedBI.IsExtendAmount1Valid()) {
      SolveWithAmountBit(rSharedBI, 1);
    }
    return HasIndexSolutions();
  }

  AddressSolvingShared* BaseIndexAmountBitExtendMode::AddressSolvingSharedInstance() const
  {
    return new BaseIndexAmountBitSolvingShared();
  }

  void BaseIndexAmountBitExtendMode::SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const
  {
    BaseIndexExtendMode::SetOperandResults(rShared, rBaseOperand);

    const AddressingOperandConstraint* addr_opr_constr = rShared.GetAddressingOperandConstraint();
    ImmediateOperand* extend_amount_opr = addr_opr_constr->ExtendAmountOperand();
    extend_amount_opr->SetValue(AmountBit());
  }

  uint32 BaseIndexAmountBitExtendMode::AmountBit() const
  {
    const IndexSolution* chosen_index_solution = GetChosenIndexSolution();
    return chosen_index_solution->AmountBit();
  }

  void BaseIndexAmountBitExtendMode::CreateFreeBaseIndexSolution(const BaseIndexSolvingShared& rBaseIndexShared, const AddressingRegister& rIndexChoice, const uint64 index_value)
  {
    const auto& base_index_shared = dynamic_cast<const BaseIndexAmountBitSolvingShared&>(rBaseIndexShared);
    if (base_index_shared.IsExtendAmount0Valid()) {
      CreateFreeBaseIndexSolutionWithAmount(rBaseIndexShared, rIndexChoice, index_value, 0);
    }
    if (base_index_shared.IsExtendAmount1Valid()) {
      CreateFreeBaseIndexSolutionWithAmount(rBaseIndexShared, rIndexChoice, index_value, 1);
    }
  }

  Object* AluImmediateMode::Clone() const
  {
    return new AluImmediateMode(*this);
  }

  bool AluImmediateMode::Solve(const AddressSolvingShared& rShared)
  {
    const AddressingOperandConstraint* addr_opr_constr = rShared.GetAddressingOperandConstraint();
    auto alu_imm_opr_constr = dynamic_cast<const AluImmediateOperandConstraint*>(addr_opr_constr);
    ImmediateOperand* offset_opr = alu_imm_opr_constr->OffsetOperand();
    OperandConstraint* offset_constr = offset_opr->GetOperandConstraint();

    // TODO(Noah): Implement solving with offset operand constraints when it is deemed necessary to do so.
    if (offset_constr->HasConstraint()) {
      return false;
    }

    // TODO(Noah): Implement solving with target constraints when it is deemed necessary to do so.
    if (rShared.TargetConstraint() != nullptr) {
      return false;
    }

    ChoicesOperand* offset_shift_opr = alu_imm_opr_constr->OffsetShiftOperand();
    uint32 shift_amount = get_shift_amount(offset_shift_opr->Name())*offset_shift_opr->Value();
    AluImmediateConstraint alu_imm_constr_builder(mOperationType, offset_opr->Size(), shift_amount);

    ConstraintSet alu_imm_constr;
    const AddressTagging* addr_tagging = rShared.GetAddressTagging();
    uint64 untagged_base_value = addr_tagging->UntagAddress(BaseValue(), rShared.IsInstruction());
    alu_imm_constr_builder.GetConstraint(untagged_base_value, rShared.Size(), &alu_imm_constr);

    rShared.ApplyVirtualUsableConstraint(&alu_imm_constr, mVmTimeStamp);
    if (alu_imm_constr.IsEmpty()) return false;

    alu_imm_constr.SubConstraintSet(*(rShared.PcConstraint()));
    if (alu_imm_constr.IsEmpty()) return false;

    uint64 align_offset = BaseValue() & ~AlignMask(rShared, shift_amount);
    alu_imm_constr.AlignOffsetWithSize(AlignMask(rShared, shift_amount), align_offset, rShared.Size());
    if (alu_imm_constr.IsEmpty()) return false;

    alu_imm_constr.ShiftRight(AlignShift(rShared, shift_amount));
    mTargetAddress = (alu_imm_constr.ChooseValue() << AlignShift(rShared, shift_amount)) + align_offset;

    uint64 base_value_tag_value = addr_tagging->GetTagValue(BaseValue());
    mTargetAddress = addr_tagging->TagAddress(mTargetAddress, base_value_tag_value, rShared.IsInstruction());

    // If the the base register value can be tagged and the target address cannot (or vice versa), this solution won't
    // work, so reject it.
    if (!addr_tagging->AreTaggingCapacitiesEqual(mTargetAddress, BaseValue(), rShared.IsInstruction())) {
      return false;
    }

    SolveOffsetValue(shift_amount);

    return true;
  }

  bool AluImmediateMode::SolveFree(const AddressSolvingShared& rShared)
  {
    if (not rShared.SolveFree()) {
      return false;
    }

    // TODO(Noah): Implement solving with target constraints when it is deemed necessary to do so.
    if (rShared.TargetConstraint() != nullptr) {
      return false;
    }

    mTargetAddress = rShared.FreeTarget();

    const AddressingOperandConstraint* addr_opr_constr = rShared.GetAddressingOperandConstraint();
    auto alu_imm_opr_constr = dynamic_cast<const AluImmediateOperandConstraint*>(addr_opr_constr);

    ImmediateOperand* offset_opr = alu_imm_opr_constr->OffsetOperand();
    mOffsetValue = offset_opr->Value();

    ChoicesOperand* offset_shift_opr = alu_imm_opr_constr->OffsetShiftOperand();
    uint32 shift_amount = get_shift_amount(offset_shift_opr->Name())*offset_shift_opr->Value();

    SolveBaseValue(shift_amount);

    return true;
  }

  void AluImmediateMode::SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const
  {
    AddressingMode::SetOperandResults(rShared, rBaseOperand);

    const AddressingOperandConstraint* addr_opr_constr = rShared.GetAddressingOperandConstraint();
    ImmediateOperand* offset_opr = addr_opr_constr->OffsetOperand();
    if (offset_opr != nullptr) {
      offset_opr->SetValue(mOffsetValue);
    }
  }

  uint64 AluImmediateMode::AlignMask(const AddressSolvingShared& rShared, cuint32 offset_shift_amount) const
  {
    uint64 align_mask = rShared.AlignMask();
    if (offset_shift_amount > rShared.AlignShift()) {
      align_mask = ~get_mask64(offset_shift_amount);
    }

    return align_mask;
  }

  uint32 AluImmediateMode::AlignShift(const AddressSolvingShared& rShared, cuint32 offset_shift_amount) const
  {
    uint32 align_shift = rShared.AlignShift();
    if (offset_shift_amount > rShared.AlignShift()) {
      align_shift = offset_shift_amount;
    }

    return align_shift;
  }

  void AluImmediateMode::SolveBaseValue(cuint32 shift_amount)
  {
    switch (mOperationType) {
    case EAluOperationType::ADD:
      mRegisterValue = mTargetAddress - (mOffsetValue << shift_amount);
      break;
    case EAluOperationType::SUB:
      mRegisterValue = mTargetAddress + (mOffsetValue << shift_amount);
      break;
    default:
      LOG(fail) << "{AluImmediateMode::SolveBaseValue} unhandled ALU operation type: " << EAluOperationType_to_string(mOperationType) << endl;
      FAIL("unhandled-alu-operation-type");
    }
  }

  void AluImmediateMode::SolveOffsetValue(cuint32 shift_amount)
  {
    switch (mOperationType) {
    case EAluOperationType::ADD:
      mOffsetValue = (mTargetAddress - BaseValue()) >> shift_amount;
      break;
    case EAluOperationType::SUB:
      mOffsetValue = (BaseValue() - mTargetAddress) >> shift_amount;
      break;
    default:
      LOG(fail) << "{AluImmediateMode::SolveOffsetValue} unhandled ALU operation type: " << EAluOperationType_to_string(mOperationType) << endl;
      FAIL("unhandled-alu-operation-type");
    }
  }

  AddressSolvingShared* DataProcessingMode::AddressSolvingSharedInstance() const
  {
    return new DataProcessingSolvingShared();
  }

  DataProcessingMode::DataProcessingMode(const DataProcessingOperandStructure& rDataProcOprStruct, const std::vector<Operand*>& rSubOperands)
    : AddressingMode(), mOperationType(rDataProcOprStruct.OperationType()), mUop(rDataProcOprStruct.Uop()), mSubOperands(rSubOperands), mOperandSolutions()
  {
    MapOperandSolutions(rDataProcOprStruct);
  }

  const std::string DataProcessingMode::ToString() const
  {
    stringstream out_str;
    out_str << Type() << ": target-address: 0x" << hex << mTargetAddress << " sub-operands:";
    for (const auto& opr_solution_entry : mOperandSolutions) {
      const OperandSolution& opr_solution = opr_solution_entry.second;
      const Register* reg_ptr = opr_solution.GetRegister();

      if (reg_ptr != nullptr) {
        out_str << " " << reg_ptr->Name();
      }
      else {
        out_str << " " << opr_solution.GetName();
      }

      out_str << " value: 0x" << hex << opr_solution.GetValue();
    }

    out_str << " weight: " << dec << mWeight << " time-stamp: " << mVmTimeStamp;

    return out_str.str();
  }

  Object* DataProcessingMode::Clone() const
  {
    return new DataProcessingMode(*this);
  }

  bool DataProcessingMode::Solve(const AddressSolvingShared& rShared)
  {
    AddressSolutionStrategyFactory solution_strategy_factory;
    const auto& data_proc_shared = dynamic_cast<const DataProcessingSolvingShared&>(rShared);
    const ConstraintSet* target_addr_constr = data_proc_shared.GetTargetAddressConstraint();
    const Generator* gen = data_proc_shared.GetGenerator();
    unique_ptr<AddressSolutionStrategy> solution_strategy(solution_strategy_factory.CreateSolutionStrategy(mOperationType, target_addr_constr, data_proc_shared.AlignShift(), mUop, gen->ThreadId(), data_proc_shared.GetConditionFlags()));
    return solution_strategy->Solve(&mOperandSolutions, mTargetAddress, rShared.GetAddressTagging());
  }

  bool DataProcessingMode::SolveFree(const AddressSolvingShared& rShared)
  {
    if (not rShared.SolveFree()) {
      return false;
    }

    return Solve(rShared);
  }

  void DataProcessingMode::NonSystemRegisterOperands(std::vector<const RegisterOperand*>& rRegOperands) const
  {
    for (const Operand* opr : mSubOperands) {
      if (IsNonSystemRegisterOperand(*opr)) {
        rRegOperands.push_back(dynamic_cast<const RegisterOperand*>(opr));
      }
    }
  }

  void DataProcessingMode::SetRegisterOperandChoice(const std::string& oprName, Register* regPtr)
  {
    for (auto& opr_solution_entry : mOperandSolutions) {
      OperandSolution& opr_solution = opr_solution_entry.second;
      if (opr_solution.GetName() == oprName) {
        opr_solution.SetRegister(regPtr);
        break;
      }
    }
  }

  void DataProcessingMode::SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const
  {
    Generator* gen_ptr = rShared.GetGenerator();
    Instruction* instr_ptr = rShared.GetInstruction();
    for (const auto& opr_solution_entry : mOperandSolutions) {
      const OperandSolution& opr_solution = opr_solution_entry.second;
      Operand* opr = opr_solution.GetOperand();

      if (IsNonSystemRegisterOperand(*opr)) {
        auto reg_opr = dynamic_cast<RegisterOperand*>(opr);
        const Register* reg_ptr = opr_solution.GetRegister();
        reg_opr->SetChoiceResultDirect(*gen_ptr, *instr_ptr, reg_ptr->Name());

        if (not reg_ptr->IsInitialized()) {
          instr_ptr->SetOperandDataValue(reg_opr->Name(), opr_solution.GetValue(), reg_ptr->Size());
        }
      }
      else {
        opr->SetValue(opr_solution.GetValue());
      }
    }
  }

  void DataProcessingMode::MapOperandSolutions(const DataProcessingOperandStructure& rDataProcOprStruct)
  {
    for (Operand* opr : mSubOperands) {
      string role = rDataProcOprStruct.GetOperandRole(opr->Name());
      mOperandSolutions.emplace(role, OperandSolution(opr));
    }
  }

  AddressSolver::AddressSolver(AddressingOperand* pOpr, AddressingMode* pModeTemp, uint64 alignment)
    : mpBaseOperand(nullptr), mpModeTemplate(pModeTemp), mpAddressSolvingShared(nullptr), mpChosenSolution(nullptr), mSolutionChoices(), mFilteredChoices()
  {
    mpAddressSolvingShared = mpModeTemplate->AddressSolvingSharedInstance();
    mpBaseOperand = mpAddressSolvingShared->SetAddressingOperand(pOpr, alignment);
  }

  AddressSolver::AddressSolver()
    : mpBaseOperand(nullptr), mpModeTemplate(nullptr), mpAddressSolvingShared(nullptr), mpChosenSolution(nullptr), mSolutionChoices(), mFilteredChoices()
  {

  }

  AddressSolver::~AddressSolver()
  {
    mpBaseOperand = nullptr;
    delete mpModeTemplate;
    delete mpAddressSolvingShared;
    mpChosenSolution = nullptr;

    for (auto solution_choice : mSolutionChoices) {
      delete solution_choice;
    }

    for (auto filtered_choice: mFilteredChoices) {
      delete filtered_choice;
    }
  }

  const AddressingMode* AddressSolver::Solve(Generator& gen, Instruction& instr, uint32 size, bool isInstr, const EMemAccessType memAccessType)
  {
    // initial setup of the shared address solving data structure.
    mpAddressSolvingShared->Initialize(gen, instr, size, isInstr, memAccessType);

    vector<AddressingMode* > base_choices;
    if (not GetAvailableBaseChoices(gen, instr, base_choices)) {
      LOG(notice) << "{AddressSolver::Solve} no base choices available." << endl;
      return nullptr;
    }

    return SolveWithModes(gen, instr, base_choices);
  }

  const AddressingMode* AddressSolver::SolveMultiRegister(Generator& gen, Instruction& instr, uint32 size, bool isInstr, const EMemAccessType memAccessType)
  {
    // initial setup of the shared address solving data structure.
    mpAddressSolvingShared->Initialize(gen, instr, size, isInstr, memAccessType);

    vector<AddressingMode* > reg_choice_combos;
    if (not GetRegisterChoiceCombinations(gen, instr, reg_choice_combos)) {
      LOG(notice) << "{AddressSolver::SolveMultiRegister} no register choice combinations available." << endl;
      return nullptr;
    }

    return SolveWithModes(gen, instr, reg_choice_combos);
  }

  const AddressingMode* AddressSolver::SolveWithModes(const Generator& gen, const Instruction& instr, const vector<AddressingMode* >& rModes)
  {
    if (not mpAddressSolvingShared->Setup()) {
      LOG(notice) << "{AddressSolver::SolveWithModes} failed to setup constraints for the shared solving data structure." << endl;
      return nullptr;
    }

    for (auto mode : rModes) {
      if (mode->IsFree()) {
        if (mode->SolveFree(*mpAddressSolvingShared)) {
          LOG(info) << "{AddressSolver::SolveWithModes} choice: " << mode->ToString() << endl;
          mSolutionChoices.push_back(mode);
          continue;
        }
      } else {
        if (mode->Solve(*mpAddressSolvingShared)) {
          LOG(info) << "{AddressSolver::SolveWithModes} choice: " << mode->ToString() << endl;
          mSolutionChoices.push_back(mode);
          continue;
        }
      }
      delete mode;
    }

    // try to set address shortage flag.
    if (ShouldEnableAddressShortage(instr)) {
      auto gen_mode = gen.GetGenMode();

      if (not gen_mode->IsAddressShortage()) {
        gen_mode->EnableGenMode(EGenModeTypeBaseType(EGenModeType::AddressShortage));
      }
    }

    ChooseSolution(gen, instr);
    return mpChosenSolution;
  }

  bool AddressSolver::GetAvailableBaseChoices(Generator& gen, Instruction& instr, vector<AddressingMode* >& rBaseChoices) const
  {
    vector<const Choice* > choices_list;
    mpBaseOperand->GetAvailableChoices(choices_list);

    const RegisterFile* reg_file = gen.GetRegisterFile();
    for (auto choice_item : choices_list) {
      Register* reg_ptr = reg_file->RegisterLookup(choice_item->Name());
      if (reg_ptr->IsInitialized()) {
        if (reg_ptr->HasAttribute(ERegAttrType::HasValue)) {
          uint64 reg_value = reg_ptr->Value();
          if (mpModeTemplate->BaseValueUsable(reg_value, mpAddressSolvingShared)) {
            AddressingMode* clone_addr_mode = dynamic_cast<AddressingMode* >(mpModeTemplate->Clone());
            clone_addr_mode->SetBase(reg_ptr);
            clone_addr_mode->SetWeight(choice_item->Weight());
            clone_addr_mode->SetBaseValue(reg_value);
            rBaseChoices.push_back(clone_addr_mode);
          }
        }
      }
      else if (gen.HasISS()) { // TODO gate out no-iss mode for now.
        if (mpAddressSolvingShared->OperandConflict(reg_ptr)) {
          continue;
        }
        AddressingMode* clone_addr_mode = dynamic_cast<AddressingMode* >(mpModeTemplate->Clone());
        clone_addr_mode->SetBase(reg_ptr);
        clone_addr_mode->SetWeight(choice_item->Weight());
        clone_addr_mode->SetFree(true);
        rBaseChoices.push_back(clone_addr_mode);
      }
    }

    return (rBaseChoices.size() > 0);
  }

  // For the situation one wants to control the selection process from amongst the pool of usable registers for a given operand
  //
  // Inputs: const RegisterOperand* pRegOperand
  // Outputs: vector<const Register*> rUsableRegisters
  bool AddressSolver::GetUsableRegisters(const Generator& gen, const RegisterOperand* pRegOperand, vector<Register*>& rUsableRegisters) const
  {
    vector<const Choice*> choices_list;
    pRegOperand->GetAvailableChoices(choices_list);
    const RegisterFile* reg_file = gen.GetRegisterFile();

    for (const auto *choice : choices_list) {
      const Choice* reg_choice = choice;
      Register* temp_reg_ptr = reg_file->RegisterLookup(reg_choice->Name());

      if (IsRegisterUsable(temp_reg_ptr, gen.HasISS())) {
        rUsableRegisters.push_back(temp_reg_ptr);
      }
    }

    if (rUsableRegisters.size() < 1) {
        return false;
    }

    return true;
  }

  // Attempting to solve for every possible combination of registers is likely to be too computationally intensive when
  // there are more than two register operands. GetRegisterChoiceCombinations() will enumerate all possible combinations
  // for two register operands and will randomly choose registers for any remaining operands.
  bool AddressSolver::GetRegisterChoiceCombinations(const Generator& gen, Instruction& instr, vector<AddressingMode* >& rRegChoiceCombos) const
  {
    // Get register operands from addressing mode template
    vector<const RegisterOperand*> reg_operands;
    mpModeTemplate->NonSystemRegisterOperands(reg_operands);

    if (reg_operands.empty()) {
      return false;
    }

    // TODO(Noah): Randomly choose which register operands will be fixed, rather than choosing the ones at the end of
    // the list, when there is time to do so. We can do this by implementing a method to shuffle the register operands
    // vector.
    // TODO(Noah): Randomly choose different registers for different AddressingModes, rather than repeatedly using the
    // same registers for each AddressingMode, when there is time to do so.
    for (size_t i = 2; i < reg_operands.size(); i++) {
      const RegisterOperand* reg_operand = reg_operands[i];
      Register* reg_ptr = nullptr;
      vector<Register*> usable_registers;

      // If a given operand has no usable register choices, we can't create any valid register choice combinations.
      if ( not GetUsableRegisters(gen, reg_operand, usable_registers)) {
        return false;
      }

      // Randomization here prevents an arbitrary prioritizing of registers for the thrid and above operands
      auto my_random = [](int i) {return Random::Instance()->Random64(0, MAX_UINT64) % i;};
      random_shuffle(usable_registers.begin(), usable_registers.end(), my_random);

      //With a random starting location, walk through the usable registers vector until an uninitialized one is found
      for (size_t trial = 0; trial < usable_registers.size(); trial++) {
        reg_ptr = usable_registers[ trial ];

        if (not reg_ptr->IsInitialized())
        {
            // We break to select the first unintialized register we find for use with operands third and above.
            break;
        }
      }

      if (reg_ptr != nullptr) {
        mpModeTemplate->SetRegisterOperandChoice(reg_operand->Name(), reg_ptr);
      }
      else {
        // If a given operand has no usable register choices, we can't create any valid register choice combinations.
        return false;
      }
    }

    // Enumerate the choices for the first register operand
    const RegisterOperand* first_reg_operand = reg_operands[0];
    vector<const Choice*> first_choices_list;
    first_reg_operand->GetAvailableChoices(first_choices_list);
    const RegisterFile* reg_file = gen.GetRegisterFile();
    for (const Choice* first_reg_choice : first_choices_list) {
      Register* first_reg_ptr = reg_file->RegisterLookup(first_reg_choice->Name());

      if (IsRegisterUsable(first_reg_ptr, gen.HasISS())) {

        // If there is a second register operand, enumerate its choices
        if (reg_operands.size() > 1) {
          const RegisterOperand* second_reg_operand = reg_operands[1];
          vector<const Choice*> second_choices_list;
          second_reg_operand->GetAvailableChoices(second_choices_list);

          for (const Choice* second_reg_choice : second_choices_list) {
            Register* second_reg_ptr = reg_file->RegisterLookup(second_reg_choice->Name());

            if (IsRegisterUsable(second_reg_ptr, gen.HasISS())) {
              auto clone_addr_mode = dynamic_cast<AddressingMode*>(mpModeTemplate->Clone());
              clone_addr_mode->SetBase(first_reg_ptr);
              clone_addr_mode->SetWeight(first_reg_choice->Weight() * second_reg_choice->Weight());

              if (first_reg_ptr->IsInitialized()) {
                clone_addr_mode->SetBaseValue(first_reg_ptr->Value());
              }

              clone_addr_mode->SetRegisterOperandChoice(first_reg_operand->Name(), first_reg_ptr);
              clone_addr_mode->SetRegisterOperandChoice(second_reg_operand->Name(), second_reg_ptr);
              rRegChoiceCombos.push_back(clone_addr_mode);
            }
          }
        }
        else {
          auto clone_addr_mode = dynamic_cast<AddressingMode*>(mpModeTemplate->Clone());
          clone_addr_mode->SetBase(first_reg_ptr);
          clone_addr_mode->SetWeight(first_reg_choice->Weight());

          if (first_reg_ptr->IsInitialized()) {
            clone_addr_mode->SetBaseValue(first_reg_ptr->Value());
          }

          clone_addr_mode->SetRegisterOperandChoice(first_reg_operand->Name(), first_reg_ptr);
          rRegChoiceCombos.push_back(clone_addr_mode);
        }
      }
    }

    return (rRegChoiceCombos.size() > 0);
  }

  void AddressSolver::ChooseSolution(const Generator& rGen, const Instruction& rInstr)
  {
    if (not UpdateSolution()) {
      return;
    }

    // << "initial solution: " << mpChosenSolution->ToString() << endl;

    vector<AddressSolutionFilter* > filter_vec;
    rGen.GetAddressFilteringRegulator()->GetAddressSolutionFilters(*mpModeTemplate, filter_vec);

    for (auto addr_filter : filter_vec) {
      uint32 num_remain = 0;
      if (not addr_filter->FilterSolutions(*this, rInstr, num_remain)) {
        continue; // no change with the filter, continue to the next filter.
      }

      if (0 == num_remain) {
        break;
      }

      UpdateSolution();
      LOG(info) << "{AddressSolver::ChooseSolution} Used: " << addr_filter->Type() << " has " << dec << num_remain << " remainder solution, updated to: " << mpChosenSolution->ToString() << endl;
    }
  }

  bool AddressSolver::UpdateSolution()
  {
    AddressingMode* addr_mode = nullptr;
    bool choice_usable = false;
    size_t remaining_choices_count = mSolutionChoices.size();
    while ((not choice_usable) and (remaining_choices_count > 0)) {
      addr_mode = choose_weighted_item(mSolutionChoices);

      if (nullptr == addr_mode) {
        break;
      }

      if (not addr_mode->ChooseSolution(*mpAddressSolvingShared)) {
        break;
      }

      choice_usable = mpAddressSolvingShared->MapTargetAddressRange(addr_mode->TargetAddress(), addr_mode->VmTimeStampReference());

      if (not choice_usable) {
        addr_mode->SetWeight(0);
        remaining_choices_count--;
      }
    }

    if (choice_usable) {
      mpChosenSolution = addr_mode;
    }

    RemoveUnusableSolutionChoices();

    return choice_usable;
  }

  void AddressSolver::RemoveUnusableSolutionChoices()
  {
    auto delete_if_zero_weight = [this](AddressingMode* addr_mode) {
      if (addr_mode->Weight() == 0) {
        if (addr_mode == mpChosenSolution) {
          mFilteredChoices.push_back(addr_mode);
          //LOG(warn) << "{AddressSolver::RemoveUnusableSolutionChoices} about to delete current chosen solution.  This shouldn't happen, temporarily move to filtered solution." << endl;
          LOG(fail) << "{AddressSolver::RemoveUnusableSolutionChoices} about to delete last known good chosen solution: " << addr_mode->ToString() << ".  This shouldn't happen." << " pointer 0x" << hex << uint64(addr_mode) << endl;
          FAIL("deleting-will-cause-dangling-pointer");
        }
        else {
          delete addr_mode;
        }
        return true;
      }
      else {
        return false;
      }
    };

    // remove_if will shift the valid elements to the beginning of the vector while erase will remove the invalid
    // elements, which have been shifted to the end of the vector.
    mSolutionChoices.erase(remove_if(mSolutionChoices.begin(), mSolutionChoices.end(), delete_if_zero_weight), mSolutionChoices.end());
  }

  bool AddressSolver::IsRegisterUsable(const Register* regPtr, cbool hasIss) const
  {
    bool usable = false;
    if (regPtr->IsInitialized() and regPtr->HasAttribute(ERegAttrType::HasValue)) {
      usable = true;
    }
    else if (hasIss and (not mpAddressSolvingShared->OperandConflict(regPtr))) {
      usable = true;
    }

    return usable;
  }

  const AddressingOperand* AddressSolver::GetAddressingOperand() const
  {
    return mpAddressSolvingShared->GetAddressingOperand();
  }

  void AddressSolver::SetOperandResults()
  {
    mpChosenSolution->SetOperandResults(*mpAddressSolvingShared, *mpBaseOperand);
  }

  // We want to enable address shortage mode if we only have one solution for a load or store
  // instruction that wasn't forced by constraints or by the instruction format.
  bool AddressSolver::ShouldEnableAddressShortage(const Instruction& instr) const
  {
    bool enable_addr_shortage = true;

    auto constraint = mpBaseOperand->GetOperandConstraint();
    auto target_constr = mpAddressSolvingShared->TargetConstraint();
    auto target_list_constr = mpAddressSolvingShared->TargetListConstraint();
    bool forced_target = ((nullptr != target_constr) or (target_list_constr.size() > 0));
    auto implied_reg_opr = dynamic_cast<ImpliedRegisterOperand*>(mpBaseOperand);
    if (not instr.IsLoadStore()) {
      enable_addr_shortage = false;
    }
    else if (mSolutionChoices.size() > 1) {
      enable_addr_shortage = false;
    }
    else if (constraint->ConstraintForced()) {
      enable_addr_shortage = false;
    }
    else if (forced_target) {
      enable_addr_shortage = false;
    }
    else if (implied_reg_opr != nullptr) {
      enable_addr_shortage = false;
    }

    return enable_addr_shortage;
  }

  bool AddressSolverWithOnlyChoice::GetAvailableBaseChoices(Generator& gen, Instruction& instr, std::vector<AddressingMode* >& rBaseChoices) const
  {
    auto cast_opr = dynamic_cast<const ImpliedRegisterOperand *> (mpBaseOperand);
    if (nullptr == cast_opr) {
      LOG(fail) << "{AddressSolverWithOnlyChoice::GetAvailableBaseChoices} failed to cast base operand." << endl;
      FAIL("failed-to-cast-base-operand");
    }

    const RegisterFile* reg_file = gen.GetRegisterFile();
    Register* reg_ptr = reg_file->RegisterLookup(cast_opr->Name());
    if (reg_ptr->IsInitialized()) {
      if (reg_ptr->HasAttribute(ERegAttrType::HasValue)) {
        uint64 reg_value = reg_ptr->Value();
        if (mpModeTemplate->BaseValueUsable(reg_value, mpAddressSolvingShared)) {
          AddressingMode* clone_addr_mode = dynamic_cast<AddressingMode* >(mpModeTemplate->Clone());
          clone_addr_mode->SetBase(reg_ptr);
          clone_addr_mode->SetWeight(100);
          clone_addr_mode->SetBaseValue(reg_value);
          rBaseChoices.push_back(clone_addr_mode);
        }
      }
    }
    else if (gen.HasISS()) { // TODO gate out no-iss mode for now.
      if (not mpAddressSolvingShared->OperandConflict(reg_ptr)) {
        AddressingMode* clone_addr_mode = dynamic_cast<AddressingMode* >(mpModeTemplate->Clone());
        clone_addr_mode->SetBase(reg_ptr);
        clone_addr_mode->SetWeight(100);
        clone_addr_mode->SetFree(true);
        rBaseChoices.push_back(clone_addr_mode);
      }
    }

    return (rBaseChoices.size() > 0);
  }

}
