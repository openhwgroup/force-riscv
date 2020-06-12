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
#include <InstructionStructure.h>
#include <Defines.h>
#include <StringUtils.h>

using text = std::string;
using namespace Force;

const lest::test specification[] = {

CASE( "InstructionStructure module testing" ) {

  SETUP( "setup and test OperandStructure class" )  {
    OperandStructure opr_struct;
    opr_struct.SetBits("31-24,21,15-10"); // set bits ranges for the operand

    int vec_size = opr_struct.mEncodingBits.size();
    EXPECT( vec_size == 3 ); // verify the ranges are broken into 3 parts
    EXPECT(opr_struct.mMask == 0x7fffu); //!< verify the operand mask is of the right size

    SECTION( "Casual testing of the bin string parsing function." ) {
      uint32 hex_value = parse_bin32("010111101100001");
      EXPECT( hex_value == 0x2f61U );
    }

    SECTION( "Test OperandStructure::Encoding method" ) {
      uint32 encoding_value = opr_struct.Encoding(parse_bin32("010111101100001"));
      EXPECT( encoding_value == 0x5e208400U );
    }
  }
},

CASE( "Test AluOperandStructure" ) {

  SETUP( "Setup AluOperandStructure" )  {
    AluOperandStructure alu_opr_struct;

    SECTION( "Test offset shift" ) {
      std::string offset_shift = "sh12";
      alu_opr_struct.SetOffsetShift(offset_shift);
      EXPECT(alu_opr_struct.OffsetShift() == offset_shift);
    }

    SECTION( "Test offset shift" ) {
      std::string immediate = "imm12";
      alu_opr_struct.SetImmediate(immediate);
      EXPECT(alu_opr_struct.Immediate() == immediate);
    }

    SECTION( "Test operation type" ) {
      EAluOperationType operation_type = EAluOperationType::SUB;
      alu_opr_struct.SetOperationType(operation_type);
      EXPECT(alu_opr_struct.OperationType() == operation_type);
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
