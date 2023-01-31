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
#ifndef Force_RecoveryAddressGenerator_H
#define Force_RecoveryAddressGenerator_H

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Generator;
  class GenPageRequest;
  class VmMapper;

  /*!
    \class RecoveryAddressGenerator
    \brief Class to generate addresses to use for recovery operations such as in fault handlers or register reloading.
  */
  class RecoveryAddressGenerator : public Object {
  public:
    explicit RecoveryAddressGenerator(const Generator* pGenerator); //!< Constructor
    SUPERCLASS_DESTRUCTOR_DEFAULT(RecoveryAddressGenerator); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(RecoveryAddressGenerator);

    const std::string ToString() const override { return Type(); } //!< Return a string describing the current state of the Object.

    uint64 GenerateAddress(cuint64 align, cuint64 size, cbool isInstr, const EMemAccessType memAccessType) const; //!< Generate an address.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(RecoveryAddressGenerator);
    virtual const GenPageRequest* CreateGenPageRequest(cbool isInstr, const EMemAccessType memAccessType) const = 0; //!< Create a GenPageRequest instance.
    VmMapper* GetCurrentVmMapper() const; //!< Get the active VmMapper instance.
  private:
    const Generator* mpGenerator; //!< Generator
  };

}

#endif  // Force_RecoveryAddressGenerator_H
