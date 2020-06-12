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
#include <AluImmediateConstraint.h>
#include <Constraint.h>
#include <Enums.h>

using text = std::string;
using namespace Force;

const lest::test specification[] = {

CASE( "Test AluImmediateConstraint" ) {

  SETUP( "Setup AluImmediateConstraint" )  {
    ConstraintSet result_constr;

    SECTION( "Test building ADD constraint" ) {
      AluImmediateConstraint alu_imm_constr(EAluOperationType::ADD, 12, 12);
      alu_imm_constr.GetConstraint(0x75001000, 16, &result_constr);
      EXPECT(result_constr.ToSimpleString() == "0x75001000-0x7600000f");
    }

    SECTION( "Test building SUB constraint" ) {
      AluImmediateConstraint alu_imm_constr(EAluOperationType::SUB, 8, 0);
      alu_imm_constr.GetConstraint(0x6500, 8, &result_constr);
      EXPECT(result_constr.ToSimpleString() == "0x6401-0x6507");
    }

    SECTION( "Test exceeding max value" ) {
      AluImmediateConstraint alu_imm_constr(EAluOperationType::ADD, 10, 3);
      alu_imm_constr.GetConstraint(0xfffffffffffff9ac, 4, &result_constr);
      EXPECT(result_constr.ToSimpleString() == "0x0-0x19a7,0xfffffffffffff9ac-0xffffffffffffffff");
    }
  }
},

};

int main(int argc, char * argv[])
{
    Force::Logger::Initialize();
    int ret = lest::run(specification, argc, argv);
    Force::Logger::Destroy();
    return ret;
}
