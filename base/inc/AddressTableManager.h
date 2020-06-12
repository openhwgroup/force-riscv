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
#ifndef Force_AddressTableManager_H
#define Force_AddressTableManager_H

#include <Object.h>
#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <map>
#include <vector>

namespace Force {

  class AddressTable;
  class Generator;

  /*!
   \class AddressTableManager
   \brief Top level module for managing all address tables.
  */
  class AddressTableManager : public Object {
  public:
    AddressTableManager(); //!< Default constructor
    virtual ~AddressTableManager(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(AddressTableManager);

    const std::string ToString() const override { return Type(); } //!< Return a string describing the current state of the Object.

    virtual void Setup(Generator* pGen); //!< Setup Constructor.
    void Initialize(cuint32 index); //!< Initialize.
    void GenerateAddress(cuint64 align, cuint64 size, cbool isInstr, const EMemAccessType memAccess, bool* pNewTable=nullptr) const; //!< Generate an address.
    void GenerateRecoveryAddress() const; //!< Generate a recovery address.
    virtual void GetReloadRegisters(cuint32 targetMemBank, cuint32 targetEl, std::map<std::string, uint64>& reloadMap) const = 0; //!< Get index registers requiring updates along with their respective target values.
    inline const AddressTable* GetCurrentAddressTable() const { return mpCurrentAddressTable; } //!< Return pointer to the current address table.
    const AddressTable* GetAddressTable(cuint32 memBank) const;  //!< Get address table for the specified memory bank.
    inline void SetFastMode(cbool fastMode) { mFastMode = fastMode; } //!< Set FastMode flag.
    virtual uint64 GenerateNewTable(uint64 baseAddress, uint64 tableSize, EMemBankType bankType) = 0; //!< Generate a new address table.
    uint32 NewTableId() { return msNextTableId ++; } //!< Return a new table ID.
  protected:
    AddressTableManager(const AddressTableManager& rOther); //!< Copy constructor.
    virtual void UpdateCurrentAddressTable() = 0; //!< Called to update address table manager.
    const std::vector<AddressTable*>& GetAddressTables() const { return mAddressTables; } //!< Get address tables.
    const Generator* GetGenerator() const { return mpGenerator; } //!< Get generator.
    void SetCurrentAddressTableByBank(const EMemBankType bankType); //!< Set the current address table according to the specified memory bank.
  protected:
    Generator* mpGenerator; //!< Pointer to generator object.
  private:
    void SyncDifferentMemoryBankAddressTable() const;  //!< In fast mode, the default and secondary address table hold the same value, except the bank type.
  private:
    static uint32 msNextTableId; //!< Next table ID
    AddressTable* mpCurrentAddressTable; //!< Pointer to current address table.
    std::vector<AddressTable*> mAddressTables; //!< Address table pointers.
    bool mFastMode; //!< Whether is fast mode.
  };

}
#endif
