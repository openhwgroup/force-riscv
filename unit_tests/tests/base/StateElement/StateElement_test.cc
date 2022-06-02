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
#include "StateElement.h"

#include <memory>

#include "lest/lest.hpp"

#include "Defines.h"
#include "Enums.h"
#include "Log.h"
#include ARCH_ENUM_HEADER

using text = std::string;
using namespace Force;

const lest::test specification[] = {

CASE("Test MemoryStateElement") {

  SETUP("Setup MemoryStateElement")  {
    MemoryStateElement state_elem(0x3298, 0xFFC3298577B8D920, MAX_UINT64, 1);

    SECTION("Test cloning") {
      std::unique_ptr<MemoryStateElement> clone_state_elem(dynamic_cast<MemoryStateElement*>(state_elem.Clone()));
      std::vector<uint64> expected_values = {0xFFC3298577B8D920};
      EXPECT(clone_state_elem->GetValues() == expected_values);
      std::vector<uint64> expected_masks = {MAX_UINT64};
      EXPECT(clone_state_elem->GetMasks() == expected_masks);
      EXPECT(clone_state_elem->GetStartAddress() == 0x3298ull);
    }

    SECTION("Test converting to a string") {
      EXPECT(state_elem.ToString() == "0x3298");
    }

    SECTION("Test getting a string describing the type") {
      EXPECT(state_elem.Type() == "MemoryStateElement");
    }

    SECTION("Test getting the name") {
      EXPECT(state_elem.GetName() == "0x3298");
    }

    SECTION("Test getting the StateElement type") {
      EXPECT(state_elem.GetStateElementType() == EStateElementType::Memory);
    }

    SECTION("Test getting the values") {
      std::vector<uint64> expected_values = {0xFFC3298577B8D920};
      EXPECT(state_elem.GetValues() == expected_values);
    }

    SECTION("Test getting the masks") {
      std::vector<uint64> expected_masks = {MAX_UINT64};
      EXPECT(state_elem.GetMasks() == expected_masks);
    }

    SECTION("Test getting the priority") {
      EXPECT(state_elem.GetPriority() == 1u);
    }

    SECTION("Test merging with another MemoryStateElement") {
      MemoryStateElement state_elem_a(0xF5B20, 0x00009987C34B0000, 0x0000FFFFFFFF0000, 1);
      MemoryStateElement state_elem_b(0xF5B20, 0x0000000000008564, 0x000000000000FFFF, 2);
      EXPECT(state_elem_a.CanMerge(state_elem_b));
      EXPECT(state_elem_b.CanMerge(state_elem_a));
      state_elem_a.Merge(state_elem_b);

      std::vector<uint64> expected_values = {0x00009987C34B8564};
      EXPECT(state_elem_a.GetValues() == expected_values);
      std::vector<uint64> expected_masks = {0x0000FFFFFFFFFFFF};
      EXPECT(state_elem_a.GetMasks() == expected_masks);
      EXPECT(state_elem_a.GetPriority() == 1u);
    }

    SECTION("Test merging with another MemoryStateElement of higher priority (lower priority number)") {
      MemoryStateElement state_elem_a(0xC9278, 0x53276F0000000000, 0xFFFFFF0000000000, 3);
      MemoryStateElement state_elem_b(0xC9278, 0x000000007600009B, 0x00000000FF0000FF, 2);
      EXPECT(state_elem_a.CanMerge(state_elem_b));
      EXPECT(state_elem_b.CanMerge(state_elem_a));
      state_elem_a.Merge(state_elem_b);

      std::vector<uint64> expected_values = {0x53276F007600009B};
      EXPECT(state_elem_a.GetValues() == expected_values);
      std::vector<uint64> expected_masks = {0xFFFFFF00FF0000FF};
      EXPECT(state_elem_a.GetMasks() == expected_masks);
      EXPECT(state_elem_a.GetPriority() == 2u);
    }

    SECTION("Test merging with another MemoryStateElement with an overlapping mask") {
      MemoryStateElement state_elem_a(0xFFFFFF70, 0x0037FF0000000000, 0x00FFFF0000000000, 1);
      MemoryStateElement state_elem_b(0xFFFFFF70, 0x00054020900C015A, MAX_UINT64, 1);
      EXPECT_NOT(state_elem_a.CanMerge(state_elem_b));
      EXPECT_NOT(state_elem_b.CanMerge(state_elem_a));
      EXPECT_FAIL(state_elem_a.Merge(state_elem_b), "state-element-merge-failure");
    }

    SECTION("Test merging with a MemoryStateElement with a different address") {
      MemoryStateElement state_elem_a(0x39C2D0, 0xF285C3D905840000, 0xFFFFFFFFFFFF0000, 1);
      MemoryStateElement state_elem_b(0x3958F8, 0x0000000000001289, 0x000000000000FFFF, 2);
      EXPECT_NOT(state_elem_a.CanMerge(state_elem_b));
      EXPECT_NOT(state_elem_b.CanMerge(state_elem_a));
      EXPECT_FAIL(state_elem_a.Merge(state_elem_b), "state-element-merge-failure");
    }

    SECTION("Test getting the starting address") {
      EXPECT(state_elem.GetStartAddress() == 0x3298ull);
    }

    SECTION("Test identifying a MemoryStateElement as a duplicate") {
      MemoryStateElement duplicate_state_elem(0x3298, 0xA7132930ED11F959, MAX_UINT64, 2);
      EXPECT(state_elem.IsDuplicate(duplicate_state_elem));
    }

    SECTION("Test identifying a MemoryStateElement as not a duplicate") {
      MemoryStateElement different_state_elem(0xB86E362854FC, 0xFFC3298577B8D920, MAX_UINT64, 1);
      EXPECT_NOT(state_elem.IsDuplicate(different_state_elem));
    }
  }
},

CASE("Test RegisterStateElement") {

  SETUP("Setup RegisterStateElement")  {
    RegisterStateElement state_elem(EStateElementType::VectorRegister, "V5", 5, {0xDFEC0437F47B4887, 0x08CFB21FA0EB7F0F}, {MAX_UINT64, MAX_UINT64}, 10);

    SECTION("Test cloning") {
      std::unique_ptr<RegisterStateElement> clone_state_elem(dynamic_cast<RegisterStateElement*>(state_elem.Clone()));
      std::vector<uint64> expected_values = {0xDFEC0437F47B4887, 0x08CFB21FA0EB7F0F};
      EXPECT(clone_state_elem->GetValues() == expected_values);
      std::vector<uint64> expected_masks = {MAX_UINT64, MAX_UINT64};
      EXPECT(clone_state_elem->GetMasks() == expected_masks);
      EXPECT(clone_state_elem->GetRegisterIndex() == 5u);
    }

    SECTION("Test converting to a string") {
      EXPECT(state_elem.ToString() == "V5");
    }

    SECTION("Test getting a string describing the type") {
      EXPECT(state_elem.Type() == "RegisterStateElement");
    }

    SECTION("Test getting the name") {
      EXPECT(state_elem.GetName() == "V5");
    }

    SECTION("Test getting the StateElement type") {
      EXPECT(state_elem.GetStateElementType() == EStateElementType::VectorRegister);
    }

    SECTION("Test getting the values") {
      std::vector<uint64> expected_values = {0xDFEC0437F47B4887, 0x08CFB21FA0EB7F0F};
      EXPECT(state_elem.GetValues() == expected_values);
    }

    SECTION("Test getting the masks") {
      std::vector<uint64> expected_masks = {MAX_UINT64, MAX_UINT64};
      EXPECT(state_elem.GetMasks() == expected_masks);
    }

    SECTION("Test getting the priority") {
      EXPECT(state_elem.GetPriority() == 10u);
    }

    SECTION("Test merging with another RegisterStateElement") {
      // The Q extension is not currently supported by Force. However, this test does not depend on
      // that support and covers a valuable case.
      RegisterStateElement state_elem_a(EStateElementType::FloatingPointRegister, "Q17", 17, {0x40EC0872FA5D6012, 0x0}, {MAX_UINT64, 0x0}, 7);
      RegisterStateElement state_elem_b(EStateElementType::FloatingPointRegister, "Q17", 17, {0x0, 0x1E1A5E31090EB737}, {0x0, MAX_UINT64}, 4);
      EXPECT(state_elem_a.CanMerge(state_elem_b));
      EXPECT(state_elem_b.CanMerge(state_elem_a));
      state_elem_a.Merge(state_elem_b);

      std::vector<uint64> expected_values = {0x40EC0872FA5D6012, 0x1E1A5E31090EB737};
      EXPECT(state_elem_a.GetValues() == expected_values);
      std::vector<uint64> expected_masks = {MAX_UINT64, MAX_UINT64};
      EXPECT(state_elem_a.GetMasks() == expected_masks);
      EXPECT(state_elem_a.GetPriority() == 4u);
    }

    SECTION("Test merging with a RegisterStateElement with a different type") {
      RegisterStateElement state_elem_a(EStateElementType::GPR, "x3", 3, {0x56E7762C00000000}, {0xFFFFFFFF00000000}, 2);

      // This RegisterStateElement has an incongruous name in order to validate the type-checking
      // logic in Merge()
      RegisterStateElement state_elem_b(EStateElementType::SystemRegister, "x3", 0x100, {0x000000009F8252C0}, {0x00000000FFFFFFFF}, 2);

      EXPECT_NOT(state_elem_a.CanMerge(state_elem_b));
      EXPECT_NOT(state_elem_b.CanMerge(state_elem_a));
      EXPECT_FAIL(state_elem_a.Merge(state_elem_b), "state-element-merge-failure");
    }

    SECTION("Test merging with a RegisterStateElement with a different name") {
      RegisterStateElement state_elem_a(EStateElementType::GPR, "x21", 21, {0xC2F95D884FCF9F4A}, {MAX_UINT64}, 5);
      RegisterStateElement state_elem_b(EStateElementType::GPR, "x19", 19, {0xAEDBBCA106618434}, {MAX_UINT64}, 11);
      EXPECT_FAIL(state_elem_a.Merge(state_elem_b), "state-element-merge-failure");
    }

    SECTION("Test merging with a RegisterStateElement with a different number of values") {
      // The Q extension is not currently supported by Force. However, this test does not depend on
      // that support and covers a valuable case.
      RegisterStateElement state_elem_a(EStateElementType::FloatingPointRegister, "Q11", 11, {0x0000008A00F714FA, 0x17CEC5D84EB45ADE}, {0x000000FFFFFFFFFF, MAX_UINT64}, 4);

      // This RegisterStateElement has an incongruous number of values in order to validate the
      // value count logic in Merge()
      RegisterStateElement state_elem_b(EStateElementType::FloatingPointRegister, "Q11", 11, {0x78D2A30000000000}, {0xFFFFFF0000000000}, 67);

      EXPECT_NOT(state_elem_a.CanMerge(state_elem_b));
      EXPECT_NOT(state_elem_b.CanMerge(state_elem_a));
      EXPECT_FAIL(state_elem_a.Merge(state_elem_b), "state-element-merge-failure");
    }

    SECTION("Test merging with empty RegisterStateElements") {
      RegisterStateElement state_elem_a(EStateElementType::SystemRegister, "mtvec", 0x305, {}, {}, 9);
      RegisterStateElement state_elem_b(EStateElementType::SystemRegister, "mtvec", 0x305, {}, {}, 15);
      EXPECT(state_elem_a.CanMerge(state_elem_b));
      EXPECT(state_elem_b.CanMerge(state_elem_a));
      state_elem_a.Merge(state_elem_b);

      std::vector<uint64> expected_values = {};
      EXPECT(state_elem_a.GetValues() == expected_values);
      std::vector<uint64> expected_masks = {};
      EXPECT(state_elem_a.GetMasks() == expected_masks);
      EXPECT(state_elem_a.GetPriority() == 9u);
    }

    SECTION("Test getting the register index") {
      EXPECT(state_elem.GetRegisterIndex() == 5u);
    }

    SECTION("Test identifying a RegisterStateElement as a duplicate") {
      RegisterStateElement duplicate_state_elem(EStateElementType::VectorRegister, "V5", 5, {0xD8094D1B2880629E, 0xE63AC01A9EA5DD7C}, {MAX_UINT64, MAX_UINT64}, 5);
      EXPECT(state_elem.IsDuplicate(duplicate_state_elem));
    }

    SECTION("Test identifying a RegisterStateElement as not a duplicate due to a different type") {
      // This RegisterStateElement has an incongruous name in order to validate the type-checking
      // logic in IsDuplicate()
      RegisterStateElement different_state_elem(EStateElementType::FloatingPointRegister, "V5", 5, {0xDFEC0437F47B4887, 0x08CFB21FA0EB7F0F}, {MAX_UINT64, MAX_UINT64}, 10);
      EXPECT_NOT(state_elem.IsDuplicate(different_state_elem));
    }

    SECTION("Test identifying a RegisterStateElement as not a duplicate due to a different name") {
      RegisterStateElement different_state_elem(EStateElementType::VectorRegister, "V11", 11, {0xDFEC0437F47B4887, 0x08CFB21FA0EB7F0F}, {MAX_UINT64, MAX_UINT64}, 10);
      EXPECT_NOT(state_elem.IsDuplicate(different_state_elem));
    }
  }
},

CASE("Test VmContextStateElement") {

  SETUP("Setup VmContextStateElement")  {
    VmContextStateElement state_elem("satp", "MODE", 0x9, 5);

    SECTION("Test cloning") {
      std::unique_ptr<VmContextStateElement> clone_state_elem(dynamic_cast<VmContextStateElement*>(state_elem.Clone()));
      std::vector<uint64> expected_values = {0x9};
      EXPECT(clone_state_elem->GetValues() == expected_values);
      std::vector<uint64> expected_masks = {MAX_UINT64};
      EXPECT(clone_state_elem->GetMasks() == expected_masks);
      EXPECT(clone_state_elem->GetRegisterName() == "satp");
      EXPECT(clone_state_elem->GetRegisterFieldName() == "MODE");
    }

    SECTION("Test converting to a string") {
      EXPECT(state_elem.ToString() == "satp.MODE");
    }

    SECTION("Test getting a string describing the type") {
      EXPECT(state_elem.Type() == "VmContextStateElement");
    }

    SECTION("Test getting the name") {
      EXPECT(state_elem.GetName() == "satp.MODE");
    }

    SECTION("Test getting the StateElement type") {
      EXPECT(state_elem.GetStateElementType() == EStateElementType::VmContext);
    }

    SECTION("Test getting the values") {
      std::vector<uint64> expected_values = {0x9};
      EXPECT(state_elem.GetValues() == expected_values);
    }

    SECTION("Test getting the masks") {
      std::vector<uint64> expected_masks = {MAX_UINT64};
      EXPECT(state_elem.GetMasks() == expected_masks);
    }

    SECTION("Test getting the priority") {
      EXPECT(state_elem.GetPriority() == 5u);
    }

    SECTION("Verify that VmContextStateElements can't be merged") {
      VmContextStateElement state_elem_a("satp", "MODE", 0x0, 20);
      VmContextStateElement state_elem_b("satp", "MODE", 0x8, 25);
      EXPECT_NOT(state_elem_a.CanMerge(state_elem_b));
      EXPECT_NOT(state_elem_b.CanMerge(state_elem_a));
      EXPECT_FAIL(state_elem_a.Merge(state_elem_b), "state-element-merge-failure");
    }

    SECTION("Test getting the register name") {
      EXPECT(state_elem.GetRegisterName() == "satp");
    }

    SECTION("Test getting the register field name") {
      EXPECT(state_elem.GetRegisterFieldName() == "MODE");
    }

    SECTION("Test identifying a VmContextStateElement as a duplicate") {
      VmContextStateElement duplicate_state_elem("satp", "MODE", 0x9, 5);
      EXPECT(state_elem.IsDuplicate(duplicate_state_elem));
    }

  }
},

CASE("Test PrivilegeLevelStateElement") {

  SETUP("Setup PrivilegeLevelStateElement")  {
    PrivilegeLevelStateElement state_elem(EPrivilegeLevelType::S, 3);

    SECTION("Test cloning") {
      std::unique_ptr<PrivilegeLevelStateElement> clone_state_elem(dynamic_cast<PrivilegeLevelStateElement*>(state_elem.Clone()));
      std::vector<uint64> expected_values = {0x1};
      EXPECT(clone_state_elem->GetValues() == expected_values);
      std::vector<uint64> expected_masks = {MAX_UINT64};
      EXPECT(clone_state_elem->GetMasks() == expected_masks);
    }

    SECTION("Test converting to a string") {
      EXPECT(state_elem.ToString() == "S");
    }

    SECTION("Test getting a string describing the type") {
      EXPECT(state_elem.Type() == "PrivilegeLevelStateElement");
    }

    SECTION("Test getting the name") {
      EXPECT(state_elem.GetName() == "S");
    }

    SECTION("Test getting the StateElement type") {
      EXPECT(state_elem.GetStateElementType() == EStateElementType::PrivilegeLevel);
    }

    SECTION("Test getting the values") {
      std::vector<uint64> expected_values = {0x1};
      EXPECT(state_elem.GetValues() == expected_values);
    }

    SECTION("Test getting the masks") {
      std::vector<uint64> expected_masks = {MAX_UINT64};
      EXPECT(state_elem.GetMasks() == expected_masks);
    }

    SECTION("Test getting the priority") {
      EXPECT(state_elem.GetPriority() == 3u);
    }

    SECTION("Verify that PrivilegeLevelStateElements can't be merged") {
      PrivilegeLevelStateElement state_elem_a(EPrivilegeLevelType::M, 10);
      PrivilegeLevelStateElement state_elem_b(EPrivilegeLevelType::U, 7);
      EXPECT_NOT(state_elem_a.CanMerge(state_elem_b));
      EXPECT_NOT(state_elem_b.CanMerge(state_elem_a));
      EXPECT_FAIL(state_elem_a.Merge(state_elem_b), "state-element-merge-failure");
    }

    SECTION("Test identifying a PrivilegeLevelStateElement as a duplicate") {
      PrivilegeLevelStateElement duplicate_state_elem(EPrivilegeLevelType::H, 4);
      EXPECT(state_elem.IsDuplicate(duplicate_state_elem));
    }
  }
},

CASE("Test PcStateElement") {

  SETUP("Setup PcStateElement")  {
    PcStateElement state_elem(0xC3A5D885B2E8, 30);

    SECTION("Test cloning") {
      std::unique_ptr<PcStateElement> clone_state_elem(dynamic_cast<PcStateElement*>(state_elem.Clone()));
      std::vector<uint64> expected_values = {0xC3A5D885B2E8};
      EXPECT(clone_state_elem->GetValues() == expected_values);
      std::vector<uint64> expected_masks = {MAX_UINT64};
      EXPECT(clone_state_elem->GetMasks() == expected_masks);
    }

    SECTION("Test converting to a string") {
      EXPECT(state_elem.ToString() == "PC");
    }

    SECTION("Test getting a string describing the type") {
      EXPECT(state_elem.Type() == "PcStateElement");
    }

    SECTION("Test getting the name") {
      EXPECT(state_elem.GetName() == "PC");
    }

    SECTION("Test getting the StateElement type") {
      EXPECT(state_elem.GetStateElementType() == EStateElementType::PC);
    }

    SECTION("Test getting the values") {
      std::vector<uint64> expected_values = {0xC3A5D885B2E8};
      EXPECT(state_elem.GetValues() == expected_values);
    }

    SECTION("Test getting the masks") {
      std::vector<uint64> expected_masks = {MAX_UINT64};
      EXPECT(state_elem.GetMasks() == expected_masks);
    }

    SECTION("Test getting the priority") {
      EXPECT(state_elem.GetPriority() == 30u);
    }

    SECTION("Verify that PcStateElements can't be merged") {
      PcStateElement state_elem_a(0x902939FEC3CE, 33);
      PcStateElement state_elem_b(0x3EB90AA0D408, 12);
      EXPECT_NOT(state_elem_a.CanMerge(state_elem_b));
      EXPECT_NOT(state_elem_b.CanMerge(state_elem_a));
      EXPECT_FAIL(state_elem_a.Merge(state_elem_b), "state-element-merge-failure");
    }

    SECTION("Test identifying a PcStateElement as a duplicate") {
      PcStateElement duplicate_state_elem(0x5B9557B5A8EA, 8);
      EXPECT(state_elem.IsDuplicate(duplicate_state_elem));
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
