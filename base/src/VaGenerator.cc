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
#include <VaGenerator.h>
#include <VmMapper.h>
#include <UtilityFunctions.h>
#include <Constraint.h>
#include <PcSpacing.h>
#include <GenException.h>
#include <GenRequest.h>
#include <AddressFilteringRegulator.h>
#include <VmConstraint.h>
#include <Log.h>
#include <Variable.h>
#include <Generator.h>
#include <AddressTagging.h>
#include <AddressReuseMode.h>

#include <memory>

using namespace std;

/*!
  \file VaGenerator
  \brief Code for virtual address generator.
*/
namespace Force {

  VaGenerator::VaGenerator(VmMapper* vmMapper, const GenPageRequest* pPageReq, const ConstraintSet* pTargetConstr, bool isOperand, const AddressReuseMode* pAddrReuseMode)
    : mpVmMapper(vmMapper), mCallerName(), mpPageRequest(pPageReq), mpTargetConstraint(pTargetConstr), mpRangeConstraint(nullptr), mpLocalPageRequest(nullptr), mpOperandConstraint(nullptr), mAlignMask(0), mSize(0), mAlignShift(0), mBranchSize(0), mIsInstruction(false), mIsOperand(isOperand), mAccurateBranch(false), mNewPagesAdded(false), mHardVmConstraints(), mpAddrReuseMode(nullptr)
  {
    if (pAddrReuseMode != nullptr) {
      mpAddrReuseMode = new AddressReuseMode(*pAddrReuseMode);
    }
    else {
      mpAddrReuseMode = new AddressReuseMode();
    }
  }

  VaGenerator::VaGenerator()
    : mpVmMapper(nullptr), mCallerName(), mpPageRequest(nullptr), mpTargetConstraint(nullptr), mpRangeConstraint(nullptr), mpLocalPageRequest(nullptr), mpOperandConstraint(nullptr), mAlignMask(0), mSize(0), mAlignShift(0), mBranchSize(0), mIsInstruction(false), mIsOperand(true), mAccurateBranch(false), mNewPagesAdded(false), mHardVmConstraints(), mpAddrReuseMode(new AddressReuseMode())
  {
  }

  VaGenerator::~VaGenerator()
  {
    // set pointers to nullptr.
    mpPageRequest = nullptr;
    mpVmMapper = nullptr;
    mpTargetConstraint = nullptr;
    mpRangeConstraint = nullptr;
    delete mpOperandConstraint;
    delete mpLocalPageRequest;
    delete mpAddrReuseMode;

    for (auto vm_constr : mHardVmConstraints) {
      delete vm_constr;
    }
  }

  void VaGenerator::SetReachConstraint(const std::string& rCallerName, ConstraintSet* pOprConstr, bool isOperand)
  {
    mCallerName = rCallerName;
    mpOperandConstraint = pOprConstr;
    mIsOperand = isOperand;
  }

  void VaGenerator::ApplyPcConstraint(ConstraintSet* pConstr) const
  {
    // Remove the PC vicinity region to avoid PC collision.
    auto pc_spacing = PcSpacing::Instance();
    const ConstraintSet* pc_space_constr = nullptr;
    if (mAccurateBranch)
      pc_space_constr = pc_spacing->GetBranchPcSpaceConstraint(mpVmMapper, mBranchSize);
    else
      pc_space_constr = pc_spacing->GetPcSpaceConstraint();
    pConstr->SubConstraintSet(*pc_space_constr);
  }

  bool VaGenerator::TargetAddressForced(uint64& rAddress) const
  {
    if (nullptr == mpTargetConstraint) return false;

    uint32 target_constr_size = mpTargetConstraint->Size();
    if (target_constr_size > 1) return false;

    if (0 == target_constr_size) {
      stringstream err_stream;
      err_stream << "Operand \"" << mCallerName << "\" failed to generate; target constraint is empty.";
      throw OperandError(err_stream.str());
    }

    rAddress = mpTargetConstraint->OnlyValue();
    if (nullptr != mpOperandConstraint) {
      const AddressTagging* addr_tagging = mpVmMapper->GetAddressTagging();
      //!< target constraint address  may have tagged part
      auto untagged_address = addr_tagging->UntagAddress(rAddress, mIsInstruction);
      if (not mpOperandConstraint->ContainsValue(untagged_address)) {
        stringstream err_stream;
        err_stream << "Operand \"" << mCallerName << "\" failed to generate; forced target address not reachable: 0x" << hex << rAddress;
        throw OperandError(err_stream.str());
      }
    }

    return true;
  }

  void VaGenerator::ApplyTargetConstraint(ConstraintSet* pConstr) const
  {
    if (nullptr == mpTargetConstraint)
      return;

    if (pConstr->IsEmpty())
      return;

    if (nullptr != mpOperandConstraint) {
      ConstraintSet* target_constr = mpTargetConstraint->Clone();
      unique_ptr<ConstraintSet> target_constr_storage(target_constr);
      target_constr->ApplyConstraintSet(*mpOperandConstraint);
      if (target_constr->IsEmpty()) {
        stringstream err_stream;
        err_stream << "Operand \"" << mCallerName << "\" failed to generate; target constraint not reachable: " << mpTargetConstraint->ToSimpleString();
        throw OperandError(err_stream.str());
      }
      pConstr->ApplyConstraintSet(*target_constr);
    }
    else {
      pConstr->ApplyConstraintSet(*mpTargetConstraint);
    }
  }

  uint64 VaGenerator::GenerateAddress(uint64 align, uint64 size, bool isInstr, EMemAccessType memAccess, const ConstraintSet *pRangeConstraint)
  {
    mAlignMask = ~(align - 1);
    mIsInstruction = isInstr;
    mpRangeConstraint = pRangeConstraint;
    mSize = size;
    mAlignShift = get_align_shift(align);

    // << "{VaGenerator::GenerateAddress} start gen addr align=0x" << hex << align << " size=0x" << size << " isinstr=" << isInstr << " acc_type=" << EMemAccessType_to_string(memAccess) << endl;

    if (nullptr == mpPageRequest) {
      // if page request object not specified, create a local one.
      mpLocalPageRequest = mpVmMapper->GenPageRequestRegulated(mIsInstruction, memAccess);
      mpPageRequest = mpLocalPageRequest;
    }

    auto addr_regulator = mpVmMapper->GetAddressFilteringRegulator();
    addr_regulator->GetVmConstraints(*mpPageRequest, *mpVmMapper, mHardVmConstraints);

    uint64 addr = 0;
    bool addr_usable = false;
    //uint64 gen_count = 0; DEBUG - FOR TIMEOUT LOOP
    while (not addr_usable) {
      if (TargetAddressForced(addr)) {
        MapAddressRange(addr);
        break;
      }

      // GenerateConstrainedAddress() should eventually throw an exception if a usable address can't be generated,
      // preventing an infinite loop here.

      addr = GenerateConstrainedAddress();
      addr_usable = MapAddressRange(addr);

      /*++gen_count; DEBUG - TIMEOUT LOOP
      if (gen_count >= 10)
      {
        LOG(fail) << "{VaGenerator::GenerateAddress} unable to map address after 10 tries, aborting execution" << endl;
        FAIL("gen_constrained_addr_loop_timeout");
      }*/
    }

    // << "generated address: 0x" << hex << addr << endl;
    return addr;
  }

  bool VaGenerator::MapAddressRange(cuint64 addr) const
  {
    bool addr_usable = false;

    bool mapped_to_new_pages = mpVmMapper->MapAddressRange(addr, mSize, mIsInstruction, mpPageRequest);

    if (mapped_to_new_pages) {
      mNewPagesAdded = true;

      // Need to verify the address is usable when new pages are allocated because we can't know before the virtual
      // addresses have been mapped.
      EMemDataType memDataType = mIsInstruction ? EMemDataType::Instruction : EMemDataType::Data;
      ConstraintSet generated_addr_constr(addr, addr + mSize - 1);

      mpVmMapper->ApplyVirtualUsableConstraint(memDataType, mpPageRequest->MemoryAccessType(), *mpAddrReuseMode, &generated_addr_constr);
      ApplyHardConstraints(&generated_addr_constr);

      if (generated_addr_constr.Size() == mSize) {
        addr_usable = true;
      }
    }
    else {
      // If the address range mapped into existing pages, it should be usable.
      addr_usable = true;
    }

    return addr_usable;
  }

  uint64 VaGenerator::GenerateConstrainedAddress()
  {
    unique_ptr<ConstraintSet> va_constr; // responsible for releasing the storage when going out of scope.
    if (nullptr != mpRangeConstraint) {
      va_constr.reset(mpRangeConstraint->Clone());
      //if(!va_constr->IsEmpty())      LOG(notice) << "Generate Address constraints:"         << va_constr->ToSimpleString() << endl;
    }
    else {
      // No range constraint specified, so start with allowing all values
      va_constr.reset(new ConstraintSet(0, MAX_UINT64));
    }

    EMemDataType memDataType = mIsInstruction ? EMemDataType::Instruction : EMemDataType::Data;
    mpVmMapper->ApplyVirtualUsableConstraint(memDataType, mpPageRequest->MemoryAccessType(), *mpAddrReuseMode, va_constr.get());

    MergeAddressErrorConstraint(va_constr.get()); // TBD: generate some address error space
    ApplyHardConstraints(va_constr.get());

    ApplyPcConstraint(va_constr.get());

    va_constr->AlignWithSize(mAlignMask, mSize);

    ApplyTargetConstraint(va_constr.get());

    va_constr->ShiftRight(mAlignShift);

    uint64 constrained_addr = 0;
    try
    {
      constrained_addr = va_constr->ChooseValue() << mAlignShift;
    }
    catch (const ConstraintError& constraint_error)
    {
      if (mIsOperand)
      {
        stringstream err_stream;
        err_stream << "Operand \"" << mCallerName << "\" failed to generate; va generators va_constr exception:" << constraint_error.what();
        throw OperandError(err_stream.str());
      }
      else
        throw ConstraintError(constraint_error.what());
    }
    return constrained_addr;
  }

  void VaGenerator::ApplyHardConstraints(ConstraintSet* pConstr) const
  {
    for (auto vm_constr : mHardVmConstraints) {
      //ConstraintSet copy_constr(*pConstr); // DEBUG
      // << "incoming constraint: " << copy_constr.ToSimpleString() << endl; // DEBUG
      // << " applying " << EVmConstraintType_to_string(vm_constr->Type()) << " constraint: " << vm_constr->GetConstraintSet()->ToSimpleString() << endl; // DEBUG
      vm_constr->ApplyOn(*pConstr);
      if (pConstr->IsEmpty()) {
        LOG(info) << "{VaGenerator::ApplyHardConstraints} constraint-set disallowed by hard VmConstrint: " << EVmConstraintType_to_string(vm_constr->Type()) << endl;
        return;
      }
    }
  }

  void VaGenerator::MergeAddressErrorConstraint(ConstraintSet* pConstr) const
  {
    auto addr_err_constr = mpVmMapper->GetVmConstraint(EVmConstraintType::AddressError);
    if (addr_err_constr == nullptr)
      return;

    if (mIsInstruction) {
      const VariableModerator* var_mod = mpVmMapper->GetGenerator()->GetVariableModerator(EVariableType::Value);
      auto addr_err_var = dynamic_cast<const ValueVariable*>(var_mod->GetVariableSet()->FindVariable("Branch address error"));
      if (addr_err_var->Value() == 0)
        return;
      if (mpRangeConstraint != nullptr) {
        auto range_constr = mpRangeConstraint->Clone();
        unique_ptr<ConstraintSet> storage_ptr(range_constr); // release the storage
        range_constr->ApplyConstraintSet(*addr_err_constr);
        pConstr->MergeConstraintSet(*range_constr);
      }
      else {
        // TBD: merge address error for branch register instructions, need page supports
      }
    }
    else {
      // TBD: merge address error for load/store instructions, need page supports
    }
  }
  uint64 VaGenerator::GenerateAddressWithRangeConstraintBaseValue(uint64 align, uint64 size, bool isInstr, EMemAccessType memAccess, const ConstraintSet *pRangeConstraint, const uint64 base_value)
  {
    mAlignMask = ~(align - 1);
    mIsInstruction = isInstr;
    mpRangeConstraint = pRangeConstraint;
    mSize = size;
    mAlignShift = get_align_shift(align);

    if (nullptr == mpPageRequest) {
      // if page request object not specified, create a local one.
      mpLocalPageRequest = mpVmMapper->GenPageRequestRegulated(mIsInstruction, memAccess);
      mpPageRequest = mpLocalPageRequest;
    }

    auto addr_regulator = mpVmMapper->GetAddressFilteringRegulator();
    addr_regulator->GetVmConstraints(*mpPageRequest, *mpVmMapper, mHardVmConstraints);

    uint64 addr = 0;
    bool addr_usable = false;
    while (not addr_usable) {
      if (TargetAddressForced(addr)) {
        MapAddressRange(addr);
        break;
      }
      // GenerateConstrainedAddressWithAilgnOffset() should eventually throw an exception if a usable address can't be generated,
      // preventing an infinite loop here.
      addr = GenerateConstrainedAddressWithAilgnOffset(base_value);
      addr_usable = MapAddressRange(addr);
    }
    return addr;
  }
  uint64 VaGenerator::GenerateConstrainedAddressWithAilgnOffset(uint64 base_value)
  {
    unique_ptr<ConstraintSet> va_constr; // responsible for releasing the storage when going out of scope.
    if (nullptr != mpRangeConstraint) {
      va_constr.reset(mpRangeConstraint->Clone());
      //if(!va_constr->IsEmpty())      LOG(notice) << "Generate Address constraints:"         << va_constr->ToSimpleString() << endl;
    }
    else {
      // No range constraint specified, so start with allowing all values
      va_constr.reset(new ConstraintSet(0, MAX_UINT64));
    }

    EMemDataType memDataType = mIsInstruction ? EMemDataType::Instruction : EMemDataType::Data;
    mpVmMapper->ApplyVirtualUsableConstraint(memDataType, mpPageRequest->MemoryAccessType(), *mpAddrReuseMode, va_constr.get());

    MergeAddressErrorConstraint(va_constr.get()); // TBD: generate some address error space
    ApplyHardConstraints(va_constr.get());
    ApplyPcConstraint(va_constr.get());

    uint64 align_offset = base_value & ~(mAlignMask);
    va_constr->AlignOffsetWithSize(mAlignMask,align_offset, mSize);

    ApplyTargetConstraint(va_constr.get());

    va_constr->ShiftRight(mAlignShift);
    uint64 constrained_addr = 0;
    try
    {
      constrained_addr = (va_constr->ChooseValue() << mAlignShift) + align_offset;
    }
    catch (const ConstraintError& constraint_error)
    {
      if (mIsOperand)
      {
        stringstream err_stream;
        err_stream << "Operand \"" << mCallerName << "\" failed to generate; va generators va_constr exception:" << constraint_error.what();
        throw OperandError(err_stream.str());
      }
      else
        throw ConstraintError(constraint_error.what());
    }
    return constrained_addr;
  }

}
