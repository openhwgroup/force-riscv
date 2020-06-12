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
}

#endif 
