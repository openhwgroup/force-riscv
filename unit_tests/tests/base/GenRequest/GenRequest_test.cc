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
#include "GenRequest.h"

#include "lest/lest.hpp"

#include "GenRequestQueue.h"
#include "Log.h"

using text = std::string;

const lest::test specification[] = {

CASE( "Test set 1 for GenRequest module" ) {

  using namespace Force;
  using namespace std;

    SETUP( "setup GenRequest module scenario" )   {
        //-----------------------------------------
	// include necessary setup code here
	//-----------------------------------------
      GenRequestQueue gen_req_queue;
      GenInstructionRequest* i_req_1 = new GenInstructionRequest("ADDI##RISCV");
      GenInstructionRequest* i_req_2 = new GenInstructionRequest("SUB##RISCV");
      GenInstructionRequest* i_req_3 = new GenInstructionRequest("MUL##RISCV");

      gen_req_queue.PrependRequest(i_req_1);
      auto round_id = gen_req_queue.StartRound(); // mark start of the generation round.
      gen_req_queue.PrependRequest(i_req_2);
      gen_req_queue.PrependRequest(i_req_3);

      EXPECT( gen_req_queue.Size() == 3u);

      SECTION( "test section description" ) {
	// Test the generation round loop
	while (!gen_req_queue.RoundFinished(round_id)) {
	  GenRequest* gen_req = gen_req_queue.PopFront();
	  delete gen_req;
	}
	EXPECT( gen_req_queue.Size() == 1u );
	GenRequest* gen_req_last = gen_req_queue.PopFront();
	EXPECT( gen_req_last->ToString() == "GenInstructionRequest: ADDI##RISCV" );
	delete gen_req_last;
      }
    }
},

CASE( "Test set 2 for GenRequest module" ) {

  using namespace Force;
  using namespace std;

    SETUP( "setup GenRequest module scenario" )   {
        //-----------------------------------------
	// include necessary setup code here
	//-----------------------------------------
      GenRequestQueue gen_req_queue;
      GenInstructionRequest* i_req = new GenInstructionRequest("XORI##RISCV");

      auto round_id = gen_req_queue.StartRound(); // mark start of the generation round.
      gen_req_queue.PrependRequest(i_req);

      SECTION( "test section description" ) {
	// Test the generation round loop
	while (!gen_req_queue.RoundFinished(round_id)) {
	  GenRequest* gen_req = gen_req_queue.PopFront();
	  delete gen_req;
	}
	EXPECT( gen_req_queue.Size() == 0u );
      }
    }
},

CASE( "Test set 3 for GenRequest module" ) {

  using namespace Force;
  using namespace std;

    SETUP( "setup GenRequest module scenario" )   {
        //-----------------------------------------
	// include necessary setup code here
	//-----------------------------------------
      GenRequestQueue gen_req_queue;
      GenInstructionRequest* i_req_1 = new GenInstructionRequest("ADDI##RISCV");
      GenInstructionRequest* i_req_2 = new GenInstructionRequest("SUB##RISCV");
      GenInstructionRequest* i_req_3 = new GenInstructionRequest("MUL##RISCV");

      gen_req_queue.PrependRequest(i_req_1);
      GenRequestQueue::ConstGRequestQIter round_end_iter = gen_req_queue.StartRound(); // mark start of the generation round.
      gen_req_queue.PrependRequest(i_req_2);
      gen_req_queue.PrependRequest(i_req_3);

      const GenRequest* gen_req_end = *round_end_iter;
      EXPECT( gen_req_end->ToString() == "GenInstructionRequest: ADDI##RISCV" );
      EXPECT( gen_req_queue.Size() == 3u);

      SECTION( "test section description" ) {
	// Test the generation round loop
        uint8_t test_idx = 0;
	while (!gen_req_queue.RoundFinished(round_end_iter)) {
	  GenRequest* gen_req = gen_req_queue.PopFront();
	  LOG(trace) << "Test idx: " << unsigned(test_idx) << " Test Instruction " <<  gen_req->ToString() << std::endl;
          EXPECT(( ((test_idx == 1) && gen_req->ToString() == "GenInstructionRequest: SUB##RISCV") ||
	   	   ((test_idx == 0) && gen_req->ToString() == "GenInstructionRequest: MUL##RISCV")));
	  delete gen_req;
          test_idx++;
	}
	EXPECT( gen_req_queue.Size() == 1u );
	GenRequest* gen_req_last = gen_req_queue.PopFront();
	EXPECT( gen_req_last->ToString() == "GenInstructionRequest: ADDI##RISCV" );
	delete gen_req_last;
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
