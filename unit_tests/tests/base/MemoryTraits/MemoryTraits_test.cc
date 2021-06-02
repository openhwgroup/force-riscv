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

#include ARCH_ENUM_HEADER

#include <set>
#include <vector>

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

CASE("Test MemoryTraitsRegistry") {

  SETUP("Setup MemoryTraitsRegistry")  {
    MemoryTraitsRegistry mem_traits_registry;

    SECTION("Test adding traits") {
      uint32 device_trait_id = mem_traits_registry.AddTrait(EMemoryAttributeType::Device);
      uint32 trait_1_id = mem_traits_registry.AddTrait("Trait 1");
      EXPECT(mem_traits_registry.GetTraitId(EMemoryAttributeType::Device) == device_trait_id);
      EXPECT(mem_traits_registry.GetTraitId("Trait 1") == trait_1_id);
    }

    SECTION("Test adding duplicate traits") {
      mem_traits_registry.AddTrait(EMemoryAttributeType::NormalCacheable);
      mem_traits_registry.AddTrait("Trait 2");
      EXPECT_FAIL(mem_traits_registry.AddTrait(EMemoryAttributeType::NormalCacheable), "trait-already-exists");
      EXPECT_FAIL(mem_traits_registry.AddTrait("Trait 2"), "trait-already-exists");
    }

    SECTION("Test getting non-existent trait IDs") {
      EXPECT(mem_traits_registry.GetTraitId(EMemoryAttributeType::Device) == 0u);
      EXPECT(mem_traits_registry.GetTraitId("Trait 7") == 0u);
    }

    SECTION("Test requesting trait IDs") {
      uint32 non_cacheable_trait_id = mem_traits_registry.RequestTraitId(EMemoryAttributeType::NormalNonCacheable);
      uint32 trait_3_id = mem_traits_registry.RequestTraitId("Trait 3");
      EXPECT(mem_traits_registry.RequestTraitId(EMemoryAttributeType::NormalNonCacheable) == non_cacheable_trait_id);
      EXPECT(mem_traits_registry.RequestTraitId("Trait 3") == trait_3_id);
    }

    SECTION("Test adding mutually exclusive traits") {
      mem_traits_registry.AddMutuallyExclusiveTraits({EMemoryAttributeType::NormalCacheable, EMemoryAttributeType::NormalNonCacheable});
      mem_traits_registry.AddMutuallyExclusiveTraits({"Trait 4", "Trait 5", "Trait 6"});

      std::vector<uint32> cacheable_trait_exclusive_ids;
      mem_traits_registry.GetMutuallyExclusiveTraitIds(mem_traits_registry.GetTraitId(EMemoryAttributeType::NormalCacheable), cacheable_trait_exclusive_ids);
      uint32 non_cacheable_trait_id = mem_traits_registry.GetTraitId(EMemoryAttributeType::NormalNonCacheable);
      EXPECT(cacheable_trait_exclusive_ids.size() == 1ull);
      EXPECT(cacheable_trait_exclusive_ids[0] == non_cacheable_trait_id);

      std::vector<uint32> trait_5_exclusive_ids;
      mem_traits_registry.GetMutuallyExclusiveTraitIds(mem_traits_registry.GetTraitId("Trait 5"), trait_5_exclusive_ids);
      uint32 trait_4_id = mem_traits_registry.GetTraitId("Trait 4");
      uint32 trait_6_id = mem_traits_registry.GetTraitId("Trait 6");
      EXPECT(trait_5_exclusive_ids.size() == 2ull);

      bool found_trait_4_id = any_of(trait_5_exclusive_ids.begin(), trait_5_exclusive_ids.end(),
        [trait_4_id](cuint32 traitId) { return (traitId == trait_4_id); });

      EXPECT(found_trait_4_id);

      bool found_trait_6_id = any_of(trait_5_exclusive_ids.begin(), trait_5_exclusive_ids.end(),
        [trait_6_id](cuint32 traitId) { return (traitId == trait_6_id); });

      EXPECT(found_trait_6_id);
    }

    SECTION("Test adding empty list of mutually exclusive traits") {
      std::vector<EMemoryAttributeType> enum_traits;
      mem_traits_registry.AddMutuallyExclusiveTraits(enum_traits);
      std::vector<std::string> string_traits;
      mem_traits_registry.AddMutuallyExclusiveTraits(string_traits);

      std::vector<uint32> exclusive_ids;
      mem_traits_registry.GetMutuallyExclusiveTraitIds(1, exclusive_ids);
      EXPECT(exclusive_ids.empty());
    }
  }
},

CASE("Test MemoryTraitsManager") {

  SETUP("Setup MemoryTraitsManager")  {
    MemoryTraitsManager mem_traits_manager;

    SECTION("Test adding traits") {
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
