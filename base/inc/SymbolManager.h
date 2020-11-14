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
#ifndef Force_SymbolManager_H
#define Force_SymbolManager_H

#include <Object.h>
#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <map>

namespace Force {

  /*!
    \class Symbol
    \brief Symbol class
  */
  class Symbol {
  public:
    explicit Symbol(const std::string& name); //!< Constructor.
    ~Symbol() { } //!< Destructor.
    void SetAddress(uint64 address) { mAddress = address; mSet = true; } //!< Set address.
    bool IsSet() const { return mSet; } //!< Return whether the address is set.
    uint64 Address() const { return mAddress; } //!< Return address associated with symbol.
    const std::string& Name() const { return mName; } //!< Return symbol name.
  private:
    std::string mName; //!< Symbol name.
    uint64 mAddress; //!< Address associated with the symbol.
    bool mSet; //!< Whether the symbol has address set.
  };
  
  /*!
    \class SymbolManager
    \brief Symbol manager class.
  */
  class SymbolManager : public Object 
  {
  public:
    explicit SymbolManager(EMemBankType bankType); //!< Constructor, with memory bank type given.
    ~SymbolManager(); //!< Destructor.

    Object* Clone() const override              { return nullptr; }            //!< Override for Object Clone.  Returns nullptr, object not meant to be copied.
    const std::string ToString() const override { return "SymbolManager"; } //!< Override for Object ToString.
    const char* Type() const override           { return "SymbolManager"; } //!< Override for Object Type.

    const std::map<std::string, Symbol* >& GetSymbols() const { return mSymbolMap; } //!< Return a const reference to the symbol map.
    bool HasSymbols() const { return mSymbolMap.size() > 0; } //!< Check if any symbol is defined.
    void AddSymbol(const std::string& rName, uint64 address); //!< Add an symbol.
    ASSIGNMENT_OPERATOR_ABSENT(SymbolManager);
    COPY_CONSTRUCTOR_ABSENT(SymbolManager);

  protected:
    EMemBankType mBankType; //!< Memory bank type.
    std::map<std::string, Symbol* > mSymbolMap; //!< Map containing Symbols.
  };
  
}

#endif
