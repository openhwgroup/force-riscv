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
#include "ReservationConstraint.h"

#include <sstream>

#include "Constraint.h"
#include "Log.h"

using namespace std;

/*!
  \file ReservationConstraint.cc
  \brief Code supporting storing indices of reserved registers.
*/

namespace Force {

  ReservationConstraint::ReservationConstraint()
    : mpReadReserved(nullptr), mpWriteReserved(nullptr), mReadReservedByType(), mWriteReservedByType()
  {
    mpReadReserved = new ConstraintSet();
    mpWriteReserved = new ConstraintSet();

    for (ERegReserveTypeBaseType i = 0; i < ERegReserveTypeSize; i++) {
      mReadReservedByType.push_back(new ConstraintSet());
    }

    for (ERegReserveTypeBaseType i = 0; i < ERegReserveTypeSize; i++) {
      mWriteReservedByType.push_back(new ConstraintSet());
    }
  }

  ReservationConstraint::ReservationConstraint(const ReservationConstraint& rOther)
    : mpReadReserved(nullptr), mpWriteReserved(nullptr), mReadReservedByType(), mWriteReservedByType()
  {
    mpReadReserved = rOther.mpReadReserved->Clone();
    mpWriteReserved = rOther.mpWriteReserved->Clone();

    for (ERegReserveTypeBaseType i = 0; i < ERegReserveTypeSize; i++) {
      ConstraintSet* read_reserved = rOther.mReadReservedByType[i];
      mReadReservedByType.push_back(read_reserved->Clone());
    }

    for (ERegReserveTypeBaseType i = 0; i < ERegReserveTypeSize; i++) {
      ConstraintSet* write_reserved = rOther.mWriteReservedByType[i];
      mWriteReservedByType.push_back(write_reserved->Clone());
    }
  }

  ReservationConstraint::~ReservationConstraint()
  {
    delete mpReadReserved;
    delete mpWriteReserved;

    for (ConstraintSet* read_constr : mReadReservedByType) {
      delete read_constr;
    }

    for (ConstraintSet* write_constr : mWriteReservedByType) {
      delete write_constr;
    }
  }

  const std::string ReservationConstraint::ToString() const
  {
    stringstream output;

    output << "All reserved for read: " << mpReadReserved->ToSimpleString() << endl;
    output << "All reserved for write: " << mpWriteReserved->ToSimpleString() << endl;

    for (ERegReserveTypeBaseType i = 0; i < ERegReserveTypeSize; i++) {
      ConstraintSet* read_constr = mReadReservedByType[i];
      output << "Reserved for read with reservation type " << ERegReserveType_to_string(ERegReserveType(i)) << ": "  << read_constr->ToSimpleString() << endl;
    }

    for (ERegReserveTypeBaseType i = 0; i < ERegReserveTypeSize; i++) {
      ConstraintSet* write_constr = mWriteReservedByType[i];
      output << "Reserved for write with reservation type " << ERegReserveType_to_string(ERegReserveType(i)) << ": "  << write_constr->ToSimpleString() << endl;
    }

    return output.str();
  }

  void ReservationConstraint::ReserveRegisters(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType)
  {
    if (AreRegistersReserved(rRegIndices, access, reserveType)) {
      LOG(fail) << "{ReservationConstraint::ReserveRegisters} Attempting to reserve registers with indices " << rRegIndices.ToSimpleString() << ", which are already reserved." << endl;
      FAIL("register-already-reserved");
    }

    ReserveTypeAccess(rRegIndices, access, reserveType);

    switch (access) {
      case ERegAttrType::Read:
        mpReadReserved->MergeConstraintSet(rRegIndices);
        break;
      case ERegAttrType::Write:
        mpWriteReserved->MergeConstraintSet(rRegIndices);
        break;
      case ERegAttrType::ReadWrite:
        mpReadReserved->MergeConstraintSet(rRegIndices);
        mpWriteReserved->MergeConstraintSet(rRegIndices);
        break;
    default:
        LOG(fail) << "{ReservationConstraint::ReserveRegisters} Reserving register with access: " << ERegAttrType_to_string(access) << " is not allowed" << endl;
        FAIL("unsupported-access-type");
    }
  }

  void ReservationConstraint::UnreserveRegisters(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType)
  {
    if (not AreRegistersReserved(rRegIndices, access, reserveType)) {
      LOG(fail) << "{ReservationConstraint::UnreserveRegisters} Attempting to unreserve registers with indices " << rRegIndices.ToSimpleString() << ", which are not currently reserved." << endl;
      FAIL("register-not-reserved");
    }

    UnreserveTypeAccess(rRegIndices, access, reserveType);

    switch (access) {
      case ERegAttrType::Read:
        if (not IsAccessTypeReserved(rRegIndices, ERegAttrType::Read)) {
          mpReadReserved->SubConstraintSet(rRegIndices);
        }

        break;
      case ERegAttrType::Write:
        if (not IsAccessTypeReserved(rRegIndices, ERegAttrType::Write)) {
          mpWriteReserved->SubConstraintSet(rRegIndices);
        }

        break;
      case ERegAttrType::ReadWrite:
        if (not IsAccessTypeReserved(rRegIndices, ERegAttrType::Read)) {
          mpReadReserved->SubConstraintSet(rRegIndices);
        }

        if (not IsAccessTypeReserved(rRegIndices, ERegAttrType::Write)) {
          mpWriteReserved->SubConstraintSet(rRegIndices);
        }

        break;
      default:
        LOG(fail) << "{ReservationConstraint::UnreserveRegisters} Unreserving register with access: " << ERegAttrType_to_string(access) << " is not allowed" << endl;
        FAIL("unsupported-access-type");
    }
  }

  bool ReservationConstraint::AreRegistersReserved(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType) const
  {
    bool reserved = false;

    ConstraintSet* read_constr = nullptr;
    ConstraintSet* write_constr = nullptr;
    bool read_reserved = false;
    bool write_reserved = false;

    switch (access) {
    case ERegAttrType::Read:
      read_constr = mReadReservedByType[ERegReserveTypeBaseType(reserveType)];
      reserved = read_constr->ContainsConstraintSet(rRegIndices);
      break;
    case ERegAttrType::Write:
      write_constr = mWriteReservedByType[ERegReserveTypeBaseType(reserveType)];
      reserved = write_constr->ContainsConstraintSet(rRegIndices);
      break;
    case ERegAttrType::ReadWrite:
      read_constr = mReadReservedByType[ERegReserveTypeBaseType(reserveType)];
      read_reserved = read_constr->ContainsConstraintSet(rRegIndices);
      write_constr = mWriteReservedByType[ERegReserveTypeBaseType(reserveType)];
      write_reserved = write_constr->ContainsConstraintSet(rRegIndices);
      reserved = read_reserved and write_reserved;
      break;
    default:
      LOG(fail) << "{ReservationConstraint::AreRegistersReserved} Access: " << ERegAttrType_to_string(access) << " is not allowed" << endl;
      FAIL("unsupported-access-type");
    }

    return reserved;
  }

  void ReservationConstraint::ExcludeReservedByAccess(const ERegAttrType access, ConstraintSet* pIndexConstr) const
  {
    switch (access) {
    case ERegAttrType::Read:
      pIndexConstr->SubConstraintSet(*mpReadReserved);
      break;
    case ERegAttrType::Write:
      pIndexConstr->SubConstraintSet(*mpWriteReserved);
      break;
    case ERegAttrType::ReadWrite:
      pIndexConstr->SubConstraintSet(*mpReadReserved);
      pIndexConstr->SubConstraintSet(*mpWriteReserved);
      break;
    default:
      LOG(fail) << "{ReservationConstraint::ExcludeReserved} Access: " << ERegAttrType_to_string(access) << " is not allowed" << endl;
      FAIL("unsupported-access-type");
    }
  }

  bool ReservationConstraint::HasReserved(const ERegAttrType access, const ConstraintSet*& prReadConstr, const ConstraintSet*& prWriteConstr) const
  {
    bool has_reserved = false;

    switch (access) {
    case ERegAttrType::Read:
      if (not mpReadReserved->IsEmpty()) {
        prReadConstr = mpReadReserved;
        has_reserved = true;
      }

      break;
    case ERegAttrType::Write:
      if (not mpWriteReserved->IsEmpty()) {
        prWriteConstr = mpWriteReserved;
        has_reserved = true;
      }

      break;
    case ERegAttrType::ReadWrite:
      if (not mpReadReserved->IsEmpty()) {
        prReadConstr = mpReadReserved;
        has_reserved = true;
      }

      if (not mpWriteReserved->IsEmpty()) {
        prWriteConstr = mpWriteReserved;
        has_reserved = true;
      }

      break;
    default:
      LOG(fail) << "{ReservationConstraint::HasReservations} access: " << ERegAttrType_to_string(access) << " is not allowed" << endl;
      FAIL("unsupported-access-type");
    }

    return has_reserved;
  }

  void ReservationConstraint::ReserveTypeAccess(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType)
  {
    ConstraintSet* read_constr = nullptr;
    ConstraintSet* write_constr = nullptr;

    switch (access) {
    case ERegAttrType::Read:
      read_constr = mReadReservedByType[ERegReserveTypeBaseType(reserveType)];
      read_constr->MergeConstraintSet(rRegIndices);
      break;
    case ERegAttrType::Write:
      write_constr = mWriteReservedByType[ERegReserveTypeBaseType(reserveType)];
      write_constr->MergeConstraintSet(rRegIndices);
      break;
    case ERegAttrType::ReadWrite:
      read_constr = mReadReservedByType[ERegReserveTypeBaseType(reserveType)];
      read_constr->MergeConstraintSet(rRegIndices);
      write_constr = mWriteReservedByType[ERegReserveTypeBaseType(reserveType)];
      write_constr->MergeConstraintSet(rRegIndices);
      break;
    default:
      LOG(fail) << "{ReservationConstraint::ReserveTypeAccess} Access: " << ERegAttrType_to_string(access) << " is not allowed" << endl;
      FAIL("unsupported-access-type");
    }
  }

  void ReservationConstraint::UnreserveTypeAccess(const ConstraintSet& rRegIndices, const ERegAttrType access, const ERegReserveType reserveType)
  {
    ConstraintSet* read_constr = nullptr;
    ConstraintSet* write_constr = nullptr;

    switch (access) {
    case ERegAttrType::Read:
      read_constr = mReadReservedByType[ERegReserveTypeBaseType(reserveType)];
      read_constr->SubConstraintSet(rRegIndices);
      break;
    case ERegAttrType::Write:
      write_constr = mWriteReservedByType[ERegReserveTypeBaseType(reserveType)];
      write_constr->SubConstraintSet(rRegIndices);
      break;
    case ERegAttrType::ReadWrite:
      read_constr = mReadReservedByType[ERegReserveTypeBaseType(reserveType)];
      read_constr->SubConstraintSet(rRegIndices);
      write_constr = mWriteReservedByType[ERegReserveTypeBaseType(reserveType)];
      write_constr->SubConstraintSet(rRegIndices);
      break;
    default:
      LOG(fail) << "{ReservationConstraint::UnreserveTypeAccess} Access: " << ERegAttrType_to_string(access) << " is not allowed" << endl;
      FAIL("unsupported-access-type");
    }
  }

  bool ReservationConstraint::IsAccessTypeReserved(const ConstraintSet& rRegIndices, const ERegAttrType access) const
  {
    bool reserved = false;

    for (ERegReserveTypeBaseType i = 0; i < ERegReserveTypeSize; i++) {
      if (AreRegistersReserved(rRegIndices, access, ERegReserveType(i))) {
        reserved = true;
        break;
      }
    }

    return reserved;
  }

}
