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
#include "Variable.h"

#include "lest/lest.hpp"

#include "Choices.h"
#include "Defines.h"
#include "Enums.h"
#include "Log.h"
#include "Random.h"

using text = std::string;

const lest::test specification[] = {

CASE( "Basic Test Variable" ) {
   SETUP( "setup and test Variable class" ) {
     using namespace Force;
     
     VariableSet valueSet(EVariableType::Value);
     valueSet.AddVariable("instruction history limit", "10");
     valueSet.AddVariable("address bit width", "64");
     
     VariableSet choiceSet(EVariableType::Choice);
     choiceSet.AddVariable("look up window", "1-5:10,6-10:80,11-15:10");
     choiceSet.AddVariable("learning level", "1-3:20,4-5:40,6:40");
     
     SECTION ("test value variable") {
       auto var1 = dynamic_cast<const ValueVariable*>(valueSet.FindVariable("instruction history limit"));
       EXPECT(var1->Value() == 0xau);
       valueSet.ModifyVariable("instruction history limit","20");
       EXPECT(var1->Value() == 0x14u);

       auto var2 = dynamic_cast<const ValueVariable*>(valueSet.FindVariable("address bit width"));
       EXPECT(var2->Value() == 64u);
       valueSet.ModifyVariable("address bit width", "32");
       EXPECT(var2->Value() == 32u);
     }
     SECTION ("test value variable clone()") {
       auto value_var = dynamic_cast<const ValueVariable*>(valueSet.FindVariable("instruction history limit"));
       auto clone_raw = dynamic_cast<ValueVariable*>(value_var->Clone());
       std::unique_ptr<ValueVariable> clone_var(clone_raw);
       EXPECT(clone_var->Value() == 0xau);
     }

     SECTION ("test duplicate value variable name") {
       EXPECT_FAIL(valueSet.AddVariable("instruction history limit", "10"),"duplicate-variable-name");
     }

     SECTION ("test choice variable") {
       auto var = dynamic_cast<const ChoiceVariable*>(choiceSet.FindVariable("look up window"));    
       uint32 lower, high; 
       auto pChoiceTree = var->GetChoiceTree();
       auto pChoice1 = dynamic_cast<RangeChoice*>(pChoiceTree->Chosen(9));
       pChoice1->GetRange(lower, high);
       EXPECT(lower == 1u);
       EXPECT(high == 5u);

       choiceSet.ModifyVariable("look up window", "1-10:9,11-15:1");
       auto pChoice2 =  dynamic_cast<RangeChoice*>(pChoiceTree->Chosen(9));
       pChoice2->GetRange(lower, high);
       EXPECT(lower==11u);
       EXPECT(high == 15u);
     }
     
     SECTION ("test choice variable clone()") {
       auto choice_var = dynamic_cast<const ChoiceVariable*>(choiceSet.FindVariable("look up window")); 
       auto clone_raw = dynamic_cast<ChoiceVariable*>(choice_var->Clone());
       std::unique_ptr<ChoiceVariable> clone_var(clone_raw);
       
       uint32 lower, high; 
       auto pChoiceTree = clone_var->GetChoiceTree();
       auto pChoice1 = dynamic_cast<RangeChoice*>(pChoiceTree->Chosen(9));
       pChoice1->GetRange(lower, high);
       EXPECT(lower == 1u);
       EXPECT(high == 5u);
     }
   }
},

};

int main( int argc, char * argv[] )
{
  Force::Logger::Initialize();
  Force::Random::Initialize();
  int ret = lest::run( specification, argc, argv );
  Force::Random::Destroy();
  Force::Logger::Destroy();
  return ret;

}      
