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
#include <GeneratorRISCV.h>
#include <Register.h>
#include <Generator.h>
#include <GenPC.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <ConditionFlags.h>
#include <UtilityFunctions.h>
//#include <RestoreLoopRISCV.h>
#include <Log.h>

#include <algorithm>

using namespace std;

namespace Force {

  GeneratorRISCV::~GeneratorRISCV()
  {
  }

  Object* GeneratorRISCV::Clone() const
  {
    return new GeneratorRISCV(*this);
  }

  void GeneratorRISCV::Setup(uint32 threadId)
  {
    Generator::Setup(threadId);

    mpPrivilegeField = mpRegisterFile->RegisterLookup("privilege")->RegisterFieldLookup("MODE");

    //RestoreLoopManagerRepository* restore_loop_manager_repository = RestoreLoopManagerRepository::Instance();
    //restore_loop_manager_repository->AddRestoreLoopManager(threadId, new RestoreLoopManagerRISCV(this));
  }

  uint32 GeneratorRISCV::InstructionSpace() const
  {
    return 36;
  }

  uint32 GeneratorRISCV::BntReserveSpace() const
  {
    return 36;
  }

  uint32 GeneratorRISCV::BntMinSpace() const
  {
    return 4;
  }

  void GeneratorRISCV::SetPrivilegeLevel(uint32 priv)
  {
    LOG(notice) << "{GeneratorRISCV::SetPrivilegeLevel} setting privilege level to 0x" << hex << priv << endl;
    mpPrivilegeField->SetValue(priv);
  }

  uint32 GeneratorRISCV::PrivilegeLevel() const
  {
    return mpPrivilegeField->FieldValue();
  }

  ConditionFlags GeneratorRISCV::GetConditionFlags() const
  {
    return ConditionFlags(0, 0, 0, 0);
  }

  std::string GeneratorRISCV::GetGPRName(uint32 index) const
  {
    return "x" + to_string(index);
  }

  std::string GeneratorRISCV::GetGPRExcludes() const
  {
    return "0";
  }

  bool GeneratorRISCV::RegisterUpdateSystem(const std::string& name, const std::string& field)
  {
    bool updateVm = false;

    auto reg = mpRegisterFile->RegisterLookup(name);
    VmManager* vm_mgr = GetVmManager();
    VmRegime*  vm_regime = vm_mgr->CurrentVmRegime();

    auto reg_context = vm_regime->RegisterContext();

    if (any_of(reg_context.begin(), reg_context.end(), [reg](Register* item) { return reg == item; })) {
      LOG(notice) << "{GeneratorRISCV::RegisterUpdateSystem} given register name: " << name << " is in the current VM Regime register context list" << endl;
      updateVm = true;
    }
    return updateVm;
  }

  void GeneratorRISCV::UpdateSystemWithRegister(const std::string& name, const std::string& field, uint64 value)
  {
    if (RegisterUpdateSystem(name, field) == true)
      UpdateVm();
  }

  void GeneratorRISCV::SetupInstructionGroup(EInstructionGroupType iGrpType)
  {
  }

  /*!
    Currently only used in address solving related code, therefore only taking care of GPR, SP.
  */
  bool GeneratorRISCV::OperandTypeCompatible(ERegisterType regType, EOperandType oprType) const
  {
    bool compatible = false;
    switch (regType) {
    case ERegisterType::GPR:
      compatible = (oprType == EOperandType::GPR);
      break;
    default:
      LOG(fail) << "{GeneratorRISCV::OperandTypeCompatible} not handled register type: " << ERegisterType_to_string(regType) << endl;
      FAIL("not-handled-register-type");
    }
    return compatible;
  }

  bool GeneratorRISCV::OperandTypeToResourceType(EOperandType opType, EResourceType& rResType) const
  {
    bool convert_okay = true;
    switch (opType) {
    case EOperandType::Register:
    case EOperandType::GPR:
      rResType = EResourceType::GPR;
      break;
    case EOperandType::FPR:
      rResType = EResourceType::FPR;
      break;
    case EOperandType::SysReg:
      // don't need to maintain SysReg resource dependency.
    default:
      rResType = EResourceType(0);
      convert_okay = false;
    }
    return convert_okay;
  }

  uint32 GeneratorRISCV::GetResourceCount(EResourceType rResType) const
  {
    switch (rResType) {
    case EResourceType::GPR:
    case EResourceType::FPR:
      return 32;
    default:
      return 1; // TODO temporary, should not getting inapplicable types in the first place.
      //LOG(fail) << "{GeneratorRISCV::GetResourceCount} unexpected resource type: " << EResourceType_to_string(rResType) << endl;
      //FAIL("unexpected-resource-type");
    }

    return 0;
  }

  void GeneratorRISCV::AdvancePC()
  {
    mpGenPC->Advance(4);
  }
}
