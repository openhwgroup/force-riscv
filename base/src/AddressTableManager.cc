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
#include "AddressTableManager.h"

#include <algorithm>

#include "AddressTable.h"
#include "Architectures.h"
#include "Config.h"
#include "Generator.h"
#include "Log.h"

using namespace std;

namespace Force {

  uint32 AddressTableManager::msNextTableId = 1;

  AddressTableManager::AddressTableManager()
    : Object(), mpGenerator(nullptr), mpCurrentAddressTable(nullptr), mAddressTables(), mFastMode(false)
  {
  }

  AddressTableManager::AddressTableManager(const AddressTableManager& rOther)
    : Object(rOther), mpGenerator(nullptr), mpCurrentAddressTable(nullptr), mAddressTables(), mFastMode(false)
  {
    for (auto const addr_table : rOther.mAddressTables)
    {
      AddressTable* clone_table = nullptr;
      if (nullptr != addr_table)
      {
        clone_table = dynamic_cast<AddressTable* >(addr_table->Clone());
      }
      mAddressTables.push_back(clone_table);
    }
  }

  AddressTableManager::~AddressTableManager()
  {
    for (auto address_table_ptr : mAddressTables)
    {
      delete address_table_ptr;
    }
  }

  void AddressTableManager::Setup(Generator* pGen)
  {
    mpGenerator = pGen;

    const vector<EMemBankType>& mem_banks = mpGenerator->GetArchInfo()->GetMemoryBanks();
    transform(mem_banks.cbegin(), mem_banks.cend(), back_inserter(mAddressTables),
      [](const EMemBankType bankType) { return new AddressTable(bankType); });


    for (auto address_table_ptr : mAddressTables)
    {
      address_table_ptr->Setup(pGen);
    }
    Config* cfg_ptr = Config::Instance();
    bool valid = false;
    string opt_str = cfg_ptr->GetOptionString("handlers_set", valid);
    if (valid)
    {
      mFastMode = opt_str == "Fast";
    }
  }

  void AddressTableManager::Initialize(cuint32 index)
  {
    uint64 shared_base_address = 0; // Means need allocate new physical page.
    const vector<EMemBankType>& mem_banks = mpGenerator->GetArchInfo()->GetMemoryBanks();
    for (auto bank_type : mem_banks)
    {
      mAddressTables[EMemBankTypeBaseType(bank_type)]->Initialize(index, shared_base_address);
      if (mFastMode) shared_base_address = mAddressTables[EMemBankTypeBaseType(bank_type)]->GetBaseAddress();
    }

    UpdateCurrentAddressTable();
    mpCurrentAddressTable->InitializeTableIndexRegister();
  }

  void AddressTableManager::GenerateAddress(cuint64 align, cuint64 size, cbool isInstr, const EMemAccessType memAccess, bool* pNewTable) const
  {
    if (nullptr == mpCurrentAddressTable)
    {
      LOG(fail) << "{AddressTableManager::GenerateAddress} address table not initialized" << endl;
      FAIL("address-table-not-initialized");
    }

    LOG(notice) << mpCurrentAddressTable->ToString() << endl;
    mpCurrentAddressTable->GenerateAddress(align, size, isInstr, memAccess, pNewTable);
    if (mFastMode)
    {
      SyncDifferentMemoryBankAddressTable();
    }
  }

  void AddressTableManager::GenerateRecoveryAddress() const
  {
    GenerateAddress(mpGenerator->InstructionAlignment(), mpGenerator->InstructionSpace(), true, EMemAccessType::Unknown);
  }

  const AddressTable* AddressTableManager::GetAddressTable(cuint32 memBank) const
  {
    return mAddressTables[memBank];
  }

  void AddressTableManager::SetCurrentAddressTableByBank(const EMemBankType bankType)
  {
    mpCurrentAddressTable = mAddressTables[EMemBankTypeBaseType(bankType)];
  }

  void AddressTableManager::SyncDifferentMemoryBankAddressTable() const
  {
    uint64 current_address = mpCurrentAddressTable->GetCurrentAddress();

    for(auto table_ptr : mAddressTables){
      table_ptr->SetCurrentAddress(current_address);	
    }
  }

}
