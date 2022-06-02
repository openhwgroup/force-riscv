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
#ifndef Force_RecoveryAddressGeneratorRISCV_H
#define Force_RecoveryAddressGeneratorRISCV_H

#include "RecoveryAddressGenerator.h"

namespace Force {

  /*!
    \class RecoveryAddressGeneratorRISCV
    \brief RISC-V implementation class to generate addresses to use for recovery operations such as in fault handlers or register reloading.
  */
  class RecoveryAddressGeneratorRISCV : public RecoveryAddressGenerator {
  public:
    explicit RecoveryAddressGeneratorRISCV(const Generator* pGenerator); //!< Constructor
    SUBCLASS_DESTRUCTOR_DEFAULT(RecoveryAddressGeneratorRISCV); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(RecoveryAddressGeneratorRISCV);

    Object* Clone() const override { return new RecoveryAddressGeneratorRISCV(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "RecoveryAddressGeneratorRISCV"; } //!< Return a string describing the actual type of the Object.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(RecoveryAddressGeneratorRISCV);
    const GenPageRequest* CreateGenPageRequest(cbool isInstr, const EMemAccessType memAccessType) const override; //!< Create a GenPageRequest instance with the appropriate configuration for recovery address generation.
  };

}

#endif  // Force_RecoveryAddressGeneratorRISCV_H
