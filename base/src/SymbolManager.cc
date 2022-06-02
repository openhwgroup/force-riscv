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
#include "SymbolManager.h"

#include "Log.h"

using namespace std;

namespace Force
{
  
  Symbol::Symbol(const std::string& name)
    : mName(name), mAddress(0), mSet(false)
  {

  }

  SymbolManager::SymbolManager(EMemBankType bankType)
    : mBankType(bankType), mSymbolMap()
  {

  }

  SymbolManager::~SymbolManager()
  {
    for (auto map_item : mSymbolMap) {
      delete map_item.second;
    }
  }

  void SymbolManager::AddSymbol(const std::string& rName, uint64 address)
  {
    if (mSymbolMap.find(rName) != mSymbolMap.end()) {
      LOG(fail) << "Symbol already exist: " << rName << endl;
      FAIL("adding-duplicated-symbol");
    }

    Symbol* new_symbol = new Symbol(rName);
    new_symbol->SetAddress(address);
    mSymbolMap[rName] = new_symbol;
  }
  
}
