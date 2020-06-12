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
#include <RecoveryAddressGenerator.h>
#include <Generator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <VaGenerator.h>
#include <GenRequest.h>
#include <Log.h>
#include <memory>

using namespace std;

/*!
  \file RecoveryAddressGenerator.cc
  \brief Code supporting generating addresses to use for recovery operations such as in fault handlers or register reloading.
*/

namespace Force {

  RecoveryAddressGenerator::RecoveryAddressGenerator(const Generator* pGenerator)
    : Object(), mpGenerator(pGenerator)
  {
  }

  uint64 RecoveryAddressGenerator::GenerateAddress(cuint64 align, cuint64 size, cbool isInstr, const EMemAccessType memAccessType) const
  {
    std::unique_ptr<const GenPageRequest> page_req(CreateGenPageRequest(isInstr, memAccessType));
    VmMapper* vm_mapper = GetCurrentVmMapper();
    VaGenerator va_gen(vm_mapper, page_req.get());
    uint64 addr = va_gen.GenerateAddress(align, size, isInstr, memAccessType);

    LOG(notice) << "{RecoveryAddressGenerator::GenerateAddress} generated addr=0x" << hex << addr << endl;

    return addr;
  }

  VmMapper* RecoveryAddressGenerator::GetCurrentVmMapper() const
  {
    VmManager* vm_manager = mpGenerator->GetVmManager();
    return vm_manager->CurrentVmMapper();
  }

}
