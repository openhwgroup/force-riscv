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
#ifndef Force_AddressReuseMode_H
#define Force_AddressReuseMode_H

#include "Defines.h"
#include "Enums.h"

namespace Force {

  /*!
    \class AddressReuseMode
    \brief Class to track which address reuse types are enabled.
  */
  class AddressReuseMode {
  public:
    AddressReuseMode(); //!< Default constructor
    AddressReuseMode(const AddressReuseMode& rOther);
    DESTRUCTOR_DEFAULT(AddressReuseMode); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(AddressReuseMode);

    void EnableReuseType(const EAddressReuseType addrReuseType) { mModeBits |= EAddressReuseTypeBaseType(addrReuseType); } //!< Enable the specified reuse type.
    void DisableReuseType(const EAddressReuseType addrReuseType) { mModeBits &= ~EAddressReuseTypeBaseType(addrReuseType); } //!< Disable the specified reuse type.
    void DisableAllReuseTypes(); //!< Disable all reuse types.
    bool IsReuseTypeEnabled(const EAddressReuseType addrReuseType) const; //!< Return whether the specified reuse type is enabled.
  private:
    EAddressReuseTypeBaseType mModeBits; //!< Bits representing which reuse types are enabled
  };

}

#endif  // Force_AddressReuseMode_H
