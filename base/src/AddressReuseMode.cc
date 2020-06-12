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
#include <AddressReuseMode.h>
#include <UtilityFunctions.h>

/*!
  \file AddressReuseMode.cc
  \brief Code supporting tracking which address reuse types are enabled.
*/

namespace Force {

  AddressReuseMode::AddressReuseMode()
    : mModeBits(0)
  {
  }

  void AddressReuseMode::DisableAllReuseTypes()
  {
    mModeBits = 0;
  }

  bool AddressReuseMode::IsReuseTypeEnabled(const EAddressReuseType addrReuseType) const
  {
    EAddressReuseTypeBaseType addrReuseTypeValue = EAddressReuseTypeBaseType(addrReuseType);
    return ((mModeBits & addrReuseTypeValue) == addrReuseTypeValue);
  }

}
