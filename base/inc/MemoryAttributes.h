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
#ifndef FORCE_MEMORYATTRIBUTES_H
#define FORCE_MEMORYATTRIBUTES_H

#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <Defines.h>
#include <PagingChoicesAdapter.h>
#include <Register.h>
#include <string>

namespace Force
{

  /*!
    \class MemoryAttributes
    \brief Generic interface definition for interacting with architectural and implementation specific memory attributes
  */
  class MemoryAttributes
  {
  public:
    MemoryAttributes() { }
    virtual ~MemoryAttributes() { }
    ASSIGNMENT_OPERATOR_ABSENT(MemoryAttributes);
    COPY_CONSTRUCTOR_ABSENT(MemoryAttributes);

    //TODO potentially refactor initialize and reload to be more generic
    // - would likely want this to become managed object which holds a ptr to regfile and/or choices adapter
    // - mair_name string passing would need to be modified
    virtual void   Initialize(const RegisterFile* pRegisterFile, const PagingChoicesAdapter& rChoicesAdapter, const std::string& rMairName) = 0; //!< Initialize memory attributes.
    virtual void   Reload(const RegisterFile* pRegisterFile, const PagingChoicesAdapter& rChoicesAdapter, const std::string& rMairName) = 0; //!< Reload memory attributes.
    virtual uint64 Value() const = 0; //!< Return memory attributes value.

    virtual const ConstraintSet* ArchTypeConstraint() const = 0; //!< Return arch memory attribute type constraint.
    virtual const ConstraintSet* ImplTypeConstraint() const = 0; //!< Return impl memory attribute type constraint.

    //TODO might need to refactor interfaces which reference index as that may not be architecturally agnostic
    virtual uint32 ArchTypeToIndex(uint32 archType) const = 0; //!< Return an index value with matching arch memory attribute type.
    virtual uint32 ImplTypeToIndex(EMemAttributeImplType implType) const = 0; //!< Return an index value with matching impl memory attribute type.
    virtual EMemAttributeImplType ArchTypeToImplType(uint32 archType) const = 0; //!< Convert architectural memory attribute type to implementation defined type.
    virtual EMemAttributeImplType GetImplMemoryTypeByIndex(uint32 index) const = 0; //!< Return implementation defined memory attribute by index.
    virtual EMemAttributeImplType ChooseMemoryAttribute(const PagingChoicesAdapter& rChoicesAdapter, EExceptionConstraintType exceptConstrType, uint32& index) const = 0; //!< Choose a memory attribute based on choices settings.
    virtual const std::map<std::string, uint64>& GetAttributeList() const = 0; //!< return map of attribute names->value
    virtual uint32 GetArchChoiceValue(uint32 archType) const = 0; //!< return arch choice value for given archType
  };

}
#endif //FORCE_MEMORYATTRIBUTES_H
