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
#include <VmControlBlock.h>
#include <RegisterReload.h>
#include <Generator.h>
#include <Register.h>
#include <Log.h>

#include <sstream>

using namespace std;

/*!
  \file VmControlBlock.cc
  \brief Code support base VmControlBlock and VmDirectMapControlBlock
*/

namespace Force {

  VmControlBlock::VmControlBlock(EPrivilegeLevelType privType, EMemBankType memType)
    : VmContext(), mpGenerator(nullptr), mPrivilegeLevel(privType), mDefaultMemoryBank(memType), mBigEndian(false), mStage(1), mMaxPhysicalAddress(0)
  {
  }

  VmControlBlock::VmControlBlock()
    : VmContext(), mpGenerator(nullptr), mPrivilegeLevel(EPrivilegeLevelType(0)), mDefaultMemoryBank(EMemBankType(0)), mBigEndian(false), mStage(1), mMaxPhysicalAddress(0)
  {
  }

  VmControlBlock::VmControlBlock(const VmControlBlock& rOther)
    : VmContext(rOther), mpGenerator(nullptr), mPrivilegeLevel(EPrivilegeLevelType(0)), mDefaultMemoryBank(EMemBankType(0)), mBigEndian(false), mStage(1), mMaxPhysicalAddress(0)
  {
  }

  VmControlBlock::~VmControlBlock()
  {
    mpGenerator = nullptr;
  }

  const string VmControlBlock::ToString() const
  {
    stringstream out_str;
    out_str << Type()
            << " Big-Endian=" << mBigEndian
            << " Stage=" << mStage
            << " PrivilegeLevel=" << EPrivilegeLevelType_to_string(mPrivilegeLevel)
            << " Mem=" << EMemBankType_to_string(mDefaultMemoryBank)
            << " Max-PA=0x" << hex << mMaxPhysicalAddress;

    out_str << AdditionalAttributesString();

    for (const auto& param : mContextParams)
    {
      if (nullptr != param) {
        out_str << " " << EVmContextParamType_to_string(param->ParamType()) << "=0x" << hex << param->Value();
      }
    }

    return out_str.str();
  }

  void VmControlBlock::Setup(Generator* pGen)
  {
    mpGenerator = pGen;
  }

  void VmControlBlock::Initialize()
  {
    mBigEndian = GetBigEndian();
    mMaxPhysicalAddress = GetMaxPhysicalAddress();
  }

  bool VmControlBlock::Validate(std::string& rErrMsg) const
  {
    bool valid_context = true;

    valid_context &= (mBigEndian == GetBigEndian());
    if (!valid_context)
    {
      rErrMsg += " Endianness,";
    }

    valid_context &= (mMaxPhysicalAddress == GetMaxPhysicalAddress());
    if (!valid_context)
    {
      rErrMsg += " Max Physical Address,";
    }

    // validate the context parameters populated in the setup function
    for (const auto param : mContextParams) {
      if (nullptr != param) {
        valid_context &= param->Validate(mpGenerator, rErrMsg);
        //LOG(notice) << "{VmControlBlock::Validate} " << EVmContextParamType_to_string(param->ParamType()) << "=0x" << hex << param->Value()
        //            << " valid_context=" << valid_context << endl;
      }
    }

    return valid_context;
  }

  RegisterReload* VmControlBlock::GetRegisterReload() const
  {
    RegisterReload *pRegContext = new RegisterReload(mpGenerator->GetRegisterFile(), mpGenerator->GetChoicesModerator(EChoicesType::RegisterFieldValueChoices));

    FillRegisterReload(pRegContext);

    pRegContext->Finalize();
    return pRegContext;
  }

  void VmControlBlock::FillRegisterReload(RegisterReload* pRegContext) const
  {
    auto reg_file = mpGenerator->GetRegisterFile();

    //Add register field updates for the parameters managed by mContextParams
    for (const auto& param : mContextParams)
    {
      if (nullptr != param) {
        auto reg_ptr = reg_file->RegisterLookup(param->RegisterName());
        pRegContext->AddRegisterFieldUpdate(reg_ptr, param->FieldName(), param->Value());
      }
    }
  }

}
