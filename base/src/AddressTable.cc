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
#include <AddressTable.h>
#include <Generator.h>
#include <RecoveryAddressGenerator.h>
#include <Register.h>
#include <Variable.h>
#include <AddressTableManager.h>
#include <Log.h>

#include <sstream>

using namespace std;

/*!
  \file AddressTable.cc
  \brief Code to implement address table for recovery and fast preamble addresses.
*/

namespace Force
{

  AddressTable::AddressTable(const EMemBankType bankType)
    : Object(), mpRecoveryAddrGenerator(nullptr), mpTableIndexRegister(nullptr), mBankType(bankType), mpTableSize(nullptr), mpGenerator(nullptr), mBaseAddress(0), mCurrentAddress(0), mCurrentTableSize(0)
  {
  }

  AddressTable::AddressTable(const AddressTable& rOther)
    : Object(rOther), mpRecoveryAddrGenerator(nullptr), mpTableIndexRegister(nullptr), mBankType(rOther.mBankType), mpTableSize(nullptr), mpGenerator(nullptr), mBaseAddress(0), mCurrentAddress(0), mCurrentTableSize(0)
  {
  }

  AddressTable::~AddressTable()
  {
    delete mpRecoveryAddrGenerator;
  }

  const std::string AddressTable::ToString() const
  {
    stringstream out_str;

    out_str << Type() << hex << " MemoryBank=" << EMemBankType_to_string(mBankType)
        << ",BaseAddress=0x"    << mBaseAddress
        << ",TableSize=0x"      << mCurrentTableSize
        << ",RegisterIndex=0x"  << mpTableIndexRegister->IndexValue()
        << ",RegisterValue=0x"  << mpTableIndexRegister->Value()
        << ",CurrentAddress=0x" << mCurrentAddress;

    return out_str.str();
  }

  void AddressTable::Setup(Generator* pGenerator)
  {
    mpGenerator = pGenerator;
    const VariableModerator* var_mod = mpGenerator->GetVariableModerator(EVariableType::Value);
    mpTableSize = var_mod->GetVariableSet()->FindVariable("Address table memory size");
  }

  void AddressTable::Initialize(cuint32 tableIndex, cuint64 baseAddress)
  {
    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    mpTableIndexRegister = reg_file->RegisterLookupByIndex(tableIndex, ERegisterType::GPR, 64);
    mBaseAddress = GenerateNewTable(baseAddress);
    mCurrentAddress = mBaseAddress - 8;
  }

  void AddressTable::InitializeTableIndexRegister() const
  {
    if (mpTableIndexRegister->IsInitialized())
    {
      LOG(fail) << "{AddressTable::InitializeTableIndexRegister} recovery register already initialized" << endl;
      FAIL("reintialize-recovery-register");
    }

    mpGenerator->InitializeRegister(mpTableIndexRegister->Name(), "", mBaseAddress);
  }

  void AddressTable::GenerateAddress(cuint64 align, cuint64 size, cbool isInstr, const EMemAccessType memAccess, bool* pNewTable)
  {
    uint64 table_entry_addr = mpTableIndexRegister->Value();

    // If second to last entry in table, create a new table; then, write 1 followed by the address of the new table into
    // the existing table
    if (table_entry_addr == (mBaseAddress + mCurrentTableSize - 16))
    {
      uint64 new_table_addr = GenerateNewTable();
      mpGenerator->InitializeMemory(table_entry_addr, EMemBankTypeBaseType(mBankType), 8, 1, false, true);
      mpGenerator->InitializeMemory(table_entry_addr + 8, EMemBankTypeBaseType(mBankType), 8, new_table_addr, false, true);

      mBaseAddress = new_table_addr;
      mCurrentAddress = new_table_addr - 8;
      table_entry_addr = new_table_addr;

      if (pNewTable != nullptr) {
        (*pNewTable) = true;
      }
    }
    else if (pNewTable != nullptr) {
      (*pNewTable) = false;
    }

    // Generate a random VA and insert it into the table
    if (table_entry_addr == mCurrentAddress + 8)
    {
      uint64 table_entry_val = mpRecoveryAddrGenerator->GenerateAddress(align, size, isInstr, memAccess);
      mpGenerator->InitializeMemory(table_entry_addr, EMemBankTypeBaseType(mBankType), 8, table_entry_val, false, true);
      mCurrentAddress = table_entry_addr;

      LOG(notice) << "{AddressTable::GenerateAddress} generated addr=0x" << hex << table_entry_val <<
        " at addr=0x" << table_entry_addr << " memory bank=" << EMemBankType_to_string(mBankType) << endl;
    }
  }

  void AddressTable::SetRecoveryAddressGenerator(const RecoveryAddressGenerator* pRecoveryAddrGenerator)
  {
    if (mpRecoveryAddrGenerator != nullptr) {
      LOG(fail) << "{AddressTable::SetRecoveryAddressGenerator} recovery address generator already set" << endl;
      FAIL("reset-recovery-addr-generator");
    }

    mpRecoveryAddrGenerator = pRecoveryAddrGenerator;
  }

  uint64 AddressTable::GenerateNewTable(cuint64 baseAddress)
  {
    mCurrentTableSize = mpTableSize->Value();
    return mpGenerator->GetAddressTableManager()->GenerateNewTable(baseAddress, mCurrentTableSize, mBankType);
  }

}
