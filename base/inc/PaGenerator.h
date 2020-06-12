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
#ifndef Force_PaGenerator_H
#define Force_PaGenerator_H

#include <Defines.h>

namespace Force {

  class ConstraintSet;

  class PaGenerator {
  public:
    explicit PaGenerator(const ConstraintSet* baseConstr) : mpBaseConstraint(baseConstr), mIsInstruction(true) { } //!< Most used constructor.
    ~PaGenerator(); //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(PaGenerator);
    COPY_CONSTRUCTOR_DEFAULT(PaGenerator);
    uint64 GenerateAddress(uint64 align, uint64 size, bool isInstr, const ConstraintSet *rangeConstraintSet = nullptr); //!< Generate an address.
    uint64 GetAddressFromBottom(uint64 align, uint64 size, const ConstraintSet *rangeConstraintSet = nullptr); //!< Get an address from the bottom of the address constraint set.
    uint64 GetAddressFromTop(uint64 align, uint64 size, const ConstraintSet *rangeConstraintSet = nullptr); //!< Get an address from the top of the address constraint set.
  protected:
    PaGenerator() : mpBaseConstraint(nullptr), mIsInstruction(true) { } //!< Default constructor.

  protected:
    const ConstraintSet* mpBaseConstraint; //!< Pointer to base constraint.
    bool mIsInstruction; //!< Indicate if this is generating for instruction usage.
  };

}

#endif
