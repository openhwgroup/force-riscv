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
#include <BaseOffsetConstraint.h>
#include <Constraint.h>
#include <Log.h>

//------------------------------------------------
// include necessary header files here
//------------------------------------------------

using text = std::string;
using namespace Force;

static const uint32 Imm26_Offset_Base = 0x2000000; // 26 bit immediate offset

const lest::test specification[] = {

  CASE( "BaseOffsetConstraint test case 1" ) {

    SETUP( "setup test 1.1" )  {
    BaseOffsetConstraint bo_constr_builder(Imm26_Offset_Base, 26,  2, MAX_UINT64);
    uint32 access_size = 20;

      SECTION( "test check imm26=100 case" ) {
    // Branching to PC + 400, pc of branch: 0x8d4867911c, target pc: 0x8d486792ac
    uint64 offset_value = 100;
    ConstraintSet offset_constr(offset_value);
    ConstraintSet base_offset_constr;
    uint64 base_value = 0x8d4867911c;
    uint64 target_lower = base_value + (offset_value << 2);
    uint64 target_upper = target_lower + access_size - 1;
    bo_constr_builder.GetConstraint(base_value, access_size, &offset_constr, base_offset_constr);
    EXPECT(base_offset_constr.LowerBound() == target_lower);
    EXPECT(base_offset_constr.UpperBound() == target_upper);
      }

      SECTION( "test check imm26=100-200 case" ) {
    // Branching to PC + [400:800], pc of branch: 0x8d4867911c, target pc: 0x8d486792ac-0x8d4867947c+(20-1)
    uint64 offset_lower = 0x100;
    uint64 offset_upper = 0x200;
    ConstraintSet offset_constr(offset_lower, offset_upper);
    ConstraintSet base_offset_constr;
    uint64 base_value = 0x8d4867911c;
    uint64 target_lower = base_value + (offset_lower << 2);
    uint64 target_upper = base_value + (offset_upper << 2) + (access_size - 1);
    bo_constr_builder.GetConstraint(base_value, access_size, &offset_constr, base_offset_constr);
    EXPECT(base_offset_constr.LowerBound() == target_lower);
    EXPECT(base_offset_constr.UpperBound() == target_upper);
      }
      SECTION( "test check imm26=100-200 case swapped" ) {
    uint64 offset_lower = 0x2000;
    uint64 offset_upper = 0x1000;
    ConstraintSet offset_constr(offset_lower, offset_upper);
    ConstraintSet base_offset_constr;
    uint64 base_value = 0x8d4867911c;
    uint64 target_lower = base_value + (offset_upper << 2);
    uint64 target_upper = base_value + (offset_lower << 2) + (access_size - 1);
    bo_constr_builder.GetConstraint(base_value, access_size, &offset_constr, base_offset_constr);
    EXPECT(base_offset_constr.LowerBound() == target_lower);
    EXPECT(base_offset_constr.UpperBound() == target_upper);
      }
      SECTION( "test check imm26: both offsets negative" ) {
    uint64 offset_lower = 0x2000100;
    uint64 offset_upper = 0x2000200;
    ConstraintSet offset_constr(offset_lower, offset_upper);
    ConstraintSet base_offset_constr;
    uint64 base_value = 0x8d4867911c;
    uint64 target_lower = base_value + (offset_lower << 2) - 0x10000000;
    uint64 target_upper = base_value + (offset_upper << 2) + (access_size - 1) - 0x10000000;
    bo_constr_builder.GetConstraint(base_value, access_size, &offset_constr, base_offset_constr);
    EXPECT(base_offset_constr.LowerBound() == target_lower);
    EXPECT(base_offset_constr.UpperBound() == target_upper);
      }
      SECTION( "test check imm26: one offset positive one offset negative" ) {
    uint64 offset_lower = 0x3ffff00;
    uint64 offset_upper = 0x300;
    ConstraintSet offset_constr(offset_lower, offset_upper);
    ConstraintSet base_offset_constr;
    uint64 base_value = 0x8d4867911c;
    uint64 target_upper = base_value + (offset_upper << 2) + access_size - 1;
    uint64 target_lower = base_value - (0x10000000 - (offset_lower << 2));
    bo_constr_builder.GetConstraint(base_value, access_size, &offset_constr, base_offset_constr);
    EXPECT(base_offset_constr.LowerBound() == target_lower); 
    EXPECT(base_offset_constr.UpperBound() == target_upper);
      }
      SECTION( "test check imm26: both offsets positive and wrapped") {
    uint64 offset_lower = 0x2000;
    uint64 offset_upper = 0x3000;
    ConstraintSet offset_constr(offset_lower, offset_upper);
    ConstraintSet base_offset_constr;
    uint64 base_value = 0xfffffffffffff000;
    uint64 target_upper = base_value + (offset_upper << 2) + access_size - 1;
    uint64 target_lower = base_value + (offset_lower << 2);
    bo_constr_builder.GetConstraint(base_value, access_size, &offset_constr, base_offset_constr);
    EXPECT(base_offset_constr.LowerBound() == target_lower); 
    EXPECT(base_offset_constr.UpperBound() == target_upper);
      }
        SECTION( "test check imm26: both offsets positive and one wrapped") {
    uint64 offset_lower = 0x20;
    uint64 offset_upper = 0x3fe;
    ConstraintSet offset_constr(offset_lower, offset_upper);
    ConstraintSet base_offset_constr;
    uint64 base_value = 0xfffffffffffff000;
    uint64 target_upper = base_value + (offset_upper << 2) + access_size - 1;
    uint64 target_lower = base_value + (offset_lower << 2);
    uint64 target_size = target_upper + 1 - target_lower;
    bo_constr_builder.GetConstraint(base_value, access_size, &offset_constr, base_offset_constr);
    EXPECT(base_offset_constr.Size() == target_size);
      } 
      SECTION( "test check imm26: both offsets negative and wrapped" ) {
    uint64 offset_lower = 0x2000100;
    uint64 offset_upper = 0x2000200;
    ConstraintSet offset_constr(offset_lower, offset_upper);
    ConstraintSet base_offset_constr;
    uint64 base_value = 0x800000;
    uint64 target_lower = base_value + (offset_lower << 2) - 0x10000000;
    uint64 target_upper = base_value + (offset_upper << 2) + (access_size - 1) - 0x10000000;
    bo_constr_builder.GetConstraint(base_value, access_size, &offset_constr, base_offset_constr);
    EXPECT(base_offset_constr.LowerBound() == target_lower);
    EXPECT(base_offset_constr.UpperBound() == target_upper);
      }      
      SECTION( "test check imm26: both offsets negative and one wrapped" ) {
    uint64 offset_upper = 0x3dffffd;
    uint64 offset_lower = 0x3000200;
    ConstraintSet offset_constr(offset_lower, offset_upper);
    ConstraintSet base_offset_constr;
    uint64 base_value = 0x800000;
    uint64 target_lower = base_value + (offset_lower << 2) - 0x10000000;
    uint64 target_upper = base_value + (offset_upper << 2) + (access_size - 1) - 0x10000000;
    uint64 target_size = target_upper + 1 - target_lower;
    bo_constr_builder.GetConstraint(base_value, access_size, &offset_constr, base_offset_constr);
    EXPECT(base_offset_constr.Size() == target_size);
      }      
    }
  },

  CASE( "BaseOffsetConstraint test invalid parameter" ) {

    SETUP( "setup" )  {
      ConstraintSet base_offset_constr;

      SECTION( "test offset too large" ) {
        BaseOffsetConstraint bo_constr_builder(0, 64, 0, MAX_UINT64, true);
        EXPECT_FAIL(bo_constr_builder.GetConstraint(0x33b982c3c836, 16, nullptr, base_offset_constr), "invalid-parameter-value");
      }

      SECTION( "test offset too larget after scaling" ) {
        BaseOffsetConstraint bo_constr_builder(0, 32, 32, MAX_UINT64, true);
        EXPECT_FAIL(bo_constr_builder.GetConstraint(0xa78fd4bec0a8, 32, nullptr, base_offset_constr), "invalid-parameter-value");
      }
    }
  },
};

int main( int argc, char * argv[] )
{
    Force::Logger::Initialize();
    int ret = lest::run( specification, argc, argv );
    Force::Logger::Destroy();
    return ret;
}
