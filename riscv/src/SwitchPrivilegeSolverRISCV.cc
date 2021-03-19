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
#include <SwitchPrivilegeSolverRISCV.h>

#include <Choices.h>
#include <ChoicesModerator.h>
#include <GenException.h>
#include <GenRequest.h>
#include <Generator.h>
#include <Log.h>
#include <PageRequestRegulator.h>
#include <Register.h>
#include <RegisterReload.h>
#include <RegisteredSetModifier.h>
#include <StringUtils.h>
#include <VaGenerator.h>
#include <VmInfo.h>
#include <VmManager.h>
#include <VmMapper.h>

#include <memory>

using namespace std;

/*!
  \file SwitchPrivilegeSolverRISCV.cc
  \brief Code solving privilege level switch scenarios.
*/

namespace Force {

  SwitchPrivilegeSolver::SwitchPrivilegeSolver(Generator* pGenerator)
    : mpGenerator(pGenerator), mResult(), mTargetAddrSpecified(false), mSkipAddrValidation(false), mAddrChoicesModId(0), mCurrentPrivLevel(MAX_UINT32), mCurrentState(), mTargetState(), mVmReloadRegisters(), mStateRegFieldNames({"UIE", "SIE", "MIE", "SUM", "MXR", "MPRV"})
  {
  }

  const SwitchPrivilegeResultInfo& SwitchPrivilegeSolver::Solve(const map<string, string>& paramsDict)
  {
    Setup(paramsDict);

    ValidateAndGenerateTargetState();

    mResult.mSuccess = ValidateAndGenerateSwitchScheme();

    if (mResult.mSuccess) {
      ValidateAndGenerateStatusValue();

      ValidateAndGenerateTargetAddress();
      GenerateIntermediateReturnAddress();

      LOG(notice) << "{SwitchPrivilegeSolver::Solve} target privilege level " << dec << mResult.mTargetPrivLevel << ", xstatus value 0x" << hex << mResult.mStatusVal << endl;
    }

    return mResult;
  }

  void SwitchPrivilegeSolver::Setup(const map<string, string>& rParams)
  {
    SetupCurrentState();
    SetupDefaultTargetState();

    // Reset values
    mResult = SwitchPrivilegeResultInfo();
    mTargetAddrSpecified = false;
    mSkipAddrValidation = false;
    mAddrChoicesModId = 0;

    for (const auto& param : rParams)
    {
      LOG(notice) << "{SwitchPrivilegeSolver::Setup} Key: " << param.first << ", Value: " << param.second << endl;

      if (param.first == "PrivilegeLevel") {
        bool priv_level_parsed = false;

        if (param.second == "Random") {
          mResult.mTargetPrivLevel = 4;
          priv_level_parsed = true;
        }

        if (not priv_level_parsed) {
          mResult.mTargetPrivLevel = EPrivilegeLevelTypeBaseType(try_string_to_EPrivilegeLevelType(param.second, priv_level_parsed));
        }

        if (not priv_level_parsed) {
          mResult.mTargetPrivLevel = parse_uint32(param.second);
        }

        if (mResult.mTargetPrivLevel > 4) {
          LOG(fail) << "{SwitchPrivilegeSolver::Setup} unexpected target privilege level " << dec << mResult.mTargetPrivLevel << endl;
          FAIL("unexpected-privilege-level");
        }
      }
      else if (param.first == "TargetAddr") {
        mResult.mTargetAddr = parse_uint64(param.second);
        mTargetAddrSpecified = true;
      }
      else if (param.first == "SkipAddrValidation") {
        mSkipAddrValidation = (parse_uint32(param.second) > 0);
      }
      else if (param.first == "AddrChoicesModID") {
        mAddrChoicesModId = parse_uint32(param.second);
      }
      else if (IsTargetStateParameter(param.first)) {
        SetTargetStateValue(param.first, param.second);
      }
      else {
        LOG(fail) << "{SwitchPrivilegeSolver::Setup} Unsupported parameter: " << param.first << endl;
        FAIL("unsupported-parameter");
      }
    }

    mTargetState["SUM"] = ParseTargetStateParameterValue("Same");
    mTargetState["MXR"] = ParseTargetStateParameterValue("Same");
    mTargetState["MPRV"] = ParseTargetStateParameterValue("Same");
  }

  void SwitchPrivilegeSolver::ValidateAndGenerateTargetState()
  {
    if (mResult.mTargetPrivLevel == 4) {
      mResult.mTargetPrivLevel = ChooseValue("Privilege Switch - Target Privilege");
    }

    mTargetState["UIE"] = ComputeTargetStateValue(mCurrentState["UIE"], mTargetState["UIE"], "Privilege Switch - Interrupt Mask");
    mTargetState["SIE"] = ComputeTargetStateValue(mCurrentState["SIE"], mTargetState["SIE"], "Privilege Switch - Interrupt Mask");
    mTargetState["MIE"] = ComputeTargetStateValue(mCurrentState["MIE"], mTargetState["MIE"], "Privilege Switch - Interrupt Mask");
    mTargetState["SUM"] = ComputeTargetStateValue(mCurrentState["SUM"], mTargetState["SUM"], "Privilege Switch - SUM Bit");
    mTargetState["MXR"] = ComputeTargetStateValue(mCurrentState["MXR"], mTargetState["MXR"], "Privilege Switch - MXR Bit");
    mTargetState["MPRV"] = ComputeTargetStateValue(mCurrentState["MPRV"], mTargetState["MPRV"], "Privilege Switch - MPRV Bit");
  }

  bool SwitchPrivilegeSolver::ValidateAndGenerateSwitchScheme()
  {
    if ((mCurrentPrivLevel > 3) or (mResult.mTargetPrivLevel > 3)) {
      LOG(notice) << "{SwitchPrivilegeSolver::ValidateAndGenerateSwitchScheme} invalid privilege level" << endl;

      return false;
    }

    if ((mResult.mTargetPrivLevel > mCurrentPrivLevel) or (mCurrentPrivLevel == 0)) {
      LOG(info) << "{SwitchPrivilegeSolver::ValidateAndGenerateSwitchScheme} switching to higher privilege level; current privilege level = " << dec << mCurrentPrivLevel << ", target privilege level = " << mResult.mTargetPrivLevel << endl;

      if ((mCurrentPrivLevel == 0) and (mResult.mTargetPrivLevel == 3)) {
        mResult.mInstrSeqCode = 3;  // Generate 2 ECALL instructions
      }
      else {
        mResult.mInstrSeqCode = 1;  // Generate 1 ECALL instruction
      }
    }
    else {
      LOG(info) << "{SwitchPrivilegeSolver::ValidateAndGenerateSwitchScheme} switching to lower or same privilege level; current privilege level = " << dec << mCurrentPrivLevel << ", target privilege level = " << mResult.mTargetPrivLevel << endl;

      mResult.mInstrSeqCode = ChooseValue("Privilege level switch to lower or same level");
    }

  switch (mResult.mInstrSeqCode) {
    case 1:
      if (mCurrentPrivLevel == 0) {
        const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
        Register* medeleg_reg = reg_file->RegisterLookup("medeleg");

        uint64 medeleg_val = medeleg_reg->Value();
        if ((medeleg_val & 0x100) == 0x100) {
          // Environment calls from U mode are delegated to S mode
          mResult.mDataBlockPrivLevel = 1;
        }
        else {
          // Environment calls from U mode are handled in M mode
          mResult.mDataBlockPrivLevel = 3;
        }
      }
      else {
        mResult.mDataBlockPrivLevel = 3;
      }

      break;
    case 2:
      mResult.mDataBlockPrivLevel = mCurrentPrivLevel;
      break;
    case 3:
      mResult.mDataBlockPrivLevel = 3;
      break;
    default:
      LOG(fail) << "{SwitchPrivilegeSolver::ValidateAndGenerateSwitchScheme} unexpected instruction sequence code " << dec << mResult.mInstrSeqCode << endl;
      FAIL("unexpected-instr-seq-code");
  }

    LOG(notice) << "{SwitchPrivilegeSolver::ValidateAndGenerateSwitchScheme} instruction sequence code " << dec << mResult.mInstrSeqCode << endl;

    return true;
  }

  void SwitchPrivilegeSolver::ValidateAndGenerateStatusValue()
  {
    string status_reg_name;
    vector<string> status_field_names = {"SUM", "MXR"};
    switch (mResult.mDataBlockPrivLevel) {
    case 1:
      status_reg_name = "sstatus";
      status_field_names.push_back("SPP");
      break;
    case 3:
      status_reg_name = "mstatus";
      status_field_names.push_back("MPP");
      break;
    default:
      LOG(fail) << "{SwitchPrivilegeSolver::ValidateAndGenerateStatusValue} unexpected data block privilege level " << dec << mResult.mDataBlockPrivLevel << endl;
      FAIL("unexpected-privilege-level");
    }

    // On execution of an xRET instruction xPIE is copied to xIE, so we need to set the xPIE bit
    // instead of the xIE bit for the data block privilege level
    switch(mResult.mTargetPrivLevel) {
    case 0:
      status_field_names.push_back("UIE");
      break;
    case 1:
      if (mResult.mDataBlockPrivLevel == 1) {
        status_field_names.push_back("SPIE");
      }
      else {
        status_field_names.push_back("SIE");
      }

      break;
    case 3:
      status_field_names.push_back("MPIE");
      status_field_names.push_back("MPRV");
      break;
    default:
      LOG(fail) << "{SwitchPrivilegeSolver::ValidateAndGenerateStatusValue} unexpected target privilege level " << dec << mResult.mTargetPrivLevel << endl;
      FAIL("unexpected-privilege-level");
    }

    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    Register* status_reg = reg_file->RegisterLookup(status_reg_name);
    mResult.mStatusVal = status_reg->Value();

    uint64 field_mask = status_reg->GetRegisterFieldMask(status_field_names);
    mResult.mStatusVal &= ~field_mask;
    for (const string& status_field_name : status_field_names) {
      uint64 status_field_val = 0;

      if ((status_field_name == "SPP") or (status_field_name == "MPP")) {
        status_field_val = mResult.mTargetPrivLevel;
      }
      else if (status_field_name.substr(1) == "PIE") {
        status_field_val = mTargetState[status_field_name.substr(0, 1) + "IE"];
      }
      else {
        status_field_val = mTargetState[status_field_name];
      }

      RegisterField* status_field = status_reg->RegisterFieldLookup(status_field_name);
      mResult.mStatusVal |= (status_field_val << status_field->Lsb());
    }
  }

  void SwitchPrivilegeSolver::ValidateAndGenerateTargetAddress()
  {
    VmMapper* vm_mapper = GetVmMapper(mResult.mTargetPrivLevel);
    ValidateVmContextAndUpdate(vm_mapper);

    if (mSkipAddrValidation) {
      if (mTargetAddrSpecified) {
        LOG(notice) << "{SwitchPrivilegeSolver::ValidateAndGenerateTargetAddress} using specified target address: 0x" << hex << mResult.mTargetAddr << " without validation" << endl;
      }
      else {
        LOG(fail) << "{SwitchPrivilegeSolver::ValidateAddressValidationParameter} no target address specified. Unable to skip address validation without a specified target address." << endl;
        FAIL("no-target-address");
      }

      return;
    }

    unique_ptr<GenPageRequest> page_req(mpGenerator->GenPageRequestInstance(true, EMemAccessType::Unknown));
    page_req->SetPrivilegeLevel(EPrivilegeLevelType(mResult.mTargetPrivLevel));

    const PageRequestRegulator* page_req_regulator = mpGenerator->GetPageRequestRegulator();
    page_req_regulator->RegulateBranchPageRequest(vm_mapper, nullptr, page_req.get());

    if (mTargetAddrSpecified and vm_mapper->VerifyVirtualAddress(mResult.mTargetAddr, 64, true, page_req.get())) {
      LOG(notice) << "{SwitchPrivilegeSolver::ValidateAndGenerateTargetAddress} specified target address: 0x" << hex << mResult.mTargetAddr << " is valid" << endl;
    }
    else {
      LOG(notice) << "{SwitchPrivilegeSolver::ValidateAndGenerateTargetAddress} specified target address: 0x" << hex << mResult.mTargetAddr << " is not valid. Generating a new target address." << endl;

      RegisteredSetModifier* reg_set_modifier = mpGenerator->GetRegisteredSetModifier();
      if (mAddrChoicesModId != 0) {
        reg_set_modifier->ApplyModificationSet(mAddrChoicesModId);
      }

      VaGenerator va_gen(vm_mapper, page_req.get());
      mResult.mTargetAddr = va_gen.GenerateAddress(4, 64, true, EMemAccessType::Unknown);

      LOG(notice) << "{SwitchPrivilegeSolver::ValidateAndGenerateTargetAddress} generated target address: 0x" << hex << mResult.mTargetAddr << endl;

      if (mAddrChoicesModId != 0) {
        reg_set_modifier->RevertModificationSet(mAddrChoicesModId);
      }
    }
  }

  void SwitchPrivilegeSolver::GenerateIntermediateReturnAddress()
  {
    // Only need an intermediate address for a 2 ECALL sequence
    if (mResult.mInstrSeqCode == 3) {
      uint32 intermediate_priv_level = 1;

      VmMapper* vm_mapper = GetVmMapper(intermediate_priv_level);

      const PageRequestRegulator* page_req_regulator = mpGenerator->GetPageRequestRegulator();
      unique_ptr<GenPageRequest> page_req(mpGenerator->GenPageRequestInstance(true, EMemAccessType::Unknown));
      page_req->SetPrivilegeLevel(EPrivilegeLevelType(intermediate_priv_level));
      page_req_regulator->RegulateBranchPageRequest(vm_mapper, nullptr, page_req.get());

      VaGenerator va_gen(vm_mapper, page_req.get());
      mResult.mIntermediateRetAddr = va_gen.GenerateAddress(4, 64, true, EMemAccessType::Unknown);
    }
  }

  void SwitchPrivilegeSolver::SetupCurrentState()
  {
    mCurrentPrivLevel = mpGenerator->PrivilegeLevel();

    mCurrentState.clear();
    for (const string& reg_field_name : mStateRegFieldNames) {
      mCurrentState[reg_field_name] = GetRegisterFieldValue("mstatus", reg_field_name);
    }
  }

  void SwitchPrivilegeSolver::SetupDefaultTargetState()
  {
    mResult.mTargetPrivLevel = 4;  // Always random by default

    uint32 default_target_state_val = ChooseValue("Privilege Switch - Default Target State");

    LOG(notice) << "{SwitchPrivilegeSolver::SetupDefaultTargetState} default target state value: " << dec << default_target_state_val << endl;

    mTargetState.clear();
    for (const string& reg_field_name : mStateRegFieldNames) {
      mTargetState[reg_field_name] = default_target_state_val;
    }
  }

  void SwitchPrivilegeSolver::SetTargetStateValue(const string& rParamName, const string& rParamVal)
  {
    uint32 target_state_val = ParseTargetStateParameterValue(rParamVal);
    if (rParamName == "InterruptMask") {
      mTargetState["UIE"] = target_state_val;
      mTargetState["SIE"] = target_state_val;
      mTargetState["MIE"] = target_state_val;
    }
    else {
      mTargetState[rParamName] = target_state_val;
    }
  }

  void SwitchPrivilegeSolver::ValidateVmContextAndUpdate(const VmMapper* pTargetMapper)
  {
    unique_ptr<RegisterReload> reg_reload(pTargetMapper->GetRegisterReload());
    if (reg_reload == nullptr) {
      LOG(fail) << "{ValidateVmContextAndUpdate} register reload pointer not available" << endl;
      FAIL("no-register-reload");
    }

    mVmReloadRegisters.clear();
    mVmReloadRegisters["satp"] = reg_reload->GetRegisterValue("satp");
    LOG(debug) << "[SwitchPrivilegeSolver::ValidateVmContextAndUpdate] satp: 0x" << std::hex << reg_reload->GetRegisterValue("satp") << std::dec << std::endl;
  }

  VmMapper* SwitchPrivilegeSolver::GetVmMapper(cuint32 privLevel) const
  {
    VmManager* vm_manager = mpGenerator->GetVmManager();

    unique_ptr<VmInfo> vm_info(vm_manager->VmInfoInstance());
    vm_info->SetPrivilegeLevel(privLevel);
    vm_info->GetOtherStates(*mpGenerator);

    VmMapper* vm_mapper = vm_manager->GetVmMapper(*vm_info);
    vm_mapper->Initialize();

    return vm_mapper;
  }

  bool SwitchPrivilegeSolver::IsTargetStateParameter(const string& rParamName) const
  {
    //const set<string> target_state_keys = {"InterruptMask", "SUM", "MXR", "MPRV"};
    const set<string> target_state_keys = {"InterruptMask"};

    return (target_state_keys.find(rParamName) != target_state_keys.end());
  }

  uint32 SwitchPrivilegeSolver::ParseTargetStateParameterValue(const string& rParamVal) const
  {
    const map<string, uint32> target_state_values = {{"0", 0}, {"1", 1}, {"Same", 2}, {"Flip", 3}, {"Random", 4}};

    uint32 target_state_val = 4;
    auto itr = target_state_values.find(rParamVal);
    if (itr != target_state_values.end()) {
      target_state_val = itr->second;
    }

    return target_state_val;
  }

  uint32 SwitchPrivilegeSolver::GetRegisterFieldValue(const string& rRegName, const string& rRegFieldName) const
  {
    uint64 reg_field_val = MAX_UINT32;  // Indicate no value set by default
    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    Register* reg = reg_file->RegisterLookup(rRegName);
    if (reg->HasAttribute(ERegAttrType::HasValue)) {
      mpGenerator->ReadRegister(rRegName, rRegFieldName, reg_field_val);
    }

    return static_cast<uint32>(reg_field_val);
  }

  uint32 SwitchPrivilegeSolver::ComputeTargetStateValue(cuint32 currentVal, cuint32 targetVal, const string& rChoicesTreeName) const
  {
    uint32 target_val = targetVal;

    // If there is no original value and the target value is dependent on the original value, choose
    // the target value randomly.
    if (currentVal == MAX_UINT32 and ((target_val == 2) or (target_val == 3))) {
      target_val = 4;
    }

    switch (target_val) {
    case 0:
    case 1:
      // Literal value
      break;
    case 2:
      // Match current state
      target_val = currentVal;
      break;
    case 3:
      // Flip current state
      target_val = (currentVal == 0) ? 1 : 0;
      break;
    case 4:
      // Random
      target_val = ChooseValue(rChoicesTreeName);
      break;
    default:
      LOG(fail) << "{SwitchPrivilegeSolver::ComputeTargetStateValue} Invalid target state value " << dec << target_val << endl;
      FAIL("invalid-target-state-value");
    }

    return target_val;
  }

  uint32 SwitchPrivilegeSolver::ChooseValue(const string& rChoicesTreeName) const
  {
    ChoicesModerator* pChoicesModerator = mpGenerator->GetChoicesModerator(EChoicesType::GeneralChoices);
    if (pChoicesModerator == nullptr) {
      LOG(fail) << "{SwitchPrivilegeSolver::ChooseValue} general choices moderator not found" << endl;
      FAIL("choices-moderator-not-found");
    }

    unique_ptr<ChoiceTree> choice_tree(pChoicesModerator->CloneChoiceTree(rChoicesTreeName));
    const Choice* choice = choice_tree->Choose();
    uint32 choice_val = static_cast<uint32>(choice->Value());

    LOG(notice) << "{SwitchPrivilegeSolver::ChooseValue} " << rChoicesTreeName << ", selected value: " << dec << choice_val << endl;

    return choice_val;
  }

}
