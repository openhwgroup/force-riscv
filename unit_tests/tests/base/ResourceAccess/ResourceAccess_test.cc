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
#include "lest/lest.hpp"

#include "Constraint.h"
#include "GenException.h"
#include "Log.h"

#define protected public
#define private public
#include "ResourceAccess.h"

using text = std::string;
using namespace std;
using namespace Force;

/*!
  Typical setup for a ResourceAccessQueue object.
 */
static void setup_ResourceAccessQueue(ResourceAccessQueue& my_queue, uint32 queue_len = 32)
{
  my_queue.Setup(queue_len);
  {
    ResourceTypeAges* gpr_ages = new ResourceTypeAges(EResourceType::GPR);
    gpr_ages->Setup(32);
    my_queue.mTypeAges.push_back(gpr_ages);
  }
  {
    ResourceTypeAges* fpr_ages = new ResourceTypeAges(EResourceType::FPR);
    fpr_ages->Setup(32);
    my_queue.mTypeAges.push_back(fpr_ages);
  }
  {
    ResourceTypeAges* pdr_ages = new ResourceTypeAges(EResourceType::PREDREG);
    pdr_ages->Setup(16);
    my_queue.mTypeAges.push_back(pdr_ages);
  }

  {
    ResourceTypeEntropy* gpr_entropy = new ResourceTypeEntropy(EResourceType::GPR);
    my_queue.mTypeEntropies.push_back(gpr_entropy);
  }
  {
    ResourceTypeEntropy* fpr_entropy = new ResourceTypeEntropy(EResourceType::FPR);
    my_queue.mTypeEntropies.push_back(fpr_entropy);
  }
  {
    ResourceTypeEntropy* pdr_entropy = new ResourceTypeEntropy(EResourceType::PREDREG);
    my_queue.mTypeEntropies.push_back(pdr_entropy);
  }

}

/*!
  Local data structure to help initialize a ResourceAccessStage object.
*/
struct ResourceAccessTuple {
public:
  ResourceAccessTuple(const string& access, const string& type, const string& indices)
    : mAccess(access), mType(type), mIndices(indices)
  {
  }

public:
  string mAccess;
  string mType;
  string mIndices;
};

typedef vector<ResourceAccessTuple> ResourceAccessStageTestData;

static void setup_ResourceAccessStage(ResourceAccessStage& my_stage, const vector<ResourceAccessTuple>& access_array)
{
  for (uint32 i = 0; i < access_array.size(); ++ i) {
    const ResourceAccessTuple& res_acc_tuple = access_array[i];
    ERegAttrType access = string_to_ERegAttrType(res_acc_tuple.mAccess);
    EResourceType res_type = string_to_EResourceType(res_acc_tuple.mType );
    ConstraintSet* constr = new ConstraintSet(res_acc_tuple.mIndices);
    my_stage.RecordAccess(access, res_type, constr);
  }
}

/*!
  A wrapper class around vector<ResourceAccessStageTestData* > representing a resource-access testing sequence.
 */
class AccessStageSequence {
public:
  explicit AccessStageSequence(initializer_list<ResourceAccessStageTestData> initList) : mSequence()
  {
    copy(initList.begin(), initList.end(), back_inserter(mSequence));
  }

  ~AccessStageSequence(){}

  void PopulateAccessQueue (ResourceAccessQueue& accessQueue)
  {
    for (auto stage_data : mSequence) {
      auto stage_ptr = accessQueue.CreateHotResource();
      setup_ResourceAccessStage(*stage_ptr, stage_data);
      accessQueue.Commit(stage_ptr);
    }
  }

private:
  AccessStageSequence() : mSequence() {}
private:
  vector<ResourceAccessStageTestData > mSequence;
};

const lest::test specification[] = {

CASE( "Test ResourceAccessStage class" ) {

  SETUP( "ResourceAccessStage test setup" )  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    ResourceAccessStage my_accesses;

    SECTION( "do some initial structural checking here" ) {
      EXPECT( my_accesses.mSourceAccesses.size() == uint32(EResourceTypeSize) );
      EXPECT( my_accesses.mDestAccesses.size() == uint32(EResourceTypeSize) );
    }

    SECTION( "test ResourceAccessStage::RecordAccess method" ) {
      //---------------------------------------------------------------
      // include necessary operations on the object being tested here
      //---------------------------------------------------------------
      ConstraintSet* gpr_read_constr1 = new ConstraintSet("1-2"); // ownership transfer to my_accesses.
      my_accesses.RecordAccess(ERegAttrType::Read, EResourceType::GPR, gpr_read_constr1);
      EXPECT(my_accesses.GetSourceAccess(EResourceType::GPR) == gpr_read_constr1);

      // the new read constraint set would be merged with existing constraint set
      ConstraintSet* gpr_read_constr2 = new ConstraintSet("3-4"); // ownership transfer to my_accesses.
      my_accesses.RecordAccess(ERegAttrType::Read, EResourceType::GPR, gpr_read_constr2);
      EXPECT(my_accesses.GetSourceAccess(EResourceType::GPR)->ToSimpleString() == "0x1-0x4");

      // add write access
      ConstraintSet* gpr_write_constr1 = new ConstraintSet("3-4"); // ownership transfer to my_accesses.
      my_accesses.RecordAccess(ERegAttrType::Write, EResourceType::GPR, gpr_write_constr1);
      EXPECT(my_accesses.GetSourceAccess(EResourceType::GPR)->ToSimpleString() == "0x1-0x4"); // ensure read access not affected.
      EXPECT(my_accesses.GetDestAccess(EResourceType::GPR)->ToSimpleString() == "0x3-0x4");

      // the new write constraint set would be merged with existing constraint set
      ConstraintSet* gpr_write_constr2 = new ConstraintSet("5-6"); // ownership transfer to my_accesses.
      my_accesses.RecordAccess(ERegAttrType::ReadWrite, EResourceType::GPR, gpr_write_constr2); // ReadWrite should be treat as write here
      EXPECT(my_accesses.GetDestAccess(EResourceType::GPR)->ToSimpleString() == "0x3-0x6");
      EXPECT(my_accesses.GetSourceAccess(EResourceType::GPR)->ToSimpleString() == "0x1-0x4"); // ensure read access not affected.

      // add new FPR write constaint set.
      ConstraintSet* fpr_write_constr = new ConstraintSet("7-8"); // ownership transfer to my_accesses.
      my_accesses.RecordAccess(ERegAttrType::Write, EResourceType::FPR, fpr_write_constr);
      EXPECT(my_accesses.GetDestAccess(EResourceType::FPR)->ToSimpleString() == "0x7-0x8");
      EXPECT(my_accesses.GetDestAccess(EResourceType::GPR)->ToSimpleString() == "0x3-0x6"); // ensure GPR write access not affected.
      EXPECT(my_accesses.GetSourceAccess(EResourceType::GPR)->ToSimpleString() == "0x1-0x4"); // ensure GPR read access not affected.

      // add new FPR read constraint set
      ConstraintSet* fpr_read_constr = new ConstraintSet("10"); // ownership transfer to my_accesses.
      my_accesses.RecordAccess(ERegAttrType::Read, EResourceType::FPR, fpr_read_constr);
      EXPECT(my_accesses.GetSourceAccess(EResourceType::FPR)->ToSimpleString() == "0xa");
      EXPECT(my_accesses.GetDestAccess(EResourceType::GPR)->ToSimpleString() == "0x3-0x6"); // ensure GPR write access not affected.
      EXPECT(my_accesses.GetSourceAccess(EResourceType::GPR)->ToSimpleString() == "0x1-0x4"); // ensure GPR read access not affected.

      // Now test resulting dependency constraint.
      auto gpr_source_constr = my_accesses.GetDependenceConstraint(EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(gpr_source_constr->ToSimpleString() == "0x1-0x4");
      auto gpr_dest_constr = my_accesses.GetDependenceConstraint(EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(gpr_dest_constr->ToSimpleString() == "0x3-0x6");
      auto fpr_source_constr = my_accesses.GetDependenceConstraint(EResourceType::FPR, EDependencyType::OnSource);
      EXPECT(fpr_source_constr->ToSimpleString() == "0xa");
      auto fpr_dest_constr = my_accesses.GetDependenceConstraint(EResourceType::FPR, EDependencyType::OnTarget);
      EXPECT(fpr_dest_constr->ToSimpleString() == "0x7-0x8");
    }
  }
},

CASE( "Test ResourceTypeAges class" ) {

  SETUP( "ResourceAccessStage test setup" )  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    ResourceTypeAges my_ages(EResourceType::GPR);
    my_ages.Setup(32);

    SECTION( "initial structural checking" ) {
      EXPECT( my_ages.mAges.size() == 32u);
    }

    SECTION( "test basic states after initial setup" ) {
      auto age_entry = my_ages.GetAccessAge(10);
      EXPECT(age_entry.Age() == uint32(-1));
      EXPECT(age_entry.AccessType() == EAccessAgeType::Invalid);
      EXPECT(age_entry.ResourceType() == EResourceType::GPR);
      EXPECT(age_entry.Index() == 10u);
    }
  }
},

CASE( "Test case 1 ResourceAccessQueue class" ) {

  SETUP( "ResourceAccessQueue test setup" )  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    ResourceAccessQueue my_queue;
    setup_ResourceAccessQueue(my_queue);

    SECTION( "initial structural checking" ) {
      EXPECT(my_queue.mQueue.size() == 32u);
      EXPECT(my_queue.mHistoryLimit == 32u);
      EXPECT(my_queue.mTypeAges[0]->ResourceType() == EResourceType::GPR);
      EXPECT(my_queue.mTypeAges[1]->ResourceType() == EResourceType::FPR);
      EXPECT(my_queue.mTypeEntropies[0]->ResourceType() == EResourceType::GPR);
      EXPECT(my_queue.mTypeEntropies[1]->ResourceType() == EResourceType::FPR);
    }

    SECTION( "test simple case for Commit method, testing the location correctness" ) {
      vector<ResourceAccessTuple> access_array1 = { {"Read", "GPR", "1"}, {"Read", "GPR", "2"}, {"Write", "GPR", "3"} }; // R3 = R1 + R2
      vector<ResourceAccessTuple> access_array2 = { {"Read", "GPR", "4"}, {"Read", "GPR", "5"}, {"Write", "GPR", "6"} }; // R6 = R4 + R5
      vector<ResourceAccessTuple> access_array3 = { {"Read", "GPR", "7"}, {"Read", "GPR", "8"}, {"Write", "GPR", "9"} }; // R9 = R5 + R6

      auto stage1_ptr = my_queue.CreateHotResource();
      setup_ResourceAccessStage(*stage1_ptr, access_array1);
      my_queue.Commit(stage1_ptr);

      auto stage2_ptr = my_queue.CreateHotResource();
      setup_ResourceAccessStage(*stage2_ptr, access_array2);
      my_queue.Commit(stage2_ptr);

      auto stage3_ptr = my_queue.CreateHotResource();
      setup_ResourceAccessStage(*stage3_ptr, access_array3);
      my_queue.Commit(stage3_ptr);

      auto dist1_ptr = my_queue.GetAccessStage(1);
      EXPECT(dist1_ptr == stage3_ptr);
      auto dist2_ptr = my_queue.GetAccessStage(2);
      EXPECT(dist2_ptr == stage2_ptr);
      auto dist3_ptr = my_queue.GetAccessStage(3);
      EXPECT(dist3_ptr == stage1_ptr);
      // << my_queue.ToString() << endl;
    }
  }
},

CASE( "Test case 2 ResourceAccessQueue class" ) {

  SETUP( "ResourceAccessQueue test setup" )  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    ResourceAccessQueue my_queue;
    setup_ResourceAccessQueue(my_queue);
    
    SECTION( "test simple case resource age updating, source removal" ) {
      AccessStageSequence stage_sequence({
	{ {"Read", "GPR", "1"}, {"Read", "GPR", "2"}, {"Write", "GPR", "3"} }, // R3 = R1 + R2
	{ {"Read", "GPR", "4"}, {"Read", "GPR", "5"}, {"Write", "GPR", "6"} }, // R6 = R4 + R5
	{ {"Read", "GPR", "1"}, {"Read", "GPR", "4"}, {"Write", "GPR", "2"} }  // R2 = R1 + R4
      });

      // populate resource access queue with the stage_sequence entries.
      stage_sequence.PopulateAccessQueue(my_queue);

      auto stage1_ptr = my_queue.GetAccessStage(3); // this points to R3 = R1 + R2
      auto stage2_ptr = my_queue.GetAccessStage(2); // this points to R6 = R4 + R5
      auto stage3_ptr = my_queue.GetAccessStage(1); // this points to R2 = R1 + R4     

      EXPECT(stage1_ptr->ToSimpleString() == "+GPR_DST(0x3)"); // source R1, dest R2 should go away due to R2 = R1 + R4 slot now
      EXPECT(stage2_ptr->ToSimpleString() == "+GPR_SRC(0x5)+GPR_DST(0x6)"); // source R4 should go away due to R2 = R1 + R4 slot now
      EXPECT(stage3_ptr->ToSimpleString() == "+GPR_SRC(0x1,0x4)+GPR_DST(0x2)");
      // << my_queue.ToString() << endl;

      auto gpr_entropy = my_queue.mTypeEntropies[0];
      EXPECT(gpr_entropy->SourceEntropy().Entropy() == 3u);
      EXPECT(gpr_entropy->DestEntropy().Entropy() == 3u);
    }

    SECTION( "test simple case resource age updating, source and dest removal" ) {
      AccessStageSequence stage_sequence({
	{ {"Read", "FPR", "1"}, {"Read", "FPR", "2"}, {"Write", "FPR", "3"} }, // V3 = V1 + V2
	{ {"Read", "FPR", "4"}, {"Read", "FPR", "5"}, {"Write", "FPR", "6"} }, // V6 = V4 + V5
	{ {"Read", "FPR", "3"}, {"Read", "FPR", "6"}, {"Write", "FPR", "4"} }  // V4 = V3 + V6
      });

      // populate resource access queue with the stage_sequence entries.
      stage_sequence.PopulateAccessQueue(my_queue);

      auto stage1_ptr = my_queue.GetAccessStage(3); // this points to V3 = V1 + V2
      auto stage2_ptr = my_queue.GetAccessStage(2); // this points to V6 = V4 + V5
      auto stage3_ptr = my_queue.GetAccessStage(1); // this points to V4 = V3 + V6

      EXPECT(stage1_ptr->ToSimpleString() == "+FPR_SRC(0x1-0x2)"); // dest V3 should go away due to V4 = V3 + V6 slot now
      EXPECT(stage2_ptr->ToSimpleString() == "+FPR_SRC(0x5)"); // source V4, dest V6 should go away due to V4 = V3 + V6 slot now
      EXPECT(stage3_ptr->ToSimpleString() == "+FPR_SRC(0x3,0x6)+FPR_DST(0x4)");     
      // << my_queue.ToString() << endl;

      auto gpr_entropy = my_queue.mTypeEntropies[1];
      EXPECT(gpr_entropy->SourceEntropy().Entropy() == 5u);
      EXPECT(gpr_entropy->DestEntropy().Entropy() == 1u);
    }

  }
},

CASE( "Test case  3 ResourceAccessQueue class" ) {
  SETUP( "ResourceAccessQueue test setup" )  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    ResourceAccessQueue my_queue;
    setup_ResourceAccessQueue(my_queue, 2);
    LOG(notice) << "Start test case 3" << endl;
    SECTION( "test simple case retire and resource entropy" ) {
      AccessStageSequence stage_sequence({
	{ {"Read", "FPR", "1"}, {"Read", "FPR", "2"}, {"Write", "FPR", "3"} }, // V3 = V1 + V2
	{ {"Read", "FPR", "4"}, {"Read", "FPR", "5"}, {"Write", "FPR", "6"} }, // V6 = V4 + V5
	{ {"Read", "FPR", "3"}, {"Read", "FPR", "6"}, {"Write", "FPR", "4"} },  // V4 = V3 + V6
	{ {"Read", "FPR", "6"}, {"Read", "FPR", "7"},   {"Read", "FPR", "8"}, {"Write", "FPR", "6"} } // V6 = V6 + V7 + V8
      });

      // populate resource access queue with the stage_sequence entries.
      stage_sequence.PopulateAccessQueue(my_queue);
      
      auto stage1_ptr = my_queue.GetAccessStage(4); // this points to V3 = V1 + V2
      auto stage2_ptr = my_queue.GetAccessStage(3); // this points to V6 = V4 + V5
      auto stage3_ptr = my_queue.GetAccessStage(2); // this points to V4 = V3 + V6
      auto stage4_ptr = my_queue.GetAccessStage(1); // this points to V6 = V6 + V7 + V8
      
      EXPECT(stage1_ptr == stage3_ptr);
      EXPECT(stage2_ptr == stage4_ptr);
      EXPECT(stage3_ptr->ToSimpleString() == "+FPR_SRC(0x3)+FPR_DST(0x4)"); // source R4, dest R6 should go away due to R4 = R3 + R6 slot now
      EXPECT(stage4_ptr->ToSimpleString() == "+FPR_SRC(0x7-0x8)+FPR_DST(0x6)");     
 
      auto fpr_entropy = my_queue.mTypeEntropies[1];
      EXPECT(fpr_entropy->SourceEntropy().Entropy() == 3u);
      EXPECT(fpr_entropy->DestEntropy().Entropy() == 2u);
    }

    SECTION( "test resource entropy state" ) {
      AccessStageSequence stage_sequence({
	{ {"Read", "FPR", "1"}, {"Read", "FPR", "2"}, {"Write", "FPR", "3"} }, // V3 = V1 + V2
	{ {"Read", "FPR", "4"}, {"Read", "FPR", "5"}, {"Write", "FPR", "6"} }, // V6 = V4 + V5
	{ {"Read", "FPR", "3"}, {"Read", "FPR", "6"}, {"Write", "FPR", "3"} },  // V3 = V3 + V6
	{ {"Read", "FPR", "6"}, {"Read", "FPR", "3"}, {"Write", "FPR", "6"} } // V6 = V6 + V3
      });

      // set up entroy turn-on threshold and turn-off threshold
      auto fpr_entropy = my_queue.mTypeEntropies[1];
      fpr_entropy->SourceEntropy().SetThresholds(4, 2);
      fpr_entropy->DestEntropy().SetThresholds(2,1);

      // populate resource access queue with the stage_sequence entries.
      stage_sequence.PopulateAccessQueue(my_queue);
      
      auto stage1_ptr = my_queue.GetAccessStage(4); // this points to V3 = V1 + V2
      auto stage2_ptr = my_queue.GetAccessStage(3); // this points to V6 = V4 + V5
      auto stage3_ptr = my_queue.GetAccessStage(2); // this points to V3 = V3 + V6
      auto stage4_ptr = my_queue.GetAccessStage(1); // this points to V6 = V6 + V3
      
      EXPECT(stage1_ptr == stage3_ptr);
      EXPECT(stage2_ptr == stage4_ptr);
      EXPECT(stage3_ptr->ToSimpleString() == "");
      EXPECT(stage4_ptr->ToSimpleString() == "+FPR_SRC(0x3)+FPR_DST(0x6)");     
 
      EXPECT(fpr_entropy->SourceEntropy().Entropy() == 1u);
      EXPECT(fpr_entropy->DestEntropy().Entropy() == 1u);

      EXPECT( fpr_entropy->SourceEntropy().State() == EEntropyStateType::CoolDown);
 
      EXPECT( fpr_entropy->DestEntropy().State() == EEntropyStateType::Stable);
    }
    
  }
},

CASE( "Test case 4 ResourceAccessQueue class with optiaml constraint lookup" ) {

  SETUP( "ResourceAccessQueue test case 3 setup" )  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    ResourceAccessQueue my_queue;
    setup_ResourceAccessQueue(my_queue);

    SECTION( "test a long sequence of execution with optimal lookup" ) {
      AccessStageSequence stage_sequence({
	{ {"Write", "GPR", "26"}, {"Read", "GPR", "12"} }, // LDR  X26, [X12{, #imm}]  1
	{ {"Write", "GPR", "15"}, {"Read", "GPR",  "5"} }, // LDR  X15, [X5{, #imm}]   2
	{ {"Write", "GPR",  "1"}, {"Read", "GPR", "10"} }, // LDR  X1, [X10{, #imm}]   3
	{ {"Write", "GPR", "13"}, {"Read", "GPR", "22"} }, // LDR  X13, [X22{, #imm}]  4
	{ {"Write", "GPR", "12"}, {"Read", "GPR", "21"} }, // LDR  X12, [X21{, #imm}]  5
	{ {"Write", "GPR", "31"}, {"Read", "GPR", "1" } }, // LDR  XZR, [X1{, #imm}]   6
	{ {"Write", "GPR", "25"}, {"Read", "GPR", "23"} }, // LDR  X25, [X23{, #imm}]  7
	{ {"Write", "GPR", "23"}, {"Read", "GPR", "16"} }, // LDR  X23, [X16{, #imm}]  8
	{ {"Write", "GPR", "20"}, {"Read", "GPR", "15"} }, // LDR  X20, [X15{, #imm}]  9
	{ {"Write", "GPR", "16"}, {"Read", "GPR",  "0"} }, // LDR  X16, [X0{, #imm}]  10
	{ {"Write", "GPR", "27"}, {"Read", "GPR", "24"} }, // LDR  X27, [X24{, #imm}] 11
	{ {"Write", "GPR", "22"}, {"Read", "GPR",  "3"} }, // LDR  X22, [X3{, #imm}]  12
	{ {"Write", "GPR", "29"}, {"Read", "GPR",  "6"} }, // LDR  X29, [X6{, #imm}]  13
	{ {"Write", "GPR",  "8"}, {"Read", "GPR", "19"} }, // LDR  X8, [X19{, #imm}]  14
	{ {"Write", "GPR", "11"}, {"Read", "GPR", "14"} }, // LDR  X11, [X14{, #imm}] 15
	{ {"Write", "GPR",  "7"}, {"Read", "GPR", "21"} }, // LDR  X7, [X21{, #imm}]  16
	{ {"Write", "GPR", "22"}, {"Read", "GPR", "23"} }, // LDR  X22, [X23{, #imm}] 17
	{ {"Write", "GPR", "27"}, {"Read", "GPR", "11"} }, // LDR  X27, [X11{, #imm}] 18
	{ {"Write", "GPR", "10"}, {"Read", "GPR",  "7"} }, // LDR  X10, [X7{, #imm}]  19
	{ {"Write", "GPR", "30"}, {"Read", "GPR", "17"} }, // LDR  X30, [X17{, #imm}] 20
	{ {"Write", "GPR",  "6"}, {"Read", "GPR", "15"} }, // LDR  X6, [X15{, #imm}]  21
	{ {"Write", "GPR", "21"}, {"Read", "GPR", "12"} }, // LDR  X21, [X12{, #imm}] 22
	{ {"Write", "GPR",  "5"}, {"Read", "GPR", "26"} }, // LDR  X5, [X26{, #imm}]  23
	{ {"Write", "GPR", "10"}, {"Read", "GPR", "11"} }, // LDR  X10, [X11{, #imm}] 24
	{ {"Write", "GPR",  "1"}, {"Read", "GPR", "13"} }, // LDR  X1, [X13{, #imm}]  25
	{ {"Write", "GPR", "18"}, {"Read", "GPR", "27"} }, // LDR  X18, [X27{, #imm}] 26
	{ {"Write", "GPR", "21"}, {"Read", "GPR", "31"} }, // LDR  X21, [SP{, #imm}]  27
	{ {"Write", "GPR", "17"}, {"Read", "GPR", "10"} }, // LDR  X17, [X10{, #imm}] 28
	{ {"Write", "GPR", "20"}, {"Read", "GPR", "29"} }, // LDR  X20, [X29{, #imm}] 29
	{ {"Write", "GPR", "12"}, {"Read", "GPR", "25"} }, // LDR  X12, [X25{, #imm}] 30
	{ {"Write", "GPR",  "0"}, {"Read", "GPR", "16"} }, // LDR  X0, [X16{, #imm}]  31
	{ {"Write", "GPR",  "7"}, {"Read", "GPR", "28"} }, // LDR  X7, [X28{, #imm}]  32
	{ {"Write", "GPR",  "3"}, {"Read", "GPR", "18"} }, // LDR  X3, [X18{, #imm}]  33
      });

      // populate resource access queue with the stage_sequence entries.
      stage_sequence.PopulateAccessQueue(my_queue);

      WindowLookUpFar lookup_far;
      WindowLookUpNear lookup_near;

      // check for constraints on instruction 34
      lookup_far.SetRange(1, 20);
      auto dest34_constr = my_queue.GetOptimalResourceConstraint(11, lookup_far, EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(dest34_constr->ToSimpleString() == "0x1a");
      lookup_far.SetRange(21, 30);
      auto src34_constr = my_queue.GetOptimalResourceConstraint(23, lookup_far, EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(src34_constr == nullptr);

      AccessStageSequence instr34({ { {"Write", "GPR", "26"}, {"Read", "GPR", "11"} } }); // LDR  X26, [X11{, #imm]  34
      instr34.PopulateAccessQueue(my_queue); // execute instruction 34

      // check for constraints on instruction 35
      lookup_far.SetRange(21, 30);
      auto dest35_constr = my_queue.GetOptimalResourceConstraint(25, lookup_far, EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(dest35_constr->ToSimpleString() == "0x13");
      lookup_far.SetRange(1, 20);
      auto src35_constr = my_queue.GetOptimalResourceConstraint(18, lookup_far, EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(src35_constr->ToSimpleString() == "0x16");

      AccessStageSequence instr35({ { {"Write", "GPR", "19"}, {"Read", "GPR", "22"} } }); // LDR  X19, [X22{, #imm]  35
      instr35.PopulateAccessQueue(my_queue); // execute instruction 35

      // check for constraints on instruction 36
      lookup_near.SetRange(1, 20);
      auto dest36_constr = my_queue.GetOptimalResourceConstraint(20, lookup_near, EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(dest36_constr->ToSimpleString() == "0x17");
      lookup_near.SetRange(1, 20);
      auto src36_constr = my_queue.GetOptimalResourceConstraint(18, lookup_near, EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(src36_constr->ToSimpleString() == "0x1e");

      AccessStageSequence instr36({ { {"Write", "GPR", "23"}, {"Read", "GPR", "30"} } }); // LDR  X23, [X30{, #imm]  36
      instr36.PopulateAccessQueue(my_queue); // execute instruction 36

      // check for constraints on instruction 37
      lookup_far.SetRange(1, 20);
      auto dest37_constr = my_queue.GetOptimalResourceConstraint(4, lookup_far, EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(dest37_constr->ToSimpleString() == "0x12");
      lookup_far.SetRange(21, 30);
      auto src37_constr = my_queue.GetOptimalResourceConstraint(24, lookup_far, EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(src37_constr->ToSimpleString() == "0x8");

      AccessStageSequence instr37({ { {"Write", "GPR", "18"}, {"Read", "GPR", "8"} } });  // LDR  X18, [X8{, #imm]  37
      instr37.PopulateAccessQueue(my_queue); // execute instruction 37

      // check for constraints on instruction 38
      lookup_far.SetRange(1, 20);
      auto dest38_constr = my_queue.GetOptimalResourceConstraint(11, lookup_far, EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(dest38_constr->ToSimpleString() == "0x1f");
      lookup_near.SetRange(1, 20);
      auto src38_constr = my_queue.GetOptimalResourceConstraint(11, lookup_near, EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(src38_constr->ToSimpleString() == "0x15");

      AccessStageSequence instr38({ { {"Write", "GPR", "31"}, {"Read", "GPR", "21"} } }); // LDR  XZR, [X21{, #imm]  38
      instr38.PopulateAccessQueue(my_queue); // execute instruction 38

      // check for constraints on instruction 39
      lookup_far.SetRange(1, 20);
      auto dest39_constr = my_queue.GetOptimalResourceConstraint(10, lookup_far, EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(dest39_constr->ToSimpleString() == "0x1d");
      lookup_far.SetRange(21, 30);
      auto src39_constr = my_queue.GetOptimalResourceConstraint(30, lookup_far, EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(src39_constr == nullptr);

      AccessStageSequence instr39({ { {"Write", "GPR", "29"}, {"Read", "GPR", "1"} } }); // LDR  X29, [X1{, #imm]  39
      instr39.PopulateAccessQueue(my_queue); // execute instruction 39

      // check for constraints on instruction 40
      lookup_near.SetRange(1, 20);
      auto dest40_constr = my_queue.GetOptimalResourceConstraint(4, lookup_near, EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(dest40_constr->ToSimpleString() == "0x1e");
      lookup_near.SetRange(1, 20);
      auto src40_constr = my_queue.GetOptimalResourceConstraint(2, lookup_near, EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(src40_constr->ToSimpleString() == "0x1f");
     
      //AccessStageSequence instr40 = { { {"Write", "GPR", "30"}, {"Read", "GPR", "31"} } }; // LDR  X30, [SP{, #imm]  40
      //instr40.PopulateAccessQueue(my_queue); // execute instruction 40
      // << "queue now: " << my_queue.ToString() << endl;
    }
  }
},

CASE( "Test case 5 ResourceAccessQueue class with random constraint lookup" ) {

  SETUP( "ResourceAccessQueue test case 4 setup" )  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    ResourceAccessQueue my_queue;
    setup_ResourceAccessQueue(my_queue);

    SECTION( "test a long sequence of execution with random lookup" ) {
      AccessStageSequence stage_sequence({
	{ {"Write", "GPR",  "5"}, {"Read", "GPR",  "4"} }, // LDR  X5,  [X4{, #imm}]   1
	{ {"Write", "GPR", "28"}, {"Read", "GPR", "24"} }, // LDR  X28, [X24{, #imm}]  2
	{ {"Write", "GPR", "11"}, {"Read", "GPR", "19"} }, // LDR  X11, [X19{, #imm}]  3
	{ {"Write", "GPR", "19"}, {"Read", "GPR", "20"} }, // LDR  X19, [X20{, #imm}]  4
	{ {"Write", "GPR", "30"}, {"Read", "GPR", "28"} }, // LDR  X30, [X28{, #imm}]  5
	{ {"Write", "GPR", "22"}, {"Read", "GPR", "18"} }, // LDR  X22, [X18{, #imm}]  6
	{ {"Write", "GPR", "28"}, {"Read", "GPR", "19"} }, // LDR  X28, [X19{, #imm}]  7
	{ {"Write", "GPR", "14"}, {"Read", "GPR",  "3"} }, // LDR  X14, [X3{, #imm}]   8
	{ {"Write", "GPR",  "3"}, {"Read", "GPR",  "1"} }, // LDR  X3,  [X1{, #imm}]   9
	{ {"Write", "GPR",  "6"}, {"Read", "GPR", "15"} }, // LDR  X6,  [X15{, #imm}] 10
	{ {"Write", "GPR", "12"}, {"Read", "GPR", "24"} }, // LDR  X12, [X24{, #imm}] 11
	{ {"Write", "GPR", "30"}, {"Read", "GPR", "12"} }, // LDR  X30, [X12{, #imm}] 12
	{ {"Write", "GPR", "13"}, {"Read", "GPR",  "3"} }, // LDR  X13, [X3{, #imm}]  13
	{ {"Write", "GPR", "27"}, {"Read", "GPR", "14"} }, // LDR  X27, [X14{, #imm}] 14
	{ {"Write", "GPR", "14"}, {"Read", "GPR", "25"} }, // LDR  X14, [X25{, #imm}] 15
	{ {"Write", "GPR", "19"}, {"Read", "GPR",  "9"} }, // LDR  X19, [X9{, #imm}]  16
	{ {"Write", "GPR",  "8"}, {"Read", "GPR", "28"} }, // LDR  X8,  [X28{, #imm}] 17
	{ {"Write", "GPR",  "6"}, {"Read", "GPR", "13"} }, // LDR  X6,  [X13{, #imm}] 18
	{ {"Write", "GPR", "31"}, {"Read", "GPR",  "2"} }, // LDR  XZR, [X2{, #imm}]  19
	{ {"Write", "GPR",  "0"}, {"Read", "GPR",  "5"} }, // LDR  X0,  [X5{, #imm}]  20
	{ {"Write", "GPR", "29"}, {"Read", "GPR",  "0"} }, // LDR  X29, [X0{, #imm}]  21
	{ {"Write", "GPR",  "5"}, {"Read", "GPR", "31"} }, // LDR  X5,  [SP{, #imm}]  22
	{ {"Write", "GPR",  "0"}, {"Read", "GPR", "17"} }, // LDR  X0,  [X17{, #imm}] 23
	{ {"Write", "GPR",  "2"}, {"Read", "GPR", "22"} }, // LDR  X2,  [X22{, #imm}] 24
	{ {"Write", "GPR",  "4"}, {"Read", "GPR",  "0"} }, // LDR  X4,  [X0{, #imm}]  25
	{ {"Write", "GPR", "12"}, {"Read", "GPR",  "7"} }, // LDR  X12, [X7{, #imm}]  26
	{ {"Write", "GPR",  "7"}, {"Read", "GPR", "11"} }, // LDR  X7,  [X11{, #imm}] 27
	{ {"Write", "GPR", "18"}, {"Read", "GPR",  "2"} }, // LDR  X18, [X2{, #imm}]  28
	{ {"Write", "GPR", "20"}, {"Read", "GPR", "30"} }, // LDR  X20, [X30{, #imm}] 29
	{ {"Write", "GPR", "17"}, {"Read", "GPR", "27"} }, // LDR  X17, [X27{, #imm}] 30
	{ {"Write", "GPR", "22"}, {"Read", "GPR",  "4"} }, // LDR  X22, [X4{, #imm}]  31
	{ {"Write", "GPR",  "2"}, {"Read", "GPR",  "6"} }, // LDR  X2,  [X6{, #imm}]  32
	{ {"Write", "GPR", "24"}, {"Read", "GPR", "14"} }, // LDR  X24, [X14{, #imm}] 33
      });

      // populate resource access queue with the stage_sequence entries.
      stage_sequence.PopulateAccessQueue(my_queue);

      // check for constraints on instruction 34
      auto dest34_constr = my_queue.GetRandomResourceConstraint(21, 30, EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(dest34_constr->ToSimpleString() == "0x1,0x3,0xf");
      auto src34_constr = my_queue.GetRandomResourceConstraint(21, 30, EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(src34_constr == nullptr);

      AccessStageSequence instr34({ { {"Write", "GPR", "15"}, {"Read", "GPR", "15"} } }); // LDR  X15, [X15{, #imm]  34
      instr34.PopulateAccessQueue(my_queue); // execute instruction 34

      // check for constraints on instruction 35
      auto dest35_constr = my_queue.GetRandomResourceConstraint(21, 30, EResourceType::GPR, EDependencyType::OnSource);
      EXPECT(dest35_constr->ToSimpleString() == "0x1,0x3");
      auto src35_constr = my_queue.GetRandomResourceConstraint(21, 30, EResourceType::GPR, EDependencyType::OnTarget);
      EXPECT(src35_constr == nullptr);

      //AccessStageSequence instr35 = { { {"Write", "GPR", "1"}, {"Read", "GPR", "14"} } }; // LDR  X1, [X14{, #imm]  35
      //instr35.PopulateAccessQueue(my_queue); // execute instruction 35
      // << "queue now: " << my_queue.ToString() << endl;
    }
  }
}

};

int main( int argc, char * argv[] )
{
    Force::Logger::Initialize();
    int ret = lest::run( specification, argc, argv );
    Force::Logger::Destroy();
    return ret;
}
