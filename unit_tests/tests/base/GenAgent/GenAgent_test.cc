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
#include "GenAgent.h"

#include <vector>

#include "lest/lest.hpp"

#include "Log.h"
#include "UtilityFunctions.h"

using text = std::string;
using namespace std;
using namespace Force;

namespace Force {

  class GenInstructionAgent : public GenAgent {
  public:
    GenInstructionAgent() : GenAgent() { } //!< Constructor.

    Object* Clone() const override  //!< Return a cloned GenInstructionAgent object of the same type and content.
    {
      return new GenInstructionAgent(*this);
    }

    const char* Type() const override { return "GenInstructionAgent"; } //!< Return type of the GenInstructionAgent object.

    virtual EGenAgentType GenAgentType() const { return EGenAgentType::GenInstructionAgent; } //!< Return type of the generator agent.
  protected:
    GenInstructionAgent(const GenInstructionAgent& other) : GenAgent(other) { } //!< Copy constructor, do not copy the request pointer.
  };

}

const lest::test specification[] = {

CASE( "test set 1 for GenAgent module" ) {

    SETUP( "setup description" )  {
      vector<GenAgent* > gen_agents;
      check_enum_size(EGenAgentTypeSize);
      gen_agents.assign(EGenAgentTypeSize, nullptr);

      GenInstructionAgent * instr_agent = new GenInstructionAgent();
      EGenAgentType agent_type = instr_agent->GenAgentType();
      gen_agents[int(agent_type)] = instr_agent;
      EXPECT( gen_agents[int(agent_type)] == instr_agent );

      for (auto agent_ptr : gen_agents) {
	delete agent_ptr;
      }
    }
}};

int main( int argc, char * argv[] )
{
    Force::Logger::Initialize();
    int ret = lest::run( specification, argc, argv );
    Force::Logger::Destroy();
    return ret;
}
