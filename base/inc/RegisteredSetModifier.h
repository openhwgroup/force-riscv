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
#ifndef Force_RegisteredSet_H
#define Force_RegisteredSet_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <Object.h>

#include <string>
#include <list>
#include <map>

namespace Force {

  class Generator;
  class ModificationSet;

  /*!
    \class RegisteredSetModifier
    \brief register choices modifications
   */
  class RegisteredSetModifier : public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned RegisteredSetModifier object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the DataBlock object.
    const char* Type() const override { return "RegisteredSetModifier"; } //!< Return the type of the object

    RegisteredSetModifier() : Object(), mpGenerator(nullptr), mRegisteredModificationSets(), mCommittedStack()  { } //!< Constructor
    ~RegisteredSetModifier(); //!< destructor
    ASSIGNMENT_OPERATOR_ABSENT(RegisteredSetModifier);
    void Setup(const Generator* pGen); //!< set up
    void RegisterModificationSet(EChoicesType choicesType, uint32 mod_id); //!< register choices modifications into the registered tree modification list with modification ID
    void ApplyModificationSet(uint32 mod_id); //!< Apply the choices modifications from the oldest to the newest if their id is >= mod_id, move applied ones from registered list to applied list
    void RevertModificationSet(uint32 mod_id); //!< revert the choices modifications from the newest to the oldest if their session id is >= mod_id,  removed the reverted from the commit list
  private:
    RegisteredSetModifier(const RegisteredSetModifier& rOther) : Object(rOther), mpGenerator(nullptr),
      mRegisteredModificationSets(), mCommittedStack() { } // copy constructor

    const Generator *mpGenerator; //!< the pointer to generator
    std::list<ModificationSet*> mRegisteredModificationSets; //!< the list for registered tree modifications, ordered by session id by increase
    std::list<ModificationSet*> mCommittedStack; //!< the list for committed tree modifications, ordered by session id by increase
  };

}
#endif
