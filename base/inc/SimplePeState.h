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
#ifndef Force_SimplePeState_H
#define Force_SimplePeState_H

#include <Defines.h>
#include <vector>

namespace Force {

  class Generator;
  class Register;

  /*!
    \class SimpleRegisterState
    \brief Simple Register state struct.
  */
  class SimpleRegisterState {
  public:
    explicit SimpleRegisterState(Register* pReg); //!< Constructor.
    SimpleRegisterState() : mpRegister(nullptr), mValue(0) { } //!< Default constructor.
  private:
    Register* mpRegister; //!< Pointer to register object.
    uint64 mValue; //!< Register value.

    friend class SimplePeState;
  };

  /*!
    \class SimplePeState
    \brief Class containing essential simple PE state information for
  */
  class SimplePeState {
  public:
    SimplePeState() : mRegisterStates(), mpGpr(nullptr) { } //!< Default constructor.
    ASSIGNMENT_OPERATOR_ABSENT(SimplePeState);
    COPY_CONSTRUCTOR_DEFAULT(SimplePeState);
    void SaveState(Generator* pGen, const std::vector<Register* >& rRegContext); //!< Save PE state.
    bool RestoreState(); //!< Restore PE state;
  private:
    std::vector<SimpleRegisterState> mRegisterStates;
    Register* mpGpr; //!< Pointer to an useable GPR register.
  };

}

#endif
