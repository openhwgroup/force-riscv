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
#include <AddressSolvingShared.h>
#include <AddressSolver.h>
#include <Generator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <MemoryConstraint.h>
#include <AddressTagging.h>
#include <Operand.h>
#include <OperandConstraint.h>
#include <Constraint.h>
#include <Register.h>
#include <Instruction.h>
#include <InstructionConstraint.h>
#include <PcSpacing.h>
#include <Random.h>
#include <Choices.h>
#include <ChoicesModerator.h>
#include <VaGenerator.h>
#include <GenException.h>
#include <AddressFilteringRegulator.h>
#include <AddressReuseMode.h>
#include <VectorLayout.h>
#include <Log.h>

#include <memory>

/*!
  \file AddressSolvingShared
  \brief Code for shared data structure used in address solving.
*/

using namespace std;

namespace Force {

  AddressSolvingShared::AddressSolvingShared()
    : mpGenerator(nullptr), mpInstruction(nullptr), mpAddressingOperand(nullptr), mpAddressingOperandConstraint(nullptr), mpVmMapper(nullptr), mpAddressTagging(nullptr), mpTargetConstraint(nullptr), mpPcConstraint(nullptr), mAlignMask(0), mFreeTarget(0), mAlignment(0), mAlignShift(0),
      mSize(0), mVmTimeStamp(0), mIsInstruction(false), mMemDataType(EMemDataType::Data), mMemAccessType(EMemAccessType::Unknown), mpAddrReuseMode(nullptr), mFreeTried(false), mFreeValid(false), mHardVmConstraints(), mIntraAllocations(), mpTargetListConstraint()
  {
  }

  AddressSolvingShared::~AddressSolvingShared()
  {
    for (auto intra_iter : mIntraAllocations) {
      delete intra_iter.second;
    }

    for (auto hard_vm_constr : mHardVmConstraints) {
      delete hard_vm_constr;
    }
  }

  RegisterOperand* AddressSolvingShared::SetAddressingOperand(AddressingOperand* pOpr, uint64 alignment)
  {
    mpAddressingOperand = pOpr;
    mpAddressingOperandConstraint = mpAddressingOperand->GetOperandConstraint()->CastInstance<AddressingOperandConstraint>();
    mAlignment = alignment;
    mAlignMask = ~(alignment - 1);
    mAlignShift = get_align_shift(alignment);
    return mpAddressingOperandConstraint->BaseOperand();
  }

  void AddressSolvingShared::Initialize(Generator& gen, Instruction& instr, uint32 size, bool isInstr, const EMemAccessType memAccessType)
  {
    mpGenerator = &gen;
    mpInstruction = &instr;
    mSize = size;
    mIsInstruction = isInstr;
    mMemAccessType = memAccessType;
    if (mIsInstruction) {
      mMemDataType = EMemDataType::Instruction;
    }
  }

  bool AddressSolvingShared::Setup()
  {
    mpVmMapper = mpAddressingOperandConstraint->GetVmMapper();
    mpAddressTagging = mpVmMapper->GetAddressTagging();
    mpTargetConstraint = mpAddressingOperandConstraint->TargetConstraint();
    mpPcConstraint = GetPcSpaceConstraint();
    mpGenerator->GetAddressFilteringRegulator()->GetVmConstraints(*(mpAddressingOperandConstraint->GetPageRequest()), *mpVmMapper, mHardVmConstraints);
    ChooseDataReuse();
    return true;
  }

  const ConstraintSet* AddressSolvingShared::GetPcSpaceConstraint() const
  {
    auto pc_spacing = PcSpacing::Instance();
    return pc_spacing->GetPcSpaceConstraint();
  }

  void AddressSolvingShared::ChooseDataReuse()
  {
    if (mpAddrReuseMode != nullptr) {
      mpAddrReuseMode->DisableAllReuseTypes();
    }
    else {
      mpAddrReuseMode = new AddressReuseMode();
    }

    if ((mMemDataType != EMemDataType::Data) or mpAddressingOperandConstraint->HasDataConstraints())
      return;

    const ChoicesModerator* choices_mod = mpGenerator->GetChoicesModerator(EChoicesType::OperandChoices);
    unique_ptr<ChoiceTree> read_after_read_choices(choices_mod->CloneChoiceTree("Read after read address reuse"));
    const Choice* read_after_read_choice = read_after_read_choices->Choose();
    if (read_after_read_choice->Value() == 1) {
      mpAddrReuseMode->EnableReuseType(EAddressReuseType::ReadAfterRead);
    }

    unique_ptr<ChoiceTree> read_after_write_choices(choices_mod->CloneChoiceTree("Read after write address reuse"));
    const Choice* read_after_write_choice = read_after_write_choices->Choose();
    if (read_after_write_choice->Value() == 1) {
      mpAddrReuseMode->EnableReuseType(EAddressReuseType::ReadAfterWrite);
    }

    unique_ptr<ChoiceTree> write_after_read_choices(choices_mod->CloneChoiceTree("Write after read address reuse"));
    const Choice* write_after_read_choice = write_after_read_choices->Choose();
    if (write_after_read_choice->Value() == 1) {
      mpAddrReuseMode->EnableReuseType(EAddressReuseType::WriteAfterRead);
    }

    unique_ptr<ChoiceTree> write_after_write_choices(choices_mod->CloneChoiceTree("Write after write address reuse"));
    const Choice* write_after_write_choice = write_after_write_choices->Choose();
    if (write_after_write_choice->Value() == 1) {
      mpAddrReuseMode->EnableReuseType(EAddressReuseType::WriteAfterWrite);
    }
  }

  void AddressSolvingShared::ApplyVirtualUsableConstraint(ConstraintSet* constrSet, uint32& rTimeStamp) const
  {
    rTimeStamp = mVmTimeStamp; // update time stamp

    for (auto vm_constr : mHardVmConstraints) {
      //ConstraintSet copy_constr(*constrSet); // DEBUG
      // << "incoming constraint: " << copy_constr.ToSimpleString() << " applying " << EVmConstraintType_to_string(vm_constr->Type()) << " constraint: " << vm_constr->GetConstraintSet()->ToSimpleString() << endl; // DEBUG
      vm_constr->ApplyOn(*constrSet);
      if (constrSet->IsEmpty()) {
        LOG(info) << "{AddressSolvingShared::ApplyVirtualUsableConstraint} constraint-set disallowed by hard VmConstrint: " << EVmConstraintType_to_string(vm_constr->Type()) << endl;
        return;
      }
    }

    mpVmMapper->ApplyVirtualUsableConstraint(mMemDataType, mMemAccessType, *mpAddrReuseMode, constrSet);

  }

  bool AddressSolvingShared::ReVerifyTargetAddressRange(uint64 targetAddress) const
  {
    // Need to verify the address is usable when new pages are allocated because we can't know before the virtual
    // addresses have been mapped.
    const AddressTagging* addr_tagging = GetAddressTagging();
    uint64 untagged_target_address = addr_tagging->UntagAddress(targetAddress, IsInstruction());
    ConstraintSet generated_addr_constr(untagged_target_address, untagged_target_address + Size() - 1);
    uint32 temp_var = 0;
    ApplyVirtualUsableConstraint(&generated_addr_constr, temp_var);

    return (generated_addr_constr.Size() == Size());
  }

  bool AddressSolvingShared::MapTargetAddressRange(uint64 targetAddress, uint32& rTimeStamp) const
  {
    bool addr_usable = false;
    bool mapped_to_new_pages = GetVmMapper()->MapAddressRange(targetAddress, Size(), IsInstruction(), mpAddressingOperandConstraint->GetPageRequest());
    if (mapped_to_new_pages) {
      ++ mVmTimeStamp;
      addr_usable = ReVerifyTargetAddressRange(targetAddress);
    }
    else if (rTimeStamp < mVmTimeStamp) {
      addr_usable = ReVerifyTargetAddressRange(targetAddress);
    }
    else {
      // If the address range mapped into existing pages, it should be usable.
      addr_usable = true;
    }

    // << "{AddressSolvingShared::MapTargetAddressRange} address 0x" << hex << targetAddress << " in time stamp: " << dec << rTimeStamp << " VM time stamp: " << mVmTimeStamp << " mapped to new page: " << mapped_to_new_pages << " usable: " << addr_usable << endl;
    rTimeStamp = mVmTimeStamp; // update caller's time-stamp.

    return addr_usable;
  }

  bool AddressSolvingShared::SolveFree() const
  {
    if (mFreeTried) {
      // << "solve free returning : tried? " << mFreeTried << " valid? " << mFreeValid << endl;
      return mFreeValid;
    }

    mFreeTried = true;
    VaGenerator va_gen(mpVmMapper, mpAddressingOperandConstraint->GetPageRequest(), mpTargetConstraint);
    if (mpInstruction->IsBranch()) {
      va_gen.SetAccurateBranch(mpInstruction->ByteSize());
    }
    try {
      mFreeTarget = va_gen.GenerateAddress(mAlignment, mSize, mIsInstruction, mMemAccessType);

      // Applying a random tag could violate the target constraint, so only generate a tag when there is no target
      // constraint.
      if (mpTargetConstraint == nullptr) {
        mFreeTarget = mpAddressTagging->TagAddressRandomly(mFreeTarget, mIsInstruction);
      }

      mFreeValid = true;
    }
    catch (const OperandError& oprErr) {

    }
    catch (const ConstraintError& constrErr) {

    }

    if (va_gen.NewPagesAdded()) {
      ++ mVmTimeStamp;
    }

    // << "solve result: " << mFreeValid << endl;
    return mFreeValid;
  }

  const ConstraintSet* AddressSolvingShared::GetAllocatedConstraint(ERegisterType regType) const
  {
    map<ERegisterType, ConstraintSet* >::iterator alloc_iter = mIntraAllocations.find(regType);
    ConstraintSet* op_allocated = nullptr;
    if (alloc_iter == mIntraAllocations.end()) {
      ConstraintSet alloc_constr;
      for (auto opr_ptr : mpInstruction->GetOperands()) {
        if (opr_ptr == mpAddressingOperand) {
          break; // Only need to worry about operands before the addressing operand.
        }

        if (opr_ptr->IsRegisterOperand()) {
          if (mpGenerator->OperandTypeCompatible(regType, opr_ptr->OperandType())) {
            auto reg_opr = dynamic_cast<const RegisterOperand* >(opr_ptr);
            reg_opr->GetChosenRegisterIndices(*mpGenerator, alloc_constr);
          }
        }
      }

      if (not alloc_constr.IsEmpty()) {
        op_allocated = new ConstraintSet(alloc_constr);
        mIntraAllocations[regType] = op_allocated;
      }
    }
    else {
      op_allocated = alloc_iter->second;
    }
    return op_allocated;
  }

  bool AddressSolvingShared::OperandConflict(const Register* pReg) const
  {
    bool op_conflict = false;
    ERegisterType reg_type = pReg->RegisterType();
    // << "checking register: " << pReg->Name() << " of type: " << ERegisterType_to_string(reg_type) << endl;

    auto allocated_constr = GetAllocatedConstraint(reg_type);
    if (nullptr != allocated_constr) {
      op_conflict = allocated_constr->ContainsValue(pReg->IndexValue());
      // << " op_conflict : " << allocated_constr->ToSimpleString() << " conflict? " << op_conflict << endl;
    }

    return op_conflict;
  }

  const ConstraintSet* RegisterBranchSolvingShared::GetPcSpaceConstraint() const
  {
    auto pc_spacing = PcSpacing::Instance();
    return pc_spacing->GetBranchPcSpaceConstraint(mpVmMapper, mpInstruction->ByteSize());
  }

  uint64 BaseOffsetSolvingShared::FreeOffset(AddressingMode* pAddrMode ) const
  {
    if (not mOffsetSolved) {
      auto offset_opr = mpAddressingOperandConstraint->OffsetOperand();
      offset_opr->Generate(*mpGenerator, *mpInstruction);
      mOffsetValue = offset_opr->Value();
      if (offset_opr->IsSigned()) {
        mOffsetValue = sign_extend64(mOffsetValue, offset_opr->Size());
      }
      mOffsetValue = pAddrMode->AdjustOffset(mOffsetValue);
      mOffsetSolved = true;
    }
    return mOffsetValue;
  }

  RegisterOperand* BaseOffsetShiftSolvingShared::SetAddressingOperand(AddressingOperand* pOpr, uint64 alignment)
  {
    RegisterOperand* p_register_operand = AddressSolvingShared::SetAddressingOperand(pOpr, alignment);

    // We need to align to the larger of the specified alignment and the alignment dictated by the offset scale, as we
    // can't reach target addresses in between the increments determined by the offset scale
    if (mOffsetScale > mAlignShift) {
      mScaleAlignShift = mOffsetScale;
      mScaleAlignMask = ~((0x1 << mOffsetScale) - 1);
    } else {
      mScaleAlignShift = mAlignShift;
      mScaleAlignMask = mAlignMask;
    }

    return p_register_operand;
  }

  BaseIndexSolvingShared::~BaseIndexSolvingShared()
  {
    for (auto index_choice : mIndexChoices) {
      delete index_choice;
    }
  }

  bool BaseIndexSolvingShared::Setup()
  {
    if (not AddressSolvingShared::Setup())
      return false;

    auto lsbi_constr = mpAddressingOperandConstraint->CastInstance<BaseIndexLoadStoreOperandConstraint>();
    mpIndexOperand = lsbi_constr->IndexOperand();

    if (not GetAvailableIndexChoices()) {
      LOG(notice) << "{BaseIndexSolvingShared::Setup} no index choice available." << endl;
      return false;
    }
    const string& ea_name = lsbi_constr->ExtentAmountName();
    mExtendType = get_extend_type_amount(ea_name, mExtendAmount);
    LOG(notice) << "{BaseIndexSolvingShared::Setup} extend type: " << EExtendType_to_string(mExtendType) << " extend amount: " << dec << mExtendAmount << endl;
    return true;
  }

  bool BaseIndexSolvingShared::GetAvailableIndexChoices()
  {
    vector<const Choice* > choices_list;
    mpIndexOperand->GetAvailableChoices(choices_list);

    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    for (auto choice_item : choices_list) {
      Register* reg_ptr = reg_file->RegisterLookup(choice_item->Name());
      if (reg_ptr->IsInitialized()) {
        if (reg_ptr->HasAttribute(ERegAttrType::HasValue)) {
          uint64 reg_value = reg_ptr->Value();
          auto addr_reg = new AddressingRegister();
          addr_reg->SetRegister(reg_ptr);
          addr_reg->SetWeight(choice_item->Weight());
          addr_reg->SetRegisterValue(reg_value);
          mIndexChoices.push_back(addr_reg);
        }
      }
      else if (mpGenerator->HasISS()) { // TODO Gate out no-iss mode for now.
        if (OperandConflict(reg_ptr)) {
          continue;
        }

        auto addr_reg = new AddressingRegister();
        addr_reg->SetRegister(reg_ptr);
        addr_reg->SetWeight(choice_item->Weight());
        addr_reg->SetFree(true);
        mIndexChoices.push_back(addr_reg);
      }
    }

    return (mIndexChoices.size() > 0);
  }

  VectorStridedSolvingShared::VectorStridedSolvingShared()
    : AddressSolvingShared(), mpStrideOpr(nullptr), mStrideChoices(), mElemCount(0)
  {
  }

  VectorStridedSolvingShared::~VectorStridedSolvingShared()
  {
    for (AddressingRegister* stride_choice : mStrideChoices) {
      delete stride_choice;
    }
  }

  bool VectorStridedSolvingShared::Setup()
  {
    if (not AddressSolvingShared::Setup())
    {
      return false;
    }

    auto strided_opr_constr = mpAddressingOperandConstraint->CastInstance<VectorStridedLoadStoreOperandConstraint>();
    mpStrideOpr = strided_opr_constr->StrideOperand();

    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(mpInstruction->GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();
    mElemCount = vec_layout->mElemCount;

    SetupStrideChoices();
    if (mStrideChoices.empty()) {
      LOG(notice) << "{VectorStridedSolvingShared::Setup} no stride choice available." << endl;
      return false;
    }

    return true;
  }

  void VectorStridedSolvingShared::SetupStrideChoices()
  {
    vector<const Choice*> choices_list;
    mpStrideOpr->GetAvailableChoices(choices_list);

    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    for (const Choice* choice_item : choices_list) {
      Register* reg = reg_file->RegisterLookup(choice_item->Name());

      if (reg->IsInitialized()) {
        if (reg->HasAttribute(ERegAttrType::HasValue)) {
          auto addr_reg = new AddressingRegister();
          addr_reg->SetRegister(reg);
          addr_reg->SetWeight(choice_item->Weight());
          addr_reg->SetRegisterValue(reg->Value());
          mStrideChoices.push_back(addr_reg);
        }
      }
      else if (mpGenerator->HasISS() and (not OperandConflict(reg))) {
        auto addr_reg = new AddressingRegister();
        addr_reg->SetRegister(reg);
        addr_reg->SetWeight(choice_item->Weight());
        addr_reg->SetRegisterValue(reg->ReloadValue());
        addr_reg->SetFree(true);
        mStrideChoices.push_back(addr_reg);
      }
    }
  }

  VectorIndexedSolvingShared::VectorIndexedSolvingShared()
    : AddressSolvingShared(), mpIndexOpr(nullptr), mIndexChoices(), mElemSize(0), mElemCount(0)
  {
  }

  VectorIndexedSolvingShared::~VectorIndexedSolvingShared()
  {
    for (AddressingRegister* index_choice : mIndexChoices) {
      delete index_choice;
    }
  }

  bool VectorIndexedSolvingShared::Setup()
  {
    if (not AddressSolvingShared::Setup())
    {
      return false;
    }

    auto indexed_opr_constr = mpAddressingOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    mpIndexOpr = indexed_opr_constr->IndexOperand();

    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(mpInstruction->GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();
    mElemSize = vec_layout->mElemSize;
    mElemCount = vec_layout->mElemCount;

    SetupIndexChoices();
    if (mIndexChoices.empty()) {
      LOG(notice) << "{VectorIndexedSolvingShared::Setup} no index choice available." << endl;
      return false;
    }

    return true;
  }

  uint64 VectorIndexedSolvingShared::GetVectorElementValue(const std::vector<uint64>& rVecRegValues, cuint32 elemIndex) const
  {
    uint32 reg_val_index = (mElemSize * elemIndex) / sizeof_bits<uint64>();
    uint32 reg_val_shift = ((mElemSize * elemIndex) % sizeof_bits<uint64>()) * mElemSize;
    uint64 reg_val_mask = get_mask64(mElemSize, reg_val_shift);
    uint64 elem_val = (rVecRegValues[reg_val_index] & reg_val_mask) >> reg_val_shift;

    return elem_val;
  }

  void VectorIndexedSolvingShared::GetVectorRegisterValues(const std::vector<uint64>& rVecElemValues, std::vector<uint64>& rVecRegValues) const
  {
    uint32 elem_values_per_reg_val = sizeof_bits<uint64>() / mElemSize;
    uint32 reg_val_count = mElemCount / elem_values_per_reg_val;
    for (uint32 reg_val_index = 0; reg_val_index < reg_val_count; reg_val_index++) {
      uint32 elem_start_index = reg_val_index * elem_values_per_reg_val;
      uint32 elem_end_index = elem_start_index + elem_values_per_reg_val;

      uint64 reg_val = 0;
      for (uint32 elem_index = elem_start_index; elem_index < elem_end_index; elem_index++) {
        reg_val <<= mElemSize;
        reg_val |= rVecElemValues[elem_end_index - elem_index - 1];
      }

      rVecRegValues.push_back(reg_val);
    }
  }

  void VectorIndexedSolvingShared::SetupIndexChoices()
  {
    vector<const Choice*> choices_list;
    mpIndexOpr->GetAvailableChoices(choices_list);

    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    for (const Choice* choice_item : choices_list) {
      Register* reg = reg_file->RegisterLookup(choice_item->Name());

      if (reg->IsInitialized()) {
        if (reg->HasAttribute(ERegAttrType::HasValue)) {
          auto addr_reg = new AddressingRegister();
          addr_reg->SetRegister(reg);
          addr_reg->SetWeight(choice_item->Weight());

          auto large_reg = dynamic_cast<LargeRegister*>(reg);
          addr_reg->SetRegisterValue(large_reg->Values());

          mIndexChoices.push_back(addr_reg);
        }
      }
      else if (mpGenerator->HasISS() and (not OperandConflict(reg))) {
        auto addr_reg = new AddressingRegister();
        addr_reg->SetRegister(reg);
        addr_reg->SetWeight(choice_item->Weight());
        addr_reg->SetFree(true);

        auto large_reg = dynamic_cast<LargeRegister*>(reg);
        addr_reg->SetRegisterValue(large_reg->ReloadValues());

        mIndexChoices.push_back(addr_reg);
      }
    }
  }

  bool BaseIndexAmountBitSolvingShared::Setup()
  {
    if (not BaseIndexSolvingShared::Setup())
      return false;
    auto lsbi_constr = mpAddressingOperandConstraint->CastInstance<BaseIndexLoadStoreOperandConstraint>();

    auto extend_amount_opr = lsbi_constr->ExtendAmountOperand();
    auto ea_opr_constr = extend_amount_opr->GetOperandConstraint();
    if (ea_opr_constr->HasConstraint()) {
      auto ea_constr = ea_opr_constr->GetConstraint();
      if (ea_constr->ContainsValue(0)) mExtendAmount0Valid = true;
      if (ea_constr->ContainsValue(1)) mExtendAmount1Valid = true;
    } else {
      if (mExtendAmount == 0) {
        // mExtendAmount is 0, doesn't matter using 0 or 1, we just pick one to use.
        if (Random::Instance()->Random32(0, 1)) {
          mExtendAmount1Valid = true;
        }
        else {
          mExtendAmount0Valid = true;
        }
      }
      else {
        mExtendAmount0Valid = true;
        mExtendAmount1Valid = true;
      }
    }

    LOG(notice) << "{BaseIndexAmountBitSolvingShared::Setup} extend type: " << EExtendType_to_string(mExtendType) << " extend amount: " << dec << mExtendAmount << " amount 0 valid? " << mExtendAmount0Valid << " 1 valid? " << mExtendAmount1Valid << endl;
    return true;
  }

  DataProcessingSolvingShared::~DataProcessingSolvingShared()
  {
    if (mpTargetAddressConstraint != nullptr) {
      delete mpTargetAddressConstraint;
      mpTargetAddressConstraint = nullptr;
    }
  }

  bool DataProcessingSolvingShared::Setup()
  {
    bool success = AddressSolvingShared::Setup();

    if (success) {
      mpTargetAddressConstraint = new ConstraintSet(0, MAX_UINT64);
      uint32 temp_var = 0;
      ApplyVirtualUsableConstraint(mpTargetAddressConstraint, temp_var);
      mpTargetAddressConstraint->SubConstraintSet(*(PcConstraint()));
      mpTargetAddressConstraint->AlignWithSize(AlignMask(), Size());
      if (TargetConstraint() != nullptr) {
        mpTargetAddressConstraint->ApplyConstraintSet(*(TargetConstraint()));
      }

      mCondFlags = mpGenerator->GetConditionFlags();
    }

    return success;
  }

}
