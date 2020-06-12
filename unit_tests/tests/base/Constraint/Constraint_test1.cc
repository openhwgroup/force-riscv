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

#include <Constraint.h>
#include <GenException.h>

using text = std::string;
using namespace std;
using namespace Force;

#undef CASE
#define CASE( name ) lest_CASE( specification(), name )

extern lest::tests & specification();

CASE( "test set 1 for Constraint module" ) {

  SETUP( "setup ConstraintSet, adding constraints with no overlap" )  {
    ConstraintSet my_constr_set;
    my_constr_set.AddRange(0x1000, 0x3fff0);
    my_constr_set.AddRange(0x40000, 0x7ffff);
    my_constr_set.AddRange(0x80100, 0x80100);
    my_constr_set.AddRange(0x90001, 0x90003);
    my_constr_set.AddValue(0x91000);

    SECTION( "test adding non overlapping value/range" ) {
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x80100,0x90001-0x90003,0x91000");
      EXPECT(my_constr_set.CalculateSize() == 0x7eff6u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION( "test adding range merge overshadow first item" ) {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AddRange(0x3fff7, 0x100);
      EXPECT(my_constr_set.ToSimpleString() == "0x100-0x3fff7,0x40000-0x7ffff,0x80100,0x90001-0x90003,0x91000");
      EXPECT(my_constr_set.CalculateSize() == 0x7fefdu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);
    }

    SECTION( "test add single value before single value" ) {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AddRange(0x800ff, 0x800ff);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x800ff-0x80100,0x90001-0x90003,0x91000");
      EXPECT(my_constr_set.CalculateSize() == 0x7eff7u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 3u);
    }

    SECTION( "test add single value after single value" ) {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AddRange(0x91001, 0x91001);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x80100,0x90001-0x90003,0x91000-0x91001");
      EXPECT(my_constr_set.CalculateSize() == 0x7eff7u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 3u);
    }

    SECTION( "test add value range around single value" ) {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AddRange(0x80001, 0x800ff);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x80001-0x80100,0x90001-0x90003,0x91000");
      EXPECT(my_constr_set.CalculateSize() == 0x7f0f5u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);

      ConstraintSet copy_set(my_constr_set);
      EXPECT(copy_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x80001-0x80100,0x90001-0x90003,0x91000");
      EXPECT(copy_set.CalculateSize() == 0x7f0f5u);
      EXPECT(copy_set.CalculateSize() == my_constr_set.Size());

      my_constr_set.AddRange(0xa0000, 0xa0000);
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AddRange(0x91001, 0x9fffe);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x80001-0x80100,0x90001-0x90003,0x91000-0x9fffe,0xa0000");
      EXPECT(my_constr_set.CalculateSize() == 0x8e0f4u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);

      ConstraintSet copy_set2(my_constr_set);
      EXPECT(copy_set2.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x80001-0x80100,0x90001-0x90003,0x91000-0x9fffe,0xa0000");
      EXPECT(copy_set2.CalculateSize() == 0x8e0f4u);
      EXPECT(copy_set2.CalculateSize() == my_constr_set.Size());
    }

    SECTION( "test add value range around value range" ) {
      my_constr_set.AddRange(0xa0000, 0xa0000);
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AddRange(0x8fff0, 0x90002);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x80100,0x8fff0-0x90003,0x91000,0xa0000");
      EXPECT(my_constr_set.CalculateSize() == 0x7f008u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AddRange(0x90023, 0x90004);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x80100,0x8fff0-0x90023,0x91000,0xa0000");
      EXPECT(my_constr_set.CalculateSize() == 0x7f028u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);
    }

    SECTION( "test add value range overlapping multiple constraints" ) {
      my_constr_set.AddRange(0xa0000, 0xa0000);
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AddRange(0x3fff1, 0x90005);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x90005,0x91000,0xa0000");
      EXPECT(my_constr_set.CalculateSize() == 0x8f008u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 5u);

      ConstraintSet copy_set(my_constr_set);
      EXPECT(copy_set.ToSimpleString() == "0x1000-0x90005,0x91000,0xa0000");
      EXPECT(copy_set.CalculateSize() == 0x8f008u);
      EXPECT(copy_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION( "test add single value that connect two adjacent values constraints" ) {
      my_constr_set.AddRange(0xa0000, 0xa0000);
      my_constr_set.AddValue(0x80001);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x7ffff,0x80001,0x80100,0x90001-0x90003,0x91000,0xa0000");
      EXPECT(my_constr_set.CalculateSize() ==  0x7eff8u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.AddValue(0x80000);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x80001,0x80100,0x90001-0x90003,0x91000,0xa0000");
      EXPECT(my_constr_set.CalculateSize() ==  0x7eff9u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 4u);

      ConstraintSet copy_set(my_constr_set);
      EXPECT(copy_set.ToSimpleString() == "0x1000-0x3fff0,0x40000-0x80001,0x80100,0x90001-0x90003,0x91000,0xa0000");
      EXPECT(copy_set.CalculateSize() == 0x7eff9u);
      EXPECT(copy_set.CalculateSize() == my_constr_set.Size());
    }
  }
}

CASE( "test set 2 for Constraint module" ) {

  SETUP( "setup ConstraintSet, adding initial range" )  {

    ConstraintSet my_constr_set(0x500000000ULL, 0x50fffffffULL);
    SECTION("basic sub checks") {
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0x10000000u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("subtract a single value from a big range") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubValue(0x500010000);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500010001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xfffffffu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      my_constr_set.SubValue(0x4000); // should have no effect.
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500010001-0x50fffffff");

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubValue(0x500000000);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000001-0x50000ffff,0x500010001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffffffeu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubValue(0x50000ffff);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000001-0x50000fffe,0x500010001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffffffdu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubValue(0x500010001);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000001-0x50000fffe,0x500010002-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffffffcu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubValue(0x50fffffff);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000001-0x50000fffe,0x500010002-0x50ffffffe");
      EXPECT(my_constr_set.CalculateSize() == 0xffffffbu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubValue(0x50000fffd);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000001-0x50000fffc,0x50000fffe,0x500010002-0x50ffffffe");
      EXPECT(my_constr_set.CalculateSize() == 0xffffffau);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubValue(0x50000fffe);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000001-0x50000fffc,0x500010002-0x50ffffffe");
      EXPECT(my_constr_set.CalculateSize() == 0xffffff9u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);
    }

    SECTION("subtract a range from another range or single value") {
      my_constr_set.SubValue(0x500010000);
      my_constr_set.SubValue(0x4000-0x4fff); // should have no effect.
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500010001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xfffffffu);

      my_constr_set.SubValue(0x508001000);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500010001-0x508000fff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffffffeu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x500010001, 0x508000fff);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0x800efffu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);

      my_constr_set.AddRange(0x500010002, 0x508000ffe);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500010002-0x508000ffe,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffffffcu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x500010000, 0x50001ffff);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500020000-0x508000ffe,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffefffeu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x500020000, 0x50002ffff);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500030000-0x508000ffe,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffdfffeu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x507fff000, 0x508001000);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500030000-0x507ffefff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffddfffu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x507ffe000, 0x507ffefff);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500030000-0x507ffdfff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffdcfffu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x501000000, 0x501000fff);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500030000-0x500ffffff,0x501001000-0x507ffdfff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffdbfffu);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x500030001, 0x50003000f);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500030000,0x500030010-0x500ffffff,0x501001000-0x507ffdfff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffdbff0u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x500fffff0, 0x500fffffe);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500030000,0x500030010-0x500ffffef,0x500ffffff,0x501001000-0x507ffdfff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xffdbfe1u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x500030011, 0x500ffffee);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500030000,0x500030010,0x500ffffef,0x500ffffff,0x501001000-0x507ffdfff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xf00c003u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x500030000, 0x50003000f);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500030010,0x500ffffef,0x500ffffff,0x501001000-0x507ffdfff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xf00c002u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x500030000, 0x50003001f);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500ffffef,0x500ffffff,0x501001000-0x507ffdfff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xf00c001u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);

      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x500030000, 0x500ffffef);
      EXPECT(my_constr_set.ToSimpleString() == "0x500000000-0x50000ffff,0x500ffffff,0x501001000-0x507ffdfff,0x508001001-0x50fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xf00c000u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 2u);
    }
  }
}

CASE( "test set 3 for Constraint module" ) {

  SETUP( "setup ConstraintSet, setting up initial ranges" )  {

    ConstraintSet my_constr_set(0x4000000000ULL, 0x400fffffffULL);
    my_constr_set.SubRange(0x4000010000ULL, 0x400001ffffULL);
    my_constr_set.SubRange(0x4000030000, 0x400003ffff);
    my_constr_set.SubRange(0x4000050000, 0x400005ffff);
    my_constr_set.SubRange(0x4000070000, 0x400007ffff);
    my_constr_set.SubRange(0x4000090000, 0x400009ffff);
    //EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x4000020000-0x400002ffff,0x4000040000-0x400004ffff,0x4000060000-0x400006ffff,0x4000080000-0x400008ffff,0x40000a0000-0x400fffffff");

    SECTION("subtract a range that removes two adjacent ranges") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x4000030000, 0x4000077fff);
      EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x4000020000-0x400002ffff,0x4000080000-0x400008ffff,0x40000a0000-0x400fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xff90000u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 3u);
    }

    SECTION("subtract a range that removes two adjacent ranges") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x4000040000, 0x400006ffff);
      EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x4000020000-0x400002ffff,0x4000080000-0x400008ffff,0x40000a0000-0x400fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xff90000u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 3u);
    }

    SECTION("subtract a range that removes most of two adjacent ranges") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x4000040001, 0x400006fffe);
      EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x4000020000-0x400002ffff,0x4000040000,0x400006ffff,0x4000080000-0x400008ffff,0x40000a0000-0x400fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xff90002u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 3u);
    }

    SECTION("subtract a range that removes large portion of two adjacent ranges") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x4000040100, 0x400006feff);
      EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x4000020000-0x400002ffff,0x4000040000-0x40000400ff,0x400006ff00-0x400006ffff,0x4000080000-0x400008ffff,0x40000a0000-0x400fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xff90200u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 1u);
    }

    SECTION("subtract a range that removes multiple adjacent ranges") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x4000010000, 0x4000097fff);
      EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x40000a0000-0x400fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xff70000u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 5u);
    }

    SECTION("subtract a range that removes multiple adjacent ranges, leaving start end single values") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x4000020001, 0x400008fffe);
      EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x4000020000,0x400008ffff,0x40000a0000-0x400fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xff70002u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 5u);
    }

    SECTION("subtract a range that removes multiple adjacent ranges, leaving parts of start end ranges") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x4000020100, 0x400008feff);
      EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x4000020000-0x40000200ff,0x400008ff00-0x400008ffff,0x40000a0000-0x400fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xff70200u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 3u);
    }

    SECTION("subtract a range that removes multiple adjacent ranges, leaving part of start ranges") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x4000020100, 0x4000097fff);
      EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x4000020000-0x40000200ff,0x40000a0000-0x400fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xff70100u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 4u);
    }

    SECTION("subtract a range that removes multiple adjacent ranges, leaving part of end ranges") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.SubRange(0x4000020000, 0x400008feff);
      EXPECT(my_constr_set.ToSimpleString() == "0x4000000000-0x400000ffff,0x400008ff00-0x400008ffff,0x40000a0000-0x400fffffff");
      EXPECT(my_constr_set.CalculateSize() == 0xff70100u);
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
      EXPECT(ConstraintSet::msConstraintDeleteCount == 4u);
    }

    SECTION("test chosen value methods") {
      uint64 set_size = my_constr_set.CalculateSize();
      EXPECT(set_size == 0xffb0000u);
      EXPECT(my_constr_set.ChosenValueFromFront(0) == 0x4000000000ULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0xffff) == 0x400000ffffULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0x10000) == 0x4000020000ULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0x1ffff) == 0x400002ffffULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0x20000) == 0x4000040000ULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0x2ffff) == 0x400004ffffULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0x30000) == 0x4000060000ULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0x3ffff) == 0x400006ffffULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0x40000) == 0x4000080000ULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0x4ffff) == 0x400008ffffULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0x50000) == 0x40000a0000ULL);
      EXPECT(my_constr_set.ChosenValueFromFront(0xffaffff) == 0x400fffffffULL);

      EXPECT(my_constr_set.ChosenValueFromBack(0, set_size) == 0x4000000000ULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0xffff, set_size) == 0x400000ffffULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0x10000, set_size) == 0x4000020000ULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0x1ffff, set_size) == 0x400002ffffULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0x20000, set_size) == 0x4000040000ULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0x2ffff, set_size) == 0x400004ffffULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0x30000, set_size) == 0x4000060000ULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0x3ffff, set_size) == 0x400006ffffULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0x40000, set_size) == 0x4000080000ULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0x4ffff, set_size) == 0x400008ffffULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0x50000, set_size) == 0x40000a0000ULL);
      EXPECT(my_constr_set.ChosenValueFromBack(0xffaffffu, set_size) == 0x400fffffffULL);

      EXPECT(my_constr_set.ChooseValue() >= 0x4000000000ULL);
      EXPECT(my_constr_set.ChooseValue() <= 0x400fffffffULL);
      uint64 chosen_value = my_constr_set.ChooseValue();
      EXPECT(my_constr_set.ContainsValue(chosen_value));
      EXPECT(my_constr_set.ContainsValue(0x4000010000ULL) == false);
      EXPECT(my_constr_set.ContainsValue(0x400fffffffULL) == true);
    }

    SECTION("test choosing value from empty ConstraintSet") {
      ConstraintSet empty_constr_set;
      EXPECT_THROWS_AS(empty_constr_set.ChooseValue(), ConstraintError);
    }

    SECTION("test choosing value from ConstraintSet of all values") {
      ConstraintSet all_values_constr_set(0, MAX_UINT64);
      EXPECT_NO_THROW(all_values_constr_set.ChooseValue());
    }

    SECTION("test choosing value from empty ConstraintSet") {
      ConstraintSet empty_constr_set;
      EXPECT_THROWS_AS(empty_constr_set.ChooseValue(), ConstraintError);
    }

    SECTION("test choosing value from ConstraintSet of all values") {
      ConstraintSet all_values_constr_set(0, MAX_UINT64);
      EXPECT_NO_THROW(all_values_constr_set.ChooseValue());
    }
  }
}
