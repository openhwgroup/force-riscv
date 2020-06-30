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
#ifndef Force_RegisterInitPolicyRISCV_H
#define Force_RegisterInitPolicyRISCV_H

#include <RegisterInitPolicy.h>
#include <SetupRootPageTableRISCV.h>

namespace Force {

  class ConstraintSet;

  /*!
    \class PpnInitPolicy
    \brief A class to help managing register field random initialization.
  */

  class PpnInitPolicy : public RegisterInitPolicy {
  public:
    Object*           Clone()    const override { return new PpnInitPolicy(*this); } //!< Return correct cloned object.
    const std::string ToString() const override { return Type(); } //!< ToString function as declared in Object.
    const char*       Type()     const override { return "PpnInitPolicy"; } //!< Correct type of the object.

    PpnInitPolicy() : RegisterInitPolicy() { }  //!< Constructor.
    ~PpnInitPolicy() { } //!< Destructor.

    void InitializeRegisterField(RegisterField* pRegField, const ChoiceTree* pChoiceTree = nullptr) const override; //!< Initialize register field.
    uint64 RegisterFieldReloadValue(RegisterField* pRegField) const override; //!< Reload register field value.
  protected:
    PpnInitPolicy(const PpnInitPolicy& rOther) : RegisterInitPolicy(rOther) { } //!< Copy constructor.
  };

  /*!
    \class VlInitPolicy
    \brief A class to help initialize the vl register.
  */
  class VlInitPolicy : public RegisterInitPolicy {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VlInitPolicy);
    SUBCLASS_DESTRUCTOR_DEFAULT(VlInitPolicy);
    ASSIGNMENT_OPERATOR_ABSENT(VlInitPolicy);

    Object* Clone() const override { return new VlInitPolicy(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VlInitPolicy"; } //!< Return a string describing the actual type of the Object.

    void InitializeRegister(Register* pRegister) const override; //!< Initialize register.
    uint64 RegisterReloadValue(Register* pRegister) const override; //!< Reload register value.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VlInitPolicy);
  };

  /*!
    \class VstartInitPolicy
    \brief A class to help initialize the vstart register.
  */
  class VstartInitPolicy : public RegisterInitPolicy {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VstartInitPolicy);
    SUBCLASS_DESTRUCTOR_DEFAULT(VstartInitPolicy);
    ASSIGNMENT_OPERATOR_ABSENT(VstartInitPolicy);

    Object* Clone() const override { return new VstartInitPolicy(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VstartInitPolicy"; } //!< Return a string describing the actual type of the Object.

    void InitializeRegister(Register* pRegister) const override; //!< Initialize register.
    uint64 RegisterReloadValue(Register* pRegister) const override; //!< Reload register value.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VstartInitPolicy);
  };

  /*!
    \class VtypeInitPolicy
    \brief A class to help initialize the vtype register.
  */
  class VtypeInitPolicy : public RegisterInitPolicy {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VtypeInitPolicy);
    SUBCLASS_DESTRUCTOR_DEFAULT(VtypeInitPolicy);
    ASSIGNMENT_OPERATOR_ABSENT(VtypeInitPolicy);

    Object* Clone() const override { return new VtypeInitPolicy(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VtypeInitPolicy"; } //!< Return a string describing the actual type of the Object.

    void InitializeRegisterField(RegisterField* pRegField, const ChoiceTree* pChoiceTree = nullptr) const override; //!< Initialize register field.
    uint64 RegisterFieldReloadValue(RegisterField* pRegField) const override; //!< Reload register field value.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VtypeInitPolicy);
  };

}

#endif
