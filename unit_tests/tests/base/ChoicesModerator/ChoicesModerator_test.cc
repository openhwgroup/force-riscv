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

#include <Choices.h>
#include <ChoicesModerator.h>
#include <Random.h>
#include <GenException.h>
#include <string.h>

using namespace std;
using namespace Force;

using text = std::string;

const lest::test specification[] = {

CASE( "test case set 1 for Choices Moderator module" ) {

    SETUP( "setup testing with a simple choices tree and its moderator" )  {
        //-----------------------------------------
	// include necessary setup code here
	//-----------------------------------------
      ChoiceTree* choices_tree0 = new ChoiceTree("Tree0", 0, 10);
      Choice * choice00 = new Choice("Choice 00", 0, 100);
      choices_tree0->AddChoice(choice00);
      Choice * choice01 = new Choice("Choice 01", 1, 200);
      choices_tree0->AddChoice(choice01);
      Choice * choice02 = new Choice("Choice 02", 2, 133);
      choices_tree0->AddChoice(choice02);
      Choice * choice03 = new Choice("Choice 03", 3, 50);
      choices_tree0->AddChoice(choice03);
      Choice * choice04 = new Choice("Choice 04", 4, 250);
      choices_tree0->AddChoice(choice04);

      ChoiceTree* choices_tree1 = new ChoiceTree("Tree1", 0, 10);
      Choice * choice10 = new Choice("Choice 10", 0, 100);
      choices_tree1->AddChoice(choice10);
      Choice * choice11 = new Choice("Choice 11", 1, 200);
      choices_tree1->AddChoice(choice11);
      Choice * choice12 = new Choice("Choice 12", 2, 133);
      choices_tree1->AddChoice(choice12);
      Choice * choice13 = new Choice("Choice 13", 3, 50);
      choices_tree1->AddChoice(choice13);
      Choice * choice14 = new Choice("Choice 14", 4, 250);
      choices_tree1->AddChoice(choice14);

      ChoicesSet my_set(EChoicesType::GeneralChoices);
      my_set.AddChoiceTree(choices_tree0);
      my_set.AddChoiceTree(choices_tree1);

      ChoicesModerator my_moderator(&my_set);
      
      SECTION( "test ChoicesModerator methods" ) {
      std::map<std::string, uint32> modifications;
      modifications["Choice 00"] = 50;
      modifications["Choice 01"] = 100;
      uint32 id = 0; 
      my_moderator.AddChoicesModification("Tree0", modifications, id);
      my_moderator.CommitModificationSet(id); 
      ChoiceTree* modified_tree = my_moderator.CloneChoiceTree("Tree0");
	  EXPECT(modified_tree->Chosen(60)->Value() == 1u);
      EXPECT(modified_tree->Chosen(180)->Value() == 2u);
      delete modified_tree;
      my_moderator.RevertModificationSet(id);
      modified_tree = my_moderator.CloneChoiceTree("Tree0");
      EXPECT(modified_tree->Chosen(60)->Value() == 0u);
      EXPECT(modified_tree->Chosen(180)->Value() == 1u);

      }
    }
},

CASE( "test case set 2 for Choices Moderator module" ) {

    SETUP( "setup testing with a simple choices tree and its moderator" )  {
        //-----------------------------------------
	// include necessary setup code here
	//-----------------------------------------
      ChoiceTree* choices_tree0 = new ChoiceTree("Tree0", 0, 10);
      Choice * choice00 = new Choice("Choice 00", 0, 100);
      choices_tree0->AddChoice(choice00);
      Choice * choice01 = new Choice("Choice 01", 1, 200);
      choices_tree0->AddChoice(choice01);
      Choice * choice02 = new Choice("Choice 02", 2, 133);
      choices_tree0->AddChoice(choice02);
      Choice * choice03 = new Choice("Choice 03", 3, 50);
      choices_tree0->AddChoice(choice03);
      Choice * choice04 = new Choice("Choice 04", 4, 250);
      choices_tree0->AddChoice(choice04);

      ChoiceTree* choices_tree1 = new ChoiceTree("Tree1", 0, 10);
      Choice * choice10 = new Choice("Choice 10", 0, 100);
      choices_tree1->AddChoice(choice10);
      Choice * choice11 = new Choice("Choice 11", 1, 200);
      choices_tree1->AddChoice(choice11);
      Choice * choice12 = new Choice("Choice 12", 2, 133);
      choices_tree1->AddChoice(choice12);
      Choice * choice13 = new Choice("Choice 13", 3, 50);
      choices_tree1->AddChoice(choice13);
      Choice * choice14 = new Choice("Choice 14", 4, 250);
      choices_tree1->AddChoice(choice14);

      ChoicesSet my_set(EChoicesType::GeneralChoices);
      my_set.AddChoiceTree(choices_tree0);
      my_set.AddChoiceTree(choices_tree1);

      ChoicesModerator my_moderator(&my_set);
      
      SECTION( "test ChoicesModerator methods" ) {
      std::map<std::string, uint32> modifications;
      modifications["Choice 00"] = 50;
      modifications["Choice 01"] = 100;
      uint32 id0 = 0; 
      my_moderator.AddChoicesModification("Tree0", modifications, id0);
      modifications.clear();
      modifications["Choice 02"] = 150;
      modifications["Choice 03"] = 200;
      uint32 id1 = 1;
      my_moderator.AddChoicesModification("Tree0", modifications, id1);
      EXPECT(id0 != id1);
      my_moderator.CommitModificationSet(id0);
      my_moderator.CommitModificationSet(id1);
      ChoiceTree* modified_tree = my_moderator.CloneChoiceTree("Tree0");
      EXPECT(modified_tree->Chosen(300)->Value() == 3u);
      EXPECT(modified_tree->Chosen(500)->Value() == 4u);
      delete modified_tree;
      my_moderator.RevertModificationSet(id0);
      modified_tree = my_moderator.CloneChoiceTree("Tree0");
      EXPECT(modified_tree->Chosen(60)->Value() == 0u);
      EXPECT(modified_tree->Chosen(180)->Value() == 1u);

      }
    }
}, 

CASE( "test case set 3 for Choices Moderator module" ) {

    SETUP( "setup testing with a simple choices tree and its moderator" )  {
        //-----------------------------------------
	// include necessary setup code here
	//-----------------------------------------
      ChoiceTree* choices_tree0 = new ChoiceTree("Tree0", 0, 10);
      Choice * choice00 = new Choice("Choice 00", 0, 100);
      choices_tree0->AddChoice(choice00);
      Choice * choice01 = new Choice("Choice 01", 1, 200);
      choices_tree0->AddChoice(choice01);
      Choice * choice02 = new Choice("Choice 02", 2, 133);
      choices_tree0->AddChoice(choice02);
      Choice * choice03 = new Choice("Choice 03", 3, 50);
      choices_tree0->AddChoice(choice03);
      Choice * choice04 = new Choice("Choice 04", 4, 250);
      choices_tree0->AddChoice(choice04);

      ChoiceTree* choices_tree1 = new ChoiceTree("Tree1", 0, 10);
      Choice * choice10 = new Choice("Choice 10", 0, 100);
      choices_tree1->AddChoice(choice10);
      Choice * choice11 = new Choice("Choice 11", 1, 200);
      choices_tree1->AddChoice(choice11);
      Choice * choice12 = new Choice("Choice 12", 2, 133);
      choices_tree1->AddChoice(choice12);
      Choice * choice13 = new Choice("Choice 13", 3, 50);
      choices_tree1->AddChoice(choice13);
      Choice * choice14 = new Choice("Choice 14", 4, 250);
      choices_tree1->AddChoice(choice14);

      ChoicesSet my_set(EChoicesType::GeneralChoices);
      my_set.AddChoiceTree(choices_tree0);
      my_set.AddChoiceTree(choices_tree1);

      ChoicesModerator my_moderator(&my_set);
      
      SECTION( "test ChoicesModerator methods" ) {
      std::map<std::string, uint32> modifications;
      modifications["Choice 00"] = 50;
      modifications["Choice 01"] = 100;
      uint32 id0 = 0;
      my_moderator.AddChoicesModification("Tree0", modifications, id0);
      my_moderator.CommitModificationSet(id0);
      modifications.clear();
      modifications["Choice 02"] = 150;
      modifications["Choice 03"] = 200;
      uint32 id1 = 1;
      my_moderator.AddChoicesModification("Tree0", modifications, id1);
      EXPECT(id0 != id1);
      my_moderator.CommitModificationSet(id1); 
      ChoiceTree* modified_tree = my_moderator.CloneChoiceTree("Tree0");
      EXPECT(modified_tree->Chosen(300)->Value() == 3u);
      EXPECT(modified_tree->Chosen(500)->Value() == 4u);
      delete modified_tree;
      
      my_moderator.RevertModificationSet(id0);
      modified_tree = my_moderator.CloneChoiceTree("Tree0");
      EXPECT(modified_tree->Chosen(60)->Value() == 0u);
      EXPECT(modified_tree->Chosen(180)->Value() == 1u);
      delete modified_tree;

      my_moderator.RevertModificationSet(id1);
      modified_tree = my_moderator.CloneChoiceTree("Tree0");
      EXPECT(modified_tree->Chosen(300)->Value() == 2u);
      EXPECT(modified_tree->Chosen(500)->Value() == 4u);
      delete modified_tree;

      modifications.clear();
      modifications["Choice 00"] = 0;
      modifications["Choice 01"] = 0;
      modifications["Choice 02"] = 0;
      modifications["Choice 03"] = 0;
      uint32 id2 = 2;
      my_moderator.AddChoicesModification("Tree0", modifications, id2);
      EXPECT(id0 != id2);
      my_moderator.CommitModificationSet(id2); 
      
      modified_tree = my_moderator.CloneChoiceTree("Tree0");
      EXPECT(modified_tree->Chosen(1)->Value() == 4u);
      EXPECT(modified_tree->Chosen(249)->Value() == 4u);
      EXPECT(modified_tree->Choose()->Value() == 4u);
      delete modified_tree;

      try {
        my_moderator.CloneChoiceTree("Tree3");
      } catch (const exception& e) {
        EXPECT( 0 == strcmp("ChoicesError", dynamic_cast<const GenException*>(&e)->GenExceptionType()));
      }

      EXPECT_NO_THROW(modified_tree = my_moderator.TryCloneChoiceTree("Tree3"));
      EXPECT(nullptr == modified_tree);
      }
    }
},

CASE( "Test invalid choices modifications" ) {

  SETUP( "Setup testing with a simple choices tree and its moderator" )  {
    ChoicesSet choices_set(EChoicesType::GeneralChoices);
    ChoiceTree* choice_tree = new ChoiceTree("Tree0", 3, 20);
    choice_tree->AddChoice(new Choice("Choice 00", 4, 30));
    choice_tree->AddChoice(new Choice("Choice 01", 6, 70));
    choices_set.AddChoiceTree(choice_tree);

    ChoicesModerator choices_moderator(&choices_set);

    SECTION( "Test modifiying non-existent choice tree" ) {
      std::map<std::string, uint32> modifications;
      modifications["Choice 00"] = 0;
      modifications["Choice 01"] = 10;
      EXPECT_FAIL(choices_moderator.AddChoicesModification("Tree1", modifications, 0), "Invalid choices modifications");
      EXPECT_FAIL(choices_moderator.DoChoicesModification("Tree2", modifications), "Invalid choices modifications");
    }

    SECTION( "Test modifiying non-existent choice" ) {
      std::map<std::string, uint32> modifications;
      modifications["Choice 00"] = 5;
      modifications["Choice 02"] = 40;
      EXPECT_FAIL(choices_moderator.AddChoicesModification("Tree0", modifications, 0), "Invalid choices modifications");
      EXPECT_FAIL(choices_moderator.DoChoicesModification("Tree0", modifications), "Invalid choices modifications");
    }
  }
},

};

int main( int argc, char * argv[] )
{
    Force::Logger::Initialize();
    Force::Random::Initialize();
    Force::Random* rand_instance =  Force::Random::Instance();
    rand_instance->Seed(rand_instance->RandomSeed()); //Seed(0x12345678);
    int ret = lest::run( specification, argc, argv );
    Force::Random::Destroy();
    Force::Logger::Destroy();
    return ret;
}
