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
#include <ExceptionContext.h>
#include <Generator.h>
#include <Register.h>
#include <Instruction.h>
#include <VmManager.h>
#include <VmInfo.h>
#include <VmMapper.h>
#include <InstructionResults.h>
#include <Log.h>

#include <sstream>
#include <memory>

using namespace std;

namespace Force {

  ExceptionContext::ExceptionContext(const Generator* pGen, EExceptionClassType exceptClass)
    : mServicePrivilegeLevel(EPrivilegeLevelType(pGen->PrivilegeLevel())), mExceptionClass(exceptClass), mSourcePrivilegeLevel(EPrivilegeLevelType(0)), mPreferredReturnAddress(0), mExceptionAddress(0), mContextUpdated(false), mAddressAccurate(false), mPhysicalAddress(0), mMemoryBank(EMemBankType(0)),
      mPaValid(false), mpGenerator(pGen), mpRegisterFile(pGen->GetRegisterFile()), mpSourceInstruction(nullptr)
  {

  }

  ExceptionContext::~ExceptionContext()
  {
  }

  string ExceptionContext::ToString() const
  {
    stringstream out_str;

    out_str << "[" << EExceptionClassType_to_string(mExceptionClass) << " " << GetExceptionCategory(mExceptionClass) << "] taken to " << EPrivilegeLevelType_to_string(mServicePrivilegeLevel);

    if (mContextUpdated) {
      out_str << " from " << EPrivilegeLevelType_to_string(mSourcePrivilegeLevel) << ", address 0x" << hex << mExceptionAddress
              << "=>[";

      if (mPaValid) {
        out_str << EMemBankType_to_string(mMemoryBank) << "]0x" << mPhysicalAddress;
      }
      else {
        out_str << "?]0x?";
      }

      if (nullptr != mpSourceInstruction) {
        out_str << ", instruction: " << mpSourceInstruction->Name();
      }

      out_str << ", preferred return address 0x" << mPreferredReturnAddress;
    }

    return out_str.str();
  }

  void ExceptionContext::UpdateContext()
  {
    if (mContextUpdated) return;

    UpdateSourcePrivilegeLevel();
    UpdateSourceAddress();
    mContextUpdated = true;
  }

  bool ExceptionContext::UpdatePhysicalAddress()
  {
    if (mPaValid) return true;

    auto vm_manager = mpGenerator->GetVmManager();
    auto vm_info    = vm_manager->VmInfoInstance();
    std::unique_ptr<VmInfo> vm_info_storage(vm_info); // to release vm_info when done.
    vm_info->SetPrivilegeLevel(uint32(mServicePrivilegeLevel));
    vm_info->GetOtherStates(*mpGenerator); // obtain other states from the current PE states.
    auto source_el_mapper = vm_manager->GetVmMapper(*vm_info);
    uint32 mem_bank = 0;
    ETranslationResultType trans_result = source_el_mapper->TranslateVaToPa(mExceptionAddress, mPhysicalAddress, mem_bank);

    if (trans_result == ETranslationResultType::Mapped) {
      mMemoryBank = EMemBankType(mem_bank);
      mPaValid = true;
    }
    return mPaValid;
  }

  void ExceptionContext::UpdateSourceInstruction()
  {
    if (nullptr != mpSourceInstruction)
      return;

    UpdateContext();

    if (not UpdatePhysicalAddress())
      return; // VA to PA translation not sucessful.

    const ThreadInstructionResults* pe_instr_results = mpGenerator->GetInstructionResults();
    mpSourceInstruction = pe_instr_results->LookupInstruction(uint32(mMemoryBank), mPhysicalAddress);
  }

  string ExceptionContext::GetSourceInstructionName()
  {
    UpdateSourceInstruction();

    if (nullptr == mpSourceInstruction) {
      return "";
    }

    return mpSourceInstruction->Name();
  }

}
