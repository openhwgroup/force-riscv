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

#include <Architectures.h>
#include <UnitTestUtilities.h>

namespace Force {

  class ArchInfoTEST2 : public ArchInfo {
  public:
    ArchInfoTEST2(const std::string& name)
      : ArchInfo(name)
    {

    }
  };
}

using text = std::string;

const lest::test specification[] = {

CASE( "Test set 1 for ArchInfo module" ) {

    SETUP( "Setup Architectures" )  {
      using namespace Force;
      using namespace std;

      Architectures::Initialize();

      vector<ArchInfo*> arch_info_objs;
      arch_info_objs.push_back(new ArchInfoTest("TEST1"));

      Architectures * arch_top = Architectures::Instance();
      arch_top->AssignArchInfoObjects(arch_info_objs);

      ArchInfo* arch_info = arch_top->DefaultArchInfo();
      EXPECT( arch_info != nullptr );
      EXPECT( arch_info->Name() == "TEST1" );

      EXPECT_FAIL(arch_top->AssignArchInfoObjects(arch_info_objs), "Re-assign-arch-info-objects");
      Architectures::Destroy();
    }
},

CASE( "Test set 2 for ArchInfo module" ) {
    SETUP( "Setup Architectures" )  {
      using namespace Force;
      using namespace std;

      Architectures::Initialize();

      vector<ArchInfo*> arch_info_objs;
      arch_info_objs.push_back(new ArchInfoTest("TEST1"));
      arch_info_objs.push_back(new ArchInfoTEST2("TEST2"));

      Architectures * arch_top = Architectures::Instance();
      arch_top->AssignArchInfoObjects(arch_info_objs);

      ArchInfo* arch_info = arch_top->DefaultArchInfo();
      EXPECT( arch_info == nullptr );
      Architectures::Destroy();
    }
},

CASE( "Test set 3 for ArchInfo module" ) {
    SETUP( "Setup Architectures" )  {
      using namespace Force;
      using namespace std;

      Architectures::Initialize();

      vector<ArchInfo*> arch_info_objs;
      arch_info_objs.push_back(new ArchInfoTest("TEST1"));
      arch_info_objs.push_back(new ArchInfoTEST2("TEST1"));

      Architectures * arch_top = Architectures::Instance();
      EXPECT_FAIL(arch_top->AssignArchInfoObjects(arch_info_objs), "duplicated-arch-info-name");

      delete (arch_info_objs.back()); // avoid memory leak
      Architectures::Destroy();
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
