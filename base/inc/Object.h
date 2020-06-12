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
#ifndef Force_Object_H
#define Force_Object_H

#include <string>

namespace Force {

  /*!
    \class Object
    \brief Base interface defining class for lots of classes in the project.
  */

  class Object {
  public:
    Object() { }
    Object(const Object& rOther) { }
    virtual Object* Clone() const = 0; //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    virtual const std::string ToString() const = 0; //!< Return a string describing the current state of the Object.
    virtual const char* Type() const = 0; //!< Return a string describing the actual type of the Object.
    virtual ~Object() { } //!< Virtual destructor to ensure orderly destruction of derived classes.
  };

}

#endif
