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
#ifndef Force_RegisterInitPolicy_H
#define Force_RegisterInitPolicy_H

#include <Object.h>
#include <Defines.h>

namespace Force {

  class Register;
  class RegisterField;
  class RegisterFile;
  class Generator;
  class ChoiceTree;

  /*!
    \class RegisterInitPolicy
    \brief A class to help managing register random initialization.
  */

  class RegisterInitPolicy : public Object {
  public:
    Object*           Clone()    const override { return new RegisterInitPolicy(*this); } //!< Return correct cloned object.
    const std::string ToString() const override { return Type(); } //!< ToString function as declared in Object.
    const char*       Type()     const override { return "RegisterInitPolicy"; } //!< Correct type of the object.

    RegisterInitPolicy()  //!< Constructor.
      : Object(), mpGenerator(nullptr), mpRegisterFile(nullptr)
    {
    }

    virtual ~RegisterInitPolicy() { } //!< Destructor.
    virtual void Setup(const Generator* pGen); //!< Set up the object.
    ASSIGNMENT_OPERATOR_ABSENT(RegisterInitPolicy);

    virtual void InitializeRegister(Register* pRegister) const; //!< Initialize register.
    virtual void InitializeRegisterField(RegisterField* pRegField, const ChoiceTree* pChoiceTree = nullptr) const; //!< Initialize register field.
    virtual uint64 RegisterReloadValue(Register* pRegister) const; //!< Reload register value.
    virtual uint64 RegisterFieldReloadValue(RegisterField* pRegField) const; //!< Reload register field value.
  protected:

    RegisterInitPolicy(const RegisterInitPolicy& rOther) //!< Copy constructor.
      : Object(rOther), mpGenerator(nullptr), mpRegisterFile(nullptr)
    {
    }

    void SetRegisterFieldInitialValue(RegisterField* pRegField, uint64 value) const; //!< Set initial value on register field.

  protected:
    void SetRegisterInitialValue(Register* pRegister, uint64 value) const ; //!< set register initial value
  protected:
    const Generator* mpGenerator; //!< Pointer to generator.
    const RegisterFile* mpRegisterFile; //!< Pointer to regiser file.
  };

  /*!
    \struct InitPolicyTuple
    \brief Tuple holding register init policy info.
  */
  struct InitPolicyTuple {
  public:
    explicit InitPolicyTuple(const std::string& rPolicyName) //!< Constructor with name parameter.
      : mPolicyName(rPolicyName), mpInitPolicy(nullptr)
    {
    }

    InitPolicyTuple(const InitPolicyTuple& rOther) //!< Copy constructor.
      : mPolicyName(rOther.mPolicyName), mpInitPolicy(nullptr)
    {
    }

    ~InitPolicyTuple() { } //!< Destructor.
    void Setup(const RegisterFile* pRegisterFile); //!< Setup the init policy info.
  public:
    std::string mPolicyName; //!< Name of the initialization policy.
    ASSIGNMENT_OPERATOR_ABSENT(InitPolicyTuple);
    const RegisterInitPolicy* mpInitPolicy; //!< Pointer to initialization policy object.
  };

}

#endif
