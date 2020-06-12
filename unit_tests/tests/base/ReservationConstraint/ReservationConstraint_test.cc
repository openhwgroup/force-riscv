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
#include <lest/lest.hpp>
#include <Log.h>
#include <ReservationConstraint.h>
#include <Constraint.h>
#include <memory>

using text = std::string;
using namespace Force;

const lest::test specification[] = {

CASE( "Test ReservationConstraint" ) {

  SETUP( "Setup ReservationConstraint" )  {
    ReservationConstraint reservation_constr;

    SECTION( "Test cloning reservation constraints" ) {
      ConstraintSet reg_indices(1);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);
      std::unique_ptr<ReservationConstraint> reservation_constr_clone(dynamic_cast<ReservationConstraint*>(reservation_constr.Clone()));
      EXPECT(reservation_constr_clone->AreRegistersReserved(reg_indices, ERegAttrType::Read, ERegReserveType::User));
    }

    SECTION( "Test getting reservation constraints type" ) {
      EXPECT(reservation_constr.Type() == "ReservationConstraint");
    }

    SECTION( "Test reserving a register index for read" ) {
      ConstraintSet reg_indices(14);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);
      EXPECT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Read, ERegReserveType::User));
    }

    SECTION( "Test reserving a register index for write" ) {
      ConstraintSet reg_indices(4);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Write, ERegReserveType::Exception);
      EXPECT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Write, ERegReserveType::Exception));
    }

    SECTION( "Test reserving a register index for read and write" ) {
      ConstraintSet reg_indices(32);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Unpredictable);
      EXPECT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Unpredictable));
    }

    SECTION( "Test reserving a register index after it has already been reserved" ) {
      ConstraintSet reg_indices(13);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);
      EXPECT_FAIL(reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User), "register-already-reserved");
    }

    SECTION( "Test reserving a register index with an unsupported access type" ) {
      EXPECT_FAIL(reservation_constr.ReserveRegisters(ConstraintSet(305), ERegAttrType::HasValue, ERegReserveType::Exception), "unsupported-access-type");
    }

    SECTION( "Test unreserving a register index for read" ) {
      ConstraintSet reg_indices(9);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);
      EXPECT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Read, ERegReserveType::User));
      reservation_constr.UnreserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);
      EXPECT_NOT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Read, ERegReserveType::User));
    }

    SECTION( "Test unreserving a register index for write" ) {
      ConstraintSet reg_indices(397);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Write, ERegReserveType::Exception);
      EXPECT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Write, ERegReserveType::Exception));
      reservation_constr.UnreserveRegisters(reg_indices, ERegAttrType::Write, ERegReserveType::Exception);
      EXPECT_NOT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Write, ERegReserveType::Exception));
    }

    SECTION( "Test unreserving a register index for read and write" ) {
      ConstraintSet reg_indices(235);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Unpredictable);
      EXPECT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Unpredictable));
      reservation_constr.UnreserveRegisters(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Unpredictable);
      EXPECT_NOT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Unpredictable));
    }

    SECTION( "Test unreserving with a different access type" ) {
      ConstraintSet reg_indices(47);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);
      EXPECT_FAIL(reservation_constr.UnreserveRegisters(reg_indices, ERegAttrType::Write, ERegReserveType::User), "register-not-reserved");
    }

    SECTION( "Test unreserving after reserving with two reservation types" ) {
      ConstraintSet reg_indices(59);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::Exception);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Unpredictable);
      reservation_constr.UnreserveRegisters(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Unpredictable);
      EXPECT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Read, ERegReserveType::Exception));
      EXPECT_NOT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Unpredictable));
    }

    SECTION( "Test excluding register indices reserved for read" ) {
      reservation_constr.ReserveRegisters(ConstraintSet(22), ERegAttrType::Read, ERegReserveType::Unpredictable);
      ConstraintSet index_constr(20, 25);
      reservation_constr.ExcludeReservedByAccess(ERegAttrType::Read, &index_constr);
      EXPECT(index_constr.ToSimpleString() == "0x14-0x15,0x17-0x19");
    }

    SECTION( "Test excluding register indices reserved for write" ) {
      reservation_constr.ReserveRegisters(ConstraintSet(42), ERegAttrType::Write, ERegReserveType::User);
      ConstraintSet index_constr(42);
      reservation_constr.ExcludeReservedByAccess(ERegAttrType::Write, &index_constr);
      EXPECT(index_constr.IsEmpty());
    }

    SECTION( "Test excluding register indices reserved for read and write" ) {
      reservation_constr.ReserveRegisters(ConstraintSet(997), ERegAttrType::Read, ERegReserveType::Exception);
      reservation_constr.ReserveRegisters(ConstraintSet(843), ERegAttrType::Write, ERegReserveType::Exception);
      ConstraintSet index_constr(800, 1000);
      reservation_constr.ExcludeReservedByAccess(ERegAttrType::ReadWrite, &index_constr);
      EXPECT(index_constr.ToSimpleString() == "0x320-0x34a,0x34c-0x3e4,0x3e6-0x3e8");
    }

    SECTION( "Test excluding register indices with an unsupported access type" ) {
      ConstraintSet index_constr;
      EXPECT_FAIL(reservation_constr.ExcludeReservedByAccess(ERegAttrType::UpdatedFromISS, &index_constr), "unsupported-access-type");
    }

    SECTION( "Test excluding register indices after unreserving a doubly-reserved index" ) {
      ConstraintSet reg_indices(75);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Write, ERegReserveType::User);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Exception);
      reservation_constr.UnreserveRegisters(reg_indices, ERegAttrType::ReadWrite, ERegReserveType::Exception);
      ConstraintSet index_constr(75);
      reservation_constr.ExcludeReservedByAccess(ERegAttrType::Write, &index_constr);
      EXPECT(index_constr.IsEmpty());
    }

    SECTION( "Test checking for reserved read indices" ) {
      reservation_constr.ReserveRegisters(ConstraintSet(37), ERegAttrType::Read, ERegReserveType::Unpredictable);

      const ConstraintSet* read_constr = nullptr;
      const ConstraintSet* write_constr = nullptr;
      EXPECT(reservation_constr.HasReserved(ERegAttrType::Read, read_constr, write_constr));
      EXPECT(read_constr->ToSimpleString() == "0x25");
      EXPECT(write_constr == nullptr);
    }

    SECTION( "Test checking for reserved read indices when there are none" ) {
      const ConstraintSet* read_constr = nullptr;
      const ConstraintSet* write_constr = nullptr;
      EXPECT_NOT(reservation_constr.HasReserved(ERegAttrType::Read, read_constr, write_constr));
      EXPECT(read_constr == nullptr);
      EXPECT(write_constr == nullptr);
    }

    SECTION( "Test checking for reserved write indices" ) {
      reservation_constr.ReserveRegisters(ConstraintSet(253), ERegAttrType::Write, ERegReserveType::User);

      const ConstraintSet* read_constr = nullptr;
      const ConstraintSet* write_constr = nullptr;
      EXPECT(reservation_constr.HasReserved(ERegAttrType::Write, read_constr, write_constr));
      EXPECT(read_constr == nullptr);
      EXPECT(write_constr->ToSimpleString() == "0xfd");
    }

    SECTION( "Test checking for reserved write indices when there are none" ) {
      const ConstraintSet* read_constr = nullptr;
      const ConstraintSet* write_constr = nullptr;
      EXPECT_NOT(reservation_constr.HasReserved(ERegAttrType::Write, read_constr, write_constr));
      EXPECT(read_constr == nullptr);
      EXPECT(write_constr == nullptr);
    }

    SECTION( "Test checking for reserved read and write indices" ) {
      reservation_constr.ReserveRegisters(ConstraintSet(16), ERegAttrType::Read, ERegReserveType::User);
      reservation_constr.ReserveRegisters(ConstraintSet(109), ERegAttrType::Write, ERegReserveType::Exception);

      const ConstraintSet* read_constr = nullptr;
      const ConstraintSet* write_constr = nullptr;
      EXPECT(reservation_constr.HasReserved(ERegAttrType::ReadWrite, read_constr, write_constr));
      EXPECT(read_constr->ToSimpleString() == "0x10");
      EXPECT(write_constr->ToSimpleString() == "0x6d");
    }

    SECTION( "Test checking for reserved indices with an unsupported access type" ) {
      const ConstraintSet* read_constr = nullptr;
      const ConstraintSet* write_constr = nullptr;
      EXPECT_FAIL(reservation_constr.HasReserved(ERegAttrType::Unpredictable, read_constr, write_constr), "unsupported-access-type");
    }

    SECTION( "Test checking for reserved indices after unreserving" ) {
      ConstraintSet reg_indices(201);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::Unpredictable);
      reservation_constr.UnreserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::Unpredictable);

      const ConstraintSet* read_constr = nullptr;
      const ConstraintSet* write_constr = nullptr;
      EXPECT_NOT(reservation_constr.HasReserved(ERegAttrType::ReadWrite, read_constr, write_constr));
      EXPECT(read_constr == nullptr);
      EXPECT(write_constr == nullptr);
    }

    SECTION( "Test checking for reserved indices after unreserving a doubly-reserved index" ) {
      ConstraintSet reg_indices(37);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::Exception);
      reservation_constr.UnreserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);

      const ConstraintSet* read_constr = nullptr;
      const ConstraintSet* write_constr = nullptr;
      EXPECT(reservation_constr.HasReserved(ERegAttrType::ReadWrite, read_constr, write_constr));
      EXPECT(read_constr->ToSimpleString() == "0x25");
      EXPECT(write_constr == nullptr);
    }

    SECTION( "Test checking if a register index is reserved for a different access type" ) {
      ConstraintSet reg_indices(25);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Write, ERegReserveType::Unpredictable);
      EXPECT_NOT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Read, ERegReserveType::Unpredictable));
    }

    SECTION( "Test checking if a register index is reserved for a different reservation type" ) {
      ConstraintSet reg_indices(17);
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);
      EXPECT_NOT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Read, ERegReserveType::Exception));
    }
  }
},

CASE( "Test reserving and unreserving multiple indices simultaneously" ) {

  SETUP( "Setup ReservationConstraint" )  {
    ReservationConstraint reservation_constr;

    SECTION( "Test reserving multiple register indices" ) {
      ConstraintSet reg_indices("14,23,209");
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Write, ERegReserveType::Exception);
      EXPECT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Write, ERegReserveType::Exception));
      EXPECT_NOT(reservation_constr.AreRegistersReserved(ConstraintSet("1,23,209"), ERegAttrType::Write, ERegReserveType::Exception));
    }

    SECTION( "Test unreserving multiple register indices" ) {
      ConstraintSet reg_indices("32,41,98,107");
      reservation_constr.ReserveRegisters(reg_indices, ERegAttrType::Read, ERegReserveType::User);
      EXPECT(reservation_constr.AreRegistersReserved(reg_indices, ERegAttrType::Read, ERegReserveType::User));

      reservation_constr.UnreserveRegisters(ConstraintSet("41,98"), ERegAttrType::Read, ERegReserveType::User);
      EXPECT_NOT(reservation_constr.AreRegistersReserved(ConstraintSet("41,98"), ERegAttrType::Read, ERegReserveType::User));
      EXPECT(reservation_constr.AreRegistersReserved(ConstraintSet("32,107"), ERegAttrType::Read, ERegReserveType::User));
    }
  }
},

};

int main(int argc, char* argv[])
{
  Force::Logger::Initialize();
  int ret = lest::run(specification, argc, argv);
  Force::Logger::Destroy();
  return ret;
}
