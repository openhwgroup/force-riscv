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
#ifndef Force_AddressTableManagerRISCV_H
#define Force_AddressTableManagerRISCV_H

#include "AddressTableManager.h"
#include "Notify.h"
#include "NotifyDefines.h"

namespace Force {

  /*!
   \class AddressTableManagerRISCV
   \brief RISC-V layer module for managing all address tables.
  */
  class AddressTableManagerRISCV : public AddressTableManager, public NotificationReceiver {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(AddressTableManagerRISCV); //!< Default constructor
    SUBCLASS_DESTRUCTOR_DEFAULT(AddressTableManagerRISCV); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(AddressTableManagerRISCV);

    Object* Clone() const override { return new AddressTableManagerRISCV(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "AddressTableManagerRISCV"; } //!< Return a string describing the current state of the Object.

    void Setup(Generator* pGen) override; //!< Setup Constructor.
    void GetReloadRegisters(cuint32 targetMemBank, cuint32 targetEl, std::map<std::string, uint64>& reloadMap) const override; //!< Get index registers requiring updates along with their respective target values.
    uint64 GenerateNewTable(uint64 baseAddress, uint64 tableSize, EMemBankType bankType) override; //!< Generate a new address table in RISC-V Layer.
  protected:
    AddressTableManagerRISCV(const AddressTableManagerRISCV& rOther); //!< Copy constructor.
    void HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload) override; //!< called when privilege level changes 
    void UpdateCurrentAddressTable() override; //!< Called to update address table manager.
  };

}
#endif
