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
#include <VmDirectMapControlBlockRISCV.h>
#include <Generator.h>
#include <Config.h>
#include <Register.h>

/*!
  \file VmDirectMapControlBlockRISCV.cc
  \brief Code to configure RISC-V direct mapping control blocks.
*/

using namespace std;

namespace Force {

  VmDirectMapControlBlockRISCV::VmDirectMapControlBlockRISCV(EPrivilegeLevelType elType, EMemBankType memType)
   : VmDirectMapControlBlock(elType, memType)
  {

  }

  void VmDirectMapControlBlockRISCV::Setup(Generator* pGen)
  {
    VmDirectMapControlBlock::Setup(pGen);
  }

  void VmDirectMapControlBlockRISCV::Initialize()
  {
    VmDirectMapControlBlock::Initialize();

    VmDirectMapControlBlock::InitializeContext(mpGenerator);
  }

  uint64 VmDirectMapControlBlockRISCV::GetMaxPhysicalAddress() const
  {
    return Config::Instance()->LimitValue(ELimitType::PhysicalAddressLimit);
  }

}
