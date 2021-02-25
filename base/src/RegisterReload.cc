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
#include <RegisterReload.h>
#include <Log.h>

#include <ChoicesModerator.h>
#include <Register.h>
#include <Constraint.h>

using namespace std;

/*!
  \file DataStation.cc
  \brief Code for data station
*/

namespace Force {

  RegisterUpdate::RegisterUpdate(const Register* pReg)
    : mName(pReg->Name()), mpRegister(pReg), mUpdateValue(0), mValidateMask(0), mFieldContraints(), mDontCareBits(0)
  {
  }

  RegisterUpdate::~RegisterUpdate()
  {
    mpRegister = nullptr;

    for (auto item : mFieldContraints)
    {
      delete item.second;
    }
  }

  void RegisterUpdate::Finalize(const ChoicesModerator* pChoicesModerator)
  {
    uint64 value = mpRegister->ReloadValue(pChoicesModerator, mFieldContraints);
    mpRegister->RegisterFieldInfo(pChoicesModerator, mFieldContraints, mValidateMask, mUpdateValue);
    mUpdateValue |= (value&(~mValidateMask));
  }

  bool RegisterUpdate::Validate() const
  {
    uint64 mask = (~mDontCareBits)&mValidateMask;   // mask out don't care bits
    LOG(notice) << "{RegisterUpdate::Validate} name:" << mName << " register value:0x" << hex << mpRegister->Value() << " stored value:0x" << mUpdateValue << " mask:0x" << mValidateMask << " actual mask:0x" << mask << endl;
    return (mpRegister->Value()&mask) == (mUpdateValue & mask);
  }

  void RegisterUpdate::Apply(RegisterFile* pRegFile)
  {
    pRegFile->RegisterLookup(mName)->SetValue(mUpdateValue);
  }

  void RegisterUpdate::Update(const string& fieldName, uint64 value, uint64 dont_care_bits)
  {
    auto field_finder = mFieldContraints.find(fieldName);
    if (field_finder != mFieldContraints.end()) {
      delete field_finder->second;
      field_finder->second = nullptr;
    }
    ConstraintSet* constraint_set = new ConstraintSet(value);
    mFieldContraints[fieldName] = constraint_set;
    mDontCareBits |= dont_care_bits;
  }

  RegisterReload::RegisterReload(const RegisterFile *pRegFile, const ChoicesModerator* pChoicesMod)
    : mpRegisterFile(pRegFile), mpChoicesModerator(pChoicesMod), mReloadMap()
  {
  }

  RegisterReload::~RegisterReload()
  {
    for (auto item : mReloadMap)
    {
      delete item.second;
    }
  }

  RegisterReload::RegisterReload(const RegisterReload& rOther)
    : Object(rOther), mpRegisterFile(nullptr), mpChoicesModerator(nullptr), mReloadMap()
  {
    LOG(fail) << "{RegisterReload::RegisterReload} copy constructor not expected to be called." << endl;
    FAIL("unexpected-call-copy-constructor");
  }

  RegisterReload::RegisterReload()
    : Object(), mpRegisterFile(nullptr), mpChoicesModerator(nullptr), mReloadMap()
  {
    LOG(fail) << "{RegisterReload::RegisterReload} default constructor not expected to be called." << endl;
    FAIL("unexpected-call-default-constructor");
  }

  Object* RegisterReload::Clone() const
  {
    return new RegisterReload(*this);
  }

  const std::string RegisterReload::ToString() const
  {
    return "RegisterReload";
  }

  void RegisterReload::AddRegisterFieldUpdate(const Register* pReg, const std::string& fieldName, uint64 value, uint64 dont_care_bits)
  {
    auto reg_name = pReg->RealName();

    auto field_finder = mReloadMap.find(reg_name);
    RegisterUpdate *reg_update = nullptr;
    if (field_finder != mReloadMap.end()) {
      reg_update = field_finder->second;
    } else {
      reg_update = new RegisterUpdate(pReg);
      mReloadMap[reg_name] = reg_update;
    }

    reg_update->Update(fieldName, value, dont_care_bits);
  }

  void RegisterReload::Finalize()
  {
    for (auto item : mReloadMap)
    {
      item.second->Finalize(mpChoicesModerator);
    }
  }

  uint64 RegisterReload::GetRegisterValue(const std::string& regName) const
  {
    auto field_finder = mReloadMap.find(regName);
    RegisterUpdate *reg_update = nullptr;
    if (field_finder != mReloadMap.end()) {
      reg_update = field_finder->second;
      return reg_update->Value();
    } else {
      return 0;
    }
  }

  bool RegisterReload::Validate() const
  {
    bool valid = all_of(mReloadMap.cbegin(), mReloadMap.cend(),
      [](const pair<string, RegisterUpdate*>& rReload) { return rReload.second->Validate(); });

    return valid;
  }

  void RegisterReload::Apply(RegisterFile* pRegFile)
  {
    for (auto item : mReloadMap)
    {
      item.second->Apply(pRegFile);
    }
  }

}
