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
#ifndef Force_MemoryReservation_H
#define Force_MemoryReservation_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <string>
#include <vector>

namespace Force {

  class ConstraintSet;
  class VmMapper;

  /*!
    \class MemoryReservation
    \brief The class handles memory reservation information.
  */

  class MemoryReservation {
  public:
    explicit MemoryReservation(const std::string& name); //!< Constructor with name given.
    ~MemoryReservation(); //!< Destructor.
    const std::string& Name() const { return mName; } //!< Return MemoryReservation name.
    const ConstraintSet* GetReservation(EMemBankType memBank) const; //!< Return reservation to a certain memory bank, if any.
    const std::string ToString() const; //!< Return a string representation of the MemoryReservation object.
    void AddConstraint(const std::string& range, EMemBankType memBank); //!< Add constraint in string type.
    void AddVirtualConstraint(const std::string& range, const VmMapper* pVmMapper); //!< Add virtual constraint in string type.
    void AddRange(uint64 start, uint64 size, EMemBankType memBank); //!< Add a reservation range.
    void AddVirtualRange(uint64 start, uint64 size, const VmMapper* pVmMapper); //!< Add a virtual reservation range.
  private:
    MemoryReservation(): mName(), mInitialConstraints(), mCurrentConstraints() { } //!< Default constructor, not meant to be used.
    MemoryReservation(const MemoryReservation& rOther): mName(), mInitialConstraints(), mCurrentConstraints()  { } //!< Copy constructor, not meant to be used.

    inline void CheckBank(EMemBankType memBank) const { //!< Check if a memory bank value is out of bound.
      if (int(memBank) >= mInitialConstraints.size()) {
        BankOutOfBound(memBank);
      }
    }

    void BankOutOfBound(EMemBankType memBank) const; //!< Report a memory bank value out of bound error.
  private:
    std::string mName; //!< Name of memory reservation.
    std::vector<ConstraintSet* > mInitialConstraints; //!< Initial constraints for each bank.
    mutable std::vector<ConstraintSet* > mCurrentConstraints; //!< Current constraints for each bank.
  };

}

#endif
