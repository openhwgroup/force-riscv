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
#include "VmConstraintValidator.h"

#include <algorithm>

#include "AddressFilteringRegulator.h"
#include "Constraint.h"
#include "Log.h"
#include "VmConstraint.h"

using namespace std;

/*!
  \file VmConstraintValidator.cc
  \brief Code implementing VmConstraintValidator class.
*/

namespace Force {

  VmConstraintValidator::VmConstraintValidator(const GenPageRequest* pPageReq, const AddressFilteringRegulator* pAddrRegulator, const VmMapper* pVmMapper)
    : mpPageRequest(pPageReq), mpAddressRegulator(pAddrRegulator), mpVmMapper(pVmMapper), mVmConstraints()
  {
    mpAddressRegulator->GetVmConstraints(*mpPageRequest, *mpVmMapper, mVmConstraints);
  }

  VmConstraintValidator::~VmConstraintValidator()
  {
    for (auto vm_constr : mVmConstraints) {
      delete vm_constr;
    }
  }

  bool VmConstraintValidator::IsValid(uint64 value) const
  {
    bool valid = all_of(mVmConstraints.cbegin(), mVmConstraints.cend(),
      [value](const VmConstraint* pVmConstr) { return pVmConstr->Allows(value); });

    return valid;
  }

  void VmConstraintValidator::ApplyOn(ConstraintSet& rConstr) const
  {
    for (auto vm_constr : mVmConstraints) {
      vm_constr->ApplyOn(rConstr);
      if (rConstr.IsEmpty()) {
        LOG(info) << "{VmConstraintValidator::ApplyOn} constraint-set disallowed by VmConstrint: " << EVmConstraintType_to_string(vm_constr->Type()) << endl;
        return;
      }
    }
  }

}
