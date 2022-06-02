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
#include "ResourcePeState.h"

#include <algorithm>
#include <sstream>

#include "Defines.h"
#include "GenPC.h"
#include "Generator.h"
#include "Log.h"
#include "MemoryManager.h"
#include "Register.h"
#include "ResourceDependence.h"
#include "UtilityFunctions.h"

PICKY_IGNORE_BLOCK_START
#include "SimAPI.h"
PICKY_IGNORE_BLOCK_END

using namespace std;

/*!
  \file ResourcePeState.cc
  \brief Code for all kinds of resource state
*/

namespace Force {

  ResourcePeStateStack::~ResourcePeStateStack()
  {
    if (not IsEmpty()) {
      LOG(fail) << "{ResourcePeStateStack::~ResourcePeStateStack} dangling resource state pointer on the stack." << endl;
      FAIL("dangling-resource-state-pointer");
    }
  }

  void ResourcePeStateStack::PushResourcePeState(const ResourcePeState* pState)
  {
    if (IsDuplicated(pState)) {
      LOG(info) << "{ResourcePeStateStack::PushResourcePeState}" << " Ignore duplicated resource state: " << pState->ToString() << endl;
      delete pState;
      return;
    }
    mResourcePeStates.push_back(pState);
  }

  bool ResourcePeStateStack::RecoverResourcePeStates(Generator* pGen, SimAPI* pSim)
  {
    bool context_switch = false;
    while (not mResourcePeStates.empty()) {
      auto state = mResourcePeStates.back();
      context_switch |= state->DoStateRecovery(pGen, pSim);
      delete state;
      mResourcePeStates.pop_back();
    }

    return context_switch;
  }

  bool ResourcePeStateStack::IsEmpty() const
  {
    return mResourcePeStates.empty();
  }

  bool ResourcePeStateStack::IsDuplicated(const ResourcePeState* pState) const
  {
    bool duplicate = any_of(mResourcePeStates.cbegin(), mResourcePeStates.cend(),
      [pState](const ResourcePeState* pCurState) { return pCurState->IsIdenticalState(pState); });

    return duplicate;
  }

  IncrementalResourcePeStateStack::IncrementalResourcePeStateStack(EResourcePeStateType type)
    : ResourcePeStateStack(type), mRestoreFunction(nullptr), mNextEndIndex(0)
  {
  }

  IncrementalResourcePeStateStack::~IncrementalResourcePeStateStack()
  {
    for (const ResourcePeState* pState : mResourcePeStates) {
      delete pState;
    }

    // If mNextEndIndex is at the end of the vector, all of the state data has been processed. Otherwise, we won't clear
    // the vector, which will trigger a failure in the superclass destructor.
    if (mNextEndIndex == mResourcePeStates.size()) {
      mResourcePeStates.clear();
    }
  }

  bool IncrementalResourcePeStateStack::RecoverResourcePeStates(Generator* pGen, SimAPI* pSim)
  {
    if (mRestoreFunction != nullptr) {
      vector<const ResourcePeState*> states_to_restore;
      states_to_restore.reserve(mResourcePeStates.size() - mNextEndIndex);
      for (size_t i = mResourcePeStates.size(); i > mNextEndIndex; i--) {
        states_to_restore.push_back(mResourcePeStates[i - 1]);
      }

      mRestoreFunction(states_to_restore);
      mNextEndIndex = mResourcePeStates.size();
    }
    else {
      LOG(fail) << "{IncrementalResourcePeStateStack::RecoverResourcePeStates} no restore function set." << endl;
      FAIL("no-restore-function");
    }

    return false;
  }

  void IncrementalResourcePeStateStack::SetRestoreFunction(RestoreFunction restoreFunction)
  {
    mRestoreFunction = restoreFunction;
  }

  const std::string PCPeState::ToString() const
  {
    stringstream sstream;

    sstream << Type() << ": " << hex << mPC;
    return sstream.str();
  }

  bool PCPeState::DoStateRecovery(Generator* pGen, SimAPI* pSim) const
  {
    // << "{PCState::DoStateRecovery}" << ToString() << endl;
    pSim->WriteRegister(pGen->ThreadId(), "PC", mPC, -1ull);
    pGen->GetGenPC()->SetAligned(mPC);
    return false;
  }

  bool PCPeState::IsIdenticalState(const ResourcePeState* pState) const
  {
    auto pc_state = pState->CastInstance<const PCPeState>();
    if (pc_state == nullptr)
      return false;

    return (mPC == pc_state->GetPC());
  }


  const std::string RegisterPeState::ToString() const
  {
    stringstream sstream;

    sstream << Type() << ", register name: " << mpPhysicalRegister->Name() << ", Mask: 0x"<< hex << mMask << ", value: 0x"<< hex << mValue;
    return sstream.str();
  }

  const char* RegisterPeState::Type() const
  {
    return "RegisterPeState";
  }

  bool RegisterPeState::DoStateRecovery(Generator* pGen, SimAPI* pSim) const
  {
    // << "{RegState::DoStateRecovery}" << ToString() << endl;
    pSim->WriteRegister(pGen->ThreadId(), mpPhysicalRegister->Name().c_str(), mValue, mMask);
    mpPhysicalRegister->SetValue(mValue, mMask & mpPhysicalRegister->Mask());
    return (mpPhysicalRegister->RegisterType() == ERegisterType::SysReg);
  }

  bool RegisterPeState::IsIdenticalState(const ResourcePeState* pState) const
  {
    auto reg_state = pState->CastInstance<const RegisterPeState>();
    if (reg_state == nullptr)
      return false;

    return (GetPhysicalRegister() == reg_state->GetPhysicalRegister());
  }

  EResourcePeStateType RegisterPeState::GetStateType() const
  {
    return EResourcePeStateType::RegisterPeState;
  }

  const std::string MemoryPeState::ToString() const
  {
    stringstream sstream;

    sstream << Type() << ", bank: " << mBank << ", physical address: 0x"<< hex << mPhysicalAddress << ", Data: 0x" << hex << GetData() << endl;
      return sstream.str();
  }

  const char* MemoryPeState::Type() const
  {
    return "MemoryPeState";
  }

  bool MemoryPeState::IsIdenticalState(const ResourcePeState* pState) const
  {
    auto mem_state = pState->CastInstance<const MemoryPeState>();
    if (mem_state == nullptr)
      return false;

    return (GetBank() == mem_state->GetBank()) && (GetPhysicalAddress() == mem_state->GetPhysicalAddress());
  }

  EResourcePeStateType MemoryPeState::GetStateType() const
  {
    return EResourcePeStateType::MemoryPeState;
  }

  const char* ByteMemoryPeState::Type() const
  {
    return "ByteMemoryPeState";
  }

  bool ByteMemoryPeState::DoStateRecovery(Generator* pGen, SimAPI* pSim) const
  {
    // << "{MemoryState::DoStateRecovery}" << ToString() << endl;
    pSim->WritePhysicalMemory(GetBank(), GetPhysicalAddress(), 1, (const unsigned char *)&mData);
    pGen->GetMemoryManager()->GetMemoryBank(GetBank())->WriteMemory(GetPhysicalAddress(), (cuint8* )&mData, 1);
    return false;
  }

  uint64 ByteMemoryPeState::GetData() const
  {
    return mData;
  }

  const char* BlockMemoryPeState::Type() const
  {
    return "BlockMemoryPeState";
  }

  bool BlockMemoryPeState::DoStateRecovery(Generator* pGen, SimAPI* pSim) const
  {
    vector<uint8> data_vec(mBlockSize, 0);
    element_value_to_data_array_big_endian(mData, mBlockSize, data_vec.data());
    pSim->WritePhysicalMemory(GetBank(), GetPhysicalAddress(), mBlockSize, data_vec.data());
    pGen->GetMemoryManager()->GetMemoryBank(mBank)->WriteMemory(GetPhysicalAddress(), data_vec.data(), mBlockSize);
    return false;
  }

  bool BlockMemoryPeState::IsIdenticalState(const ResourcePeState* pState) const
  {
    auto block_mem_state = pState->CastInstance<const BlockMemoryPeState>();
    if (block_mem_state == nullptr) {
      return false;
    }

    return (GetBank() == block_mem_state->GetBank()) && (GetVirtualAddress() == block_mem_state->GetVirtualAddress());
  }

  uint64 BlockMemoryPeState::GetData() const
  {
    return mData;
  }

  uint32 BlockMemoryPeState::GetBlockSize() const
  {
    return mBlockSize;
  }

  const std::string DependencePeState::ToString() const
  {
    stringstream sstream;

    sstream << Type() << ", " << mpDependence->ToString() << endl;

    return sstream.str();
  }

  const char* DependencePeState::Type() const
  {
    return "DependencePeState";
  }

  bool DependencePeState::DoStateRecovery(Generator* pGen, SimAPI* pSim) const
  {
    // << "{DependencePeState::DoStateRecovery}" << ToString() << endl;
    pGen->SetDependenceInstance(const_cast<ResourceDependence* >(mpDependence));
    mpDependence = nullptr;

    return false;
  }

  bool DependencePeState::IsIdenticalState(const ResourcePeState* pState) const
  {
    return false;
  }

  EResourcePeStateType DependencePeState::GetStateType() const
  {
    return EResourcePeStateType::DependencePeState;
  }

}
