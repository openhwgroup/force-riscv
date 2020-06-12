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
#ifndef Force_ConditionFlags_H
#define Force_ConditionFlags_H

#include <Defines.h>

namespace Force {

  /*!
    \struct ConditionFlags
    \brief Structure to contain condition flag values.
  */
  struct ConditionFlags {
  public:
    //!< Default constructor
    ConditionFlags()
      : mNegativeFlag(0), mZeroFlag(0), mCarryFlag(0), mOverflowFlag(0)
    {
    }

    //!< Constructor
    ConditionFlags(cuint8 negativeFlag, cuint8 zeroFlag, cuint8 carryFlag, cuint8 overflowFlag)
      : mNegativeFlag(negativeFlag), mZeroFlag(zeroFlag), mCarryFlag(carryFlag), mOverflowFlag(overflowFlag)
    {
    }

    COPY_CONSTRUCTOR_DEFAULT(ConditionFlags); //!< Copy constructor
    DESTRUCTOR_DEFAULT(ConditionFlags); //!< Destructor
    ASSIGNMENT_OPERATOR_DEFAULT(ConditionFlags); //!< Assignment operator
  public:
    uint8 mNegativeFlag; //!< Negative flag.
    uint8 mZeroFlag; //!< Zero flag.
    uint8 mCarryFlag; //!< Carry flag.
    uint8 mOverflowFlag; //!< Overflow flag.
  };

}

#endif  // Force_ConditionFlags_H
