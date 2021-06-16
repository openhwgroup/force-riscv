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

#include <Constraint.h>
#include ARCH_ENUM_HEADER

#include <cstdio>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <vector>

using text = std::string;
using namespace Force;

namespace Force {

  bool has_trait(const MemoryTraitsRegistry& rMemTraitsRegistry, const std::vector<uint32>& rTraitIds, const EMemoryAttributeType trait)
  {
    uint32 trait_id = rMemTraitsRegistry.GetTraitId(trait);
    bool has_trait = any_of(rTraitIds.begin(), rTraitIds.end(),
      [trait_id](cuint32 traitId) { return (traitId == trait_id); });

    return has_trait;
  }

  class MemoryTraitsRegistryTest : public MemoryTraitsRegistry {
  public:
    MemoryTraitsRegistryTest()
      : MemoryTraitsRegistry()
    {
      AddMutuallyExclusiveTraits({EMemoryAttributeType::MainRegion, EMemoryAttributeType::IORegion, EMemoryAttributeType::EmptyRegion});
      AddMutuallyExclusiveTraits({EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::CacheableShared, EMemoryAttributeType::Uncacheable});
      AddThreadTraits({EMemoryAttributeType::CacheableShared, EMemoryAttributeType::Uncacheable});
    }
  };

}

const lest::test specification[] = {

CASE("Test MemoryTraitsRange") {

  SETUP("Setup MemoryTraitsRange")  {
    std::map<uint32, ConstraintSet*> trait_ranges;
    std::unique_ptr<ConstraintSet> trait_range_1(new ConstraintSet(0x5490, 0x54f0));
    trait_ranges.emplace(1, trait_range_1.get());
    std::unique_ptr<ConstraintSet> trait_range_2(new ConstraintSet(0x489b, 0x51c0));
    trait_ranges.emplace(2, trait_range_2.get());

    MemoryTraitsRange mem_traits_range(trait_ranges, 0x4900, 0x54c0);

    SECTION("Test copying a MemoryTraitsRange") {
      MemoryTraitsRange mem_traits_range_copy(mem_traits_range);
      EXPECT(mem_traits_range.IsCompatible(mem_traits_range_copy));
      EXPECT(mem_traits_range_copy.IsCompatible(mem_traits_range));
      EXPECT_NOT(mem_traits_range_copy.IsEmpty());
    }

    SECTION("Test merging two MemoryTraitsRanges") {
      MemoryTraitsRange other_mem_traits_range({1}, 0x4900, 0x54c0);
      std::unique_ptr<MemoryTraitsRange> merged_mem_traits_range(mem_traits_range.CreateMergedMemoryTraitsRange(other_mem_traits_range));
      EXPECT_NOT(mem_traits_range.IsCompatible(*merged_mem_traits_range));
      EXPECT_NOT(merged_mem_traits_range->IsCompatible(mem_traits_range));
      EXPECT_NOT(other_mem_traits_range.IsCompatible(*merged_mem_traits_range));
      EXPECT_NOT(merged_mem_traits_range->IsCompatible(other_mem_traits_range));
      EXPECT_NOT(merged_mem_traits_range->IsEmpty());
    }

    SECTION("Test merging two MemoryTraitsRanges with different address ranges") {
      MemoryTraitsRange other_mem_traits_range(std::vector<uint32>({2, 3}), 0x4000, 0x54c0);
      EXPECT_FAIL(mem_traits_range.CreateMergedMemoryTraitsRange(other_mem_traits_range), "address-ranges-not-equal");
    }

    SECTION("Test merging two MemoryTraitsRanges when one is empty") {
      MemoryTraitsRange other_mem_traits_range(std::vector<uint32>(), 0x4900, 0x54c0);
      std::unique_ptr<MemoryTraitsRange> merged_mem_traits_range(other_mem_traits_range.CreateMergedMemoryTraitsRange(mem_traits_range));
      EXPECT(mem_traits_range.IsCompatible(*merged_mem_traits_range));
      EXPECT(merged_mem_traits_range->IsCompatible(mem_traits_range));
      EXPECT(other_mem_traits_range.IsCompatible(*merged_mem_traits_range));
      EXPECT(merged_mem_traits_range->IsCompatible(other_mem_traits_range));
      EXPECT_NOT(merged_mem_traits_range->IsEmpty());
    }
  }
},

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

    SECTION("Test getting address ranges associated with a trait") {
      mem_traits.AddTrait(5, 0x5926, 0x5a30);
      mem_traits.AddTrait(5, 0x4280, 0x4290);
      const ConstraintSet* address_ranges = mem_traits.GetTraitAddressRanges(5);
      EXPECT(address_ranges->ToSimpleString() == "0x4280-0x4290,0x5926-0x5a30");
    }

    SECTION("Test getting address ranges associated with a trait that has no associated address ranges") {
      EXPECT(mem_traits.GetTraitAddressRanges(6) == nullptr);
    }

    SECTION("Test getting memory traits for a specified address range") {
      mem_traits.AddTrait(5, 0x33000, 0x34000);
      mem_traits.AddTrait(6, 0xf8900, 0xf9000);

      std::unique_ptr<MemoryTraitsRange> mem_traits_range(mem_traits.CreateMemoryTraitsRange(0x32000, 0xf8500));
      EXPECT_NOT(mem_traits_range->IsEmpty());

      std::unique_ptr<MemoryTraitsRange> empty_mem_traits_range(mem_traits.CreateMemoryTraitsRange(0x24000, 0x32000));
      EXPECT(empty_mem_traits_range->IsEmpty());
    }
  }
},

CASE("Test MemoryTraitsRegistry") {

  SETUP("Setup MemoryTraitsRegistry")  {
    MemoryTraitsRegistryTest mem_traits_registry;

    SECTION("Test adding traits") {
      uint32 amo_logical_trait_id = mem_traits_registry.AddTrait(EMemoryAttributeType::AMOLogical);
      uint32 trait_1_id = mem_traits_registry.AddTrait("Trait 1");
      EXPECT(mem_traits_registry.GetTraitId(EMemoryAttributeType::AMOLogical) == amo_logical_trait_id);
      EXPECT(mem_traits_registry.GetTraitId("Trait 1") == trait_1_id);
    }

    SECTION("Test adding duplicate traits") {
      mem_traits_registry.AddTrait(EMemoryAttributeType::Incoherent);
      mem_traits_registry.AddTrait("Trait 2");
      EXPECT_FAIL(mem_traits_registry.AddTrait(EMemoryAttributeType::Incoherent), "trait-already-exists");
      EXPECT_FAIL(mem_traits_registry.AddTrait("Trait 2"), "trait-already-exists");
    }

    SECTION("Test getting non-existent trait IDs") {
      EXPECT(mem_traits_registry.GetTraitId(EMemoryAttributeType::AMOSwap) == 0u);
      EXPECT(mem_traits_registry.GetTraitId("Trait 7") == 0u);
    }

    SECTION("Test getting trait name") {
      uint32 main_region_trait_id = mem_traits_registry.GetTraitId(EMemoryAttributeType::MainRegion);
      EXPECT(mem_traits_registry.GetTraitName(main_region_trait_id) == "MainRegion");
    }

    SECTION("Test getting non-existent trait name") {
      EXPECT(mem_traits_registry.GetTraitName(MAX_UINT32) == "");
    }

    SECTION("Test requesting trait IDs") {
      uint32 uncacheable_trait_id = mem_traits_registry.RequestTraitId(EMemoryAttributeType::Uncacheable);
      uint32 trait_3_id = mem_traits_registry.RequestTraitId("Trait 3");
      EXPECT(mem_traits_registry.RequestTraitId(EMemoryAttributeType::Uncacheable) == uncacheable_trait_id);
      EXPECT(mem_traits_registry.RequestTraitId("Trait 3") == trait_3_id);
    }

    SECTION("Test mutually exclusive traits") {
      std::vector<uint32> cacheable_trait_exclusive_ids;
      mem_traits_registry.GetMutuallyExclusiveTraitIds(mem_traits_registry.GetTraitId(EMemoryAttributeType::CacheableShared), cacheable_trait_exclusive_ids);

      EXPECT(cacheable_trait_exclusive_ids.size() == 2ull);
      EXPECT(has_trait(mem_traits_registry, cacheable_trait_exclusive_ids, EMemoryAttributeType::EmptyRegion));
      EXPECT(has_trait(mem_traits_registry, cacheable_trait_exclusive_ids, EMemoryAttributeType::Uncacheable));

      std::vector<uint32> io_region_exclusive_ids;
      mem_traits_registry.GetMutuallyExclusiveTraitIds(mem_traits_registry.GetTraitId(EMemoryAttributeType::IORegion), io_region_exclusive_ids);
      EXPECT(has_trait(mem_traits_registry, io_region_exclusive_ids, EMemoryAttributeType::MainRegion));
      EXPECT(has_trait(mem_traits_registry, io_region_exclusive_ids, EMemoryAttributeType::EmptyRegion));
    }

    SECTION("Test thread-specifc traits") {
      uint32 cacheable_trait_id = mem_traits_registry.GetTraitId(EMemoryAttributeType::CacheableShared);
      EXPECT(mem_traits_registry.IsThreadTrait(cacheable_trait_id));

      uint32 uncacheable_trait_id = mem_traits_registry.GetTraitId(EMemoryAttributeType::Uncacheable);
      EXPECT(mem_traits_registry.IsThreadTrait(uncacheable_trait_id));

      uint32 main_region_trait_id = mem_traits_registry.GetTraitId(EMemoryAttributeType::MainRegion);
      EXPECT_NOT(mem_traits_registry.IsThreadTrait(main_region_trait_id));
    }
  }
},

CASE("Test MemoryTraitsManager") {

  SETUP("Setup MemoryTraitsManager")  {
    MemoryTraitsManager mem_traits_manager(new MemoryTraitsRegistryTest());

    SECTION("Test adding global traits") {
      mem_traits_manager.AddTrait(0, EMemoryAttributeType::IORegion, 0x7300, 0x7400);
      mem_traits_manager.AddTrait(1, "Trait 1", 0x7200, 0x7380);

      MemoryTraitsRegistry* mem_traits_registry = mem_traits_manager.GetMemoryTraitsRegistry();
      uint32 main_region_trait_id = mem_traits_registry->GetTraitId(EMemoryAttributeType::MainRegion);
      mem_traits_manager.AddTrait(0, main_region_trait_id, 0x3200, 0x3240);

      EXPECT(mem_traits_manager.HasTrait(0, EMemoryAttributeType::IORegion, 0x7300, 0x7400));
      EXPECT(mem_traits_manager.HasTrait(1, EMemoryAttributeType::IORegion, 0x7300, 0x7400));
      EXPECT(mem_traits_manager.HasTrait(0, "Trait 1", 0x7200, 0x7380));
      EXPECT(mem_traits_manager.HasTrait(1, "Trait 1", 0x7200, 0x7380));
      EXPECT(mem_traits_manager.HasTrait(0, EMemoryAttributeType::MainRegion, 0x3200, 0x3240));
      EXPECT(mem_traits_manager.HasTrait(1, EMemoryAttributeType::MainRegion, 0x3200, 0x3240));
    }

    SECTION("Test adding thread-specific traits") {
      mem_traits_manager.AddTrait(0, EMemoryAttributeType::CacheableShared, 0x3920, 0x3950);
      mem_traits_manager.AddTrait(1, EMemoryAttributeType::CacheableShared, 0x9800, 0x9a00);
      mem_traits_manager.AddTrait(1, EMemoryAttributeType::Uncacheable, 0x7ff28, 0x7ff2f);

      EXPECT(mem_traits_manager.HasTrait(0, EMemoryAttributeType::CacheableShared, 0x3920, 0x3950));
      EXPECT_NOT(mem_traits_manager.HasTrait(0, EMemoryAttributeType::CacheableShared, 0x9800, 0x9a00));
      EXPECT_NOT(mem_traits_manager.HasTrait(0, EMemoryAttributeType::Uncacheable, 0x7ff28, 0x7ff2f));
      EXPECT_NOT(mem_traits_manager.HasTrait(1, EMemoryAttributeType::CacheableShared, 0x3920, 0x3950));
      EXPECT(mem_traits_manager.HasTrait(1, EMemoryAttributeType::CacheableShared, 0x9800, 0x9a00));
      EXPECT(mem_traits_manager.HasTrait(1, EMemoryAttributeType::Uncacheable, 0x7ff28, 0x7ff2f));
    }

    SECTION("Test adding mutually-exclusive traits") {
      mem_traits_manager.AddTrait(0, EMemoryAttributeType::IORegion, 0x6320, 0x637f);
      EXPECT_FAIL(mem_traits_manager.AddTrait(0, EMemoryAttributeType::MainRegion, 0x6250, 0x6350), "trait-conflict");
    }

    SECTION("Test checking whether an address range is associated with a non-existent trait") {
      EXPECT_NOT(mem_traits_manager.HasTrait(0, EMemoryAttributeType::AMOLogical, 0x450, 0x470));
    }

    SECTION("Test getting address ranges associated with a global trait") {
      mem_traits_manager.AddTrait(0, EMemoryAttributeType::EmptyRegion, 0xff94, 0xffb0);
      MemoryTraitsRegistry* mem_traits_registry = mem_traits_manager.GetMemoryTraitsRegistry();
      const ConstraintSet* address_ranges = mem_traits_manager.GetTraitAddressRanges(0, mem_traits_registry->GetTraitId(EMemoryAttributeType::EmptyRegion));
      EXPECT(address_ranges->ToSimpleString() == "0xff94-0xffb0");
    }

    SECTION("Test getting address ranges associated with a thread-specific trait") {
      mem_traits_manager.AddTrait(1, EMemoryAttributeType::CacheableShared, 0x330, 0x37f);
      MemoryTraitsRegistry* mem_traits_registry = mem_traits_manager.GetMemoryTraitsRegistry();
      const ConstraintSet* address_ranges = mem_traits_manager.GetTraitAddressRanges(1, mem_traits_registry->GetTraitId(EMemoryAttributeType::CacheableShared));
      EXPECT(address_ranges->ToSimpleString() == "0x330-0x37f");
    }

    SECTION("Test getting address ranges associated with a trait that has no associated address ranges") {
      EXPECT(mem_traits_manager.GetTraitAddressRanges(2, 7) == nullptr);
    }

    SECTION("Test getting memory traits for a specified address range") {
      mem_traits_manager.AddTrait(2, EMemoryAttributeType::MainRegion, 0x4420, 0x4460);
      mem_traits_manager.AddTrait(2, "Trait 5", 0x7800, 0xff00);

      std::unique_ptr<MemoryTraitsRange> mem_traits_range(mem_traits_manager.CreateMemoryTraitsRange(2, 0x4420, 0xff00));
      EXPECT_NOT(mem_traits_range->IsEmpty());

      std::unique_ptr<MemoryTraitsRange> empty_mem_traits_range(mem_traits_manager.CreateMemoryTraitsRange(2, 0x4461, 0x77ff));
      EXPECT(empty_mem_traits_range->IsEmpty());
    }

    SECTION("Test getting memory traits for a specified address range with thread-specific traits") {
      mem_traits_manager.AddTrait(1, "Trait 6", 0xb20, 0xb80);
      mem_traits_manager.AddTrait(1, EMemoryAttributeType::CacheableShared, 0x900, 0x9ff);

      std::unique_ptr<MemoryTraitsRange> mem_traits_range(mem_traits_manager.CreateMemoryTraitsRange(1, 0x930, 0xac0));
      EXPECT_NOT(mem_traits_range->IsEmpty());

      std::unique_ptr<MemoryTraitsRange> empty_mem_traits_range(mem_traits_manager.CreateMemoryTraitsRange(1, 0x800, 0x8ff));
      EXPECT(empty_mem_traits_range->IsEmpty());
    }
  }
},

CASE("Test MemoryTraitsJson") {

  SETUP("Setup MemoryTraitsJson")  {
    std::unique_ptr<MemoryTraitsRegistry> mem_traits_registry(new MemoryTraitsRegistryTest());
    MemoryTraitsJson mem_traits_json(mem_traits_registry.get());

    MemoryTraits mem_traits;
    mem_traits.AddTrait(mem_traits_registry->GetTraitId(EMemoryAttributeType::CacheableShared), 0x7360, 0x7c80);
    mem_traits.AddTrait(mem_traits_registry->GetTraitId(EMemoryAttributeType::Uncacheable), 0x2680, 0x2970);
    mem_traits.AddTrait(mem_traits_registry->GetTraitId(EMemoryAttributeType::Uncacheable), 0x3220, 0x3b20);
    mem_traits.AddTrait(mem_traits_registry->RequestTraitId("Trait 1"), 0x2400, 0x2a00);
    mem_traits.AddTrait(mem_traits_registry->RequestTraitId("Trait 2"), 0x93d0, 0x9b30);

    std::unique_ptr<MemoryTraitsRange> mem_traits_range(mem_traits.CreateMemoryTraitsRange(0x2800, 0x97ff));

    std::string out_file_path = "./memory_traits_json_text.txt";

    SECTION("Test dumping a MemoryTraitsRange to JSON") {
      std::ofstream out_file(out_file_path);

      mem_traits_json.DumpTraits(out_file, *mem_traits_range);
      out_file.close();

      std::ifstream in_file(out_file_path);
      std::string output;
      getline(in_file, output);
      EXPECT(output == "\"ArchMemAttributes\": [{\"Name\": \"CacheableShared\", \"Ranges\": [{\"StartPhysAddr\": 29536, \"EndPhysAddr\": 31872}]}, {\"Name\": \"Uncacheable\", \"Ranges\": [{\"StartPhysAddr\": 10240, \"EndPhysAddr\": 10608}, {\"StartPhysAddr\": 12832, \"EndPhysAddr\": 15136}]}], \"ImplMemAttributes\": [{\"Name\": \"Trait 1\", \"Ranges\": [{\"StartPhysAddr\": 10240, \"EndPhysAddr\": 10752}]}, {\"Name\": \"Trait 2\", \"Ranges\": [{\"StartPhysAddr\": 37840, \"EndPhysAddr\": 38911}]}]");

      in_file.close();
      remove(out_file_path.c_str());
    }

    SECTION("Test dumping an empty MemoryTraitsRange to JSON") {
      std::unique_ptr<MemoryTraitsRange> empty_mem_traits_range(mem_traits.CreateMemoryTraitsRange(0xc0c00, 0xe4800));

      std::ofstream out_file(out_file_path);

      mem_traits_json.DumpTraits(out_file, *empty_mem_traits_range);
      out_file.close();

      std::ifstream in_file(out_file_path);
      std::string output;
      getline(in_file, output);
      EXPECT(output == "\"ArchMemAttributes\": [], \"ImplMemAttributes\": []");

      in_file.close();
      remove(out_file_path.c_str());
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
