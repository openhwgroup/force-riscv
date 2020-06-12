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
#include <OperandConstraint.h>
#include <Choices.h>
#include <InstructionStructure.h>
#include <Generator.h>
#include <ChoicesModerator.h>
#include <GenException.h>
#include <Instruction.h>
#include <Constraint.h>
#include <Operand.h>
#include <OperandRequest.h>
#include <Register.h>
#include <RegisterReserver.h>
#include <InstructionConstraint.h>
#include <GenRequest.h>
#include <PageRequestRegulator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <Log.h>

#include <memory>

using namespace std;

namespace Force {

  OperandConstraint::OperandConstraint(const OperandConstraint& rOther)
    : mpConstraintSet(nullptr), mConstraintForced(false)
  {
    LOG(fail) << "OperandConstraint copy constructor not meant to be called." << endl;
    FAIL("calling-operand-constraint-copy-constructor-unexpectedly");
  }

  OperandConstraint::~OperandConstraint()
  {
    delete mpConstraintSet;
  }

  void OperandConstraint::ApplyUserRequest(const OperandRequest& rOprReq)
  {
    auto value_constr = rOprReq.GetValueConstraint();
    if (nullptr != value_constr) {
      if (nullptr != mpConstraintSet) {
        LOG(fail) << "{OperandConstraint::ApplyUserRequest} expect mpConstraintSet to be nullptr at this point." << endl;
        FAIL("dangling-constraint-set-pointer");
      }
      mpConstraintSet = value_constr->Clone();
      if (mpConstraintSet->Size() == 1) {
        mConstraintForced = true;
      }
      rOprReq.SetApplied();
    }
  }

  void OperandConstraint::SubConstraintValue(uint64 value, const OperandStructure& rOperandStruct) const
  {
    if (nullptr == mpConstraintSet) {
      mpConstraintSet = DefaultConstraintSet(rOperandStruct);
    }
    mpConstraintSet->SubValue(value);
  }

  ConstraintSet* OperandConstraint::DefaultConstraintSet(const OperandStructure& rOperandStruct) const
  {
    return new ConstraintSet(0, (1u << rOperandStruct.mSize) - 1);
  }

  void OperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {

  }

  void ImmediatePartialOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    ImmediateOperandConstraint::Setup(rGen, rInstr, rOperandStruct);
    //TODO check if mAllowReserved should be set
  }

  /*!
    Assuming supported operands either has reserved values or exclude values, but not both.
  */
  bool ImmediatePartialOperandConstraint::HasConstraint() const
  {
    if (ImmediateOperandConstraint::HasConstraint()) return true;
    if (not mAllowReserved && HasReservedConstraint()) return true;
    if (HasExcludeConstraint()) return true;
    return false;
  }

  /*!
    Assuming supported operands either has reserved values or exclude values, but not both.
  */
  const ConstraintSet* ImmediatePartialOperandConstraint::GetConstraint() const
  {
    if (ImmediateOperandConstraint::HasConstraint()) {
      if (mpConstraintSet->Size() == 1) {
        // forced user constraint
        return mpConstraintSet;
      }

      if (not mAllowReserved && HasReservedConstraint()) {
        const ConstraintSet* reserved_constr = GetReservedConstraint();

        if (reserved_constr != nullptr) {
          mpConstraintSet->ApplyConstraintSet(*reserved_constr);
        }
        else {
          LOG(fail) << "{ImmediatePartialOperandConstraint::GetConstraint} reserved constraint is not valid";
          FAIL("invalid-constraint");
        }
      }

      if (HasExcludeConstraint()) {
        const ConstraintSet* exclude_constr = GetExcludeConstraint();

        if (exclude_constr != nullptr) {
          mpConstraintSet->ApplyConstraintSet(*exclude_constr);
        }
        else {
          LOG(fail) << "{ImmediatePartialOperandConstraint::GetConstraint} exclude constraint is not valid";
          FAIL("invalid-constraint");
        }
      }
    }
    else {
      if (not mAllowReserved && HasReservedConstraint()) {
        return GetReservedConstraint();
      }
      if (HasExcludeConstraint()) {
        return GetExcludeConstraint();
      }
    }
    return mpConstraintSet;
  }

  ChoicesOperandConstraint::~ChoicesOperandConstraint()
  {
    delete mpChoiceTree;
  }

  void ChoicesOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    const ChoicesModerator* choices_mod = rGen.GetChoicesModerator(EChoicesType::OperandChoices);
    auto cast_struct = rOperandStruct.CastOperandStructure<ChoicesOperandStructure>();
    if (cast_struct->mChoices.size() < 1) {
      LOG(fail) << "{ChoicesOperandConstraint::Setup} expecting at least one choices tree for operand \"" << rOperandStruct.mName << "\"." << endl;
      FAIL("no-choices-tree-found");
    }

    try {
      mpChoiceTree = choices_mod->CloneChoiceTree(cast_struct->mChoices[0]);
    }
    catch (const ChoicesError& rChoicesErr) {
      LOG(fail) << "{ChoicesOperandConstraint::Setup} instruction: " << rInstr.FullName() << " operand: " << rOperandStruct.mName << " " << rChoicesErr.what() << endl;
      FAIL("choices-setup-error");
    }
  }

  ChoiceTree* ChoicesOperandConstraint::SetupExtraChoiceTree(uint32 index, const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    const ChoicesModerator* choices_mod = rGen.GetChoicesModerator(EChoicesType::OperandChoices);
    auto cast_struct = rOperandStruct.CastOperandStructure<ChoicesOperandStructure>();
    if (cast_struct->mChoices.size() <= index) {
      LOG(fail) << "{ChoicesOperandConstraint::SetupExtraChoiceTree} expecting at least " << dec << (index + 1) << " choices tree for operand \"" << rOperandStruct.mName << "\"." << endl;
      FAIL("incorrect-number-of-choices-tree");
    }

    ChoiceTree* ret_tree = nullptr;

    try {
      ret_tree = choices_mod->CloneChoiceTree(cast_struct->mChoices[index]);
    }
    catch (const ChoicesError& rChoicesErr) {
      LOG(fail) << "{ChoicesOperandConstraint::SetupExtraChoiceTree} instruction: " << rInstr.FullName() << " operand: " << rOperandStruct.mName << " " << rChoicesErr.what() << std::endl;
      FAIL("choices-setup-error");
    }

    return ret_tree;
  }

  void RegisterOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
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

    // << "YES reservation constraints for: " << rOperandStruct.Name() << " read constr? " << (read_reserv_constr != nullptr)  << " write constr? " << (write_reserv_constr != nullptr) << endl;
    if (nullptr == mpConstraintSet) {
      mpConstraintSet = DefaultConstraintSet(rOperandStruct);
    }
    if (nullptr != read_reserv_constr) {
      // << "read constr " << read_reserv_constr->ToSimpleString() << endl;
      mpConstraintSet->SubConstraintSet(*read_reserv_constr);
    }
    if (nullptr != write_reserv_constr) {
      // << " write constr " << write_reserv_constr->ToSimpleString() << endl;
      mpConstraintSet->SubConstraintSet(*write_reserv_constr);
    }
  }

  void RegisterOperandConstraint::AddWriteConstraint(const Generator& rGen, const OperandStructure& rOperandStruct)
  {
    if (mConstraintForced) {
      // constraint already forced, ignore reservation check
      return;
    }

    if (rOperandStruct.HasWriteAccess()) {
      // already checked write access.
      return;
    }

    auto reg_file = rGen.GetRegisterFile();
    const ConstraintSet* read_reserv_constr = nullptr;
    const ConstraintSet* write_reserv_constr = nullptr;
    if (not reg_file->GetRegisterReserver()->HasReservations(rOperandStruct.mType, ERegAttrType::Write, read_reserv_constr, write_reserv_constr)) {
      // << "no reservation constraints for: " << rOperandStruct.Name() << endl;
      return;
    }

    // << "YES reservation constraints for: " << rOperandStruct.Name() << " read constr? " << (read_reserv_constr != nullptr) << " write constr? " << (write_reserv_constr != nullptr) << endl;
    if (nullptr == mpConstraintSet) {
      mpConstraintSet = DefaultConstraintSet(rOperandStruct);
    }
    if (nullptr != write_reserv_constr) {
      // << " write constr " << write_reserv_constr->ToSimpleString() << endl;
      mpConstraintSet->SubConstraintSet(*write_reserv_constr);
    }
  }

  const OperandDataRequest* RegisterOperandConstraint::GetOperandDataRequest(const Instruction& rInstr, const std::string& rOprName)
  {
    auto instr_req = rInstr.GetInstructionConstraint()->InstructionRequest();
    return instr_req->FindOperandDataRequest(rOprName);
  }

  void ImpliedRegisterOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    auto reg_file = rGen.GetRegisterFile();
    auto implied_reg = reg_file->RegisterLookup(rOperandStruct.Name());
    mRegisterIndex = implied_reg->IndexValue();

    if (mConstraintForced) {
      LOG(info) << "constraint already forced, ignore reservation check" << endl;
      return;
    }

    const ConstraintSet* read_reserv_constr = nullptr;
    const ConstraintSet* write_reserv_constr = nullptr;
    if (nullptr == mpConstraintSet)
      LOG(notice) << "register constraint for: " << rOperandStruct.Name() << " is empty" << endl;
    if (not reg_file->GetRegisterReserver()->HasReservations(rOperandStruct.mType, rOperandStruct.mAccess, read_reserv_constr, write_reserv_constr)) {
      LOG(notice) << "no reservation constraints for: " << rOperandStruct.Name() << endl;
      return;
    }

    if (nullptr == mpConstraintSet) {
      mpConstraintSet = new ConstraintSet(mRegisterIndex);
    }
    LOG(notice)  << "register constraints for: " << rOperandStruct.Name() << " is " << mpConstraintSet->ToSimpleString() << endl;
    LOG(notice) << "YES reservation constraints for: " << rOperandStruct.Name() << " read constr? " << (read_reserv_constr != nullptr) << " write constr? " << (write_reserv_constr != nullptr) << endl;
    if (nullptr != read_reserv_constr) {
      LOG(notice) << "read constr " << read_reserv_constr->ToSimpleString() << endl;
      mpConstraintSet->SubConstraintSet(*read_reserv_constr);
    }
    if (nullptr != write_reserv_constr) {
      LOG(notice) << " write constr " << write_reserv_constr->ToSimpleString() << endl;
      mpConstraintSet->SubConstraintSet(*write_reserv_constr);
    }
  }

  void ImmediateExcludeOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    mpOperandStructure = dynamic_cast<const ExcludeOperandStructure* >(&rOperandStruct);
    if (nullptr == mpOperandStructure) {
      LOG(fail) << "{ImmediateExcludeOperandConstraint::Setup} expecting operand " << rOperandStruct.mName << " to be \"ExcludeOperandStructure\" type." << endl;
      FAIL("expecting-exclude-operand-structure");
    }
  }

  const ConstraintSet* ImmediateExcludeOperandConstraint::GetExcludeConstraint() const
  {
    return mpOperandStructure->ExcludeConstraint();
  }

  ImmediateGe1OperandConstraint::~ImmediateGe1OperandConstraint()
  {
    delete mpExcludeConstraint;
  }

  void ImmediateGe1OperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    if (mpExcludeConstraint == nullptr)
      mpExcludeConstraint = new ConstraintSet(1, (1u << rOperandStruct.mSize) - 1);
  }

  const ConstraintSet* ImmediateGe1OperandConstraint::GetExcludeConstraint() const
  {
      return mpExcludeConstraint;
  }

  AddressingOperandConstraint::AddressingOperandConstraint()
    : GroupOperandConstraint(), mPC(0), mUsePreamble(false), mNoPreamble(false), mpPageRequest(nullptr), mpTargetConstraint(nullptr), mpVmMapper(nullptr), mDataConstraints()
  {
  }

  AddressingOperandConstraint::AddressingOperandConstraint(const AddressingOperandConstraint& rOther)
    : GroupOperandConstraint(rOther), mPC(0), mUsePreamble(false), mNoPreamble(false), mpPageRequest(nullptr), mpTargetConstraint(nullptr), mpVmMapper(nullptr), mDataConstraints()
  {
  }

  void AddressingOperandConstraint::SetupVmMapper(const Generator& rGen)
  {
    mpVmMapper = rGen.GetVmManager()->CurrentVmMapper();
  }

  void AddressingOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    mPC = rGen.PC();

    if (rInstr.NoPreamble() or rInstr.NoRestriction()) {
      mUsePreamble = false;
      mNoPreamble = true; // absolutely no preamble.
    }
    else if (rGen.AddressProtection()) {
      mUsePreamble = true;
      mNoPreamble = false;
    }
    else {
      const ChoicesModerator* choices_mod = rGen.GetChoicesModerator(EChoicesType::OperandChoices);
      auto choices_raw = choices_mod->CloneChoiceTree("Use addressing preamble");
      std::unique_ptr<ChoiceTree> choices_tree(choices_raw);
      auto chosen_ptr = choices_tree->Choose();
      mUsePreamble = (chosen_ptr->Value() != 0);
      mNoPreamble = (not mUsePreamble) and choices_tree->OnlyChoice();
    }

    auto addr_op_struct = rOperandStruct.CastOperandStructure<AddressingOperandStructure>();
    mpPageRequest = rGen.GenPageRequestInstance(IsInstruction(), addr_op_struct->MemAccessType());
    if (rInstr.NoDataAbort()){
      mpPageRequest->SetGenBoolAttribute(EPageGenBoolAttrType::NoDataAbort, true);
    }
    SetupVmMapper(rGen);
    // << "instruction " << rInstr.Name() << " use preamble? " << mUsePreamble << " no preamble? " << mNoPreamble << endl;
  }

  AddressingOperandConstraint::~AddressingOperandConstraint()
  {
    delete mpPageRequest;
    delete mpTargetConstraint;
    mpVmMapper = nullptr;
  }

  bool AddressingOperandConstraint::TargetConstraintForced() const
  {
    return (nullptr != mpTargetConstraint) && (mpTargetConstraint->Size() == 1);
  }

  SignedImmediateOperand* AddressingOperandConstraint::GetSignedOffsetOperand(const Instruction& rInstr, const OperandStructure& rOperandStruct) const
  {
    auto addr_struct = dynamic_cast<const AddressingOperandStructure* >(&rOperandStruct);
    if (nullptr == addr_struct) {
      LOG(fail) << "{AddressingOperandConstraint::GetSignedOffsetOperand} expecting operand " << rOperandStruct.mName << " to be \"AddressingOperandStructure\" type." << endl;
      FAIL("expecting-addressing-operand-structure");
    }
    auto offset_ptr = rInstr.FindOperandMutable(addr_struct->Offset(), true);
    auto signed_opr = dynamic_cast<SignedImmediateOperand* >(offset_ptr);
    if (nullptr == signed_opr) {
      LOG(fail) << "{AddressingOperandConstraint::GetSignedOffsetOperand} expecting operand " << offset_ptr->Name() << " to be \"SignedImmediateOperand\" type." << endl;
      FAIL("expecting-signed-immediate_operand");
    }

    return signed_opr;
  }

  void AddressingOperandConstraint::GetBaseConstraint(uint64 baseValue, uint64 maxAddress, uint32 accessSize, ConstraintSet& rResultConstr) const
  {
    uint64 range_upper = baseValue + (accessSize - 1);
    if (range_upper < baseValue) {
      // overflowed
      rResultConstr.AddRange(baseValue, maxAddress);
      rResultConstr.AddRange(0, range_upper);
    } else {
      rResultConstr.AddRange(baseValue, range_upper);
    }
  }

  void AddressingOperandConstraint::SetUsePreamble(Generator& rGen)
  {
    mUsePreamble = true;

    vector<const RegisterOperand* > reg_vec;
    GetRegisterOperands(reg_vec);

    for (auto reg_opr : reg_vec) {
      reg_opr->AddWriteConstraint(rGen);
    }
  }

  void BranchOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    AddressingOperandConstraint::Setup(rGen, rInstr, rOperandStruct);

    const ConstraintSet* branch_target_constr = rInstr.BranchTargetConstraint();
    if (branch_target_constr != nullptr) {
      mpTargetConstraint = branch_target_constr->Clone();
    }

    auto br_struct = rOperandStruct.CastOperandStructure<BranchOperandStructure>();
    rGen.GetPageRequestRegulator()->RegulateBranchPageRequest(rGen.GetVmManager()->CurrentVmMapper(), br_struct, mpPageRequest);
    mSimulationEnabled = rGen.SimulationEnabled();
  }

  void RegisterBranchOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    BranchOperandConstraint::Setup(rGen, rInstr, rOperandStruct);
    auto br_struct = dynamic_cast<const BranchOperandStructure* >(&rOperandStruct);
    if (nullptr == br_struct) {
      LOG(fail) << "{RegisterBranchOperandConstraint::Setup} expecting operand " << rOperandStruct.mName << " to be \"BranchOperandStructure\" type." << endl;
      FAIL("expecting-branch-operand-structure");
    }

    auto base_ptr = rInstr.FindOperandMutable(br_struct->Base(), true);
    mpRegisterOperand = dynamic_cast<RegisterOperand* >(base_ptr);
    if (nullptr == mpRegisterOperand) {
      LOG(fail) << "{RegisterBranchOperandConstraint::Setup} expecting operand " << base_ptr->Name() << " to be \"RegisterOperand\" type." << endl;
      FAIL("expecting-register-operand");
    }

    // pick up the PC alignement setting
    if (rInstr.UnalignedPC()) {
      mUnalignedPC = true;
    }
    else {
      const ChoicesModerator* choices_mod = rGen.GetChoicesModerator(EChoicesType::OperandChoices);
      auto choices_raw = choices_mod->CloneChoiceTree("PC alignment");
      std::unique_ptr<ChoiceTree> choices_tree(choices_raw);
      auto chosen_ptr = choices_tree->Choose();
      mUnalignedPC = (chosen_ptr->Value() != 0);
    }
  }

  void LoadStoreOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    AddressingOperandConstraint::Setup(rGen, rInstr, rOperandStruct);

    SetupTargetConstraint(rInstr, rOperandStruct);

    auto data_constrs = rInstr.LoadStoreDataConstraints();
    mDataConstraints.assign(data_constrs.begin(), data_constrs.end());
    auto ld_struct = rOperandStruct.CastOperandStructure<LoadStoreOperandStructure>();
    rGen.GetPageRequestRegulator()->RegulateLoadStorePageRequest(mpVmMapper, ld_struct, mpPageRequest);

    // pick the data alignment requirements
    if (rInstr.AlignedData()) {
      mAlignedData = EDataAlignedType::SingleDataAligned;
    }
    else {
      if (nullptr == ld_struct) {
        LOG(fail) << "{LoadStoreOperandConstraint::Setup} expecting operand " << rOperandStruct.mName << " to be \"LoadStoreOperandStructure\" type." << endl;
        FAIL("expecting-load-store-operand-structure");
      }

      const ChoicesModerator* choices_mod =  rGen.GetChoicesModerator(EChoicesType::OperandChoices);
      ChoiceTree* choices_raw;
      if (ld_struct->AtomicOrderedAccess())
        choices_raw = choices_mod->CloneChoiceTree("Ordered data alignment");
      else
        choices_raw = choices_mod->CloneChoiceTree("Data alignment");

      std::unique_ptr<ChoiceTree> choices_tree(choices_raw);
      auto chosen_ptr = choices_tree->Choose();
      mAlignedData = EDataAlignedType(chosen_ptr->Value());
    }
    if (ld_struct->SpBased()) {
      const ChoicesModerator* choices_mod = rGen.GetChoicesModerator(EChoicesType::OperandChoices);
      ChoiceTree* choices_raw = choices_mod->CloneChoiceTree("SP alignment");

      std::unique_ptr<ChoiceTree> choices_tree(choices_raw);
      auto choice_ptr = choices_tree->Choose();
      mAlignedSp = ESpAlignedType(choice_ptr->Value());
      mSpAlignment = rGen.SpAlignment();
    }
  }

  bool LoadStoreOperandConstraint::BaseOperandSpAligned() const
  {
    if (BaseOperand() == nullptr) {
      return false;
    }
    if (BaseOperand()->ChoiceText() != "SP") {
      return false;
    }
    return ESpAlignedType::Aligned == mAlignedSp;
  }

  void LoadStoreOperandConstraint::SetupTargetConstraint(const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    const ConstraintSet* load_store_target_constr = rInstr.LoadStoreTargetConstraint();
    if (load_store_target_constr != nullptr) {
      mpTargetConstraint = load_store_target_constr->Clone();
    }

    if (rInstr.SharedTarget()) {
      const ConstraintSet* shared_constr = mpVmMapper->VirtualSharedConstraintSet();

      if (not shared_constr->IsEmpty()) {
        if (mpTargetConstraint != nullptr) {
          mpTargetConstraint->ApplyLargeConstraintSet(*shared_constr);
        }
        else {
          mpTargetConstraint = shared_constr->Clone();
        }
      }

      // Ensure the entire access is within shared memory
      auto lsop_struct = rOperandStruct.CastOperandStructure<LoadStoreOperandStructure>();
      mpTargetConstraint->AlignWithSize(MAX_UINT64, lsop_struct->DataSize());
    }
  }

  void AluImmediateOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    AddressingOperandConstraint::Setup(rGen, rInstr, rOperandStruct);

    auto alu_opr_struct = dynamic_cast<const AluOperandStructure*>(&rOperandStruct);
    if (alu_opr_struct == nullptr) {
      LOG(fail) << "{AluImmediateOperandConstraint::Setup} expecting operand " << rOperandStruct.mName << " to be \"AluOperandStructure\" type." << endl;
      FAIL("expecting-alu-operand-structure");
    }

    Operand* base_ptr = rInstr.FindOperandMutable(alu_opr_struct->Base(), true);
    mpBase = dynamic_cast<RegisterOperand*>(base_ptr);
    if (mpBase == nullptr) {
      LOG(fail) << "{AluImmediateOperandConstraint::Setup} expecting operand " << base_ptr->Name() << " to be \"RegisterOperand\" type." << endl;
      FAIL("expecting-register-operand");
    }

    if (not alu_opr_struct->Immediate().empty()) {
      Operand* offset_ptr = rInstr.FindOperandMutable(alu_opr_struct->Immediate(), true);
      mpOffset = dynamic_cast<ImmediateOperand*>(offset_ptr);

      if (mpOffset == nullptr) {
        LOG(fail) << "{AluImmediateOperandConstraint::Setup} expecting operand " << offset_ptr->Name() << " to be \"ImmediateOperand\" type." << endl;
        FAIL("expecting-immediate-operand");
      }
    }

    if (not alu_opr_struct->OffsetShift().empty()) {
      Operand* offset_shift_ptr = rInstr.FindOperandMutable(alu_opr_struct->OffsetShift(), true);
      mpOffsetShift = dynamic_cast<ChoicesOperand*>(offset_shift_ptr);
      if (mpOffsetShift == nullptr) {
        LOG(fail) << "{AluImmediateOperandConstraint::Setup} expecting operand " << offset_shift_ptr->Name() << " to be \"ChoicesOperand\" type." << endl;
        FAIL("expecting-choices-operand");
      }
    }

    mResultType = 0;
    try {
      const ChoicesModerator* choices_mod = rGen.GetChoicesModerator(EChoicesType::OperandChoices);
      ChoiceTree* result_choices = choices_mod->CloneChoiceTree("ALU result");
      std::unique_ptr<ChoiceTree> result_choices_storage(result_choices);
      mResultType = result_choices->Choose()->Value();
    }
    catch (const ChoicesError& rChoicesErr) {
      LOG(fail) << "{AluImmediateOperandConstraint::Setup} " << rChoicesErr.what() << endl;
      FAIL("missing-alu-result-choices");
    }

    if (mResultType == 0) {
      // generate load store result
      rGen.GetPageRequestRegulator()->RegulateLoadStorePageRequest(mpVmMapper, nullptr, mpPageRequest);
    }
  }

  void DataProcessingOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    AddressingOperandConstraint::Setup(rGen, rInstr, rOperandStruct);

    const ConstraintSet* load_store_target_constr = rInstr.LoadStoreTargetConstraint();
    if (load_store_target_constr != nullptr) {
      mpTargetConstraint = load_store_target_constr->Clone();
    }

    auto data_proc_opr_struct = dynamic_cast<const DataProcessingOperandStructure*>(&rOperandStruct);
    if (data_proc_opr_struct == nullptr) {
      LOG(fail) << "{DataProcessingOperandConstraint::Setup} expecting operand " << rOperandStruct.mName << " to be \"DataProcessingOperandStructure\" type." << endl;
      FAIL("expecting-data-proc-operand-structure");
    }

    Operand* base_ptr = rInstr.FindOperandMutable(data_proc_opr_struct->Base(), true);
    mpBase = dynamic_cast<RegisterOperand*>(base_ptr);
    if (mpBase == nullptr) {
      LOG(fail) << "{DataProcessingOperandConstraint::Setup} expecting operand " << base_ptr->Name() << " to be \"RegisterOperand\" type." << endl;
      FAIL("expecting-register-operand");
    }

    mResultType = 0;
    try {
      const ChoicesModerator* choices_mod = rGen.GetChoicesModerator(EChoicesType::OperandChoices);
      ChoiceTree* result_choices = choices_mod->CloneChoiceTree("Data processing result");
      std::unique_ptr<ChoiceTree> result_choices_storage(result_choices);
      mResultType = result_choices->Choose()->Value();
    }
    catch (const ChoicesError& rChoicesErr) {
      LOG(fail) << "{DataProcessingOperandConstraint::Setup} " << rChoicesErr.what() << endl;
      FAIL("missing-data-processing-result-choices");
    }

    if (mResultType == 0) {
      // generate load store result
      rGen.GetPageRequestRegulator()->RegulateLoadStorePageRequest(mpVmMapper, nullptr, mpPageRequest);
    }
  }

  void BaseOffsetLoadStoreOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    LoadStoreOperandConstraint::Setup(rGen, rInstr, rOperandStruct);
    auto bols_struct = dynamic_cast<const LoadStoreOperandStructure* >(&rOperandStruct);
    if (nullptr == bols_struct) {
      LOG(fail) << "{BaseOffsetLoadStoreOperandConstraint::Setup} expecting operand " << rOperandStruct.mName << " to be \"LoadStoreOperandStructure\" type." << endl;
      FAIL("expecting-load-store-operand-structure");
    }
    auto base_ptr = rInstr.FindOperandMutable(bols_struct->Base(), true);
    mpBase = dynamic_cast<RegisterOperand* >(base_ptr);
    if (nullptr == mpBase) {
      LOG(fail) << "{BaseOffsetLoadStoreOperandConstraint::Setup} expecting operand " << base_ptr->Name() << " to be \"RegisterOperand\" type." << endl;
      FAIL("expecting-register_operand");
    }

    if (bols_struct->Offset().size() > 0) {
      auto offset_ptr = rInstr.FindOperandMutable(bols_struct->Offset(), true);
      mpOffset = dynamic_cast<ImmediateOperand* >(offset_ptr);
      if (nullptr == mpOffset) {
        LOG(fail) << "{BaseOffsetLoadStoreOperandConstraint::Setup} expecting operand " << offset_ptr->Name() << " to be \"ImmediateOperand\" type." << endl;
        FAIL("expecting-immediate_operand");
      }
    }
  }

  void BaseIndexLoadStoreOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    LoadStoreOperandConstraint::Setup(rGen, rInstr, rOperandStruct);
    auto bils_struct = dynamic_cast<const LoadStoreOperandStructure* >(&rOperandStruct);
    if (nullptr == bils_struct) {
      LOG(fail) << "{BaseIndexLoadStoreOperandConstraint::Setup} expecting operand " << rOperandStruct.mName << " to be \"LoadStoreOperandStructure\" type." << endl;
      FAIL("expecting-load-store-operand-structure");
    }
    auto base_ptr = rInstr.FindOperandMutable(bils_struct->Base(), true);
    mpBase = dynamic_cast<RegisterOperand* >(base_ptr);
    if (nullptr == mpBase) {
      LOG(fail) << "{BaseIndexLoadStoreOperandConstraint::Setup} expecting operand " << base_ptr->Name() << " to be \"RegisterOperand\" type." << endl;
      FAIL("expecting-register_operand");
    }
    if (bils_struct->Index().size() > 0) {
      auto register_ptr = rInstr.FindOperandMutable(bils_struct->Index(), true);
      mpIndex =  dynamic_cast<RegisterOperand* >(register_ptr);
      if (nullptr == mpIndex) {
         LOG(fail) << "{BaseIndexLoadStoreOperandConstraint::Setup} expecting operand " << register_ptr->Name() << " to be \"RegisterOperand\" type." << endl;
         FAIL("expecting-register_operand");
      }
      mIndexUsePreamble = rGen.AddressProtection() or not rGen.HasISS();
      const std::vector<Operand* > operands = rInstr.GetOperands();
      auto index_type = mpIndex->OperandType();
      for (auto operand_ptr : operands) {
        if (operand_ptr == mpIndex) {
          break;
        }
        if (operand_ptr->OperandType() == index_type) {
          mIndexPreOperands.push_back(operand_ptr);
        }
      }
    }
    if (bils_struct->ExtendAmount().size() > 0) {
      mExtendAmountName = bils_struct->ExtendAmount();
      auto extend_ptr = rInstr.FindOperandMutable(bils_struct->ExtendAmount(), false);
      if (nullptr != extend_ptr){
        mpExtendAmount = dynamic_cast<ImmediateOperand* >(extend_ptr);}
      }else {
        //no mpExtendAmount operand
        LOG(info) << "{BaseIndexLoadStoreOperandConstraint::Setup} no operand " << mExtendAmountName << "." << endl;
    }
  }

  bool BaseIndexLoadStoreOperandConstraint::IndexChoiceValueDuplicate() const
  {
    bool duplicate = any_of(mIndexPreOperands.cbegin(), mIndexPreOperands.cend(),
      [this](const Operand* pOpr) { return (pOpr->Value() == mpIndex->Value()); });

    return duplicate;
  }

  void PcRelativeBranchOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    BranchOperandConstraint::Setup(rGen, rInstr, rOperandStruct);
    mpOffset = GetSignedOffsetOperand(rInstr, rOperandStruct);
  }

  void PcOffsetLoadStoreOperandConstraint::Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct)
  {
    LoadStoreOperandConstraint::Setup(rGen, rInstr, rOperandStruct);
    mpOffset = GetSignedOffsetOperand(rInstr, rOperandStruct);
  }

}
