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
#include <AddressTagging.h>
#include <UtilityFunctions.h>

using text = std::string;
using namespace Force;

namespace Force {

  class AddressTaggingTest : public AddressTagging {
  public:
    AddressTaggingTest(cbool enabledForData, cbool enabledForInstruction)
      : mEnabledForData(enabledForData), mEnabledForInstruction(enabledForInstruction)
    {
    }

    bool CanTagAddress(cuint64 address, cbool isInstruction) const override
    {
      if (isInstruction) {
        return mEnabledForInstruction;
      }
      else {
        return mEnabledForData;
      }
    }

  private:
    cbool mEnabledForData;
    cbool mEnabledForInstruction;
  };

}

uint64 apply_tag(cuint64 address, cuint64 tag_value) {
  constexpr uint64 tag_position = 56;
  uint64 tag_mask = get_mask64(0xff, tag_position);
  return ((address & ~tag_mask) | (tag_value << tag_position));
}

void test_tag_address(lest::env& lest_env, cuint64 address)
{
  AddressTaggingTest addr_tagging_always(true, true);
  AddressTaggingTest addr_tagging_data_only(true, false);
  AddressTaggingTest addr_tagging_never(false, false);
  constexpr uint64 tag_value = 0x8b;
  uint64 tagged_address = apply_tag(address, tag_value);

  uint64 tagged_address_result = addr_tagging_always.TagAddress(address, tag_value, false);
  EXPECT(tagged_address_result == tagged_address);
  tagged_address_result = addr_tagging_data_only.TagAddress(address, tag_value, true);
  EXPECT(tagged_address_result == address);
  tagged_address_result = addr_tagging_data_only.TagAddress(address, tag_value, false);
  EXPECT(tagged_address_result == tagged_address);
  tagged_address_result = addr_tagging_never.TagAddress(address, tag_value, false);
  EXPECT(tagged_address_result == address);
}

void test_untag_address(lest::env& lest_env, cuint64 address)
{
  AddressTaggingTest addr_tagging_always(true, true);
  AddressTaggingTest addr_tagging_data_only(true, false);
  AddressTaggingTest addr_tagging_never(false, false);
  constexpr uint64 tag_value = 0xc1;
  uint64 tagged_address = apply_tag(address, tag_value);

  uint64 untagged_address = addr_tagging_always.UntagAddress(tagged_address, false);
  EXPECT(untagged_address == address);
  untagged_address = addr_tagging_data_only.UntagAddress(tagged_address, true);
  EXPECT(untagged_address == tagged_address);
  untagged_address = addr_tagging_data_only.UntagAddress(tagged_address, false);
  EXPECT(untagged_address == address);
  untagged_address = addr_tagging_never.UntagAddress(tagged_address, false);
  EXPECT(untagged_address == tagged_address);
}

void test_get_tag_value(lest::env& lest_env, cuint64 address)
{
  AddressTaggingTest addr_tagging_always(true, true);
  constexpr uint64 tag_value = 0x4e;
  uint64 tagged_address = apply_tag(address, tag_value);

  uint64 tag_value_result = addr_tagging_always.GetTagValue(tagged_address);
  EXPECT(tag_value_result == tag_value);
}

const lest::test specification[] = {

CASE( "Test AddressTagging" ) {

  SETUP( "Setup AddressTagging" )  {
    AddressTaggingTest addr_tagging_always(true, true);
    constexpr uint64 lower_range_address = 0x00007f654432998b;
    constexpr uint64 upper_range_address = 0xffff079223bea97c;

    SECTION( "Test tagging address with value" ) {
      test_tag_address(lest_env, lower_range_address);
      test_tag_address(lest_env, upper_range_address);
    }

    // We would like to test tagging addresses randomly. However, it's possible the generated tag would be 0x00, which
    // would make it impossible to determine whether the tag was applied or not. While this would occur infrequently,
    // we don't want to create a unit test that generates false failures, so we defer random tag generation testing to
    // the regression level.

    SECTION( "Test untagging address" ) {
      test_untag_address(lest_env, lower_range_address);
      test_untag_address(lest_env, upper_range_address);
    }

    SECTION( "Test getting tag value" ) {
      test_get_tag_value(lest_env, lower_range_address);
      test_get_tag_value(lest_env, upper_range_address);
    }

    SECTION( "Test AreTaggingCapacitiesEqual() method" ) {
      EXPECT(addr_tagging_always.AreTaggingCapacitiesEqual(lower_range_address, 0xffff7856923881cd, false));
      EXPECT(addr_tagging_always.AreTaggingCapacitiesEqual(upper_range_address, 0x00000056008923ea, false));
    }
  }
},

};

int main(int argc, char * argv[])
{
  Force::Logger::Initialize();
  int ret = lest::run(specification, argc, argv);
  Force::Logger::Destroy();
  return ret;
}
