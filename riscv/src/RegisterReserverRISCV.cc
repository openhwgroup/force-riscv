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
#include "RegisterReserverRISCV.h"

#include "Constraint.h"
#include "Log.h"
#include "Register.h"

using namespace std;

/*
  \file RegisterReserverRISCV.cc
  \brief Code supporting providing RISC-specific functionality for reserving registers.
*/

namespace Force
{

  RegisterReserverRISCV::RegisterReserverRISCV()
    : RegisterReserver(), mReserveGroupsByOprType(), mReserveGroupsByRegType()
  {
    mReserveGroupsByOprType.emplace(EOperandType::GPR, ERegReserveGroup::GPR);
    mReserveGroupsByOprType.emplace(EOperandType::Register, ERegReserveGroup::GPR);
    mReserveGroupsByOprType.emplace(EOperandType::FPR, ERegReserveGroup::FPRSIMDR);
    mReserveGroupsByOprType.emplace(EOperandType::VECREG, ERegReserveGroup::VECREG);
    mReserveGroupsByOprType.emplace(EOperandType::SysReg, ERegReserveGroup::SystemRegister);

    mReserveGroupsByRegType.emplace(
      ERegisterType::GPR,
      vector<ERegReserveGroup>({ERegReserveGroup::GPR}));
    mReserveGroupsByRegType.emplace(
      ERegisterType::ZR,
      vector<ERegReserveGroup>({ERegReserveGroup::GPR}));
    mReserveGroupsByRegType.emplace(
      ERegisterType::FPR,
      vector<ERegReserveGroup>({ERegReserveGroup::FPRSIMDR}));
    mReserveGroupsByRegType.emplace(
      ERegisterType::SIMDR,
      vector<ERegReserveGroup>({ERegReserveGroup::FPRSIMDR}));
    mReserveGroupsByRegType.emplace(
      ERegisterType::VECREG,
      vector<ERegReserveGroup>({ERegReserveGroup::VECREG}));
    mReserveGroupsByRegType.emplace(
      ERegisterType::SysReg,
      vector<ERegReserveGroup>({ERegReserveGroup::SystemRegister}));
  }

  RegisterReserverRISCV::RegisterReserverRISCV(const RegisterReserverRISCV& rOther)
    : RegisterReserver(rOther), mReserveGroupsByOprType(rOther.mReserveGroupsByOprType), mReserveGroupsByRegType(rOther.mReserveGroupsByRegType)
  {
  }

  ERegReserveGroup RegisterReserverRISCV::GetReserveGroupForOperandType(const EOperandType oprType) const
  {
    auto itr = mReserveGroupsByOprType.find(oprType);
    if (itr == mReserveGroupsByOprType.end()) {
      LOG(fail) << "{RegisterReserverRISCV::GetReserveGroupForOperandType} unsupported operand type: " << EOperandType_to_string(oprType) << endl;
      FAIL("unsupported-operand-type");
    }

    return itr->second;
  }

  const vector<ERegReserveGroup>& RegisterReserverRISCV::GetReserveGroupsForRegisterType(const ERegisterType regType) const
  {
    auto itr = mReserveGroupsByRegType.find(regType);
    if (itr == mReserveGroupsByRegType.end()) {
      LOG(fail) << "{RegisterReserverRISCV::GetReserveGroupForRegisterType} unsupported register type: " << ERegisterType_to_string(regType) << endl;
      FAIL("unsupported-register-type");
    }

    return itr->second;
  }

  void RegisterReserverRISCV::GetRegisterIndexRange(const ERegisterType regType, ConstraintSet* pIndexConstr) const
  {
    switch (regType) {

    case ERegisterType::GPR:
      pIndexConstr->AddRange(0, 31);
      break;

    case ERegisterType::FPR:
    case ERegisterType::VECREG:
      pIndexConstr->AddRange(0, 31);
      break;
    case ERegisterType::ZR:
      pIndexConstr->AddValue(0);
      break;
    default:
      LOG(fail) << "{RegisterReserverRISCV::GetRegisterIndexRange} unsupported register type: " << ERegisterType_to_string(regType) << endl;
      FAIL("unsupported-register-type");
    }
  }

  uint32 RegisterReserverRISCV::GetPhysicalRegisterIndex(const PhysicalRegister* pPhysReg, const ERegisterType regType) const
  {
    uint32 reg_index = pPhysReg->IndexValue();

    return reg_index;
  }

}
