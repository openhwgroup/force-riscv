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
#include <AddressSolutionStrategy.h>
#include <AddressTagging.h>
#include <OperandSolution.h>
#include <OperandSolutionMap.h>
#include <Constraint.h>
#include <Register.h>
#include <Random.h>
#include <UtilityFunctions.h>
#include <Log.h>
#include <memory>
#include <set>

using namespace std;

/*!
  \file AddressSolutionStrategy.cc
  \brief Code supporting generation of different types of solution strategies.
*/

namespace Force {
  bool AddressSolutionStrategy::IsTargetAddressValidAndAligned(uint64 targetAddress) const
  {
    bool target_valid = GetAddressConstraint()->ContainsValue(targetAddress);
    bool target_aligned = (targetAddress & ~get_mask64(mAlignShift)) == targetAddress;

    return (target_valid && target_aligned);
  }

  AddressSolutionStrategy::AddressSolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId)
    : mpAddressConstr(pAddressConstr), mAlignShift(alignShift), mUop(uop), mCpuId(cpuId)
  {
  }

  bool AddressSolutionStrategy::Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging) const
  {
    // Set a limit on the number of output parameters, so we can allocate an appropriately-sized buffer. This is more
    // than the number of output parameters we would ever expect to have.
    constexpr uint8_t MAX_OUTPUT_PARAM_COUNT = 8;

    if (HasRepeatedRegisters(*pOprSolutions)) {
      return false;
    }

    uint8_t input_param_count = pOprSolutions->size();
    UopParameter input_params[input_param_count];
    uint8_t input_param_index = 0;
    for (auto& opr_solution_entry : (*pOprSolutions)) {
      OperandSolution& opr_solution = opr_solution_entry.second;
      const ConstraintSet* opr_constr = opr_solution.GetConstraint();
      uint64 opr_value = opr_constr->ChooseValue();
      opr_solution.SetValue(opr_value);

      input_params[input_param_index].type = opr_solution.GetUopParameterType();
      input_params[input_param_index].value = opr_value;
      input_param_index++;
    }

    uint8_t output_param_count = 0;
    UopParameter output_params[MAX_OUTPUT_PARAM_COUNT];

    //This is a temporary bypass of the usual Uop interface for the sake of trying solving with new data processing operations.
    bool success = false;
    if (mUop < UopAndShift32)
    {
      success = execute_uop(mCpuId, mUop, input_params, input_param_count, output_params, &output_param_count);
    }

    if (!success) {
      LOG(fail) << "{AddressSolutionStrategy::Solve} execute_uop() failed." << endl;
      FAIL("execute-uop-failure");
    }

    if (output_param_count > MAX_OUTPUT_PARAM_COUNT) {
      LOG(fail) << "{AddressSolutionStrategy::Solve} execute_uop() returned " << output_param_count << "parameters, exceeding the maximum of " << MAX_OUTPUT_PARAM_COUNT << endl;
      FAIL("execute-uop-failure");
    }

    rTargetAddress = output_params[0].value;

    if (pAddrTagging != nullptr)
    {
      bool is_instruction = false;
      uint64 untagged_target_address = pAddrTagging->UntagAddress(rTargetAddress, is_instruction);
      return IsTargetAddressValidAndAligned(untagged_target_address);
    }

    return IsTargetAddressValidAndAligned(rTargetAddress);
  }

  bool AddressSolutionStrategy::HasRepeatedRegisters(const OperandSolutionMap& rOprSolutions) const
  {
    set<string> reg_name_set;
    for (const auto& opr_solution_entry : rOprSolutions) {
      const OperandSolution& opr_solution = opr_solution_entry.second;
      const Register* reg = opr_solution.GetRegister();

      if (reg != nullptr) {
        auto insert_res = reg_name_set.insert(reg->Name());
        if (not insert_res.second) {
          return true;
        }
      }
    }

    return false;
  }

  OperandSolution* AddressSolutionStrategy::GetOperandSolution(const string& rOprRole, OperandSolutionMap* pOprSolutions) const
  {
    OperandSolution* opr_solution = nullptr;
    auto itr = pOprSolutions->find(rOprRole);
    if (itr != pOprSolutions->end()) {
      opr_solution = &(itr->second);
    }
    else {
      LOG(fail) << "{AddressSolutionStrategy::GetOperandSolution} failed to locate operand solution for " << rOprRole << endl;
      FAIL("missing-operand-solution");
    }

    return opr_solution;
  }

  const ConstraintSet* AddressSolutionStrategy::GetAddressConstraint() const
  {
    return mpAddressConstr;
  }

  uint32 AddressSolutionStrategy::GetAlignShift() const
  {
    return mAlignShift;
  }

  uint32 AddressSolutionStrategy::GetCpuId() const
  {
    return mCpuId;
  }

  MulAddSolutionStrategy::MulAddSolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId)
    : AddressSolutionStrategy(pAddressConstr, alignShift, uop, cpuId)
  {
  }

  bool MulAddSolutionStrategy::Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging) const
  {
    if (HasRepeatedRegisters(*pOprSolutions)) {
      return false;
    }

    uint64 multiplicand = 0;
    uint64 multiplier = 0;
    uint64 addend = 0;
    bool solved = false;

    OperandSolution* multiplicand_opr_solution = GetOperandSolution("multiplicand", pOprSolutions);
    const ConstraintSet* multiplicand_constr = multiplicand_opr_solution->GetConstraint();
    OperandSolution* multiplier_opr_solution = GetOperandSolution("multiplier", pOprSolutions);
    const ConstraintSet* multiplier_constr = multiplier_opr_solution->GetConstraint();
    OperandSolution* addend_opr_solution = GetOperandSolution("addend", pOprSolutions);
    const ConstraintSet* addend_constr = addend_opr_solution->GetConstraint();

    // Solving for the addend is faster, so attempt that first if the addend value is not fixed.
    if (addend_constr->Size() != 1) {
      multiplicand = multiplicand_constr->ChooseValue();
      multiplier = multiplier_constr->ChooseValue();
      solved = SolveForAddend(multiplicand, multiplier, *addend_constr, addend);
    }
    // Else if multiplicand value is not fixed
    else if (multiplicand_constr->Size() != 1) {
      multiplier = multiplier_constr->ChooseValue();
      addend = addend_constr->OnlyValue();
      solved = SolveForMultiplicationOperand(multiplier, addend, *multiplicand_constr, multiplicand);
    }
    // Else if multiplier value is not fixed
    else if (multiplier_constr->Size() != 1) {
      multiplicand = multiplicand_constr->OnlyValue();
      addend = addend_constr->OnlyValue();
      solved = SolveForMultiplicationOperand(multiplicand, addend, *multiplier_constr, multiplier);
    }
    // Else all operands are fixed
    else {
      multiplicand = multiplicand_constr->OnlyValue();
      multiplier = multiplier_constr->OnlyValue();
      addend = addend_constr->OnlyValue();

      rTargetAddress = ComputeTargetAddress(multiplicand, multiplier, addend);

      if (pAddrTagging != nullptr)
      {
        bool is_instruction = false;
        uint64 untagged_target_address = pAddrTagging->UntagAddress(rTargetAddress, is_instruction);
        solved = IsTargetAddressValidAndAligned(untagged_target_address);
      }
      else
      {
        solved = IsTargetAddressValidAndAligned(rTargetAddress);
      }
    }

    if (solved) {
      rTargetAddress = ComputeTargetAddress(multiplicand, multiplier, addend);
      multiplicand_opr_solution->SetValue(multiplicand);
      multiplier_opr_solution->SetValue(multiplier);
      addend_opr_solution->SetValue(addend);
    }

    return solved;
  }

  bool MulAddSolutionStrategy::SolveForMultiplicationOperand(cuint64 indOperandValue, cuint64 addend, const ConstraintSet& rDepOperandConstr, uint64& rDepOperandValue) const
  {
    const ConstraintSet* address_constr = GetAddressConstraint();
    unique_ptr<ConstraintSet> solution_constr(address_constr->Clone());
    solution_constr->SubtractFromElements(addend);
    if (indOperandValue != 0) {
      solution_constr->ShiftRight(GetAlignShift());
      DivideConstraintElements(indOperandValue, solution_constr.get());
      solution_constr->ApplyConstraintSet(rDepOperandConstr);

      if (not solution_constr->IsEmpty()) {
        rDepOperandValue = solution_constr->ChooseValue() << GetAlignShift();
        return true;
      }
      else {
        return false;
      }
    }
    else {
      // If one of the multiplication operands is 0, the multiplication will have no effect, so we can choose the
      // unknown multiplication operand subject only to its own constraint.
      if (solution_constr->ContainsValue(addend)) {
        rDepOperandValue = rDepOperandConstr.ChooseValue();
        return true;
      }
      else {
        return false;
      }
    }
  }

  bool MulAddSolutionStrategy::SolveForAddend(cuint64 multiplicand, cuint64 multiplier, const ConstraintSet& rAddendConstr, uint64& rAddendValue) const
  {
    const ConstraintSet* address_constr = GetAddressConstraint();
    unique_ptr<ConstraintSet> solution_constr(address_constr->Clone());
    solution_constr->SubtractFromElements(multiplicand * multiplier);
    solution_constr->ApplyConstraintSet(rAddendConstr);
    solution_constr->ShiftRight(GetAlignShift());

    if (not solution_constr->IsEmpty()) {
      rAddendValue = solution_constr->ChooseValue() << GetAlignShift();
      return true;
    }
    else {
      return false;
    }
  }

  uint64 MulAddSolutionStrategy::ComputeTargetAddress(cuint64 multiplicand, cuint64 multiplier, cuint64 addend) const
  {
    uint8_t input_param_count = 3;
    UopParameter input_params[input_param_count];
    for (uint8_t i = 0; i < input_param_count; i++) {
      input_params[i].type = UopParamUInt64;
    }

    input_params[0].value = multiplicand;
    input_params[1].value = multiplier;
    input_params[2].value = addend;

    uint8_t output_param_count = 1;
    UopParameter output_param;
    output_param.type = UopParamUInt64;
    output_param.value = 0;

    bool success = execute_uop(GetCpuId(), UopMulAdd, input_params, input_param_count, &output_param, &output_param_count);
    if (!success) {
      LOG(fail) << "{MulAddSolutionStrategy::ComputeTargetAddress} execute_uop() failed for UopMulAdd operation" << endl;
      FAIL("execute-uop-failure");
    }

    return output_param.value;
  }

  void MulAddSolutionStrategy::DivideConstraintElements(cuint64 divisor, ConstraintSet* pConstr) const
  {
    uint8 factor_range_length = 10;
    if (factor_range_length >= divisor) {
      factor_range_length = divisor - 1;
    }

    // Enumerating the entire solution space is too time-consuming, so we use the factor range here to randomly select a
    // portion of it to calculate.
    uint64 factor_lower_bound = Random::Instance()->Random64(0, divisor - factor_range_length - 1);
    pConstr->DivideElementsWithFactorRangeUnionedWithZero(divisor, factor_lower_bound, factor_lower_bound + factor_range_length);
  }

  DivSolutionStrategy::DivSolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId, cbool signedOperands)
    : AddressSolutionStrategy(pAddressConstr, alignShift, uop, cpuId), mSignedOperands(signedOperands)
  {
  }

  bool DivSolutionStrategy::Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging) const
  {
    if (HasRepeatedRegisters(*pOprSolutions)) {
      return false;
    }

    OperandSolution* dividend_opr_solution = GetOperandSolution("dividend", pOprSolutions);
    const ConstraintSet* dividend_constr = dividend_opr_solution->GetConstraint();
    OperandSolution* divisor_opr_solution = GetOperandSolution("divisor", pOprSolutions);
    const ConstraintSet* divisor_constr = divisor_opr_solution->GetConstraint();

    uint64 dividend = dividend_constr->ChooseValue();
    uint64 divisor = divisor_constr->ChooseValue();

    bool solved = false;
    if (divisor != 0) {
      rTargetAddress = ComputeTargetAddress(dividend, divisor);

      if (pAddrTagging != nullptr)
      {
        bool is_instruction = false;
        uint64 untagged_target_address = pAddrTagging->UntagAddress(rTargetAddress, is_instruction);
        solved = IsTargetAddressValidAndAligned(untagged_target_address);
      }
      else
      {
        solved = IsTargetAddressValidAndAligned(rTargetAddress);
      }
    }

    if (solved) {
      dividend_opr_solution->SetValue(dividend);
      divisor_opr_solution->SetValue(divisor);
    }

    return solved;
  }

  uint64 DivSolutionStrategy::ComputeTargetAddress(cuint64 dividend, cuint64 divisor) const
  {
    uint8_t input_param_count = 3;
    UopParameter input_params[input_param_count];
    input_params[0].type = UopParamUInt64;
    input_params[0].value = dividend;
    input_params[1].type = UopParamUInt64;
    input_params[1].value = divisor;
    input_params[2].type = UopParamBool;
    input_params[2].value = mSignedOperands;

    uint8_t output_param_count = 1;
    UopParameter output_param;
    output_param.type = UopParamUInt64;
    output_param.value = 0;

    bool success = execute_uop(GetCpuId(), UopDiv, input_params, input_param_count, &output_param, &output_param_count);
    if (!success) {
      LOG(fail) << "{DivSolutionStrategy::ComputeTargetAddress} execute_uop() failed for UopDiv operation" << endl;
      FAIL("execute-uop-failure");
    }

    return output_param.value;
  }

  AddWithCarrySolutionStrategy::AddWithCarrySolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId, const ConditionFlags condFlags)
    : AddressSolutionStrategy(pAddressConstr, alignShift, uop, cpuId), mCondFlags(condFlags)
  {
  }

  bool AddWithCarrySolutionStrategy::Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging) const
  {
    if (HasRepeatedRegisters(*pOprSolutions)) {
      return false;
    }

    uint64 addend1 = 0;
    uint64 addend2 = 0;
    bool solved = false;

    OperandSolution* addend1_opr_solution = GetOperandSolution("addend1", pOprSolutions);
    const ConstraintSet* addend1_constr = addend1_opr_solution->GetConstraint();
    OperandSolution* addend2_opr_solution = GetOperandSolution("addend2", pOprSolutions);
    const ConstraintSet* addend2_constr = addend2_opr_solution->GetConstraint();

    // If addend1 value is not fixed
    if (addend1_constr->Size() != 1) {
      addend2 = addend2_constr->ChooseValue();
      solved = SolveForAdditionOperand(addend2, *addend1_constr, addend1);
    }
    // Else if addend2 value is not fixed
    else if (addend2_constr->Size() != 1) {
      addend1 = addend1_constr->OnlyValue();
      solved = SolveForAdditionOperand(addend1, *addend2_constr, addend2);
    }
    // Else both operands are fixed
    else {
      addend1 = addend1_constr->OnlyValue();
      addend2 = addend2_constr->OnlyValue();

      rTargetAddress = ComputeTargetAddress(addend1, addend2);

      if (pAddrTagging != nullptr)
      {
        bool is_instruction = false;
        uint64 untagged_target_address = pAddrTagging->UntagAddress(rTargetAddress, is_instruction);
        solved = IsTargetAddressValidAndAligned(untagged_target_address);
      }
      else
      {
        solved = IsTargetAddressValidAndAligned(rTargetAddress);
      }
    }

    if (solved) {
      rTargetAddress = ComputeTargetAddress(addend1, addend2);
      addend1_opr_solution->SetValue(addend1);
      addend2_opr_solution->SetValue(addend2);
    }

    return solved;
  }

  bool AddWithCarrySolutionStrategy::SolveForAdditionOperand(cuint64 indOperandValue, const ConstraintSet& rDepOperandConstr, uint64& rDepOperandValue) const
  {
    const ConstraintSet* address_constr = GetAddressConstraint();
    unique_ptr<ConstraintSet> solution_constr(address_constr->Clone());
    solution_constr->SubtractFromElements(indOperandValue + mCondFlags.mCarryFlag);
    solution_constr->ApplyConstraintSet(rDepOperandConstr);
    solution_constr->ShiftRight(GetAlignShift());

    if (not solution_constr->IsEmpty()) {
      rDepOperandValue = solution_constr->ChooseValue() << GetAlignShift();
      return true;
    }
    else {
      return false;
    }
  }

  uint64 AddWithCarrySolutionStrategy::ComputeTargetAddress(cuint64 addend1, cuint64 addend2) const
  {
    uint8_t input_param_count = 3;
    UopParameter input_params[input_param_count];
    input_params[0].value = addend1;
    input_params[0].type = UopParamUInt64;
    input_params[1].value = addend2;
    input_params[1].type = UopParamUInt64;
    input_params[2].value = mCondFlags.mCarryFlag;
    input_params[2].type = UopParamBool;

    uint8_t output_param_count = 1;
    UopParameter output_param;
    output_param.type = UopParamUInt64;
    output_param.value = 0;

    bool success = execute_uop(GetCpuId(), UopAddWithCarry, input_params, input_param_count, &output_param, &output_param_count);
    if (!success) {
      LOG(fail) << "{AddWithCarrySolutionStrategy::ComputeTargetAddress} execute_uop() failed for UopAddWithCarry operation" << endl;
      FAIL("execute-uop-failure");
    }

    return output_param.value;
  }

  SubWithCarrySolutionStrategy::SubWithCarrySolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId, const ConditionFlags condFlags)
    : AddressSolutionStrategy(pAddressConstr, alignShift, uop, cpuId), mCondFlags(condFlags)
  {
  }

  bool SubWithCarrySolutionStrategy::Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging) const
  {
    if (HasRepeatedRegisters(*pOprSolutions)) {
      return false;
    }

    uint64 minuend = 0;
    uint64 subtrahend = 0;
    bool solved = false;

    OperandSolution* minuend_opr_solution = GetOperandSolution("minuend", pOprSolutions);
    const ConstraintSet* minuend_constr = minuend_opr_solution->GetConstraint();
    OperandSolution* subtrahend_opr_solution = GetOperandSolution("subtrahend", pOprSolutions);
    const ConstraintSet* subtrahend_constr = subtrahend_opr_solution->GetConstraint();

    // If minuend value is not fixed
    if (minuend_constr->Size() != 1) {
      subtrahend = subtrahend_constr->ChooseValue();
      solved = SolveForMinuend(subtrahend, *minuend_constr, minuend);
    }
    // Else if subtrahend value is not fixed
    else if (subtrahend_constr->Size() != 1) {
      minuend = minuend_constr->OnlyValue();
      solved = SolveForSubtrahend(minuend, *subtrahend_constr, subtrahend);
    }
    // Else both operands are fixed
    else {
      minuend = minuend_constr->OnlyValue();
      subtrahend = subtrahend_constr->OnlyValue();

      rTargetAddress = ComputeTargetAddress(minuend, subtrahend);

      if (pAddrTagging != nullptr)
      {
        bool is_instruction = false;
        uint64 untagged_target_address = pAddrTagging->UntagAddress(rTargetAddress, is_instruction);
        solved = IsTargetAddressValidAndAligned(untagged_target_address);
      }
      else
      {
        solved = IsTargetAddressValidAndAligned(rTargetAddress);
      }
    }

    if (solved) {
      rTargetAddress = ComputeTargetAddress(minuend, subtrahend);
      minuend_opr_solution->SetValue(minuend);
      subtrahend_opr_solution->SetValue(subtrahend);
    }

    return solved;
  }

  bool SubWithCarrySolutionStrategy::SolveForMinuend(cuint64 subtrahendValue, const ConstraintSet& rMinuendConstr, uint64& rMinuendValue) const
  {
    const ConstraintSet* address_constr = GetAddressConstraint();
    unique_ptr<ConstraintSet> solution_constr(address_constr->Clone());
    solution_constr->SubtractFromElements(~subtrahendValue + mCondFlags.mCarryFlag);
    solution_constr->ApplyConstraintSet(rMinuendConstr);
    solution_constr->ShiftRight(GetAlignShift());

    if (not solution_constr->IsEmpty()) {
      rMinuendValue = solution_constr->ChooseValue() << GetAlignShift();
      return true;
    }
    else {
      return false;
    }
  }

  bool SubWithCarrySolutionStrategy::SolveForSubtrahend(cuint64 minuendValue, const ConstraintSet& rSubtrahendConstr, uint64& rSubtrahendValue) const
  {
    const ConstraintSet* address_constr = GetAddressConstraint();
    unique_ptr<ConstraintSet> solution_constr(address_constr->Clone());
    solution_constr->SubtractFromElements(minuendValue + mCondFlags.mCarryFlag);
    solution_constr->NotElements();
    solution_constr->ApplyConstraintSet(rSubtrahendConstr);
    solution_constr->ShiftRight(GetAlignShift());

    if (not solution_constr->IsEmpty()) {
      rSubtrahendValue = solution_constr->ChooseValue() << GetAlignShift();
      return true;
    }
    else {
      return false;
    }
  }

  uint64 SubWithCarrySolutionStrategy::ComputeTargetAddress(cuint64 minuend, cuint64 subtrahend) const
  {
    uint8_t input_param_count = 3;
    UopParameter input_params[input_param_count];
    input_params[0].value = minuend;
    input_params[0].type = UopParamUInt64;
    input_params[1].value = subtrahend;
    input_params[1].type = UopParamUInt64;
    input_params[2].value = mCondFlags.mCarryFlag;
    input_params[2].type = UopParamBool;

    uint8_t output_param_count = 1;
    UopParameter output_param;
    output_param.type = UopParamUInt64;
    output_param.value = 0;

    bool success = execute_uop(GetCpuId(), UopSubWithCarry, input_params, input_param_count, &output_param, &output_param_count);
    if (!success) {
      LOG(fail) << "{SubWithCarrySolutionStrategy::ComputeTargetAddress} execute_uop() failed for UopSubWithCarry operation" << endl;
      FAIL("execute-uop-failure");
    }

    return output_param.value;
  }

  MulSolutionStrategy::MulSolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId)
    : MulAddSolutionStrategy(pAddressConstr, alignShift, uop, cpuId)
  {
  }

  bool MulSolutionStrategy::Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging) const
  {
    if (HasRepeatedRegisters(*pOprSolutions)) {
      return false;
    }

    uint64 multiplicand = 0;
    uint64 multiplier = 0;
    bool solved = false;

    OperandSolution* multiplicand_opr_solution = GetOperandSolution("multiplicand", pOprSolutions);
    const ConstraintSet* multiplicand_constr = multiplicand_opr_solution->GetConstraint();
    OperandSolution* multiplier_opr_solution = GetOperandSolution("multiplier", pOprSolutions);
    const ConstraintSet* multiplier_constr = multiplier_opr_solution->GetConstraint();

    // If multiplicand value is not fixed
    if (multiplicand_constr->Size() != 1) {
      multiplier = multiplier_constr->ChooseValue();
      solved = SolveForMultiplicationOperand(multiplier, 0, *multiplicand_constr, multiplicand);
    }
    // Else if multiplier value is not fixed
    else if (multiplier_constr->Size() != 1) {
      multiplicand = multiplicand_constr->OnlyValue();
      solved = SolveForMultiplicationOperand(multiplicand, 0, *multiplier_constr, multiplier);
    }
    // Else both operands are fixed
    else {
      multiplicand = multiplicand_constr->OnlyValue();
      multiplier = multiplier_constr->OnlyValue();

      rTargetAddress = ComputeTargetAddress(multiplicand, multiplier, 0);

      if (pAddrTagging != nullptr)
      {
        bool is_instruction = false;
        uint64 untagged_target_address = pAddrTagging->UntagAddress(rTargetAddress, is_instruction);
        solved = IsTargetAddressValidAndAligned(untagged_target_address);
      }
      else
      {
        solved = IsTargetAddressValidAndAligned(rTargetAddress);
      }
    }

    if (solved) {
      rTargetAddress = ComputeTargetAddress(multiplicand, multiplier, 0);
      multiplicand_opr_solution->SetValue(multiplicand);
      multiplier_opr_solution->SetValue(multiplier);
    }

    return solved;
  }

  AddressSolutionStrategy* AddressSolutionStrategyFactory::CreateSolutionStrategy(const EDataProcessingOperationType operationType, const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId, const ConditionFlags condFlags) const
  {
    bool signed_operands = false;
    switch (operationType) {
      case EDataProcessingOperationType::MulAdd:
        return new MulAddSolutionStrategy(pAddressConstr, alignShift, uop, cpuId);
      case EDataProcessingOperationType::Mul:
        return new MulSolutionStrategy(pAddressConstr, alignShift, uop, cpuId);
      case EDataProcessingOperationType::UDiv:
        return new DivSolutionStrategy(pAddressConstr, alignShift, uop, cpuId, signed_operands);
      case EDataProcessingOperationType::SDiv:
        signed_operands = true;
        return new DivSolutionStrategy(pAddressConstr, alignShift, uop, cpuId, signed_operands);
      case EDataProcessingOperationType::AddWithCarry:
        return new AddWithCarrySolutionStrategy(pAddressConstr, alignShift, uop, cpuId, condFlags);
      case EDataProcessingOperationType::SubWithCarry:
        return new SubWithCarrySolutionStrategy(pAddressConstr, alignShift, uop, cpuId, condFlags);
      default:
        return new AddressSolutionStrategy(pAddressConstr, alignShift, uop, cpuId);
    }

    return nullptr;
  }

}
