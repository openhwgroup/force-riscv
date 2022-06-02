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
#include "MemoryConstraint.h"

#include "lest/lest.hpp"

#include "AddressReuseMode.h"
#include "Constraint.h"
#include "Enums.h"
#include "Log.h"
#include "Random.h"

using text = std::string;
using namespace Force;

void enable_all_reuse_types(AddressReuseMode& rAddrReuseMode)
{
  for (EAddressReuseTypeBaseType i = 0; i < EAddressReuseTypeSize; i++) {
    rAddrReuseMode.EnableReuseType(EAddressReuseType(1 << i));
  }
}

const lest::test specification[] = {

CASE( "Test initialization" ) {

  SETUP( "Setup MemoryConstraint" )  {
    cuint32 thread_id = 0;
    std::unique_ptr<MemoryConstraint> mem_constr(new SingleThreadMemoryConstraint(thread_id));
    ConstraintSet constr_set(0x0, 0xffffffffffff);
    mem_constr->Initialize(constr_set);
    AddressReuseMode addr_reuse_mode;
    addr_reuse_mode.EnableReuseType(EAddressReuseType::ReadAfterRead);

    SECTION( "Test cloning" ) {
      mem_constr->MarkUsed(0x354, 0x378);
      mem_constr->MarkUsedForType(0xff32, 0x20378, EMemDataType::Data, EMemAccessType::Read, thread_id);
      std::unique_ptr<MemoryConstraint> mem_constr_clone(dynamic_cast<MemoryConstraint*>(mem_constr->Clone()));

      EXPECT(mem_constr_clone->IsInitialized());
      const ConstraintSet* usable = mem_constr_clone->Usable();
      EXPECT(usable->ToSimpleString() == "0x0-0x353,0x379-0xff31,0x20379-0xffffffffffff");

      ConstraintSet read_constr_set(0x1058a, 0x1058f);
      mem_constr_clone->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.ToSimpleString() == "0x1058a-0x1058f");
      ConstraintSet write_constr_set(0x1058a, 0x1058f);
      mem_constr_clone->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_constr_set);
      EXPECT(write_constr_set.IsEmpty());
    }

    SECTION( "Test initialization" ) {
      ConstraintSet init_constr_set(0x2500, 0x3500);
      mem_constr->Initialize(init_constr_set);
      EXPECT(mem_constr->IsInitialized());
      const ConstraintSet* usable = mem_constr->Usable();
      EXPECT(usable->ToSimpleString() == "0x2500-0x3500");
    }

    SECTION( "Test uninitialization" ) {
      mem_constr->Uninitialize();
      EXPECT_NOT(mem_constr->IsInitialized());
      const ConstraintSet* usable = mem_constr->Usable();
      EXPECT(usable->IsEmpty());
    }
  }
},

CASE( "Test marking addresses as used" ) {

  SETUP( "Setup MemoryConstraint" )  {
    cuint32 thread_id = 0;
    std::unique_ptr<MemoryConstraint> mem_constr(new SingleThreadMemoryConstraint(thread_id));
    ConstraintSet constr_set(0x0, 0xffffffffffff);
    mem_constr->Initialize(constr_set);
    AddressReuseMode addr_reuse_mode;
    enable_all_reuse_types(addr_reuse_mode);

    SECTION( "Test marking addresses as used" ) {
      mem_constr->MarkUsed(0x12, 0x7f);
      const ConstraintSet* usable = mem_constr->Usable();
      EXPECT(usable->ToSimpleString() == "0x0-0x11,0x80-0xffffffffffff");

      ConstraintSet used_constr_set(0x5e, 0x98);
      mem_constr->MarkUsed(used_constr_set);
      usable = mem_constr->Usable();
      EXPECT(usable->ToSimpleString() == "0x0-0x11,0x99-0xffffffffffff");
    }

    SECTION( "Test marking data read addresses as used" ) {
      mem_constr->MarkUsedForType(0x5892, 0x5956, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet read_constr_set(0x5890, 0x5899);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.ToSimpleString() == "0x5890-0x5899");
    }

    SECTION( "Test marking data write addresses as used" ) {
      mem_constr->MarkUsedForType(0x5892, 0x5956, EMemDataType::Data, EMemAccessType::Write, thread_id);
      ConstraintSet write_constr_set(0x5890, 0x5899);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_constr_set);
      EXPECT(write_constr_set.ToSimpleString() == "0x5890-0x5899");
    }

    SECTION( "Test marking unknown data addresses as used" ) {
      mem_constr->MarkUsedForType(0x5892, 0x5956, EMemDataType::Data, EMemAccessType::Unknown, thread_id);
      ConstraintSet unknown_constr_set(0x5890, 0x5899);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Unknown, thread_id, addr_reuse_mode, &unknown_constr_set);
      EXPECT(unknown_constr_set.ToSimpleString() == "0x5890-0x5891");
    }

    SECTION( "Test marking non-data addresses as used" ) {
      mem_constr->MarkUsedForType(0x5892, 0x5956, EMemDataType::Instruction, EMemAccessType::Read, thread_id);
      ConstraintSet instr_constr_set(0x5890, 0x5899);
      mem_constr->ApplyToConstraintSet(EMemDataType::Instruction, EMemAccessType::Read, thread_id, addr_reuse_mode, &instr_constr_set);
      EXPECT(instr_constr_set.ToSimpleString() == "0x5890-0x5891");
    }

    SECTION( "Test unmarking addresses as used" ) {
      mem_constr->MarkUsed(0x12, 0x7f);
      ConstraintSet unused_constr_set(0x35, 0x42);
      mem_constr->UnmarkUsed(unused_constr_set);
      const ConstraintSet* usable = mem_constr->Usable();
      EXPECT(usable->ToSimpleString() == "0x0-0x11,0x35-0x42,0x80-0xffffffffffff");
    }

    SECTION( "Test marking addresses as used for a non-matching thread" ) {
      mem_constr->MarkUsedForType(0x772, 0x792, EMemDataType::Data, EMemAccessType::Write, 5);
      ConstraintSet write_constr_set(0x780, 0x790);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_constr_set);
      EXPECT(write_constr_set.IsEmpty());
    }

    SECTION( "Test applying to constraint set for a non-matching thread" ) {
      mem_constr->MarkUsed(0x99a25, 0x99b25);
      ConstraintSet read_constr_set(0x99a23, 0x99f57);
      EXPECT_FAIL(mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, 4, addr_reuse_mode, &read_constr_set), "mem-constraint-not-found");
    }
  }
},

CASE( "Test applying to constraint set" ) {

  SETUP( "Setup MemoryConstraint" )  {
    cuint32 thread_id = 0;
    std::unique_ptr<MemoryConstraint> mem_constr(new SingleThreadMemoryConstraint(thread_id));
    ConstraintSet constr_set(0x0, 0xffffffffffff);
    mem_constr->Initialize(constr_set);
    AddressReuseMode addr_reuse_mode;
    enable_all_reuse_types(addr_reuse_mode);

    SECTION( "Test applying to constraint set that is wholly usable" ) {
      ConstraintSet usable_constr_set(0x125f, 0x1267);
      mem_constr->ApplyToConstraintSet(EMemDataType::Instruction, EMemAccessType::Read, thread_id, addr_reuse_mode, &usable_constr_set);
      EXPECT(usable_constr_set.ToSimpleString() == "0x125f-0x1267");
    }

    SECTION( "Test applying to constraint set that is wholly reusable" ) {
      mem_constr->MarkUsedForType(0x1000, 0x1300, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet reusable_constr_set(0x125f, 0x1267);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &reusable_constr_set);
      EXPECT(reusable_constr_set.ToSimpleString() == "0x125f-0x1267");
    }

    SECTION( "Test applying to constraint set for non-data memory" ) {
      mem_constr->MarkUsedForType(0x632, 0x640, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet instr_constr_set(0x638, 0x642);
      mem_constr->ApplyToConstraintSet(EMemDataType::Instruction, EMemAccessType::Read, thread_id, addr_reuse_mode, &instr_constr_set);
      EXPECT(instr_constr_set.ToSimpleString() == "0x641-0x642");
    }

    SECTION( "Test applying to constraint set for unknown access type" ) {
      mem_constr->MarkUsedForType(0x7ffed, 0x7ffff, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet unknown_constr_set(0x7ffed, 0x7ffff);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Unknown, thread_id, addr_reuse_mode, &unknown_constr_set);
      EXPECT(unknown_constr_set.IsEmpty());
    }

    SECTION( "Test applying to non-data constraint set with no usable addresses" ) {
      mem_constr->MarkUsed(0x0, 0xffffffffffff);
      ConstraintSet instr_constr_set(0x7ffed, 0x7ffff);
      mem_constr->ApplyToConstraintSet(EMemDataType::Instruction, EMemAccessType::Read, thread_id, addr_reuse_mode, &instr_constr_set);
      EXPECT(instr_constr_set.IsEmpty());
    }

    SECTION( "Test applying to data constraint set with only reusable addresses" ) {
      mem_constr->MarkUsedForType(0x0, 0xffffffffffff, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet read_constr_set(0x7ffed, 0x7ffff);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.ToSimpleString() == "0x7ffed-0x7ffff");
    }

    SECTION( "Test applying to constraint set before marking any data read memory as used" ) {
      mem_constr->MarkUsed(0x1260, 0x1265);
      ConstraintSet read_constr_set(0x125f, 0x1267);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.ToSimpleString() == "0x125f,0x1266-0x1267");
    }

    SECTION( "Test applying to constraint set after marking data read memory as used" ) {
      mem_constr->MarkUsedForType(0x1260, 0x1265, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet read_constr_set(0x125f, 0x1267);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.ToSimpleString() == "0x125f-0x1267");
    }

    SECTION( "Test applying to constraint set for data write operation" ) {
      mem_constr->MarkUsed(0xff357, 0x10639a);
      ConstraintSet write_constr_set(0x106000, 0x107000);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_constr_set);
      EXPECT(write_constr_set.ToSimpleString() == "0x10639b-0x107000");
    }

    SECTION( "Test applying to constraint set with no usable addresses" ) {
      mem_constr->MarkUsed(0x0, 0xffffffffffff);
      ConstraintSet read_constr_set(0x100, 0xb00000000);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.IsEmpty());
    }
  }
},

CASE( "Test MemoryConstraint with various address reuse mode settings" ) {

  SETUP( "Setup MemoryConstraint" )  {
    cuint32 thread_id = 0;
    std::unique_ptr<MemoryConstraint> mem_constr(new SingleThreadMemoryConstraint(thread_id));
    ConstraintSet constr_set(0x0, 0xffffffffffff);
    mem_constr->Initialize(constr_set);
    AddressReuseMode addr_reuse_mode;

    SECTION( "Test marking an address for read access before marking it for write access" ) {
      addr_reuse_mode.EnableReuseType(EAddressReuseType::ReadAfterRead);
      addr_reuse_mode.EnableReuseType(EAddressReuseType::WriteAfterWrite);

      mem_constr->MarkUsedForType(0x7ff, 0x852, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet read_usable_constr_set(0x770, 0x830);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_usable_constr_set);
      EXPECT(read_usable_constr_set.ToSimpleString() == "0x770-0x830");

      mem_constr->MarkUsedForType(0x810, 0x830, EMemDataType::Data, EMemAccessType::Write, thread_id);
      ConstraintSet write_usable_constr_set(0x79b, 0x825);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_usable_constr_set);
      EXPECT(write_usable_constr_set.ToSimpleString() == "0x79b-0x7fe,0x810-0x825");

      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_usable_constr_set);
      EXPECT(read_usable_constr_set.ToSimpleString() == "0x770-0x80f");
    }

    SECTION( "Test applying to constraint set with read after write reuse enabled" ) {
      addr_reuse_mode.EnableReuseType(EAddressReuseType::ReadAfterWrite);

      mem_constr->MarkUsedForType(0x5970, 0x5b70, EMemDataType::Data, EMemAccessType::Write, thread_id);
      ConstraintSet read_usable_constr_set(0x5800, 0x5a00);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_usable_constr_set);
      EXPECT(read_usable_constr_set.ToSimpleString() == "0x5800-0x5a00");
    }

    SECTION( "Test applying to constraint set with write after read reuse enabled" ) {
      addr_reuse_mode.EnableReuseType(EAddressReuseType::WriteAfterRead);

      mem_constr->MarkUsedForType(0x7ff, 0x850, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet write_usable_constr_set(0x800, 0x900);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_usable_constr_set);
      EXPECT(write_usable_constr_set.ToSimpleString() == "0x800-0x900");
    }
  }
},

CASE( "Test applying to constraint set with reuse disabled" ) {

  SETUP( "Setup MemoryConstraint" )  {
    cuint32 thread_id = 0;
    std::unique_ptr<MemoryConstraint> mem_constr(new SingleThreadMemoryConstraint(thread_id));
    ConstraintSet constr_set(0x0, 0xffffffffffff);
    mem_constr->Initialize(constr_set);
    AddressReuseMode addr_reuse_mode;

    SECTION( "Test applying to constraint set with only read reuse enabled" ) {
      addr_reuse_mode.EnableReuseType(EAddressReuseType::ReadAfterRead);
      mem_constr->MarkUsedForType(0x1000, 0x1300, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet read_constr_set(0x125f, 0x1267);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.ToSimpleString() == "0x125f-0x1267");

      mem_constr->MarkUsedForType(0x1350, 0x1400, EMemDataType::Data, EMemAccessType::Write, thread_id);
      ConstraintSet write_constr_set(0x136a, 0x13fb);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_constr_set);
      EXPECT(write_constr_set.IsEmpty());
    }

    SECTION( "Test applying to constraint set with only write reuse enabled" ) {
      addr_reuse_mode.EnableReuseType(EAddressReuseType::WriteAfterWrite);
      mem_constr->MarkUsedForType(0x2540, 0x2548, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet read_constr_set(0x2544, 0x2548);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.IsEmpty());

      mem_constr->MarkUsedForType(0x367f, 0x3688, EMemDataType::Data, EMemAccessType::Write, thread_id);
      ConstraintSet write_constr_set(0x3680, 0x3684);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_constr_set);
      EXPECT(write_constr_set.ToSimpleString() == "0x3680-0x3684");
    }

    SECTION( "Test applying to constraint set with no reuse enabled" ) {
      mem_constr->MarkUsedForType(0xf3, 0x108, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet read_constr_set(0xf7, 0x10f);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.ToSimpleString() == "0x109-0x10f");

      mem_constr->MarkUsedForType(0x7b, 0xa5, EMemDataType::Data, EMemAccessType::Write, thread_id);
      ConstraintSet write_constr_set(0x65, 0x92);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_constr_set);
      EXPECT(write_constr_set.ToSimpleString() == "0x65-0x7a");
    }

    SECTION( "Test applying to constraint set with reuse change from disabled to enabled" ) {
      mem_constr->MarkUsedForType(0x1000, 0x1300, EMemDataType::Data, EMemAccessType::Read, thread_id);
      ConstraintSet no_reuse_constr_set(0x125f, 0x1267);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &no_reuse_constr_set);
      EXPECT(no_reuse_constr_set.IsEmpty());

      addr_reuse_mode.EnableReuseType(EAddressReuseType::ReadAfterRead);
      ConstraintSet reuse_constr_set(0x125f, 0x1267);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &reuse_constr_set);
      EXPECT(reuse_constr_set.ToSimpleString() == "0x125f-0x1267");
    }
  }
},

CASE( "Test marking addresses as shared" ) {

  SETUP( "Setup MemoryConstraint" )  {
    cuint32 thread_id = 0;
    std::unique_ptr<MemoryConstraint> mem_constr(new SingleThreadMemoryConstraint(thread_id));
    ConstraintSet constr_set(0x0, 0xffffffffffff);
    mem_constr->Initialize(constr_set);
    AddressReuseMode addr_reuse_mode;
    enable_all_reuse_types(addr_reuse_mode);

    SECTION( "Test marking addresses as shared" ) {
      const ConstraintSet* shared_constr = mem_constr->Shared();
      EXPECT(shared_constr->IsEmpty());
      mem_constr->MarkShared(0xf973, 0x900ba);

      shared_constr = mem_constr->Shared();
      EXPECT(shared_constr->ToSimpleString() == "0xf973-0x900ba");
      const ConstraintSet* usable = mem_constr->Usable();
      EXPECT(usable->ToSimpleString() == "0x0-0xf972,0x900bb-0xffffffffffff");
    }

    SECTION( "Test marking addresses as shared via constraint set" ) {
      mem_constr->MarkShared(ConstraintSet(0xa5, 0xb7));

      const ConstraintSet* shared_constr = mem_constr->Shared();
      EXPECT(shared_constr->ToSimpleString() == "0xa5-0xb7");
      const ConstraintSet* usable = mem_constr->Usable();
      EXPECT(usable->ToSimpleString() == "0x0-0xa4,0xb8-0xffffffffffff");
    }

    SECTION( "Test applying to a data constraint set that is partially shared" ) {
      mem_constr->MarkShared(0x700, 0x800);
      mem_constr->MarkUsed(0x780, 0x880);
      ConstraintSet read_constr_set(0x7a0, 0x8a0);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.ToSimpleString() == "0x7a0-0x800,0x881-0x8a0");
    }

    SECTION( "Test applying to a data constraint set that is wholly shared" ) {
      mem_constr->MarkShared(0x1250, 0x1270);
      ConstraintSet shared_constr_set(0x125f, 0x1267);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &shared_constr_set);
      EXPECT(shared_constr_set.ToSimpleString() == "0x125f-0x1267");
    }

    SECTION( "Test applying to an instruction constraint set that is partially within the shared constraint" ) {
      mem_constr->MarkShared(0xfc, 0x29b);
      ConstraintSet instr_constr_set(0x200, 0x300);
      mem_constr->ApplyToConstraintSet(EMemDataType::Instruction, EMemAccessType::Read, thread_id, addr_reuse_mode, &instr_constr_set);
      EXPECT(instr_constr_set.ToSimpleString() == "0x29c-0x300");
    }

    SECTION( "Test applying to an instruction constraint set that is wholly within the shared constraint" ) {
      mem_constr->MarkShared(0x595, 0x6b2);
      ConstraintSet instr_constr_set(0x600, 0x650);
      mem_constr->ApplyToConstraintSet(EMemDataType::Instruction, EMemAccessType::Read, thread_id, addr_reuse_mode, &instr_constr_set);
      EXPECT(instr_constr_set.IsEmpty());
    }
  }
},

CASE( "Test multi-threaded address reuse" ) {

  SETUP( "Setup MemoryConstraint" )  {
    std::unique_ptr<MemoryConstraint> mem_constr(new MultiThreadMemoryConstraint(2));
    ConstraintSet constr_set(0x0, 0xffffffffffff);
    mem_constr->Initialize(constr_set);
    AddressReuseMode addr_reuse_mode;
    enable_all_reuse_types(addr_reuse_mode);

    SECTION( "Test multi-threaded memory constraint with 0 threads" ) {
      std::unique_ptr<MemoryConstraint> mem_constr_no_threads(new MultiThreadMemoryConstraint(0));
      ConstraintSet constr_set(0x230, 0x290);
      EXPECT_FAIL(mem_constr_no_threads->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, 0, addr_reuse_mode, &constr_set), "mem-constraint-not-found");
      EXPECT_FAIL(mem_constr_no_threads->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, 0, addr_reuse_mode, &constr_set), "mem-constraint-not-found");
    }

    SECTION( "Test cloning" ) {
      mem_constr->MarkUsedForType(0x580, 0x630, EMemDataType::Data, EMemAccessType::Read, 0);
      mem_constr->MarkUsedForType(0x780, 0x830, EMemDataType::Data, EMemAccessType::Write, 0);
      mem_constr->MarkUsedForType(0x980, 0xa30, EMemDataType::Data, EMemAccessType::Read, 1);
      mem_constr->MarkUsedForType(0xb80, 0xc30, EMemDataType::Data, EMemAccessType::Write, 1);
      std::unique_ptr<MemoryConstraint> mem_constr_clone(dynamic_cast<MemoryConstraint*>(mem_constr->Clone()));

      ConstraintSet thread_0_constr_set(0x400, 0x1000);
      mem_constr_clone->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, 0, addr_reuse_mode, &thread_0_constr_set);
      EXPECT(thread_0_constr_set.ToSimpleString() == "0x400-0x97f,0xa31-0xb7f,0xc31-0x1000");

      ConstraintSet thread_1_constr_set(0x400, 0x1000);
      mem_constr_clone->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, 1, addr_reuse_mode, &thread_1_constr_set);
      EXPECT(thread_1_constr_set.ToSimpleString() == "0x400-0x57f,0x631-0x77f,0x831-0x1000");
    }

    SECTION( "Test uninitialization" ) {
      mem_constr->MarkUsedForType(0xfff30, 0xfffff, EMemDataType::Data, EMemAccessType::Write, 1);
      mem_constr->Uninitialize();
      mem_constr->MarkUsed(0xfff30, 0xfffff);
      ConstraintSet write_constr_set(0xfff50, 0xfff80);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, 0, addr_reuse_mode, &write_constr_set);
      EXPECT(write_constr_set.IsEmpty());
    }

    SECTION( "Test marking data read addresses as used" ) {
      mem_constr->MarkUsed(0x1060, 0x1250);
      mem_constr->MarkUsedForType(0x997, 0x105a, EMemDataType::Data, EMemAccessType::Read, 0);

      ConstraintSet thread_0_constr_set(0x1000, 0x1100);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, 0, addr_reuse_mode, &thread_0_constr_set);
      EXPECT(thread_0_constr_set.ToSimpleString() == "0x1000-0x105f");

      ConstraintSet thread_1_constr_set(0x1000, 0x1100);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, 1, addr_reuse_mode, &thread_1_constr_set);
      EXPECT(thread_1_constr_set.ToSimpleString() == "0x105b-0x105f");
    }

    SECTION( "Test marking data write addresses as used" ) {
      mem_constr->MarkUsed(0x377, 0x480);
      mem_constr->MarkUsedForType(0x400, 0x500, EMemDataType::Data, EMemAccessType::Write, 1);

      ConstraintSet thread_0_constr_set(0x360, 0x450);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, 0, addr_reuse_mode, &thread_0_constr_set);
      EXPECT(thread_0_constr_set.ToSimpleString() == "0x360-0x376");

      ConstraintSet thread_1_constr_set(0x360, 0x450);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, 1, addr_reuse_mode, &thread_1_constr_set);
      EXPECT(thread_1_constr_set.ToSimpleString() == "0x360-0x376,0x400-0x450");
    }

    SECTION( "Test marking addresses with unknown access type as used" ) {
      mem_constr->MarkUsedForType(0x4000, 0x47ff, EMemDataType::Data, EMemAccessType::Unknown, 0);

      ConstraintSet thread_0_constr_set(0x4625, 0x4800);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, 0, addr_reuse_mode, &thread_0_constr_set);
      EXPECT(thread_0_constr_set.ToSimpleString() == "0x4800");

      ConstraintSet thread_1_constr_set(0x4625, 0x4800);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Unknown, 1, addr_reuse_mode, &thread_1_constr_set);
      EXPECT(thread_1_constr_set.ToSimpleString() == "0x4800");
    }

    SECTION( "Test marking addresses as used for an invalid thread" ) {
      mem_constr->MarkUsedForType(0x59, 0x65, EMemDataType::Data, EMemAccessType::Read, 7);
      mem_constr->MarkUsedForType(0x6ffe, 0x7926, EMemDataType::Data, EMemAccessType::Write, 3);

      ConstraintSet thread_0_constr_set(0x48, 0x62);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, 0, addr_reuse_mode, &thread_0_constr_set);
      EXPECT(thread_0_constr_set.ToSimpleString() == "0x48-0x58");

      ConstraintSet thread_1_constr_set(0x7000, 0x8000);
      mem_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, 1, addr_reuse_mode, &thread_1_constr_set);
      EXPECT(thread_1_constr_set.ToSimpleString() == "0x7927-0x8000");
    }
  }
},

};

int main(int argc, char * argv[])
{
  Force::Logger::Initialize();
  Force::Random::Initialize();
  int ret = lest::run(specification, argc, argv);
  Force::Random::Destroy();
  Force::Logger::Destroy();
  return ret;
}
