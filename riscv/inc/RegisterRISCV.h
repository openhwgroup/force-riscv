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
#ifndef Force_RegisterRISCV_H
#define Force_RegisterRISCV_H

#include <Register.h>

namespace Force {

  /*!
    \class RegisterFileRISCV
    \brief Class inheriting RegisterFile, with RISC-V specific architectural details.
  */
  class RegisterFileRISCV : public RegisterFile
  {
  public:
    Object * Clone() const override;            //!< Return a fully cloned RegisterFileRISCV object .
    const char* Type() const override { return "RegisterFileRISCV"; }    //!< Return the type of RegisterFileRISCV object in string format.

    RegisterFileRISCV();  //!< Constructor
    ~RegisterFileRISCV(); //!< Destructor

    Register* GetContainingRegister(const Register* pReg) const override; //!< Return an RISC-V register that contains the passed register object, if applicable.
    bool InitContainingRegister(Register* rContainingReg, const Register* pReg) const override; //!< initialize container reg from the reg it contains

    bool AllowReExecutionInit(const std::string& rRegName) const override; //!< Check if a register is allowed to be initialized in re-execution.
    void SetupRegisterReserver() override;  //!< set up register reserver.
    Register* RegisterLookupByIndex(uint32 index, const ERegisterType reg_type, uint32 size) const override; //!< RISC-V layer register lookup by index method.
  protected:
    RegisterFileRISCV(const RegisterFileRISCV& rOther); //!< Copy constructor, protected.
  };

}

#endif
