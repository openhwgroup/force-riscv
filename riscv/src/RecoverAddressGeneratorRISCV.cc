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
#include <RecoveryAddressGeneratorRISCV.h>
#include <VmMapper.h>
#include <GenRequest.h>

using namespace std;

/*!
  \file RecoveryAddressGeneratorRISCV.cc
  \brief RISC-V specific code supporting generating addresses to use for recovery operations such as in fault handlers or register reloading.
*/

namespace Force {

  RecoveryAddressGeneratorRISCV::RecoveryAddressGeneratorRISCV(const Generator* pGenerator)
    : RecoveryAddressGenerator(pGenerator)
  {
  }

  const GenPageRequest* RecoveryAddressGeneratorRISCV::CreateGenPageRequest(cbool isInstr, const EMemAccessType memAccessType) const
  {
    const VmMapper* vm_mapper  = GetCurrentVmMapper();

    // In a previous version, we attempted to avoid page faults altogether, which is too heavy handed. If the current
    // logic causes failures, we will revisit it, but we are not presently enforcing faultless recovery address
    // generation by default.
    GenPageRequest* page_req = vm_mapper->GenPageRequestRegulated(isInstr, memAccessType);

    page_req->SetGenAttributeValue(EPageGenAttributeType::Invalid, 0);  // Ensure Invalid attribute is set to 0.

    return page_req;
  }

}
