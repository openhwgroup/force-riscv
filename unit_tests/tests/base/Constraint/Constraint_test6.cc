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
#include <ConstraintUtils.h>
#include <GenException.h>

using text = std::string;
using namespace std;
using namespace Force;

#undef CASE
#define CASE( name ) lest_CASE( specification(), name )

extern lest::tests & specification();

void test_clone(lest::env& lest_env, const Constraint& constr)
{
  unique_ptr<Constraint> clone_constr(constr.Clone());
  EXPECT(clone_constr.get() != &constr);
  EXPECT(clone_constr->LowerBound() == constr.LowerBound());
  EXPECT(clone_constr->UpperBound() == constr.UpperBound());
}

CASE("test set 7 for Constraint module") {

  SETUP("setup ValueConstraint")  {
    ValueConstraint my_val_constr(0x50DF);

    SECTION("test ValueConstraint::Clone method") {
      test_clone(lest_env, my_val_constr);
    }

    SECTION("test ValueConstraint::Contains(uint64) method") {
      EXPECT(my_val_constr.Contains(0x50DF));
    }

    SECTION("test ValueConstraint::Contains(uint64) method with inequal value") {
      EXPECT_NOT(my_val_constr.Contains(0x6F01));
    }

    SECTION("test ValueConstraint::Contains(Constraint) method") {
      ValueConstraint val_constr(0x50DF);
      EXPECT(my_val_constr.Contains(val_constr));
    }

    SECTION("test ValueConstraint::Contains(Constraint) method with inequal value") {
      ValueConstraint val_constr(0x50E0);
      EXPECT_NOT(my_val_constr.Contains(val_constr));
    }

    SECTION("test ValueConstraint::Contains(Constraint) method with RangeConstraint") {
      RangeConstraint range_constr(0x50DF, 0x50E0);
      EXPECT_NOT(my_val_constr.Contains(range_constr));
    }

    SECTION("test ValueConstraint::GetIntersection method") {
      ValueConstraint val_constr(0x50DF);
      unique_ptr<Constraint> inter_constr(my_val_constr.GetIntersection(val_constr));
      EXPECT(inter_constr->LowerBound() == 0x50DFu);
      EXPECT(inter_constr->UpperBound() == 0x50DFu);
    }

    SECTION("test ValueConstraint::GetIntersection method with inequal value") {
      ValueConstraint val_constr(0x6F01);
      EXPECT_FAIL(my_val_constr.GetIntersection(val_constr), "no-intersection-found");
    }

    SECTION("test ValueConstraint::GetValues method") {
      vector<uint64> value_vec;
      my_val_constr.GetValues(value_vec);
      EXPECT(value_vec.size() == 1u);
      EXPECT(value_vec.front() == 0x50DFu);
    }

    SECTION("test ValueConstraing::ToSimpleString method") {
      EXPECT(my_val_constr.ToSimpleString() == "0x50df");
    }

    SECTION("test ValueConstraint::ToString method") {
      EXPECT(my_val_constr.ToString() == "Value:0x50df");
    }

    SECTION("test ValueConstraint::operator== method") {
      ValueConstraint val_constr(0x50df);
      EXPECT(my_val_constr == val_constr);
    }

    SECTION("test ValueConstraint::operator== method with range constraint") {
      RangeConstraint range_constr(0x50df, 0x50e0);
      EXPECT_NOT(my_val_constr == range_constr);
    }

    SECTION("test ValueConstraint::operator!= method") {
      ValueConstraint val_constr(0x213);
      EXPECT(my_val_constr != val_constr);
    }

    SECTION("test ValueConstraint::Value method") {
      EXPECT(my_val_constr.Value() == 0x50DFu);
    }

    SECTION("test ValueConstraint::SetValue method") {
      my_val_constr.SetValue(0x1C);
      EXPECT(my_val_constr.Value() == 0x1Cu);
    }
  }
}

CASE("test set 8 for Constraint module") {

  SETUP("setup RangeConstraint")  {
    RangeConstraint my_range_constr(0x4321, 0x6F00);

    SECTION("test ConstraintSet::Clone method") {
      test_clone(lest_env, my_range_constr);
    }

    SECTION("test RangeConstraint::Contains(uint64) method") {
      EXPECT(my_range_constr.Contains(0x50DF));
    }

    SECTION("test RangeConstraint::Contains(uint64) method with value above range") {
      EXPECT_NOT(my_range_constr.Contains(0x6F01));
    }

    SECTION("test RangeConstraint::Contains(uint64) method with value below range") {
      EXPECT_NOT(my_range_constr.Contains(0x39FF));
    }

    SECTION("test RangeConstraint::Contains(Constraint) method") {
      ValueConstraint val_constr(0x50DF);
      EXPECT(my_range_constr.Contains(val_constr));
    }

    SECTION("test RangeConstraint::Contains(Constraint) method with value above range") {
      ValueConstraint val_constr(0x6F01);
      EXPECT_NOT(my_range_constr.Contains(val_constr));
    }

    SECTION("test RangeConstraint::Contains(Constraint) method with value below range") {
      ValueConstraint val_constr(0x39FF);
      EXPECT_NOT(my_range_constr.Contains(val_constr));
    }

    SECTION("test RangeConstraint::Contains(Constraint) method with range constraint") {
      RangeConstraint range_constr(0x50DF, 0x6E32);
      EXPECT(my_range_constr.Contains(range_constr));
    }

    SECTION("test RangeConstraint::Contains(Constraint) method with range constraint above range") {
      RangeConstraint range_constr(0x6F01, 0x6F02);
      EXPECT_NOT(my_range_constr.Contains(range_constr));
    }

    SECTION("test RangeConstraint::Contains(Constraint) method with range constraint below range") {
      RangeConstraint range_constr(0x39FF, 0x4000);
      EXPECT_NOT(my_range_constr.Contains(range_constr));
    }

    SECTION("test RangeConstraint::Contains(Constraint) method with range constraint overlapping above range") {
      RangeConstraint range_constr(0x6A00, 0x73EE);
      EXPECT_NOT(my_range_constr.Contains(range_constr));
    }

    SECTION("test RangeConstraint::Contains(Constraint) method with range constraint overlapping below range") {
      RangeConstraint range_constr(0x39FF, 0x50DF);
      EXPECT_NOT(my_range_constr.Contains(range_constr));
    }

    SECTION("test RangeConstraint::Contains(Constraint) method with range constraint encompassing range") {
      RangeConstraint range_constr(0x4320, 0x6F01);
      EXPECT_NOT(my_range_constr.Contains(range_constr));
    }

    SECTION("test RangeConstraint::GetIntersection method") {
      RangeConstraint range_constr(0x6A00, 0x73EE);
      unique_ptr<Constraint> inter_constr(my_range_constr.GetIntersection(range_constr));
      EXPECT(inter_constr->LowerBound() == 0x6A00u);
      EXPECT(inter_constr->UpperBound() == 0x6F00u);
    }

    SECTION("test RangeConstraint::GetIntersection method with value constraint") {
      ValueConstraint val_constr(0x5BEF);
      unique_ptr<Constraint> inter_constr(my_range_constr.GetIntersection(val_constr));
      EXPECT(inter_constr->LowerBound() == 0x5BEFu);
      EXPECT(inter_constr->UpperBound() == 0x5BEFu);
    }

    SECTION("test RangeConstraint::GetIntersection method with value constraint out of range") {
      ValueConstraint val_constr(0x1000);
      EXPECT_FAIL(my_range_constr.GetIntersection(val_constr), "no-intersection-found");
    }

    SECTION("test RangeConstraint::GetIntersection method with enclosing range") {
      RangeConstraint range_constr(0x4000, 0x7000);
      unique_ptr<Constraint> inter_constr(my_range_constr.GetIntersection(range_constr));
      EXPECT(inter_constr->LowerBound() == 0x4321u);
      EXPECT(inter_constr->UpperBound() == 0x6F00u);
    }

    SECTION("test RangeConstraint::GetIntersection method with enclosed range") {
      RangeConstraint range_constr(0x5000, 0x6000);
      unique_ptr<Constraint> inter_constr(my_range_constr.GetIntersection(range_constr));
      EXPECT(inter_constr->LowerBound() == 0x5000u);
      EXPECT(inter_constr->UpperBound() == 0x6000u);
    }

    SECTION("test RangeConstraint::GetIntersection method with value result") {
      RangeConstraint range_constr(0x6F00, 0x6FEE);
      unique_ptr<Constraint> inter_constr(my_range_constr.GetIntersection(range_constr));
      EXPECT(inter_constr->LowerBound() == 0x6F00u);
      EXPECT(inter_constr->UpperBound() == 0x6F00u);
    }

    SECTION("test RangeConstraint::GetIntersection method with no intersection") {
      RangeConstraint range_constr(0x7000, 0x7FFF);
      EXPECT_FAIL(my_range_constr.GetIntersection(range_constr), "no-intersection-found");
    }

    SECTION("test RangeConstraint::GetValues method") {
      vector<uint64> value_vec;
      my_range_constr.GetValues(value_vec);
      EXPECT(value_vec.size() == 11232u);
      EXPECT(value_vec.front() == 0x4321u);
      EXPECT(value_vec.at(4) == 0x4325u);
      EXPECT(value_vec.back() == 0x6F00u);
    }

    SECTION("test RangeConstraint::SetLowerBound method") {
      my_range_constr.SetLowerBound(0x4111);
      EXPECT(my_range_constr.LowerBound() == 0x4111u);
    }

    SECTION("test RangeConstraint::SetUpperBound method") {
      my_range_constr.SetUpperBound(0x5333);
      EXPECT(my_range_constr.UpperBound() == 0x5333u);
    }

    SECTION("test RangeConstraint::ToSimpleString method") {
      EXPECT(my_range_constr.ToSimpleString() == "0x4321-0x6f00");
    }

    SECTION("test RangeConstraint::ToString method") {
      EXPECT(my_range_constr.ToString() == "Range:0x4321-0x6f00");
    }

    SECTION("test RangeConstraint::operator== method") {
      RangeConstraint range_constr(0x4321, 0x6F00);
      EXPECT(my_range_constr == range_constr);
    }

    SECTION("test RangeConstraint::operator== method with value constraint") {
      ValueConstraint val_constr(0x6F00);
      EXPECT_NOT(my_range_constr == val_constr);
    }

    SECTION("test RangeConstraint::operator!= method") {
      RangeConstraint range_constr(0x4321, 0x6EFF);
      EXPECT(my_range_constr != range_constr);
    }
  }
}

CASE("test set 9 for Constraint module, testing Intersects method") {

  SETUP("setup ValueConstraint and RangeConstraint")  {
    ValueConstraint val_constr1(0x7fff);
    ValueConstraint val_constr2(0x8300);
    ValueConstraint val_constr4(0x7ffe);
    RangeConstraint range_constr1(0x7fff, 0x82ff);

    SECTION("test ValueConstraint::Intersects method") {
      ValueConstraint val_constr3(0x7fff);

      EXPECT(val_constr1.Intersects(val_constr2) == false); // 0x7fff NOT intersects 0x8300
      EXPECT(val_constr1.Intersects(val_constr3) == true); // 0x7fff intersects 0x7fff
      EXPECT(val_constr1.Intersects(range_constr1) == true); // 0x7fff intersects 0x7fff-0x82ff
      EXPECT(val_constr2.Intersects(range_constr1) == false); // 0x8300 NOT intersects 0x7fff-0x82ff
      EXPECT(val_constr4.Intersects(range_constr1) == false); // 0x7ffe NOT intersects 0x7fff-0x82ff
    }

    SECTION("test RangeConstraint::Intersects method") {
      RangeConstraint range_constr2(0x8300, 0x83ff);
      RangeConstraint range_constr3(0x0, 0x7ffe);
      RangeConstraint range_constr4(0x4000, 0xffffffffffffffull);
      RangeConstraint range_constr5(0xffffffffff0000ull, 0xfffffffffffffffull);

      EXPECT(range_constr1.Intersects(val_constr1) == true); // 0x7fff-0x82ff intersects 0x7fff
      EXPECT(range_constr1.Intersects(val_constr2) == false); // 0x7fff-0x82ff NOT intersects 0x8300
      EXPECT(range_constr1.Intersects(val_constr4) == false); // 0x7fff-0x82ff NOT intersects 0x7ffe
      EXPECT(range_constr1.Intersects(range_constr2) == false); // 0x7fff-0x82ff NOT intersects 0x8300-83ff
      EXPECT(range_constr1.Intersects(range_constr3) == false); // 0x7fff-0x82ff NOT intersects 0x0-0x7ffe
      EXPECT(range_constr1.Intersects(range_constr4) == true); // 0x7fff-0x82ff intersects 0x4000-0xffffffffffffffull
      EXPECT(range_constr4.Intersects(range_constr1) == true); // 0x4000-0xffffffffffffffull intersects 0x7fff-0x82ff
      EXPECT(range_constr4.Intersects(range_constr5) == true); // 0x4000-0xffffffffffffffull intersects 0xffffffffff0000ull-0xfffffffffffffffull
      EXPECT(range_constr5.Intersects(range_constr4) == true); // 0xffffffffff0000ull-0xfffffffffffffffull intersects 0x4000-0xffffffffffffffull
    }
  }

  SETUP("setup ConstraintSet")  {
    ConstraintSet constr_set1("0x1000-0x3000,0x8000-0x8fff,0xa000-0xafff");
    ConstraintSet constr_set2("0x3001-0x301f,0x9000-0x9fff,0xb000-0xbfff");
    ConstraintSet constr_set3("0x3000");
    ConstraintSet constr_set4("0x3020-0x8fff");
    ConstraintSet constr_set5("0x3020-0x7fff,0x800f,0x9000-0x9fff");

    SECTION("test ConstraintSet::Intersects method") {
      EXPECT(constr_set1.Intersects(constr_set2) == false);
      EXPECT(constr_set1.Intersects(constr_set3) == true);
      EXPECT(constr_set3.Intersects(constr_set1) == true);
      EXPECT(constr_set2.Intersects(constr_set4) == false);
      EXPECT(constr_set4.Intersects(constr_set2) == false);
      EXPECT(constr_set1.Intersects(constr_set4) == true);
      EXPECT(constr_set4.Intersects(constr_set1) == true);
      EXPECT(constr_set1.Intersects(constr_set5) == true);
      EXPECT(constr_set5.Intersects(constr_set1) == true);
    }
  }
}

CASE("test set 10 for Constraint module, testing AlignMulDataWithSize") {
  SETUP("setup ValueConstraint and RangeConstraint") {
    ConstraintSet my_constr_set("15-109,206-308,400-639,65535-75534,855535-855582,984325");
    EXPECT(my_constr_set.ToSimpleString() == "0xf-0x6d,0xce-0x134,0x190-0x27f,0xffff-0x1270e,0xd0def-0xd0e1e,0xf0505");
    EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());

    SECTION("test ValueConstaint::AlignMulDataWithSize method") {
      my_constr_set.AlignMulDataWithSize(48,47,48);
      EXPECT(my_constr_set.ToSimpleString() == "0x2f,0xef,0x1af-0x23f,0x1001f-0x126bf");
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }

    SECTION("test RangeConstaint::AlignMulDataWithSize method") {
      EXPECT(my_constr_set.ToSimpleString() == "0xf-0x6d,0xce-0x134,0x190-0x27f,0xffff-0x1270e,0xd0def-0xd0e1e,0xf0505");
      my_constr_set.AlignMulDataWithSize(96,1,96);
      EXPECT(my_constr_set.ToSimpleString() == "0x1e1,0x10021-0x12661");
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());
    }
    SECTION("test AlignMulDataWithSize method with 0 muldata") {
      EXPECT_FAIL(my_constr_set.AlignMulDataWithSize(0,95,96), "invalid-zero-size");
      EXPECT_FAIL(my_constr_set.AlignMulDataWithSize(96,95,0), "invalid-zero-size");
    }
  }
}

CASE("test set 11 for ConstraintSet module, testing DivideElementsWithFactor") {
  SETUP("setup ConstraintSet") {
    ConstraintSet my_constr_set("15-109,206-308,400-639,65535-75534,855535,984325");
    EXPECT(my_constr_set.ToSimpleString() == "0xf-0x6d,0xce-0x134,0x190-0x27f,0xffff-0x1270e,0xd0def,0xf0505");
    EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());

    SECTION("test ConstraintSet::DivideElementsWithFactor method") {
      ConstraintSet::msConstraintDeleteCount = 0;
      my_constr_set.DivideElementsWithFactor(48,0);
      EXPECT(my_constr_set.ToSimpleString() == "0x1-0x2,0x5-0x6,0x9-0xd,0x556-0x625");
      EXPECT(my_constr_set.CalculateSize() == 217u );
      EXPECT(my_constr_set.CalculateSize() == my_constr_set.Size());

    }
  }
}

CASE("test set 12 for Constraint module, testing MergeConstraint") {
  SETUP("setup ValueConstraint and RangeConstraint") {
    ValueConstraint my_val_constr(0x50DF);
    RangeConstraint my_range_constr(0x4321, 0x6F00);
    ConstraintOneResult my_res;

    SECTION("test ValueConstraint::MergeConstraint with same value") {
      ValueConstraint* val_constr = new ValueConstraint(0x50DF);
      my_val_constr.MergeConstraint(val_constr, my_res);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
    }

    SECTION("test ValueConstraint::MergeConstraint with adjacent value below") {
      ValueConstraint* val_constr = new ValueConstraint(0x50DE);
      my_val_constr.MergeConstraint(val_constr, my_res);
      EXPECT(my_res.mType == EConstraintResultType::Replace);
      EXPECT(my_res.mpConstraint->LowerBound() == 0x50DEu);
      EXPECT(my_res.mpConstraint->UpperBound() == 0x50DFu);
    }

    SECTION("test ValueConstraint::MergeConstraint with adjacent value above") {
      ValueConstraint* val_constr = new ValueConstraint(0x50E0);
      my_val_constr.MergeConstraint(val_constr, my_res);
      EXPECT(my_res.mType == EConstraintResultType::Replace);
      EXPECT(my_res.mpConstraint->LowerBound() == 0x50DFu);
      EXPECT(my_res.mpConstraint->UpperBound() == 0x50E0u);
    }

    SECTION("test ValueConstraint::MergeConstraint with range encompassing value") {
      RangeConstraint* range_constr = new RangeConstraint(0x4321, 0x6F00);
      uint64 expected_size_change = range_constr->Size() - 1;
      my_val_constr.MergeConstraint(range_constr, my_res);
      EXPECT(my_res.mType == EConstraintResultType::Replace);
      EXPECT(my_res.mpConstraint->LowerBound() == 0x4321u);
      EXPECT(my_res.mpConstraint->UpperBound() == 0x6F00u);
      EXPECT(my_res.mSizeChange == expected_size_change);
    }

    SECTION("test ValueConstraint::MergeConstraint with range adjacent below") {
      RangeConstraint* range_constr = new RangeConstraint(0x4321, 0x50DE);
      uint64 expected_size = range_constr->Size();
      my_val_constr.MergeConstraint(range_constr, my_res);
      EXPECT(my_res.mType == EConstraintResultType::Replace);
      EXPECT(my_res.mpConstraint->LowerBound() == 0x4321u);
      EXPECT(my_res.mpConstraint->UpperBound() == 0x50DFu);
      EXPECT(my_res.mSizeChange == expected_size);
    }

    SECTION("test ValueConstraint::MergeConstraint with range adjacent above") {
      RangeConstraint* range_constr = new RangeConstraint(0x50E0, 0x6F00);
      uint64 expected_size = range_constr->Size();
      my_val_constr.MergeConstraint(range_constr, my_res);
      EXPECT(my_res.mType == EConstraintResultType::Replace);
      EXPECT(my_res.mpConstraint->LowerBound() == 0x50DFu);
      EXPECT(my_res.mpConstraint->UpperBound() == 0x6F00u);
      EXPECT(my_res.mSizeChange == expected_size);
    }

    SECTION("test RangeConstraint::MergeConstraint with value inside") {
      ValueConstraint* val_constr = new ValueConstraint(0x50DF);
      my_range_constr.MergeConstraint(val_constr, my_res);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
    }

    SECTION("test RangeConstraint::MergeConstraint with value adjacent below") {
      ValueConstraint* val_constr = new ValueConstraint(0x4320);
      my_range_constr.MergeConstraint(val_constr, my_res);
      EXPECT(my_range_constr.LowerBound() == 0x4320u);
      EXPECT(my_range_constr.UpperBound() == 0x6F00u);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
      EXPECT(my_res.mSizeChange == 1u); // One value was added
    }

    SECTION("test RangeConstraint::MergeConstraint with value adjacent above") {
      ValueConstraint* val_constr = new ValueConstraint(0x6F01);
      my_range_constr.MergeConstraint(val_constr, my_res);
      EXPECT(my_range_constr.LowerBound() == 0x4321u);
      EXPECT(my_range_constr.UpperBound() == 0x6F01u);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
      EXPECT(my_res.mSizeChange == 1u); // One value was added
    }

    SECTION("test RangeConstraint::MergeConstraint with range inside (not overlapping)") {
      RangeConstraint* range_constr = new RangeConstraint(0x50DE, 0x50DF);
      my_range_constr.MergeConstraint(range_constr, my_res);
      EXPECT(my_range_constr.LowerBound() == 0x4321u);
      EXPECT(my_range_constr.UpperBound() == 0x6F00u);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
      EXPECT(my_res.mSizeChange == 0u); // No values were added
    }

    SECTION("test RangeConstraint::MergeConstraint with range completely overlapping") {
      RangeConstraint* range_constr = new RangeConstraint(0x4320, 0x6F01);
      my_range_constr.MergeConstraint(range_constr, my_res);
      EXPECT(my_range_constr.LowerBound() == 0x4320u);
      EXPECT(my_range_constr.UpperBound() == 0x6F01u);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
      EXPECT(my_res.mSizeChange == 2u); // Two values were added
    }

    SECTION("test RangeConstraint::MergeConstraint with range overlapping below") {
      RangeConstraint* range_constr = new RangeConstraint(0x4320, 0x50DF);
      my_range_constr.MergeConstraint(range_constr, my_res);
      EXPECT(my_range_constr.LowerBound() == 0x4320u);
      EXPECT(my_range_constr.UpperBound() == 0x6F00u);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
      EXPECT(my_res.mSizeChange == 1u); // One value was added
    }

    SECTION("test RangeConstraint::MergeConstraint with range overlapping above") {
      RangeConstraint* range_constr = new RangeConstraint(0x50DF, 0x6F01);
      my_range_constr.MergeConstraint(range_constr, my_res);
      EXPECT(my_range_constr.LowerBound() == 0x4321u);
      EXPECT(my_range_constr.UpperBound() == 0x6F01u);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
      EXPECT(my_res.mSizeChange == 1u); // One value was added
    }

    SECTION("test RangeConstraint::MergeConstraint with range adjacent below") {
      RangeConstraint* range_constr = new RangeConstraint(0x431F, 0x4320);
      my_range_constr.MergeConstraint(range_constr, my_res);
      EXPECT(my_range_constr.LowerBound() == 0x431Fu);
      EXPECT(my_range_constr.UpperBound() == 0x6F00u);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
      EXPECT(my_res.mSizeChange == 2u); // Two values were added
    }

    SECTION("test RangeConstraint::MergeConstraint with range adjacent above") {
      RangeConstraint* range_constr = new RangeConstraint(0x6F01, 0x6F02);
      my_range_constr.MergeConstraint(range_constr, my_res);
      EXPECT(my_range_constr.LowerBound() == 0x4321u);
      EXPECT(my_range_constr.UpperBound() == 0x6F02u);
      EXPECT(my_res.mType == EConstraintResultType::Consumed);
      EXPECT(my_res.mSizeChange == 2u); // Two values were added
    }
  }
}

