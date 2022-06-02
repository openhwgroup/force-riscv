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
#include "AddressReuseMode.h"

#include "lest/lest.hpp"

#include "Log.h"

using text = std::string;
using namespace Force;

const lest::test specification[] = {

CASE( "Test AddressReuseMode" ) {

  SETUP( "Setup AddressReuseMode" )  {
    AddressReuseMode addr_reuse_mode;

    SECTION( "Test enabling reuse type" ) {
      addr_reuse_mode.EnableReuseType(EAddressReuseType::ReadAfterWrite);
      EXPECT(addr_reuse_mode.IsReuseTypeEnabled(EAddressReuseType::ReadAfterWrite));
    }

    SECTION( "Test disabling reuse type" ) {
      addr_reuse_mode.EnableReuseType(EAddressReuseType::WriteAfterRead);
      addr_reuse_mode.DisableReuseType(EAddressReuseType::WriteAfterRead);
      EXPECT_NOT(addr_reuse_mode.IsReuseTypeEnabled(EAddressReuseType::WriteAfterRead));
    }

    SECTION( "Test disabling reuse types" ) {
      for (EAddressReuseTypeBaseType i = 0; i < EAddressReuseTypeSize; i++) {
        addr_reuse_mode.EnableReuseType(EAddressReuseType(1 << i));
      }

      addr_reuse_mode.DisableAllReuseTypes();
      for (EAddressReuseTypeBaseType i = 0; i < EAddressReuseTypeSize; i++) {
        EXPECT_NOT(addr_reuse_mode.IsReuseTypeEnabled(EAddressReuseType(1 << i)));
      }
    }
  }
},

};

int main(int argc, char* argv[])
{
  Force::Logger::Initialize();
  int ret = lest::run(specification, argc, argv);
  Force::Logger::Destroy();
  return ret;
}
