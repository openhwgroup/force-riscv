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

#include <memory>
#include <vector>

#include <Constraint.h>
#include <GenException.h>
#include <Log.h>

using text = std::string;
using namespace std;
using namespace Force;

#undef CASE
#define CASE( name ) lest_CASE( specification(), name )

extern lest::tests & specification();

CASE("test set 5.1 for Constraint module") {

  SETUP("setup ConstraintSet")  {
    ConstraintSet my_constr_set;

    SECTION("test ConstraintSet copy constructor") {
      my_constr_set.AddValue(0x91000);
      my_constr_set.AddRange(0x4FD6, 0x4FD8);
      ConstraintSet copied_constr_set(my_constr_set);
      EXPECT(copied_constr_set.ContainsValue(0x91000u));
      EXPECT(copied_constr_set.ContainsRange(0x4FD6u, 0x4FD8u));
      EXPECT(my_constr_set.Size() == copied_constr_set.Size());
    }

    SECTION("test ConstraintSet::Clone method") {
      my_constr_set.AddValue(0x91000);
      my_constr_set.AddRange(0x4FD6, 0x4FD8);
      unique_ptr<ConstraintSet> constr_set(my_constr_set.Clone());
      EXPECT(constr_set.get() != &my_constr_set);
      EXPECT(constr_set->ToSimpleString() == "0x4fd6-0x4fd8,0x91000");
    }

    SECTION("test ConstraintSet::GetValues method") {
      my_constr_set.AddValue(0x91000);
      vector<uint64> value_vec;
      my_constr_set.GetValues(value_vec);
      EXPECT(value_vec.size() == 1u);
      EXPECT(value_vec.front() == 0x91000u);
    }

    SECTION("test ConstraintSet::GetValues method with no values") {
      vector<uint64> value_vec;
      my_constr_set.GetValues(value_vec);
      EXPECT(value_vec.empty());
    }

    SECTION("test ConstraintSet::GetValues method with too many values") {
      my_constr_set.AddRange(0x91000, 0x92000);
      vector<uint64> value_vec;
      EXPECT_FAIL(my_constr_set.GetValues(value_vec), "constraint-value-number-too-large");
    }

    SECTION("test ConstraintSet::LeadingIntersectingRange method") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      uint64 inter_size = 0;
      uint64 start_addr = my_constr_set.LeadingIntersectingRange(0x1F00, 0x2000, inter_size);
      EXPECT(start_addr == 0x1F00u);
      EXPECT(inter_size == 0xFDu);
    }

    SECTION("test ConstraintSet::LeadingIntersectingRange method with lesser stored range") {
      my_constr_set.AddRange(0x500, 0x5C2);
      uint64 inter_size = 0;
      my_constr_set.LeadingIntersectingRange(0x1F00, 0x2000, inter_size);
      EXPECT(inter_size == 0x0u);
    }

    SECTION("test ConstraintSet::LeadingIntersectingRange method greater stored range") {
      my_constr_set.AddRange(0x20B0, 0x20B1);
      uint64 inter_size = 0;
      my_constr_set.LeadingIntersectingRange(0x1F00, 0x2000, inter_size);
      EXPECT(inter_size == 0x0u);
    }

    SECTION("test ConstraintSet::OnlyValue method") {
      my_constr_set.AddValue(0x3005);
      uint64 value = my_constr_set.OnlyValue();
      EXPECT(value == 0x3005u);
    }

    SECTION("test ConstraintSet::OnlyValue method with range") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      EXPECT_THROWS_AS(my_constr_set.OnlyValue(), ConstraintError);
    }

    SECTION("test ConstraintSet::OnlyValue method with 2 constraints") {
      my_constr_set.AddValue(0xA1);
      my_constr_set.AddValue(0xB30);
      EXPECT_THROWS_AS(my_constr_set.OnlyValue(), ConstraintError);
    }

    SECTION("test ConstraintSet::SubConstraint method") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      ValueConstraint val_constr(0x10D5);
      my_constr_set.SubConstraint(val_constr);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x10d4,0x10d6-0x1ffc");
    }

    SECTION("test ConstraintSet::SubConstraint method with no constraints") {
      ValueConstraint val_constr(0x10D5);
      my_constr_set.SubConstraint(val_constr);
      EXPECT(my_constr_set.IsEmpty());
    }

    SECTION("test ConstraintSet::SubConstraint method with no intersecting constraints") {
      my_constr_set.AddRange(0x6D2, 0x7FF);
      ValueConstraint val_constr(0x10D5);
      my_constr_set.SubConstraint(val_constr);
      EXPECT(my_constr_set.ToSimpleString() == "0x6d2-0x7ff");
    }

    SECTION("test ConstraintSet::SubConstraint method, reducing range to value") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      RangeConstraint range_constr(0x1000, 0x1FFB);
      my_constr_set.SubConstraint(range_constr);
      EXPECT(my_constr_set.ToSimpleString() == "0x1ffc");
    }

    SECTION("test ConstraintSet::SubConstraint method, eliminating constraint") {
      my_constr_set.AddValue(0x5C1);
      RangeConstraint range_constr(0x510, 0x5D0);
      my_constr_set.SubConstraint(range_constr);
      EXPECT(my_constr_set.IsEmpty());
    }

    SECTION("test ConstraintSet::SubConstraint method with multiple ranges") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      my_constr_set.AddRange(0x20D1, 0x2562);
      RangeConstraint range_constr(0x1050, 0x1855);
      my_constr_set.SubConstraint(range_constr);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x104f,0x1856-0x1ffc,0x20d1-0x2562");
    }

    SECTION("test ConstraintSet::SubConstraint method with multiple ranges, reducing first range to value") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      my_constr_set.AddRange(0x20D1, 0x2562);
      RangeConstraint range_constr(0x1001, 0x1FFC);
      my_constr_set.SubConstraint(range_constr);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000,0x20d1-0x2562");
    }

    SECTION("test ConstraintSet::SubConstraint method with multiple ranges, eliminating first constraint") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      my_constr_set.AddRange(0x20D1, 0x2562);
      RangeConstraint range_constr(0x99F, 0x1FFC);
      my_constr_set.SubConstraint(range_constr);
      EXPECT(my_constr_set.ToSimpleString() == "0x20d1-0x2562");
    }

    SECTION("test ConstraintSet::SubConstraint method with multiple ranges, subtracting from last range") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      my_constr_set.AddRange(0x20D1, 0x2562);
      RangeConstraint range_constr(0x21FF, 0x2455);
      my_constr_set.SubConstraint(range_constr);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x1ffc,0x20d1-0x21fe,0x2456-0x2562");
    }

    SECTION("test ConstraintSet::SubConstraint method with multiple ranges, reducing last range to value") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      my_constr_set.AddRange(0x20D1, 0x2562);
      RangeConstraint range_constr(0x20D1, 0x2561);
      my_constr_set.SubConstraint(range_constr);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x1ffc,0x2562");
    }

    SECTION("test ConstraintSet::SubConstraint method with multiple ranges, eliminating last constraint") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      my_constr_set.AddRange(0x20D1, 0x2562);
      RangeConstraint range_constr(0x20D1, 0x2562);
      my_constr_set.SubConstraint(range_constr);
      EXPECT(my_constr_set.ToSimpleString() == "0x1000-0x1ffc");
    }

    SECTION("test ConstraintSet::SubConstraint method with multiple ranges, eliminating all constraints") {
      my_constr_set.AddRange(0x1000, 0x1FFC);
      my_constr_set.AddRange(0x20D1, 0x2562);
      RangeConstraint range_constr(0xFFF, 0x2563);
      my_constr_set.SubConstraint(range_constr);
      EXPECT(my_constr_set.IsEmpty());
    }

    SECTION("test ConstraintSet::SubConstraintSet method") {
      my_constr_set.AddRange(0x5FFD1, 0x5FFD8);
      ConstraintSet constr_set(0x5FFD2, 0x5FFD6);
      my_constr_set.SubConstraintSet(constr_set);
      EXPECT(my_constr_set.ToSimpleString() == "0x5ffd1,0x5ffd7-0x5ffd8");
    }

    SECTION("test ConstraintSet::SubConstraintSet method with empty constraint set being subtracted") {
      my_constr_set.AddRange(0x5FFD1, 0x5FFD8);
      ConstraintSet constr_set;
      my_constr_set.SubConstraintSet(constr_set);
      EXPECT(my_constr_set.ToSimpleString() == "0x5ffd1-0x5ffd8");
    }

    SECTION("test ConstraintSet::SubConstraintSet method with empty constraint being subtracted from") {
      ConstraintSet constr_set(0x5FFD2, 0x5FFD6);
      my_constr_set.SubConstraintSet(constr_set);
      EXPECT(my_constr_set.IsEmpty());
    }

    SECTION("test ConstraintSet::VectorSize method") {
      my_constr_set.AddRange(0x4FD6, 0x4FD8);
      EXPECT(my_constr_set.VectorSize() == 1u);
    }

    SECTION("test ConstraintSet::operator== method") {
      my_constr_set.AddRange(0x4FD6, 0x4FD8);
      ConstraintSet constr_set(0x4FD6, 0x4FD8);
      EXPECT(my_constr_set == constr_set);
    }

    SECTION("test ConstraintSet::operator!= method") {
      ConstraintSet constr_set(0x4FD6, 0x4FD8);
      EXPECT(my_constr_set != constr_set);
    }

    SECTION("test ConstraintSet::NotElements method") {
      my_constr_set.AddRange(0xF376, 0x32B98);
      my_constr_set.NotElements();
      EXPECT(my_constr_set.ToSimpleString() == "0xfffffffffffcd467-0xffffffffffff0c89");
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test ConstraintSet::NotElements method with empty ConstraintSet") {
      my_constr_set.NotElements();
      EXPECT(my_constr_set.IsEmpty());
    }

    SECTION("test ConstraintSet::NotElements method with value") {
      my_constr_set.AddValue(0xFFDE9B7865C2);
      my_constr_set.NotElements();
      EXPECT(my_constr_set.ToSimpleString() == "0xffff002164879a3d");
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test ConstraintSet::NotElements method with multiple constraints") {
      my_constr_set.AddRange(0x1000, 0x2000);
      my_constr_set.AddValue(0x3000);
      my_constr_set.AddRange(0x4000, 0x5000);
      my_constr_set.NotElements();
      EXPECT(my_constr_set.ToSimpleString() == "0xffffffffffffafff-0xffffffffffffbfff,0xffffffffffffcfff,0xffffffffffffdfff-0xffffffffffffefff");
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }
  }
}

CASE("test set 5.2 for ConstraintSet::SubConstraintSet") {

  SETUP("setup ConstraintSet")  {
    ConstraintSet my_constr_set;

    SECTION("A basic simple test case, no overlap even") {
      ConstraintSet minuend("0x0-0x4fffffff,0x50000040-0x7fffffff,0x80001000-0xf8c56cd1ff,0xf8c56cd400-0x1331bb9ffff,0x1331dba0000-0x1c293c0ffff,0x1c295c10000-0xffffffffffff");
      ConstraintSet subtrahend("0x1331cb90000-0x1331cb9ffff,0x1c293c11858-0x1c293c1185f,0x1c293c60000-0x1c293c7ffff");

      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "0x0-0x4fffffff,0x50000040-0x7fffffff,0x80001000-0xf8c56cd1ff,0xf8c56cd400-0x1331bb9ffff,0x1331dba0000-0x1c293c0ffff,0x1c295c10000-0xffffffffffff");
      EXPECT(minuend.Size() == minuend.CalculateSize());
    }

    SECTION("A slightly complicated test case, a large Constraint in the target covers multiple ones in subtrahend") {
      ConstraintSet minuend("0x0-0x12ffffff,0x1300f000-0x1307ffff,0x130c0000-0x161fffff,0x16201000-0x21ffffff,0x22100000-0x3effffff,0x3f07fffc-0x3fffffff,0x50000040-0x7fffffff,0x80001000-0x9bffffff,0x9e000000-0xa2ffffff,0xa3020001-0x61690924d7,0x61690934d8-0xf8c56cd1ff,0xf8c56cd400-0x1c293c0ffff,0x1c295c10000-0xffffffffffff");
      ConstraintSet subtrahend("0x5c84a60000-0x5c84a60003,0x5c84a60080-0x5c84a60083,0x5c84a60100-0x5c84a60103,0x5c84a60180-0x5c84a60183,0x5c84a60200-0x5c84a60203,0x5c84a60280-0x5c84a60283,0x5c84a60300-0x5c84a60303,0x5c84a60380-0x5c84a60383,0x5c84a60400-0x5c84a60403,0x5c84a60480-0x5c84a60483,0x5c84a60500-0x5c84a60503,0x5c84a60580-0x5c84a60583,0x5c84a60600-0x5c84a60603,0x5c84a60680-0x5c84a60683,0x5c84a60700-0x5c84a60703,0x5c84a60800-0x5c84a60ac7,0x5c84a62000-0x5c84a62003,0x5c84a62080-0x5c84a62083,0x5c84a62100-0x5c84a62103,0x5c84a62180-0x5c84a62183,0x5c84a62200-0x5c84a62203,0x5c84a62280-0x5c84a62283,0x5c84a62300-0x5c84a62303,0x5c84a62380-0x5c84a62383,0x5c84a62400-0x5c84a62403,0x5c84a62480-0x5c84a62483,0x5c84a62500-0x5c84a62503,0x5c84a62580-0x5c84a62583,0x5c84a62600-0x5c84a62603,0x5c84a62680-0x5c84a62683,0x5c84a62700-0x5c84a62703,0x5c84a62800-0x5c84a62ac7,0x61690924d8-0x61690934d7");

      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "0x0-0x12ffffff,0x1300f000-0x1307ffff,0x130c0000-0x161fffff,0x16201000-0x21ffffff,0x22100000-0x3effffff,0x3f07fffc-0x3fffffff,0x50000040-0x7fffffff,0x80001000-0x9bffffff,0x9e000000-0xa2ffffff,0xa3020001-0x5c84a5ffff,0x5c84a60004-0x5c84a6007f,0x5c84a60084-0x5c84a600ff,0x5c84a60104-0x5c84a6017f,0x5c84a60184-0x5c84a601ff,0x5c84a60204-0x5c84a6027f,0x5c84a60284-0x5c84a602ff,0x5c84a60304-0x5c84a6037f,0x5c84a60384-0x5c84a603ff,0x5c84a60404-0x5c84a6047f,0x5c84a60484-0x5c84a604ff,0x5c84a60504-0x5c84a6057f,0x5c84a60584-0x5c84a605ff,0x5c84a60604-0x5c84a6067f,0x5c84a60684-0x5c84a606ff,0x5c84a60704-0x5c84a607ff,0x5c84a60ac8-0x5c84a61fff,0x5c84a62004-0x5c84a6207f,0x5c84a62084-0x5c84a620ff,0x5c84a62104-0x5c84a6217f,0x5c84a62184-0x5c84a621ff,0x5c84a62204-0x5c84a6227f,0x5c84a62284-0x5c84a622ff,0x5c84a62304-0x5c84a6237f,0x5c84a62384-0x5c84a623ff,0x5c84a62404-0x5c84a6247f,0x5c84a62484-0x5c84a624ff,0x5c84a62504-0x5c84a6257f,0x5c84a62584-0x5c84a625ff,0x5c84a62604-0x5c84a6267f,0x5c84a62684-0x5c84a626ff,0x5c84a62704-0x5c84a627ff,0x5c84a62ac8-0x61690924d7,0x61690934d8-0xf8c56cd1ff,0xf8c56cd400-0x1c293c0ffff,0x1c295c10000-0xffffffffffff");
      EXPECT(minuend.Size() == minuend.CalculateSize());
    }

    SECTION("A larger Constraint in the target covers multiple ones in subtrahend, having the same lower bound") {
      ConstraintSet minuend("0x50000040-0x7fffffff,0x80001018-0x9bffffff,0x9e000000-0xa2ffffff");
      ConstraintSet subtrahend("0x80001018-0x8000101b,0x842f4464-0x842f4467,0x98b2e5f8-0x98b2e5fb");

      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "0x50000040-0x7fffffff,0x8000101c-0x842f4463,0x842f4468-0x98b2e5f7,0x98b2e5fc-0x9bffffff,0x9e000000-0xa2ffffff");
      EXPECT(minuend.Size() == minuend.CalculateSize());
    }

    SECTION("Subtraction removes an item in the middle.") {
      ConstraintSet minuend("0x36fdcf92-0x37454cff,0x37454d40-0x37454d4f,0x37454d60-0x37454d9f,0x37454db0-0x37454dbf,0x37454dc4-0x37454dc7,0x39eab678-0x39f49234");
      ConstraintSet subtrahend("0x37454d40-0x37454d5f,0x39eab678-0x39eab67b");

      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "0x36fdcf92-0x37454cff,0x37454d60-0x37454d9f,0x37454db0-0x37454dbf,0x37454dc4-0x37454dc7,0x39eab67c-0x39f49234");
      EXPECT(minuend.Size() == minuend.CalculateSize());
    }

    SECTION("Subtraction test for new placement algorithm.") {
      ConstraintSet minuend("0x80001000-0x9bffffff,0x9e000000-0xa2ffffff,0xa3020001-0xfdffffff,0xfe120001-0xf8c8ed4bff,0xf8c8ed4e00-0xfffeffffffff,0xffff00010000-0xffffffffffff");
      ConstraintSet subtrahend("0xf8c8ed4c00-0xf8c8ed4c07,0x1c29a2c0000-0x1c29c2bffff");
      minuend.GetConstraintsMutable().reserve(7); // reserved enough space so it will use the Expand method to build ConstraintSet.

      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "0x80001000-0x9bffffff,0x9e000000-0xa2ffffff,0xa3020001-0xfdffffff,0xfe120001-0xf8c8ed4bff,0xf8c8ed4e00-0x1c29a2bffff,0x1c29c2c0000-0xfffeffffffff,0xffff00010000-0xffffffffffff");
      EXPECT(minuend.Size() == minuend.CalculateSize());
      EXPECT(minuend.VectorSize() == 7u);
    }

    SECTION("Subtraction test for new placement algorithm, case when result is empty.") {
      ConstraintSet minuend("0xf8c8ed0000-0xf8c8edffff,0x1c29b290000-0x1c29b2bffff");
      ConstraintSet subtrahend("0xf8c8ed0000-0xf8c8edffff,0x1c29b290000-0x1c29b2bffff");

      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "");
      EXPECT(minuend.Size() == minuend.CalculateSize());
    }

    SECTION("Subtraction test for new placement algorithm, two insertions.") {
      ConstraintSet minuend("0x7e23adebf4-0x7e23aded13,0x7e23adebf4-0x7e23aded13,0x7e23aded34-0x7e23adedcf,0x7e23adedd1-0x7e23adedd3,0x7e2448d94a-0x7e2477c7af,0x7e2477c7b4-0x7e2477c92f,0x7e26e3f760-0x7e26e3f89f,0x7e272a6ffb-0x7e2730e0f7,0x7e2730e158-0x7e2735b3eb,0x7e2735b3ee-0x7e273895ff");
      ConstraintSet subtrahend("0x7e23adedd0-0x7e23adedd3,0x7e2477c7ce-0x7e2477c7cf,0x7e27306b90-0x7e27306b93,0x13321c20b98-0x13321c20b9f");
      minuend.GetConstraintsMutable().reserve(10); // reserved enough space so it will use the Expand method to build ConstraintSet.
 
      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "0x7e23adebf4-0x7e23aded13,0x7e23aded34-0x7e23adedcf,0x7e2448d94a-0x7e2477c7af,0x7e2477c7b4-0x7e2477c7cd,0x7e2477c7d0-0x7e2477c92f,0x7e26e3f760-0x7e26e3f89f,0x7e272a6ffb-0x7e27306b8f,0x7e27306b94-0x7e2730e0f7,0x7e2730e158-0x7e2735b3eb,0x7e2735b3ee-0x7e273895ff");
      EXPECT(minuend.Size() == minuend.CalculateSize());
      EXPECT(minuend.VectorSize() == 10u);
    }

    SECTION("Subtraction test for new placement algorithm, last item is deleted") {
      ConstraintSet minuend("0x1-0x5,0x7-0x9,0xb-0xd,0xf");
      ConstraintSet subtrahend("0x3-0x4,0xb,0xf");
 
      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "0x1-0x2,0x5,0x7-0x9,0xc-0xd");
      EXPECT(minuend.Size() == minuend.CalculateSize());
    }

    SECTION("Subtraction test for new placement algorithm, first item is deleted") {
      ConstraintSet minuend("0x1,0x3-0x5,0x7-0xf,0x1f-0x2f");
      ConstraintSet subtrahend("0x1,0x5,0x9,0x2c-0x2d");
      minuend.GetConstraintsMutable().reserve(5); // reserved enough space so it will use the Expand method to build ConstraintSet.

      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "0x3-0x4,0x7-0x8,0xa-0xf,0x1f-0x2b,0x2e-0x2f");
      EXPECT(minuend.Size() == minuend.CalculateSize());
      EXPECT(minuend.VectorSize() == 5u);
    }

    SECTION("Subtraction test with minuend range partly overlapped by subtrahend range and value") {
      ConstraintSet minuend("0x16-0x18,0x1a-0x1f");
      ConstraintSet subtrahend("0x17-0x1a,0x1f");

      minuend.SubConstraintSet(subtrahend);
      EXPECT(minuend.ToSimpleString() == "0x16,0x1b-0x1e");
      EXPECT(minuend.Size() == minuend.CalculateSize());
      EXPECT(minuend.VectorSize() == 2u);
    }

  }

}

CASE("test set 5.3 for ConstraintSet::MergeConstraintSet") {

  SETUP("setup ConstraintSet")  {

    ConstraintSet my_constr_set;

    SECTION("Test 1 on improved MergeConstraintSet") {
      ConstraintSet target_constr("0x34cf3c60-0x34cf3c67,0x34d1d3f0-0x34d1d3f1,0x34d1d3f3-0x34d1d412,0x34d2018a-0x34d2018b,0x34ec4180-0x34ec419f");
      ConstraintSet merge_constr("0x34d1d3f0-0x34d1d3f3,0x3f09abc0-0x3f09abdf,0x74d1d3f0-0x74d1d3f3");
      target_constr.MergeConstraintSet(merge_constr);
      
      EXPECT(target_constr.ToSimpleString() == "0x34cf3c60-0x34cf3c67,0x34d1d3f0-0x34d1d412,0x34d2018a-0x34d2018b,0x34ec4180-0x34ec419f,0x3f09abc0-0x3f09abdf,0x74d1d3f0-0x74d1d3f3");
      EXPECT(target_constr.Size() == target_constr.CalculateSize());
    }

    SECTION("Test 2 on improved MergeConstraintSet") {
      ConstraintSet target_constr("0xda86c861304-0xda86c8644eb,0xda86c8644f0-0xda86c8681ff,0xda86c868204-0xda86c869b9f,0xda86c869ba4-0x10047fffffff,0x1004a0000000-0xffffffffffff");
      ConstraintSet merge_constr("0x10048000101c-0x10049bffffff,0x10049e000000-0x10049fffffff");
      target_constr.MergeConstraintSet(merge_constr);
      
      EXPECT(target_constr.ToSimpleString() == "0xda86c861304-0xda86c8644eb,0xda86c8644f0-0xda86c8681ff,0xda86c868204-0xda86c869b9f,0xda86c869ba4-0x10047fffffff,0x10048000101c-0x10049bffffff,0x10049e000000-0xffffffffffff");
      EXPECT(target_constr.Size() == target_constr.CalculateSize());
    }

    SECTION("Test 3 on improved++ MergeConstraintSet with new placement algorithm") {
      ConstraintSet target_constr("0xf8c8ed4c00-0xf8c8ed4dff,0x1c29b260000-0x1c29b2bffff");
      ConstraintSet merge_constr("0x50000000-0x5000ffff,0x80000000-0x8000ffff,0x5c879c0000-0x5c879cffff,0xf8c8ed0000-0xf8c8edffff,0x1c29b260000-0x1c29b2bffff");
      target_constr.GetConstraintsMutable().reserve(5); // reserved enough space so it will use the Expand method to build ConstraintSet.
      target_constr.MergeConstraintSet(merge_constr);

      EXPECT(target_constr.ToSimpleString() == "0x50000000-0x5000ffff,0x80000000-0x8000ffff,0x5c879c0000-0x5c879cffff,0xf8c8ed0000-0xf8c8edffff,0x1c29b260000-0x1c29b2bffff");
      EXPECT(target_constr.Size() == target_constr.CalculateSize());
      EXPECT(target_constr.VectorSize() == 5u);
    }

    SECTION("Test 4 on improved++ MergeConstraintSet with new placement algorithm") {
      ConstraintSet target_constr("0x0-0xffff,0x2490000-0x249ffff,0x1b540000-0x1b54ffff,0x56ec0000-0x56ecffff,0xa0000000-0xdfffffff,0xf8c8ed4c00-0xf8c8ed4dff,0x13321d60000-0x13321dbffff,0x1c29b1b0000-0x1c29b2bffff,0x24257ca0000-0x24257caffff,0x242591d0000-0x242591dffff,0x24260000000-0x2427fffffff,0x242c0000000-0x242dfffffff,0x2fbe20000000-0x2fbe3fffffff,0x534f7ac30000-0x534f7ac3ffff,0x534f932a0000-0x534f932affff,0x6c8d87db0000-0x6c8d87dbffff,0xb71ca0000000-0xb71cbfffffff,0xbadfa0000000-0xbadfbfffffff,0xdf3a41bd0000-0xdf3a41bdffff,0xe3a67d4f0000-0xe3a67d4fffff");
      ConstraintSet merge_constr("0x2490000-0x249ffff,0xb320000000-0xb33fffffff,0x13321d60000-0x13321dbffff,0x242c0000000-0x242dfffffff,0x6c8d80000000-0x6c8d9fffffff,0xdc0000000000-0xe3ffffffffff");
      target_constr.MergeConstraintSet(merge_constr);

      EXPECT(target_constr.ToSimpleString() == "0x0-0xffff,0x2490000-0x249ffff,0x1b540000-0x1b54ffff,0x56ec0000-0x56ecffff,0xa0000000-0xdfffffff,0xb320000000-0xb33fffffff,0xf8c8ed4c00-0xf8c8ed4dff,0x13321d60000-0x13321dbffff,0x1c29b1b0000-0x1c29b2bffff,0x24257ca0000-0x24257caffff,0x242591d0000-0x242591dffff,0x24260000000-0x2427fffffff,0x242c0000000-0x242dfffffff,0x2fbe20000000-0x2fbe3fffffff,0x534f7ac30000-0x534f7ac3ffff,0x534f932a0000-0x534f932affff,0x6c8d80000000-0x6c8d9fffffff,0xb71ca0000000-0xb71cbfffffff,0xbadfa0000000-0xbadfbfffffff,0xdc0000000000-0xe3ffffffffff");
      EXPECT(target_constr.Size() == target_constr.CalculateSize());
    }

    SECTION("Test 5 on improved++ MergeConstraintSet with new placement algorithm") {
      ConstraintSet target_constr("0x0-0x1fffff,0x30c51000-0x30c51fff,0x252c34e000-0x252c34efff,0x6f366ac000-0x6f366b1fff,0x86112de000-0x86112f3fff,0x259e96000000-0x259e961fffff,0x4e2c1d400000-0x4e2c1d5fffff,0xa8f233518000-0xa8f233518fff,0xb382ce800000-0xb382ce9fffff");
      ConstraintSet merge_constr("0x0-0x3fffffff,0x6f00000000-0x6f3fffffff,0xe3c0000000-0xe3ffffffff,0xf980000000-0xf9bfffffff,0x4e2c1d400000-0x4e2c1d5fffff");
      target_constr.MergeConstraintSet(merge_constr);
      target_constr.GetConstraintsMutable().reserve(10); // reserved enough space so it will use the Expand method to build ConstraintSet.

      EXPECT(target_constr.ToSimpleString() == "0x0-0x3fffffff,0x252c34e000-0x252c34efff,0x6f00000000-0x6f3fffffff,0x86112de000-0x86112f3fff,0xe3c0000000-0xe3ffffffff,0xf980000000-0xf9bfffffff,0x259e96000000-0x259e961fffff,0x4e2c1d400000-0x4e2c1d5fffff,0xa8f233518000-0xa8f233518fff,0xb382ce800000-0xb382ce9fffff");
      EXPECT(target_constr.Size() == target_constr.CalculateSize());
      EXPECT(target_constr.VectorSize() == 10u);
    }

  }

}

