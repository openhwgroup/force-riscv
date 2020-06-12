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
#include <UtilityFunctions.h>
#include <GenException.h>
#include <Log.h>

using text = std::string;
using namespace std;
using namespace Force;

#undef CASE
#define CASE( name ) lest_CASE( specification(), name )

extern lest::tests & specification();

CASE("ValueConstraint::AlignWithPage")
{
  uint64 page_mask = 0u;
  uint64 value      = 0x7FFFFFFFFFFFFFFFull;
  ConstraintSet val_set;
  for (page_mask = 0x1ull; page_mask < (~0x0ull); page_mask = (page_mask << 0x1u) + 0x1u)
  {
    val_set.AddValue(value);
    uint32 mask_size = get_mask64_size(page_mask);
    uint64 aligned_val = (value & (~page_mask)) >> mask_size;
    val_set.AlignWithPage(~page_mask);
    EXPECT(aligned_val == val_set.LowerBound());
    EXPECT(aligned_val == val_set.UpperBound());
    val_set.Clear();
  }
}

CASE("RangeConstraint::AlignWithPage")
{
  uint64 page_mask = 0u;
  uint64 low_value  = 0x0555555555555555ull;
  uint64 high_value = 0x0AAAAAAAAAAAAAAAull;
  ConstraintSet val_set;
  for (page_mask = 0x1ull; page_mask < (~0x0ull); page_mask = (page_mask << 0x1u) + 0x1u)
  {
    val_set.AddRange(low_value, high_value);
    uint32 mask_size = get_mask64_size(page_mask);
    uint64 aligned_low_val  = (low_value & (~page_mask)) >> mask_size;
    uint64 aligned_high_val = ((high_value & (~page_mask)) + (page_mask)) >> mask_size;
    val_set.AlignWithPage(~page_mask);
    if (!val_set.IsEmpty())
    {
      EXPECT(aligned_low_val  == val_set.LowerBound());
      EXPECT(aligned_high_val == val_set.UpperBound());
    }
    val_set.Clear();
  }
}

CASE("ConstraintSet::AlignWithPage")
{
  SETUP("Setup unaligned set to be run with multiple page sizes")
  {
    ConstraintSet val_set;

    val_set.AddRange(0x1000ull, 0x2000ull);
    val_set.AddValue(0x4000ull);
    val_set.AddRange(0x8000ull, 0x10000ull);
    val_set.AddRange(0x100000000ull, 0x200000000ull);
    val_set.AddValue(0x300000000ull);
    val_set.AddRange(0x500000000ull, 0x500001000ull);
    val_set.AddValue(0x700000000ull);
    val_set.AddRange(0x700001000ull, 0x800000000ull);
    val_set.AddValue(0x800002000ull);
    val_set.AddRange(0xA00000000ull, 0xA00001000ull);
    val_set.AddRange(0xA00002000ull, 0xB00000000ull);
    val_set.AddRange(0xC00002000ull, 0xD00000000ull);
    val_set.AddRange(0xF00000000ull, 0xF00001000ull);
    val_set.AddRange(0xF00002000ull, 0xF00004000ull);
    val_set.AddValue(0x1100001000ull);
    val_set.AddValue(0x1100002000ull);
    val_set.AddValue(0x1200002000ull);
    val_set.AddValue(0x1400000000ull);
    val_set.AddRange(0x1500002000ull, 0x1500004000ull);
    val_set.AddRange(0x1800002000ull, 0x1800004000ull);
    val_set.AddValue(0x1900004000ull);
    val_set.AddRange(0x1B00002000ull, 0x1B00004000ull);
    val_set.AddRange(0x1C00002000ull, 0x1C00004000ull);
    val_set.AddRange(0x1D00002000ull, 0x1D00004000ull);
    val_set.AddRange(0x2000002000ull, 0x2000004000ull);
    val_set.AddRange(0x2100002000ull, 0x2300000000ull);

    SECTION("Align with 16MB page size")
    {
      ConstraintSet expect_set;
      expect_set.AddValue(0x0u);
      expect_set.AddRange(0x100u, 0x200u);
      expect_set.AddValue(0x300u);
      expect_set.AddValue(0x500u);
      expect_set.AddRange(0x700u, 0x800u);
      expect_set.AddRange(0xA00u, 0xB00u);
      expect_set.AddRange(0xC00u, 0xD00u);
      expect_set.AddValue(0xF00u);
      expect_set.AddValue(0x1100u);
      expect_set.AddValue(0x1200u);
      expect_set.AddValue(0x1400u);
      expect_set.AddValue(0x1500u);
      expect_set.AddValue(0x1800u);
      expect_set.AddValue(0x1900u);
      expect_set.AddValue(0x1B00u);
      expect_set.AddValue(0x1C00u);
      expect_set.AddValue(0x1D00u);
      expect_set.AddValue(0x2000u);
      expect_set.AddRange(0x2100u, 0x2300u);

      uint64 page_mask = 0xFFFFFFull;
      val_set.AlignWithPage(~page_mask);
      EXPECT(val_set.Size()    == val_set.CalculateSize());
      EXPECT(expect_set.Size() == val_set.Size());

      vector<Constraint*> expect_vec = expect_set.GetConstraints();
      for (auto item : expect_vec)
      {
        switch (item->Type())
        {
          case EConstraintType::Value:
            EXPECT(true == val_set.ContainsValue(item->LowerBound()));
            break;
          case EConstraintType::Range:
            EXPECT(true == val_set.ContainsRange(item->LowerBound(), item->UpperBound()));
            break;
        }
      }
    }

    SECTION("Align with 256MB page size")
    {
      ConstraintSet expect_set;
      expect_set.AddValue(0x0u);
      expect_set.AddRange(0x10u, 0x20u);
      expect_set.AddValue(0x30u);
      expect_set.AddValue(0x50u);
      expect_set.AddRange(0x70u, 0x80u);
      expect_set.AddRange(0xA0u, 0xB0u);
      expect_set.AddRange(0xC0u, 0xD0u);
      expect_set.AddValue(0xF0u);
      expect_set.AddValue(0x110u);
      expect_set.AddValue(0x120u);
      expect_set.AddValue(0x140u);
      expect_set.AddValue(0x150u);
      expect_set.AddValue(0x180u);
      expect_set.AddValue(0x190u);
      expect_set.AddValue(0x1B0u);
      expect_set.AddValue(0x1C0u);
      expect_set.AddValue(0x1D0u);
      expect_set.AddValue(0x200u);
      expect_set.AddRange(0x210u, 0x230u);

      uint64 page_mask = 0xFFFFFFFull;
      val_set.AlignWithPage(~page_mask);

      EXPECT(val_set.Size()    == val_set.CalculateSize());
      EXPECT(expect_set.Size() == val_set.Size());

      vector<Constraint*> expect_vec = expect_set.GetConstraints();
      for (auto item : expect_vec)
      {
        switch (item->Type())
        {
          case EConstraintType::Value:
            EXPECT(true == val_set.ContainsValue(item->LowerBound()));
            break;
          case EConstraintType::Range:
            EXPECT(true == val_set.ContainsRange(item->LowerBound(), item->UpperBound()));
            break;
        }
      }
    }

    SECTION("Align with 4GB page size")
    {  
      ConstraintSet expect_set;    
      expect_set.AddRange(0x0u, 0x2u);
      expect_set.AddValue(0x3u);
      expect_set.AddValue(0x5u);
      expect_set.AddRange(0x7u, 0x8u);
      expect_set.AddRange(0xAu, 0xDu);
      expect_set.AddValue(0xFu);
      expect_set.AddRange(0x11u, 0x12u);
      expect_set.AddRange(0x14u, 0x15u);
      expect_set.AddRange(0x18u, 0x19u);
      expect_set.AddRange(0x1Bu, 0x1Du);
      expect_set.AddRange(0x20u, 0x23u);

      uint64 page_mask = 0xFFFFFFFFull;
      val_set.AlignWithPage(~page_mask);
      
      EXPECT(val_set.Size()    == val_set.CalculateSize());
      EXPECT(expect_set.Size() == val_set.Size());

      //"0-3,5,7-8,0xa-0xd,0xf,0x11-0x12,0x14-0x15,0x18-0x19,0x1b-0x1d,0x20-0x23"
      vector<Constraint*> expect_vec = expect_set.GetConstraints();
      for (auto item : expect_vec)
      {
        switch (item->Type())
        {
          case EConstraintType::Value:
            EXPECT(true == val_set.ContainsValue(item->LowerBound()));
            break;
          case EConstraintType::Range:
            EXPECT(true == val_set.ContainsRange(item->LowerBound(), item->UpperBound()));
            break;
        }
      }
    }
  }
}  

CASE("ValueConstraint::Translate")
{
  uint64 page_mask = 0u;
  uint64 frame     = 0x5A5A5A5A5A5A5A5Aull;
  uint64 value     = 0x7FFFFFFFFFFFFFFFull;
  ConstraintSet val_set;

  for (page_mask = 0x1ull; page_mask < MAX_UINT64; page_mask = (page_mask << 0x1u) + 0x1u)
  {
    val_set.AddValue(value);
    uint64 page_frame = frame & page_mask;
    uint64 xlate_val  = (value & (~page_mask)) | page_frame;
    val_set.Translate(~page_mask, page_frame);

    if (!val_set.IsEmpty())
    {
      EXPECT(xlate_val == val_set.LowerBound());
      EXPECT(xlate_val == val_set.UpperBound());
    }

    val_set.Clear();
  }
}

CASE("RangeConstraint::Translate")
{
  uint64 page_mask = 0u;
  uint64 frame     = 0x5A5A5A5A5A5A5A5Aull;
  uint64 low_val   = 0x0555555555555555ull;
  uint64 high_val  = 0x0AAAAAAAAAAAAAAAull;
  ConstraintSet val_set;

  for (page_mask = 0x1ull; page_mask < MAX_UINT64; page_mask = (page_mask << 0x1u) + 0x1u)
  {
    val_set.AddRange(low_val, high_val);
    uint64 page_frame = frame & page_mask;
    uint64 xlate_low_val = (low_val & (~page_mask)) | page_frame;
    uint64 xlate_high_val = (high_val & (~page_mask)) | page_frame;
    val_set.Translate(~page_mask, page_frame);
    if (!val_set.IsEmpty())
    {
      EXPECT(xlate_low_val  == val_set.LowerBound());
      EXPECT(xlate_high_val == val_set.UpperBound());
    }
    val_set.Clear();
  }
}

CASE("ConstraintSet::Translate")
{
  SETUP("Setup tranlastion set for multiple page sizes")
  {
    ConstraintSet val_set;

    val_set.AddRange(0x1, 0x2);
    val_set.AddValue(0x4);
    val_set.AddRange(0x8, 0x10);
    val_set.AddRange(0x100, 0x200);
    val_set.AddValue(0x300);
    val_set.AddRange(0x500, 0x501);
    val_set.AddValue(0x700);
    val_set.AddRange(0x701, 0x800);
    val_set.AddValue(0x802);
    val_set.AddRange(0xA00, 0xA01);
    val_set.AddRange(0xA02, 0xB00);
    val_set.AddRange(0xC02, 0xD00);
    val_set.AddRange(0xF00, 0xF01);
    val_set.AddRange(0xF02, 0xF04);

    SECTION("Translate with 4KB page size")
    {
      ConstraintSet expect_set;

      expect_set.AddRange(0x5A5A5A001, 0x5A5A5A002);
      expect_set.AddValue(0x5A5A5A004);
      expect_set.AddRange(0x5A5A5A008, 0x5A5A5A010);
      expect_set.AddRange(0x5A5A5A100, 0x5A5A5A200);
      expect_set.AddValue(0x5A5A5A300);
      expect_set.AddRange(0x5A5A5A500, 0x5A5A5A501);
      expect_set.AddValue(0x5A5A5A700);
      expect_set.AddRange(0x5A5A5A701, 0x5A5A5A800);
      expect_set.AddValue(0x5A5A5A802);
      expect_set.AddRange(0x5A5A5AA00, 0x5A5A5AA01);
      expect_set.AddRange(0x5A5A5AA02, 0x5A5A5AB00);
      expect_set.AddRange(0x5A5A5AC02, 0x5A5A5AD00);
      expect_set.AddRange(0x5A5A5AF00, 0x5A5A5AF01);
      expect_set.AddRange(0x5A5A5AF02, 0x5A5A5AF04);

      uint64 page_mask  = 0xFFFull;
      uint64 page_frame = 0x5A5A5A000ull;
      val_set.Translate(page_mask, page_frame);

      EXPECT(val_set.Size() == val_set.CalculateSize());
      EXPECT(expect_set.ToSimpleString() == val_set.ToSimpleString());
    }

    SECTION("Translate with 16KB page size")
    {
      val_set.AddValue(0x1101);
      val_set.AddValue(0x1102);
      val_set.AddValue(0x1202);
      val_set.AddValue(0x1400);
      val_set.AddRange(0x1502, 0x1504);
      val_set.AddRange(0x1802, 0x1804);
      val_set.AddValue(0x1904);
      val_set.AddRange(0x1B02, 0x1B04);
      val_set.AddRange(0x1C02, 0x1C04);
      val_set.AddRange(0x1D02, 0x1D04);
      val_set.AddRange(0x2002, 0x2004);
      val_set.AddRange(0x2102, 0x2300);

      ConstraintSet expect_set;

      expect_set.AddRange(0xA5A5A58001, 0xA5A5A58002);
      expect_set.AddValue(0xA5A5A58004);
      expect_set.AddRange(0xA5A5A58008, 0xA5A5A58010);
      expect_set.AddRange(0xA5A5A58100, 0xA5A5A58200);
      expect_set.AddValue(0xA5A5A58300);
      expect_set.AddRange(0xA5A5A58500, 0xA5A5A58501);
      expect_set.AddValue(0xA5A5A58700);
      expect_set.AddRange(0xA5A5A58701, 0xA5A5A58800);
      expect_set.AddValue(0xA5A5A58802);
      expect_set.AddRange(0xA5A5A58A00, 0xA5A5A58A01);
      expect_set.AddRange(0xA5A5A58A02, 0xA5A5A58B00);
      expect_set.AddRange(0xA5A5A58C02, 0xA5A5A58D00);
      expect_set.AddRange(0xA5A5A58F00, 0xA5A5A58F01);
      expect_set.AddRange(0xA5A5A58F02, 0xA5A5A58F04);

      // in the 16 K but not 4K range
      expect_set.AddValue(0xA5A5A59101);
      expect_set.AddValue(0xA5A5A59102);
      expect_set.AddValue(0xA5A5A59202);
      expect_set.AddValue(0xA5A5A59400);
      expect_set.AddRange(0xA5A5A59502, 0xA5A5A59504);
      expect_set.AddRange(0xA5A5A59802, 0xA5A5A59804);
      expect_set.AddValue(0xA5A5A59904);
      expect_set.AddRange(0xA5A5A59B02, 0xA5A5A59B04);
      expect_set.AddRange(0xA5A5A59C02, 0xA5A5A59C04);
      expect_set.AddRange(0xA5A5A59D02, 0xA5A5A59D04);
      expect_set.AddRange(0xA5A5A5A002, 0xA5A5A5A004);
      expect_set.AddRange(0xA5A5A5A102, 0xA5A5A5A300);
      
      uint64 page_mask  = 0x3FFFull;
      uint64 page_frame = 0xA5A5A58000ull;
      val_set.Translate(page_mask, page_frame);

      EXPECT(val_set.Size() == val_set.CalculateSize());
      EXPECT(expect_set.ToSimpleString() == val_set.ToSimpleString());
    }

    SECTION("Translate with 64KB page size")
    {
      val_set.AddValue(0x1101);
      val_set.AddValue(0x1102);
      val_set.AddValue(0x1202);
      val_set.AddValue(0x1400);
      val_set.AddRange(0x1502, 0x1504);
      val_set.AddRange(0x1802, 0x1804);
      val_set.AddValue(0x1904);
      val_set.AddRange(0x1B02, 0x1B04);
      val_set.AddRange(0x1C02, 0x1C04);
      val_set.AddRange(0x1D02, 0x1D04);
      val_set.AddRange(0x2002, 0x2004);
      val_set.AddRange(0x2102, 0x2300);
      
      val_set.AddValue(0x9101);
      val_set.AddValue(0x9102);
      val_set.AddValue(0x9202);
      val_set.AddValue(0x9400);
      val_set.AddRange(0x9502, 0x9504);
      val_set.AddRange(0x9802, 0x9804);
      val_set.AddValue(0x9904);
      val_set.AddRange(0x9B02, 0x9B04);
      val_set.AddRange(0x9C02, 0x9C04);
      val_set.AddRange(0x9D02, 0x9D04);
      val_set.AddRange(0xA002, 0xA004);
      val_set.AddRange(0xA102, 0xA300);

      ConstraintSet expect_set;
      expect_set.AddRange(0x5A5A5A5A0001, 0x5A5A5A5A0002);
      expect_set.AddValue(0x5A5A5A5A0004);
      expect_set.AddRange(0x5A5A5A5A0008, 0x5A5A5A5A0010);
      expect_set.AddRange(0x5A5A5A5A0100, 0x5A5A5A5A0200);
      expect_set.AddValue(0x5A5A5A5A0300);
      expect_set.AddRange(0x5A5A5A5A0500, 0x5A5A5A5A0501);
      expect_set.AddValue(0x5A5A5A5A0700);
      expect_set.AddRange(0x5A5A5A5A0701, 0x5A5A5A5A0800);
      expect_set.AddValue(0x5A5A5A5A0802);
      expect_set.AddRange(0x5A5A5A5A0A00, 0x5A5A5A5A0A01);
      expect_set.AddRange(0x5A5A5A5A0A02, 0x5A5A5A5A0B00);
      expect_set.AddRange(0x5A5A5A5A0C02, 0x5A5A5A5A0D00);
      expect_set.AddRange(0x5A5A5A5A0F00, 0x5A5A5A5A0F01);
      expect_set.AddRange(0x5A5A5A5A0F02, 0x5A5A5A5A0F04);

      expect_set.AddValue(0x5A5A5A5A1101);
      expect_set.AddValue(0x5A5A5A5A1102);
      expect_set.AddValue(0x5A5A5A5A1202);
      expect_set.AddValue(0x5A5A5A5A1400);
      expect_set.AddRange(0x5A5A5A5A1502, 0x5A5A5A5A1504);
      expect_set.AddRange(0x5A5A5A5A1802, 0x5A5A5A5A1804);
      expect_set.AddValue(0x5A5A5A5A1904);
      expect_set.AddRange(0x5A5A5A5A1B02, 0x5A5A5A5A1B04);
      expect_set.AddRange(0x5A5A5A5A1C02, 0x5A5A5A5A1C04);
      expect_set.AddRange(0x5A5A5A5A1D02, 0x5A5A5A5A1D04);
      expect_set.AddRange(0x5A5A5A5A2002, 0x5A5A5A5A2004);
      expect_set.AddRange(0x5A5A5A5A2102, 0x5A5A5A5A2300);
      
      expect_set.AddValue(0x5A5A5A5A9101);
      expect_set.AddValue(0x5A5A5A5A9102);
      expect_set.AddValue(0x5A5A5A5A9202);
      expect_set.AddValue(0x5A5A5A5A9400);
      expect_set.AddRange(0x5A5A5A5A9502, 0x5A5A5A5A9504);
      expect_set.AddRange(0x5A5A5A5A9802, 0x5A5A5A5A9804);
      expect_set.AddValue(0x5A5A5A5A9904);
      expect_set.AddRange(0x5A5A5A5A9B02, 0x5A5A5A5A9B04);
      expect_set.AddRange(0x5A5A5A5A9C02, 0x5A5A5A5A9C04);
      expect_set.AddRange(0x5A5A5A5A9D02, 0x5A5A5A5A9D04);
      expect_set.AddRange(0x5A5A5A5AA002, 0x5A5A5A5AA004);
      expect_set.AddRange(0x5A5A5A5AA102, 0x5A5A5A5AA300);
      
      uint64 page_mask  = 0xFFFFull;
      uint64 page_frame = 0x5A5A5A5A0000ull;
      val_set.Translate(page_mask, page_frame);
      
      EXPECT(val_set.Size() == val_set.CalculateSize());
      EXPECT(expect_set.ToSimpleString() == val_set.ToSimpleString());
    }
  }
}

CASE("ConstraintSet::CopyInRange")
{
  ConstraintSet val_set;
  val_set.AddRange(0x1ull, 0x2ull);
  val_set.AddValue(0x4ull);
  val_set.AddRange(0x6ull, 0x7ull);
  val_set.AddValue(0x9ull);
  val_set.AddRange(0xBull, 0xCull);
  val_set.AddValue(0xEull);
  val_set.AddRange(0x10ull, 0x11ull);
  val_set.AddValue(0x13ull);
  val_set.AddRange(0x15ull, 0x16ull);
  val_set.AddValue(0x18ull);

  ConstraintSet copy_set;
  std::vector<Constraint*>& copy_vals = copy_set.GetConstraintsMutable();
  for (uint32 i=0ull; i < 0x1Aull; ++i)
  {
    for (uint32 j=i; j < 0x1Aull; ++j)
    {
      val_set.CopyInRange(i, j, copy_set);
      for (auto item : copy_vals)
      {
        EXPECT(true == val_set.ContainsConstraint(*item));
      }
      copy_vals.clear();
    }
  }
}

CASE("ConstraintSet::ReplaceInRange")
{
  SETUP("Compare using merge constraint") {

    auto test_replace_in_range_lambda = [&] (ConstraintSet& rOriginalConstr, uint64 rangeLower, uint64 rangeUpper, ConstraintSet& rReplacementConstr)
    {
      ConstraintSet compare_constr(rOriginalConstr);
      ConstraintSet replace_copy(rReplacementConstr);
      rOriginalConstr.ReplaceInRange(rangeLower, rangeUpper, rReplacementConstr);

      compare_constr.SubRange(rangeLower, rangeUpper);
      compare_constr.MergeConstraintSet(replace_copy);

      EXPECT(rOriginalConstr.ToSimpleString() == compare_constr.ToSimpleString());
      EXPECT(rOriginalConstr.Size() == compare_constr.Size());
    };

    SECTION("Simple case") {
      ConstraintSet original_constr("0x0-0xf8c56cd1ff,0xf8c56cd400-0xffffffffffff");    
      ConstraintSet replacement_constr("0xf8c56c0000-0xf8c56cd1ff,0xf8c56cd400-0xf8c56cffff");
      test_replace_in_range_lambda(original_constr, 0xf8c56c0000, 0xf8c56cffff, replacement_constr);
    }

    SECTION("Match last Constraint case short") {
      ConstraintSet original_constr("0xda86c861304-0xda86c8644eb,0xda86c8644f0-0xda86c8681ff,0xda86c868204-0xda86c869b9f,0xda86c869ba4-0xffffffffffff");
      ConstraintSet replacement_constr("0x10048000101c-0x10049bffffff,0x10049e000000-0x10049fffffff");
      test_replace_in_range_lambda(original_constr, 0x100480000000, 0x10049fffffff, replacement_constr);
    }

    SECTION("Match last Constraint case long") {
      ConstraintSet original_constr("0x0-0x4fffffff,0x50000040-0x7fffffff,0x8000101c-0x5c84a5ffff,0x5c84a60004-0x5c84a6007f,0x5c84a60084-0x5c84a600ff,0x5c84a60104-0x5c84a6017f,0x5c84a60184-0x5c84a601ff,0x5c84a60204-0x5c84a6027f,0x5c84a60284-0x5c84a602ff,0x5c84a60304-0x5c84a6037f,0x5c84a60384-0x5c84a603ff,0x5c84a60404-0x5c84a6047f,0x5c84a60484-0x5c84a604ff,0x5c84a60504-0x5c84a6057f,0x5c84a60584-0x5c84a605ff,0x5c84a60604-0x5c84a6067f,0x5c84a60684-0x5c84a606ff,0x5c84a60704-0x5c84a607ff,0x5c84a60ac8-0x5c84a61fff,0x5c84a62004-0x5c84a6207f,0x5c84a62084-0x5c84a620ff,0x5c84a62104-0x5c84a6217f,0x5c84a62184-0x5c84a621ff,0x5c84a62204-0x5c84a6227f,0x5c84a62284-0x5c84a622ff,0x5c84a62304-0x5c84a6237f,0x5c84a62384-0x5c84a623ff,0x5c84a62404-0x5c84a6247f,0x5c84a62484-0x5c84a624ff,0x5c84a62504-0x5c84a6257f,0x5c84a62584-0x5c84a625ff,0x5c84a62604-0x5c84a6267f,0x5c84a62684-0x5c84a626ff,0x5c84a62704-0x5c84a627ff,0x5c84a62ac8-0x61690924d7,0x61690934d8-0xb3330fffff,0xb333100004-0xb33310007f,0xb333100084-0xb3331000ff,0xb333100104-0xb33310017f,0xb333100184-0xb3331001ff,0xb333100204-0xb33310027f,0xb333100284-0xb3331002ff,0xb333100304-0xb33310037f,0xb333100384-0xb3331003ff,0xb333100404-0xb33310047f,0xb333100484-0xb3331004ff,0xb333100504-0xb33310057f,0xb333100584-0xb3331005ff,0xb333100604-0xb33310067f,0xb333100684-0xb3331006ff,0xb333100704-0xb3331007ff,0xb333100ac8-0xb333101fff,0xb333102004-0xb33310207f,0xb333102084-0xb3331020ff,0xb333102104-0xb33310217f,0xb333102184-0xb3331021ff,0xb333102204-0xb33310227f,0xb333102284-0xb3331022ff,0xb333102304-0xb33310237f,0xb333102384-0xb3331023ff,0xb333102404-0xb33310247f,0xb333102484-0xb3331024ff,0xb333102504-0xb33310257f,0xb333102584-0xb3331025ff,0xb333102604-0xb33310267f,0xb333102684-0xb3331026ff,0xb333102704-0xb3331027ff,0xb333102ac8-0xf8c56cd1ff,0xf8c56cd400-0x1331bb9ffff,0x1331dba0000-0x1c293c0ffff,0x1c295c10000-0xda7847b10cf,0xda7847b10d4-0xda7847b12ff,0xda7847b1304-0xda7847b44eb,0xda7847b44f0-0xda7847b81ff,0xda7847b8204-0xda7847b9b9f,0xda7847b9ba4-0xda7bfa310cf,0xda7bfa310d4-0xda7bfa312ff,0xda7bfa31304-0xda7bfa344eb,0xda7bfa344f0-0xda7bfa381ff,0xda7bfa38204-0xda7bfa39b9f,0xda7bfa39ba4-0xda7f0ff10cf,0xda7f0ff10d4-0xda7f0ff12ff,0xda7f0ff1304-0xda7f0ff44eb,0xda7f0ff44f0-0xda7f0ff81ff,0xda7f0ff8204-0xda7f0ff9b9f,0xda7f0ff9ba4-0xda8638310cf,0xda8638310d4-0xda8638312ff,0xda863831304-0xda8638344eb,0xda8638344f0-0xda8638381ff,0xda863838204-0xda863839b9f,0xda863839ba4-0xda86c8610cf,0xda86c8610d4-0xda86c8612ff,0xda86c861304-0xda86c8644eb,0xda86c8644f0-0xda86c8681ff,0xda86c868204-0xda86c869b9f,0xda86c869ba4-0xffffffffffff");
      ConstraintSet replacement_constr("0x10048000101c-0x10049bffffff,0x10049e000000-0x10049fffffff");
      test_replace_in_range_lambda(original_constr, 0x100480000000, 0x10049fffffff, replacement_constr);
    }
  }
}

