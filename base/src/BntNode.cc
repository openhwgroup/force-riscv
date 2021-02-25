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
#include <SimAPI.h>
#include <BntNode.h>
#include <Generator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <Register.h>
#include <SimplePeState.h>
#include <Log.h>
#include <ResourcePeState.h>
#include <AddressTagging.h>
#include <Instruction.h>
#include <Config.h>
#include <Constraint.h>

#include <sstream>

using namespace std;

/*!
  \file BntNode.cc
  \brief Code handling BNT recording and generating.
*/

namespace Force {

  uint32 BntNode::msBntId = 0;

  BntNode::BntNode(uint64 brTarget, bool taken, bool cond)
    : mBranchTarget(brTarget), mNextPC(0),  mAttributes(0), mId(0), mSequenceName(), mBntFunction(), mpPeState(nullptr)
  {
    SetTaken(taken);
    SetConditional(cond);
    mId = msBntId ++;
  }

  BntNode::~BntNode()
  {
    delete mpPeState;
  }

  void BntNode::SetTaken(bool taken)
  {
    SetBoolAttribute(EBntAttributeType::Taken, taken);
  }

  void BntNode::SetConditional(bool cond)
  {
    SetBoolAttribute(EBntAttributeType::Conditional, cond);
  }

  void BntNode::SetBoolAttribute(EBntAttributeType attrType, bool setIt)
  {
    uint64 set_bit = 1ull << (uint32)(attrType);
    if (setIt) {
      mAttributes |= set_bit;
    } else {
      mAttributes &= ~set_bit;
    }
  }

  void BntNode::SetAccurate(bool accu)
  {
    SetBoolAttribute(EBntAttributeType::Accurate, accu);
  }

  uint64 BntNode::TakenPath() const
  {
    uint64 t_addr = mNextPC;
    if (BranchTaken()) {
      t_addr = mBranchTarget;
    }
    return t_addr;
  }

  uint64 BntNode::NotTakenPath() const
  {
    uint64 nt_addr = mBranchTarget;
    if (BranchTaken()) {
      nt_addr = mNextPC;
    }
    return nt_addr;
  }

  void BntNode::PreserveNotTakenPath(Generator* pGen)
  {
    if (PathsSame()) {
      return;
    }

    uint64 not_taken_addr = NotTakenPath();
    auto vm_mapper = pGen->GetVmManager()->CurrentVmMapper();
    const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
    uint64 untagged_bnt_address = addr_tagging->UntagAddress(not_taken_addr, true);

    char print_buffer[32];
    snprintf(print_buffer, 32, "BNT%d", mId);
    pGen->ReserveMemory(print_buffer, untagged_bnt_address, pGen->BntReserveSpace(), 0, true);

    SavePeState(pGen);
  }

  void BntNode::SavePeState(Generator* pGen)
  {
    auto vm_regime = pGen->GetVmManager()->CurrentVmRegime();
    const vector<Register* > & reg_context = vm_regime->RegisterContext();
    mpPeState = new SimplePeState();
    mpPeState->SaveState(pGen, reg_context);
  }

  void BntNode::UpdateAccurateState(Generator* pGen, uint32 instrBytes)
  {
    uint64 last_pc = pGen->LastPC();
    mNextPC = last_pc + instrBytes;
    // uint64 pc_now = pGen->PC();
    SetAccurate(true);
    SavePeState(pGen);
  }

  const string BntNode::ToString() const
  {
    stringstream out_str;

    out_str << "BNT" << dec << mId << " target=0x" << hex << mBranchTarget << " next-PC=0x" << mNextPC << " taken=" << BranchTaken() << " conditional=" << IsConditional() << " accurate=" << IsAccurate() << " speculative=" << IsSpeculative();

    return out_str.str();
  }

  void BntNode::ReserveTakenPath(Generator* pGen)
  {
    LOG(fail) << "{BntNode::ReserveTakenPath} No implementation for normal Bnt " << endl;
    FAIL("No-implementation-for-Bnt");
  }

  void BntNode::UnreserveTakenPath(Generator* pGen)
  {
    LOG(fail) << "{BntNode::ReserveTakenPath} No implementation for normal Bnt " << endl;
    FAIL("No-implementation-for-Bnt");
  }

  SpeculativeBntNode::SpeculativeBntNode(uint64 brTarget, bool taken, bool cond) : BntNode(brTarget, taken, cond), mResourcePeStateStacks(EResourcePeStateTypeSize, nullptr), mRealPath(0ull), mInstructions(0ull), mReservedTakenPath(false)
  {
    for ( EResourcePeStateTypeBaseType type = 0; type < EResourcePeStateTypeSize; type ++)
      mResourcePeStateStacks[type] = new ResourcePeStateStack(EResourcePeStateType(type));
  }

  SpeculativeBntNode::~SpeculativeBntNode()
  {
    for (auto state_stack : mResourcePeStateStacks) {
      if (not state_stack->IsEmpty()) {
        LOG(fail) << "{SpeculativeBntNode::~SpeculativeBntNode} dangling resource state pointer on the stack." << endl;
        FAIL("dangling-resource-state-pointer");
      }
      delete state_stack;
    }

  }

  void SpeculativeBntNode::PushResourcePeState(const ResourcePeState* pState)
  {
    // << "{SpeculativeBntNode::PushResourcePeState} Push resource Pe state: " << pState->ToString() << endl;
    auto state_type =  pState->GetStateType();
    mResourcePeStateStacks[(unsigned int)state_type]->PushResourcePeState(pState);
  }

  bool SpeculativeBntNode::RecoverResourcePeStates(Generator* pGen)
  {
    bool regime_switch = false;
    SimAPI *sim_ptr = pGen->GetSimAPI();

    for (auto state_stack : mResourcePeStateStacks)
      regime_switch |= state_stack->RecoverResourcePeStates(pGen, sim_ptr);

    return regime_switch;
  }

  SpeculativeBntNode::SpeculativeBntNode() : BntNode(), mResourcePeStateStacks(),mRealPath(0ull), mInstructions(0ull), mReservedTakenPath(false)
  {

  }

  SpeculativeBntNode::SpeculativeBntNode(const SpeculativeBntNode& rOther) : BntNode(rOther), mResourcePeStateStacks(EResourcePeStateTypeSize, nullptr), mRealPath(rOther.mRealPath), mInstructions(rOther.mInstructions), mReservedTakenPath(rOther.mReservedTakenPath)
  {
    for ( EResourcePeStateTypeBaseType type = 0; type < EResourcePeStateTypeSize; type ++)
      mResourcePeStateStacks[type] = new ResourcePeStateStack(EResourcePeStateType(type));
  }

  void SpeculativeBntNode::ReserveTakenPath(Generator* pGen)
  {
    auto taken_path = TakenPath();
    auto vm_mapper = pGen->GetVmManager()->CurrentVmMapper();
    const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
    uint64 untagged_taken_address = addr_tagging->UntagAddress(taken_path, true);

    auto size = pGen->DefaultInstructionSize();
    auto end = untagged_taken_address + size - 1u;
    if (end < untagged_taken_address) {
      LOG(fail) << "{SpeculativeBntNode::ReserveTakenPath} Reserve taken path with size that will cause wrap-around." << endl;
      FAIL("reserve-taken-path-with-wrap-around");
    }

    auto virtual_constr = vm_mapper->VirtualUsableConstraintSet(true);
    if (not virtual_constr->ContainsRange(untagged_taken_address, end)) {
      LOG(info) << "{SpeculativeBntNode::ReserveTakenPath} Not reserve virtual address range:0x" << hex
                << untagged_taken_address << "-0x" << end << endl;
      mReservedTakenPath = false;
      return;
    }

    uint64 pa = 0;
    uint32 bank = 0;
    auto trans_result = vm_mapper->TranslateVaToPa(untagged_taken_address, pa, bank);
    if (trans_result != ETranslationResultType::Mapped) {
      vm_mapper->MapAddressRange(untagged_taken_address, size, true);
    }

    char print_buffer[32];
    snprintf(print_buffer, 32, "BNT%d", mId);
    LOG(info) << "{SpeculativeBntNode::ReserveTakenPath} Reserve " << print_buffer << ", virtual address range:0x"
              << hex << untagged_taken_address << "-0x" << end << endl;
    pGen->ReserveMemory(print_buffer, untagged_taken_address, size, 0, true);
    mReservedTakenPath = true;
    return;
  }

  void SpeculativeBntNode::UnreserveTakenPath(Generator* pGen)
  {
    if (not mReservedTakenPath)
      return;

    auto taken_path = TakenPath();
    auto vm_mapper = pGen->GetVmManager()->CurrentVmMapper();
    const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
    uint64 untagged_taken_address = addr_tagging->UntagAddress(taken_path, true);

    char print_buffer[32];
    auto size = pGen->DefaultInstructionSize();

    snprintf(print_buffer, 32, "BNT%d", mId);
    LOG(info) << "{SpeculativeBntNode::UnreserveTakenPath} Unreserve " << print_buffer << ", virtual address range:0x"
              << hex << untagged_taken_address << "-0x" << (untagged_taken_address + size - 1)  << endl;

    pGen->UnreserveMemory(print_buffer, untagged_taken_address, size, 0, true);
  }

  void SpeculativeBntNode::RecordExecution(const Instruction* pInstr)
  {
    mInstructions ++;
  }

  bool SpeculativeBntNode::ExecutionIsOverflow()
  {
    auto bnt_limit = Config::Instance()->LimitValue(ELimitType::SpeculativeBntInstructionLimit);
    return (mInstructions >= bnt_limit);
  }

}
