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
#include <VmContextParameter.h>
#include <Generator.h>
#include <Log.h>

#include <sstream>

using namespace std;

/*!
  \file VmContextParameter.cc
  \brief Code for virtual memory context parameter base implementation.
*/

namespace Force
{

  uint32 VmContext::msSequentialGenContextId = 0;

  VmContextParameter::VmContextParameter(EVmContextParamType paramType, const std::string& rRegName, const std::string& rFieldName)
    : mParamType(paramType), mRegisterName(rRegName), mFieldName(rFieldName), mValue(0), mInitialized(false)
  {
  }

  VmContextParameter::~VmContextParameter()
  {
  }

  VmContextParameter::VmContextParameter(const VmContextParameter& rOther)
    : mParamType(rOther.mParamType), mRegisterName(rOther.mRegisterName), mFieldName(rOther.mFieldName), mValue(0), mInitialized(false)
  {
  }

  void VmContextParameter::Initialize(Generator* pGen)
  {
    if (not mInitialized) {
      mValue = pGen->GetRegisterFieldValue(mRegisterName, mFieldName);
      mInitialized = true;
    }
  }

  uint64 VmContextParameter::Reload(Generator* pGen)
  {
    return pGen->RegisterFieldReloadValue(mRegisterName, mFieldName);
  }

  bool VmContextParameter::Validate(Generator* pGen, std::string& rErrMsg) const
  {
    if (not mInitialized) {
      return true;
    }

    uint64 curr_val = 0;
    pGen->ReadRegister(mRegisterName, mFieldName, curr_val);
    if (curr_val != mValue)
    {
      stringstream err_stream;
      err_stream << " [" << EVmContextParamType_to_string(mParamType) << "] " << mRegisterName << "." << mFieldName << " 0x" << hex << mValue << "=>0x" << curr_val;
      rErrMsg += err_stream.str();
    }
    return (curr_val == mValue);
  }

  void VmContextParameter::VerifyCompatibility(const VmContextParameter& rOther) const
  {
    if (mParamType != rOther.mParamType) {
      LOG(fail) << "{VmContextParameter::VerifyCompatibility} parameter type mismatch: " << EVmContextParamType_to_string(mParamType) << " vs: " << EVmContextParamType_to_string(rOther.mParamType) << endl;
      FAIL("parameter-type-mismatch");
    }
    if (mRegisterName != rOther.mRegisterName) {
       LOG(fail) << "{VmContextParameter::VerifyCompatibility} register name mismatch: " << mRegisterName << " vs: " << rOther.mRegisterName << endl;
      FAIL("register-name-mismatch");
    }
    if (mFieldName != rOther.mFieldName) {
      LOG(fail) << "{VmContextParameter::VerifyCompatibility} register name mismatch: " << mRegisterName << " vs: " << rOther.mRegisterName << endl;
      FAIL("field-name-mismatch");
    }
  }

  void VmContextParameter::Update(const VmContextParameter& rOther)
  {
    VerifyCompatibility(rOther);

    mValue = rOther.mValue;
    mInitialized = rOther.mInitialized;
  }

  bool VmContextParameter::LessThan(const VmContextParameter& rOther) const
  {
    VerifyCompatibility(rOther);

    if (mInitialized < rOther.mInitialized) return true;

    if (mValue < rOther.mValue) return true;

    return false;
  }

  bool VmContextParameter::GetDelta(uint64& rValue, const Generator* pGen) const
  {
    if (not mInitialized) {
      LOG(fail) << "{VmContextParameter::GetDelta} called when parameter is not yet initialized." << endl;
      FAIL("vm-context-parameter-not-yet-initialized");
    }
    uint64 pe_value = 0;
    pGen->ReadRegister(mRegisterName, mFieldName, pe_value);
    if (mValue != pe_value) {
      rValue = mValue;
      return true;
    }
    return false;
  }

  const string VmContextParameter::ParameterName() const
  {
    return mRegisterName + "." + mFieldName;
  }

  bool VmContextParameter::Matches(const VmContextParameter& rOther) const
  {
    VerifyCompatibility(rOther);

    if (mInitialized != rOther.mInitialized) return false;

    if (mValue != rOther.mValue) return false;

    return true;
  }

  const string VmContextParameter::ToString() const
  {
    stringstream out_str;
    out_str << "ContextParameter [" << EVmContextParamType_to_string(mParamType) << "] " << mRegisterName << "." << mFieldName << "=0x" << hex << mValue << "(initialized=" << mInitialized << ")" << endl;
    return out_str.str();
  }

  VmContext::VmContext()
    : Object(), mGenContextId(0), mContextParams()
  {
    ObtainGenContextId();
  }

  VmContext::VmContext(const VmContext& rOther)
    : Object(rOther), mGenContextId(0), mContextParams()
  {
    ObtainGenContextId();

    mContextParams.resize(rOther.mContextParams.size(), nullptr);

    for (uint32 index = 0; index < rOther.mContextParams.size(); ++ index) {
      auto other_item = rOther.mContextParams[index];
      if (nullptr != other_item) {
        mContextParams[index] = other_item->Clone();
      }
    }
  }

  VmContext::~VmContext()
  {
    for (auto vm_param : mContextParams) {
      delete vm_param;
    }
  }

  void VmContext::ObtainGenContextId()
  {
    mGenContextId = ++ msSequentialGenContextId; // start generator context ID from non zero.
  }

  const string VmContext::ToString() const
  {
    stringstream out_str;

    out_str << Type() << "(" << dec << mGenContextId << ") ";
    bool first_done = false;
    for (const auto& param : mContextParams) {
      if (nullptr == param) continue;
      if (not first_done) {
        first_done = true;
      }
      else {
        out_str << ", ";
      }
      out_str << param->ToString();
    }

    return out_str.str();
  }

  void VmContext::InitializeContext(Generator* pGen)
  {
    // initialize the context parameters populated in the setup function
    for (auto& param : mContextParams) {
      if (nullptr == param) continue;
      param->Initialize(pGen);
      //LOG(notice) << "{VmControlBlock::InitializeContext} " << EVmContextParamType_to_string(param.first) << "=0x" << hex << param->Value() << endl;
    }
  }

  void VmContext::GetContextDelta(std::map<std::string, uint64> & rDeltaMap, const Generator* pGen) const
  {
    for (auto param_item : mContextParams) {
      if (nullptr != param_item) {
        uint64 delta_value = 0;
        if (param_item->GetDelta(delta_value, pGen)) {
          rDeltaMap[param_item->ParameterName()] = delta_value;
        }
      }
    }
  }

  bool VmContext::operator < (const VmContext& rOther) const
  {
    if (mContextParams.size() != rOther.mContextParams.size()) {
      LOG(fail) << "{VmContext::operator<} number of parameters mismatch, mine: " << dec << mContextParams.size() << " others: " << rOther.mContextParams.size() << endl;
      FAIL("parameter-number-mismatch");
    }

    EVmContextParamTypeBaseType index = 0;
    for (; index < mContextParams.size(); ++ index) {
      const auto other_param = rOther.mContextParams[index];
      const auto my_param = mContextParams[index];

      if (NullParameterPair(my_param, other_param)) continue;

      if (my_param->LessThan(*other_param)) return true;
    }

    return false;
  }

  bool VmContext::Matches(const VmContext& rOther) const
  {
    if (mContextParams.size() != rOther.mContextParams.size()) return false;

    EVmContextParamTypeBaseType index = 0;
    for (; index < mContextParams.size(); ++ index) {
      const auto other_param = rOther.mContextParams[index];
      const auto my_param = mContextParams[index];

      if (NullParameterPair(my_param, other_param)) continue;

      if (not my_param->Matches(*other_param)) return false;
    }

    return true;
  }

  bool VmContext::UpdateContextParams(const std::map<std::string, uint64> & rRawContextParams)
  {
    for (auto const& x : rRawContextParams)
    {
      bool bValid = false;
      EVmContextParamType type = try_string_to_EVmContextParamType(x.first, bValid);

      if (!bValid)
      {
        LOG(fail) << "{VmContext::UpdateContextParams} invalid key " << x.first << std::endl;
        FAIL("context-parameter-key-invalid");
      }
      else
      {
        GetContextParameter(type)->SetValue(x.second);
      }
    }
    return true;
  }

  void VmContext::AddParameter(VmContextParameter* pContextParam)
  {
    EVmContextParamTypeBaseType base_type = EVmContextParamTypeBaseType(pContextParam->ParamType());
    if (base_type >= EVmContextParamTypeSize) {
      LOG(fail) << "{VmContext::AddParameter} adding parameter with type index: " << dec << base_type << " out of bound, should be less than: " << EVmContextParamTypeSize << endl;
      FAIL("vm-context-parameter-index-out-of-bound");
    }

    if (base_type >= mContextParams.size()) {
      mContextParams.resize(uint32(base_type) + 1, nullptr);
    }

    if (nullptr != mContextParams[base_type]) {
      LOG(fail) << "{VmContext::AddParameter} adding parameter but there is already existing parameter object assigned." << endl;
      FAIL("duplicated-vm-context-parameter");
    }

    mContextParams[base_type] = pContextParam;
  }

  VmContextParameter* VmContext::GetContextParameter(EVmContextParamType paramType) const
  {
    EVmContextParamTypeBaseType base_type = EVmContextParamTypeBaseType(paramType);

    if (base_type >= mContextParams.size()) {
      LOG(fail) << "{VmContext::GetContextParameter} access of parameter: " << EVmContextParamType_to_string(paramType) << " out of bound." << endl;
      FAIL("context-parameter-access-out-of-bound");
    }

    return mContextParams[base_type];
  }

  bool VmContext::NullParameterPair(const VmContextParameter* pParam1, const VmContextParameter* pParam2) const
  {
    if (nullptr == pParam1) {
      if (nullptr != pParam2) {
        LOG(fail) << "{VmContext::NullParameterPair} parameter 1 is nullptr but parameter 2 is not." << endl;
        FAIL("nullptr-parameter-mismatch-1");
      }
      return true;
    }

    if (nullptr == pParam2) {
      LOG(fail) << "{VmContext::NullParameterPair} parameter 1 is not nullptr but parameter 2 is nullptr." << endl;
      FAIL("nullptr-parameter-mismatch-2");
    }
    return false;
  }

  bool VmContext::UpdateContext(const VmContext* pVmContext)
  {
    if (pVmContext == nullptr) return false;

    EVmContextParamTypeBaseType index = 0;
    for (; index < pVmContext->mContextParams.size(); ++ index) {
      const auto update_param = pVmContext->mContextParams[index];
      auto my_param = GetContextParameter(EVmContextParamType(index));

      if (NullParameterPair(update_param, my_param)) continue;

      my_param->Update(*update_param);
    }

    return true;
  }

}
