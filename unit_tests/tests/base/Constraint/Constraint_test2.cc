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
#include "Constraint.h"

#include "lest/lest.hpp"

#include "GenException.h"
#include "Log.h"

using text = std::string;
using namespace std;
using namespace Force;

#undef CASE
#define CASE( name ) lest_CASE( specification(), name )

extern lest::tests & specification();

CASE( "test set 4 for Constraint module" ) {

  SETUP( "setup ConstraintSet, setting up initial ranges" )  {

    ConstraintSet my_constr_set(0x4000000000ULL, 0x400fffffffULL);
    my_constr_set.SubRange(0x4000010000ULL, 0x400001ffffULL);
    my_constr_set.SubRange(0x4000030000, 0x400003ffff);
    my_constr_set.SubRange(0x4000050000, 0x400005ffff);
    my_constr_set.SubRange(0x4000070000, 0x400007ffff);
    my_constr_set.SubRange(0x4000090000, 0x400009ffff);
    //EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x4000020000-0x400002ffff,0x4000040000-0x400004ffff,0x4000060000-0x400006ffff,0x4000080000-0x400008ffff,0x40000a0000-0x400fffffff");

    SECTION("test Contains methods") {
      EXPECT(my_constr_set.ContainsValue(0x400004789a) == true);
      EXPECT(my_constr_set.ContainsValue(0x400003789a) == false);
      EXPECT(my_constr_set.ContainsRange(0x4000020000, 0x400002ffff) == true);
      EXPECT(my_constr_set.ContainsRange(0x400001ffff, 0x400002ffff) == false);
      EXPECT(my_constr_set.ContainsRange(0x4000030000, 0x4000020000) == false);

      my_constr_set.SubRange(0x400006fff0, 0x400006fffe);
      ValueConstraint val_constr1(0x400006ffff);
      EXPECT(my_constr_set.ContainsConstraint(val_constr1) == true);
      ValueConstraint val_constr2(0x400006fffe);
      EXPECT(my_constr_set.ContainsConstraint(val_constr2) == false);
      ValueConstraint val_constr3(0x4000070000);
      EXPECT(my_constr_set.ContainsConstraint(val_constr3) == false);
    }

    SECTION("test ContainsConstraintSet method") {
      ConstraintSet test_set1("0x4000000000-0x4000000020,0x4000001000-0x400000ffff,0x4000020000");
      EXPECT(my_constr_set.ContainsConstraintSet(test_set1) == true);
      ConstraintSet test_set2("0x4000000000-0x4000000020,0x4000001000-0x4000010000,0x4000020000");
      EXPECT(my_constr_set.ContainsConstraintSet(test_set2) == false);
    }

    SECTION("test ShiftRight methods") {
      ConstraintSet my_constr_set_copy(my_constr_set);
      my_constr_set.ShiftRight(5);
      EXPECT(my_constr_set.ToSimpleString() == "0x200000000-0x2000007ff,0x200001000-0x2000017ff,0x200002000-0x2000027ff,0x200003000-0x2000037ff,0x200004000-0x2000047ff,0x200005000-0x2007fffff");
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      uint64 shifted_chosen = my_constr_set.ChooseValue();
      uint64 unshifted_chosen = shifted_chosen << 5;
      EXPECT(my_constr_set_copy.ContainsValue(unshifted_chosen));
    }
  }
}

CASE( "test set 5 for Constraint module" ) {

  SETUP( "setup ConstraintSet, setting up initial ranges with constraint string" )  {

    ConstraintSet my_constr_set("3-9,15,19-70,72-74,80-82,90-120");

    EXPECT(my_constr_set.ToSimpleString() == "0x3-0x9,0xf,0x13-0x46,0x48-0x4a,0x50-0x52,0x5a-0x78");
    EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());

    SECTION("test AlignWithSize method") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AlignWithSize(0xfffffffffffffffc, 4);
      EXPECT(my_constr_set.ToSimpleString() == "0x4,0x14-0x40,0x5c-0x74");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 4u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test AlignWithSize method with small align size relative to align mask") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AlignWithSize(0xffffffffffffffe0, 2);
      EXPECT(my_constr_set.ToSimpleString() == "0x20-0x40,0x60");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 5u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test AlignOffsetWithSize method") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AlignOffsetWithSize(0xfffffffffffffff8, 3, 8);
      EXPECT(my_constr_set.ToSimpleString() == "0x13-0x3b,0x5b-0x6b");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 4u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test AlignOffsetWithSize method with 0 align size") {
      EXPECT_FAIL(my_constr_set.AlignOffsetWithSize(0xfffffffffffffff8, 3, 0), "invalid-zero-size");
    }

    SECTION("test AlignOffsetWithSize method with invalid align offset") {
      EXPECT_FAIL(my_constr_set.AlignOffsetWithSize(0xfffffffffffffff8, 15, 8), "invalid-align-offset");
    }

    SECTION("test AlignOffsetWithSize method without completely removing any constraints") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AlignOffsetWithSize(0xfffffffffffffffe, 1, 1);
      EXPECT(my_constr_set.ToSimpleString() == "0x3-0x9,0xf,0x13-0x45,0x49,0x51,0x5b-0x77");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test AlignOffsetWithSize method with range reduced to value") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AlignOffsetWithSize(0xfffffffffffffffc, 2, 2);
      EXPECT(my_constr_set.ToSimpleString() == "0x6,0x16-0x42,0x5a-0x76");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 4u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test AlignOffsetWithSize method with range completely removed") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AlignOffsetWithSize(0xfffffffffffffffc, 2, 2);
      EXPECT(my_constr_set.ToSimpleString() == "0x6,0x16-0x42,0x5a-0x76");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 4u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test AlignOffsetWithSize method with small size") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AlignOffsetWithSize(0xfffffffffffffff8, 7, 1);
      EXPECT(my_constr_set.ToSimpleString() == "0x7,0xf,0x17-0x3f,0x5f-0x77");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 3u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test AlignOffsetWithSize method with small size and unaligned value constraint") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AlignOffsetWithSize(0xfffffffffffffffc, 1, 1);
      EXPECT(my_constr_set.ToSimpleString() == "0x5-0x9,0x15-0x45,0x49,0x51,0x5d-0x75");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 3u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test AlignOffsetWithSize method with large size") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AlignOffsetWithSize(0xfffffffffffffff8, 7, 30);
      EXPECT(my_constr_set.ToSimpleString() == "0x17-0x27");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 5u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test AlignOffsetWithSize method with large offset") {
      ConstraintSet constr_set(0x4200, 0x56FF);
      ConstraintSet::msConstraintDeleteCount = 0;
      constr_set.AlignOffsetWithSize(0xfffffffffffffc00, 537, 4);
      EXPECT(constr_set.ToSimpleString() == "0x4219-0x5619");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 0u);
      EXPECT(constr_set.CalculateSize() == constr_set.Size());
    }

    SECTION("test ConstraintSet size after AlignOffsetWithSize and ShiftRight") {
      ConstraintSet constr_set(0x3, 0x4002);
      constr_set.AlignOffsetWithSize(0xfffffffffffffffc, 3, 4);
      constr_set.ShiftRight(2);
      EXPECT(constr_set.Size() == 4096u);
    }

    SECTION("test AlignOffsetWithSize method with no constraints") {
      ConstraintSet constr_set;
      constr_set.AlignOffsetWithSize(0xfffffffffffffff8, 3, 8);
      EXPECT(constr_set.IsEmpty());
    }

    SECTION("Simple tests on ApplyConstraintSet") {
      ConstraintSet copy_constr_set(my_constr_set);
      ConstraintSet apply_constr_set1("0-1");
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraintSet(apply_constr_set1);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 6u);
      EXPECT(copy_constr_set.ToSimpleString() == "");
      EXPECT(copy_constr_set.Size() == 0u);
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());

      copy_constr_set = my_constr_set;
      ConstraintSet apply_constr_set2("1000-2000");
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraintSet(apply_constr_set2);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 6u);
      EXPECT(copy_constr_set.ToSimpleString() == "");
      EXPECT(copy_constr_set.Size() == 0u);
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());

      copy_constr_set = my_constr_set;
      ConstraintSet apply_constr_set3("0x14-0x51");
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraintSet(apply_constr_set3);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 3u);
      EXPECT(copy_constr_set.ToSimpleString() == "0x14-0x46,0x48-0x4a,0x50-0x51");
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());
    }

    SECTION("Test2 on ApplyConstraint") {
      ConstraintSet copy_constr_set(my_constr_set);
      ValueConstraint apply_constr1(0);
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraint(apply_constr1);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 6u);
      EXPECT(copy_constr_set.Size() == 0u);
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());

      copy_constr_set = my_constr_set;
      RangeConstraint apply_constr2(3000, 4000);
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraint(apply_constr2);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 6u);
      EXPECT(copy_constr_set.Size() == 0u);
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());

      copy_constr_set = my_constr_set;
      RangeConstraint apply_constr3(0x45,0x50);
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraint(apply_constr3);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 4u);
      EXPECT(copy_constr_set.ToSimpleString() == "0x45-0x46,0x48-0x4a,0x50");
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());
    }

    SECTION("more tests on ApplyConstraintSet") {
      my_constr_set.AddValue(0x1000);
      my_constr_set.AddRange(0x2000, 0x2fff);
      my_constr_set.AddRange(0x4000, 0x4fff);
      my_constr_set.AddValue(0x6000);

      EXPECT(my_constr_set.ToSimpleString() == "0x3-0x9,0xf,0x13-0x46,0x48-0x4a,0x50-0x52,0x5a-0x78,0x1000,0x2000-0x2fff,0x4000-0x4fff,0x6000");

      ConstraintSet copy_constr_set(my_constr_set);
      ConstraintSet apply_constr_set1("0-3,0x9-0xf,0x50-0x1000");
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraintSet(apply_constr_set1);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 7u);
      EXPECT(copy_constr_set.ToSimpleString() == "0x3,0x9,0xf,0x50-0x52,0x5a-0x78,0x1000");
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());

      copy_constr_set = my_constr_set;
      ConstraintSet apply_constr_set2("0x10-0x14,0x50-0x2100");
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraintSet(apply_constr_set2);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 8u);
      EXPECT(copy_constr_set.ToSimpleString() == "0x13-0x14,0x50-0x52,0x5a-0x78,0x1000,0x2000-0x2100");
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());

      copy_constr_set = my_constr_set;
      ConstraintSet apply_constr_set3("0-0x3,0x8-0x15,0x46-0x48,0x4a-0x50,0x52-0x60,0x70-0x2100,0x2700-0x4100,0x4700-0x9000");
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraintSet(apply_constr_set3);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 4u);
      EXPECT(copy_constr_set.ToSimpleString() == "0x3,0x8-0x9,0xf,0x13-0x15,0x46,0x48,0x4a,0x50,0x52,0x5a-0x60,0x70-0x78,0x1000,0x2000-0x2100,0x2700-0x2fff,0x4000-0x4100,0x4700-0x4fff,0x6000");
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());

      copy_constr_set = my_constr_set;
      ConstraintSet apply_constr_set4("0-0x3,0x15-0x48,0x70-0x2100,0x2700-0x4100,0x4700-0x9000");
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraintSet(apply_constr_set4);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 5u);
      EXPECT(copy_constr_set.ToSimpleString() == "0x3,0x15-0x46,0x48,0x70-0x78,0x1000,0x2000-0x2100,0x2700-0x2fff,0x4000-0x4100,0x4700-0x4fff,0x6000");
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());

      copy_constr_set = my_constr_set;
      ConstraintSet apply_constr_set5("0x9000,0x10000-0x1ffff");
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraintSet(apply_constr_set5);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 11u);
      EXPECT(copy_constr_set.ToSimpleString() == "");
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());

      copy_constr_set = my_constr_set;
      ConstraintSet apply_constr_set6("0x4b-0x4f,0x79-0xfff");
      ConstraintSet::msConstraintDeleteCount = 0;
      copy_constr_set.ApplyConstraintSet(apply_constr_set6);
      EXPECT(ConstraintSet::msConstraintDeleteCount == 11u);
      EXPECT(copy_constr_set.ToSimpleString() == "");
      EXPECT(copy_constr_set.CalculateSize() == copy_constr_set.Size());
    }

    SECTION(" tests on GetAlignedSizeFromBottom") {
      ConstraintSet constr_set("0x60000000-0x63ffffff");
      uint64 align = 0x1000;
      uint64 size = 0x1000;
      uint64 align_mask = ~(align - 1);
      EXPECT(constr_set.GetAlignedSizeFromBottom(align_mask, size) == 0x60000000u);
    }

    SECTION(" tests on GetAlignedSizeFromTop") {
      ConstraintSet constr_set("0x60000000-0x63ffffff");
      uint64 align = 0x1000;
      uint64 size = 0x1000;
      uint64 align_mask = ~(align - 1);
      EXPECT(constr_set.GetAlignedSizeFromTop(align_mask, size) == 0x63fff000u);
    }
  }
}

CASE( "Test applying large ConstraintSets" ) {

  SETUP( "Setup ConstraintSet" )  {
    ConstraintSet my_constr_set(0x425, 0x780);
    ConstraintSet large_constr_set("0x5-0x27,0x35-0x5f,0x72-0x200,0x205-0x21a,0x422-0x6ff,0x735,0x759-0x832,0x10f5-0x20f5,0x2198-0x2199,0x3055");

    SECTION( "Test applying large ConstraintSet" ) {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.ApplyLargeConstraintSet(large_constr_set);
      EXPECT(my_constr_set.ToSimpleString() == "0x425-0x6ff,0x735,0x759-0x780");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION( "Test applying large ConstraintSet to ConstraintSet with multiple constraints" ) {
      my_constr_set.AddRange(0x830, 0x15a8);
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.ApplyLargeConstraintSet(large_constr_set);
      EXPECT(my_constr_set.ToSimpleString() == "0x425-0x6ff,0x735,0x759-0x780,0x830-0x832,0x10f5-0x15a8");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION( "Test applying ConstraintSet with smaller upper bound" ) {
      large_constr_set.SubRange(0x700, 0x4000);
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.ApplyLargeConstraintSet(large_constr_set);
      EXPECT(my_constr_set.ToSimpleString() == "0x425-0x6ff");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION( "Test applying ConstraintSet with larger lower bound" ) {
      large_constr_set.SubRange(0x0, 0x500);
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.ApplyLargeConstraintSet(large_constr_set);
      EXPECT(my_constr_set.ToSimpleString() == "0x501-0x6ff,0x735,0x759-0x780");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION( "Test applying empty ConstraintSet" ) {
      ConstraintSet empty_constr_set;
      my_constr_set.ApplyLargeConstraintSet(large_constr_set);
      EXPECT_FAIL(my_constr_set.ApplyLargeConstraintSet(empty_constr_set), "applying-empty-constraint-set");
    }

    SECTION( "Test applying to empty ConstraintSet" ) {
      ConstraintSet empty_constr_set;
      ConstraintSet::msConstraintDeleteCount = 0;
      empty_constr_set.ApplyLargeConstraintSet(large_constr_set);
      EXPECT(empty_constr_set.IsEmpty());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 0u);
      EXPECT(empty_constr_set.CalculateSize() == empty_constr_set.Size());
    }

    SECTION( "Test applying same ConstraintSet" ) {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.ApplyLargeConstraintSet(my_constr_set);
      EXPECT(my_constr_set.ToSimpleString() == "0x425-0x780");
      EXPECT(ConstraintSet::msConstraintDeleteCount == 0u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION( "Test applying ConstraintSet with no overlap" ) {
      large_constr_set.SubRange(0x400, 0x800);
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.ApplyLargeConstraintSet(large_constr_set);
      EXPECT(my_constr_set.IsEmpty());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }
  }
}
