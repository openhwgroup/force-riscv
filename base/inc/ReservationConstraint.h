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
#ifndef Force_ReservationConstraint_H
#define Force_ReservationConstraint_H

#include <Object.h>
#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>

namespace Force {

  class ConstraintSet;
  class TypeAccessRecord;

  /*!
    \class ReservationConstraint
    \brief Class to store indices of reserved registers.
  */
  class ReservationConstraint : public Object {
  public:
    ReservationConstraint(); //!< Default constructor
    ReservationConstraint(const ReservationConstraint& rOther); //!< Copy constructor
    ~ReservationConstraint() override; //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(ReservationConstraint);

    Object* Clone() const override { return new ReservationConstraint(*this); }; //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const std::string ToString() const override; //!< Return a string describing the current state of the Object.
    const char* Type() const override { return "ReservationConstraint"; } //!< Return a string describing the actual type of the Object.

    void ReserveRegisters(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType);  //!< Reserve register indices for the specified access and reservation type. Access can only be Read, Write or ReadWrite.
    void UnreserveRegisters(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType); //!< Unreserve register indices for the specified access and reservation type. Access can only be Read, Write or ReadWrite.
    bool AreRegistersReserved(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType) const; //!< Return whether a register index is reserved for the specified access and reservation type.
    void ExcludeReservedByAccess(const ERegAttrType access, ConstraintSet* pIndexConstr) const; //!< Remove reserved indices for the specified access from the specified constraint set.
    bool HasReserved(const ERegAttrType access, const ConstraintSet*& prReadConstr, const ConstraintSet*& prWriteConstr) const; //!< Return whether there are reserved register indices for the specified access type.
  private:
    void ReserveTypeAccess(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType); //!< Add the specified access and reservation type to reservation records of the specified register indices.
    void UnreserveTypeAccess(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType); //!< Remove the specified access and reservation type from the the reservation records of the specified register indices.
    bool IsAccessTypeReserved(const ConstraintSet& rRegIndices, const ERegAttrType access) const; //!< Return whether the regsiter indices are reserved for the specified access for any reservation type.
  private:
    ConstraintSet* mpReadReserved; //!< Cache of all register indices reserved for read access
    ConstraintSet* mpWriteReserved; //!< Cache of all register indices reserved for write access
    std::vector<ConstraintSet*> mReadReservedByType; //!< Register indices reserved for read, grouped by reservation type.
    std::vector<ConstraintSet*> mWriteReservedByType; //!< Register indices reserved for write, grouped by reservation type.
  };

}

#endif  // Force_ReservationConstraint_H
