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
#include <Random.h>
#include <GenException.h>

using namespace std;
using namespace Force;

using text = std::string;

const lest::test specification[] = {

CASE( "test case set 1 for Choices module" ) {

    SETUP( "setup testing with a simple choices tree" )  {
        //-----------------------------------------
	// include necessary setup code here
	//-----------------------------------------
      ChoiceTree choices_tree("Tree", 0, 10);
      Choice * choice0 = new Choice("Choice 0", 0, 100);
      choices_tree.AddChoice(choice0);
      Choice * choice1 = new Choice("Choice 1", 1, 200);
      choices_tree.AddChoice(choice1);
      Choice * choice2 = new Choice("Choice 2", 2, 133);
      choices_tree.AddChoice(choice2);
      Choice * choice3 = new Choice("Choice 3", 3, 50);
      choices_tree.AddChoice(choice3);
      Choice * choice4 = new Choice("Choice 4", 4, 250);
      choices_tree.AddChoice(choice4);

      EXPECT(choices_tree.GetChoices().size() == 5u);

      SECTION( "test choice picking") {
	const Choice* chosen_one = choices_tree.Choose();
	EXPECT(chosen_one != nullptr);
      }

      SECTION( "test choice item selection by chosen picked_value" ) {
	EXPECT(choices_tree.Chosen(0)->Value() == 0u);
	EXPECT(choices_tree.Chosen(99)->Value() == 0u);
	EXPECT(choices_tree.Chosen(100)->Value() == 1u);
	EXPECT(choices_tree.Chosen(299)->Value() == 1u);
	EXPECT(choices_tree.Chosen(300)->Value() == 2u);
	EXPECT(choices_tree.Chosen(432)->Value() == 2u);
	EXPECT(choices_tree.Chosen(433)->Value() == 3u);
	EXPECT(choices_tree.Chosen(482)->Value() == 3u);
	EXPECT(choices_tree.Chosen(483)->Value() == 4u);
	EXPECT(choices_tree.Chosen(732)->Value() == 4u);
	EXPECT(choices_tree.Chosen(733) == nullptr);
      }

      SECTION( "test choice tree cloning" ) {
	ChoiceTree* choices_tree_clone = dynamic_cast<ChoiceTree* >(choices_tree.Clone());
	auto choices = choices_tree.GetChoices();
	auto choices_clone = choices_tree_clone->GetChoices();
	EXPECT(choices.size() == choices_clone.size());
	EXPECT(choices.back() != choices_clone.back());
	delete choices_tree_clone;
      }

      SECTION( "test all weight set to 0 case") {
	auto choices = choices_tree.GetChoices();
	for (auto choice_item : choices) {
	  choice_item->SetWeight(0);
	}
	EXPECT_THROWS_AS(choices_tree.Choose(), ChoicesError);
      }

      SECTION( "test cyclic choosing") {
	ChoiceTree* cloned_tree = dynamic_cast<ChoiceTree* >(choices_tree.Clone());
	ChoiceTree* cyclic_tree = new CyclicChoiceTree(cloned_tree);

	vector<uint64> picked_values;

	for (int i = 0; i < 5; ++ i) {
	  uint64 picked_value = cyclic_tree->CyclicChoose()->Value();
	  EXPECT(std::find(picked_values.begin(), picked_values.end(), picked_value) == picked_values.end()); // not expecting duplicated value
	  // << "picked value " << dec << picked_value << endl;
	  picked_values.push_back(picked_value);
	}

	for (int i = 0; i < 5; ++ i) {
	  uint64 new_value = cyclic_tree->CyclicChoose()->Value();
	  // << "new value " << dec << new_value << endl;
	  auto find_iter = std::find(picked_values.begin(), picked_values.end(), new_value);
	  EXPECT(find_iter != picked_values.end()); // expect to find the value in the all-values list.
	  picked_values.erase(find_iter);
	}
	delete cyclic_tree; // cloned_tree will be deleted by cyclic_tree
      }
    }
},

CASE( "test case set 2 for Choices module" ) {

    SETUP( "setup testing with a simple choices tree" )  {
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
      
      SECTION( "test ChoicesSet methods" ) {
	auto choices_tree = my_set.FindChoiceTree("Tree0");
	EXPECT(choices_tree == choices_tree0);

	EXPECT_THROWS_AS(my_set.FindChoiceTree("TreeX"), ChoicesError);
	ChoiceTree* choices_tree1_clone = dynamic_cast<ChoiceTree* >(choices_tree1->Clone()); // Will be deleted by the function call 
	EXPECT_FAIL(my_set.AddChoiceTree(choices_tree1_clone), "duplicated-choices-tree");
      }
    }  
},
CASE( "test case set for range choices" ) {
  SETUP( "setup testing with a simple choices tree" )  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    ChoiceTree* choices_tree0 = new ChoiceTree("Tree0", 0, 10);
    Choice* choice1 = new RangeChoice("Choice 1", "1-6", 10);
    choices_tree0->AddChoice(choice1);
    Choice* choice2 = new RangeChoice("Choice 2", "7-9", 80);
    choices_tree0->AddChoice(choice2);
    Choice* choice3 = new RangeChoice("Choice 3", "10", 10);
    choices_tree0->AddChoice(choice3);
    
    uint32 lower, high;
   
    auto pChoice1 = dynamic_cast<RangeChoice*>(choices_tree0->Chosen(0));
    pChoice1->GetRange(lower, high);
    EXPECT(lower == 1u);
    EXPECT(high == 6u);

    auto pChoice2 = dynamic_cast<RangeChoice*>(choices_tree0->Chosen(10));
    pChoice2->GetRange(lower, high);
    EXPECT(lower == 7u);
    EXPECT(high == 9u);

    auto pChoice3 = dynamic_cast<RangeChoice*>(choices_tree0->Chosen(90));
    pChoice3->GetRange(lower, high);
    EXPECT(lower == 10u);
    EXPECT(high == 10u);
  }
}

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
