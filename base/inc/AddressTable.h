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
#ifndef Force_AddressTable_H
#define Force_AddressTable_H

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Generator;
  class RecoveryAddressGenerator;
  class Register;
  class Variable;

  /*!
   \class AddressTable
   \brief Class used to generate available addresses.
  */
  class AddressTable : public Object {
  public:
    explicit AddressTable(const EMemBankType bankType); //!< Constructor
    virtual ~AddressTable(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(AddressTable);

    Object* Clone() const override { return new AddressTable(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const std::string ToString() const override; //!< Return a string describing the current state of the Object.
    const char* Type() const override { return "AddressTable"; } //!< Return a string describing the actual type of the Object.

    void Setup(Generator* pGenerator); //!< Setup the AddressTable object.
    void Initialize(cuint32 tableIndex, cuint64 baseAddress); //!< Initialize.
    void InitializeTableIndexRegister() const; //!< Initialize table index register.
    void GenerateAddress(cuint64 align, cuint64 size, cbool isInstr, const EMemAccessType memAccess, bool* pNewTable=nullptr); //!< Generate an address.
    const Register* GetTableIndexRegister() const { return mpTableIndexRegister; } //!< Return table index register.
    EMemBankType GetBankType() const { return mBankType; } //!< Return memory bank.
    uint64 GetBaseAddress() const { return mBaseAddress; } //!< Return base address.
    uint64 GetCurrentAddress() const { return mCurrentAddress; } //!< Return current address.
    void SetRecoveryAddressGenerator(const RecoveryAddressGenerator* pRecoveryAddrGenerator); //!< Set recovery address generator.
    void SetCurrentAddress(cuint64 currentAddr) { mCurrentAddress = currentAddr; } //!< Set current address.
  protected:
    AddressTable(const AddressTable& rOther); //!< Copy constructor
    uint64 GenerateNewTable(cuint64 baseAddress=0); //!< Generate new table region
  private:
    const RecoveryAddressGenerator* mpRecoveryAddrGenerator; //!< Recovery address generator
    const Register* mpTableIndexRegister; //!< Table index register
    const EMemBankType mBankType; //!< Memory bank type
    const Variable* mpTableSize; //!< Table size configuration variable
    Generator* mpGenerator; //!< Generator
    uint64 mBaseAddress; //!< Base address
    uint64 mCurrentAddress; //!< Current address
    uint32 mCurrentTableSize; //!< Current table size
  };

}

#endif //Force_AddressTable_H
