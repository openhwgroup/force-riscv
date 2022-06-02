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
#include "BntHookManager.h"

#include "lest/lest.hpp"

#include "Log.h"

using text = std::string;

using namespace std;
using namespace Force;

const lest::test specification[] = {

CASE( "Test case: Manager Bnt Hook" ) {

    SETUP( "setup BntHookManager" )  {
      BntHookManager bs_manager;
      bs_manager.Setup(nullptr);

        SECTION( "push and pop operation" ) {
          EXPECT(bs_manager.ToString() == "BntHookManager BntHook id: 1, sequence name: default, function name: defaultFunction ");
          
          auto id1 = bs_manager.AllocateId();
          BntHook bs_hook1(id1, "seq1", "func1");
          bs_manager.PushBntHook(bs_hook1);

          auto id2 = bs_manager.AllocateId();
          BntHook bs_hook2(id2, "", "func2");
          bs_manager.PushBntHook(bs_hook2);
          EXPECT(bs_manager.ToString() == "BntHookManager BntHook id: 1, sequence name: default, function name: defaultFunction BntHook id: 2, sequence name: seq1, function name: func1 BntHook id: 3, sequence name: seq1, function name: func2 ");

          bs_manager.RevertBntHook(0); // revert to last one
          EXPECT(bs_manager.ToString() == "BntHookManager BntHook id: 1, sequence name: default, function name: defaultFunction BntHook id: 2, sequence name: seq1, function name: func1 ");

          bs_manager.RevertBntHook(id1);
          EXPECT(bs_manager.ToString() == "BntHookManager BntHook id: 1, sequence name: default, function name: defaultFunction ");
        }

        SECTION("Stress Revert operation") {

            bs_manager.RevertBntHook(1); // revert default Bnt Hook
            EXPECT(bs_manager.ToString() == "BntHookManager ");
            EXPECT_FAIL(bs_manager.RevertBntHook(-1), "no-Bnt-Hook");
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
