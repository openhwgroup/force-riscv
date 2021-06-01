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

#include <MemoryTraits.h>

using text = std::string;
using namespace Force;

const lest::test specification[] = {

CASE("Test MemoryTraits") {

  SETUP("Setup MemoryTraits")  {
    MemoryTraits mem_traits;

    SECTION("Test adding traits") {
      mem_traits.AddTrait(1, 0xff25, 0xffff);
      mem_traits.AddTrait(2, 0xc400, 0xc600);
      EXPECT(mem_traits.HasTrait(1, 0xff25, 0xffff));
      EXPECT(mem_traits.HasTrait(2, 0xc400, 0xc600));
    }

    SECTION("Test adding trait with overlapping range") {
      mem_traits.AddTrait(7, 0x4000, 0x4800);
      mem_traits.AddTrait(7, 0x4600, 0x5000);
      EXPECT(mem_traits.HasTrait(7, 0x4000, 0x5000));
    }

    SECTION("Test checking whether an address range has an associated trait") {
      mem_traits.AddTrait(3, 0x998b0, 0x998b5);
      EXPECT(mem_traits.HasTrait(3, 0x998b0, 0x998b5));
      EXPECT_NOT(mem_traits.HasTrait(2, 0x998b0, 0x998b5));
      EXPECT_NOT(mem_traits.HasTrait(3, 0x1259, 0x12ff));
    }

    SECTION("Test checking whether part of an address range has an associated trait") {
      mem_traits.AddTrait(4, 0x370, 0x480);
      EXPECT(mem_traits.HasTraitPartial(4, 0x470, 0x500));
      EXPECT_NOT(mem_traits.HasTraitPartial(3, 0x470, 0x500));
      EXPECT_NOT(mem_traits.HasTraitPartial(4, 0x300, 0x350));
    }

/*
    SECTION("Test getting memory attributes for a specified address range") {
      mem_traits.AddTrait(5, 0x33000, 0x34000);
      mem_traits.AddTrait(6, 0xf8900, 0xf9000);
      MemoryTraitsRange mem_traits_range = mem_traits.GetMemoryTraitsRange(0x32000, 0xf8500);
      EXPECT(mem_traits_range.GetStartAddress() == 0x33000ull);
      EXPECT(mem_traits_range.GenEndAddress() == 0xf8500ull);
    }
*/
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
