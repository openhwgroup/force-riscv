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
#include "MemoryConstraintUpdate.h"

#include <memory>

#include "lest/lest.hpp"

#include "AddressReuseMode.h"
#include "Constraint.h"
#include "Log.h"
#include "MemoryConstraint.h"

using text = std::string;
using namespace Force;

const lest::test specification[] = {

CASE( "Test MarkUsedUpdate" ) {

  SETUP( "Setup MarkUsedUpdate" )  {
    MarkUsedUpdate mark_used_update(0xF9A0, 0xFC82);

    SECTION( "Test retrieving physical start address" ) {
      EXPECT(mark_used_update.GetPhysicalStartAddress() == 0xF9A0ull);
    }

    SECTION( "Test retrieving physical end address" ) {
      EXPECT(mark_used_update.GetPhysicalEndAddress() == 0xFC82ull);
    }

    SECTION( "Test updating virtual constraint" ) {
      std::unique_ptr<MemoryConstraint> virt_constr(new SingleThreadMemoryConstraint(0));
      virt_constr->Initialize(ConstraintSet(0x0, 0xFFFFFFFFFFFF));
      mark_used_update.UpdateVirtualConstraint(0x789A0, 0x78CA2, virt_constr.get());

      const ConstraintSet* usable = virt_constr->Usable();
      EXPECT(usable->ToSimpleString() == "0x0-0x7899f,0x78ca3-0xffffffffffff");
    }
  }
},

CASE( "Test MarkUsedForTypeUpdate" ) {

  SETUP( "Setup MarkUsedForTypeUpdate" )  {
    cuint32 thread_id = 0;
    MarkUsedForTypeUpdate mark_used_for_type_update(0x92A00, 0x9721C, EMemDataType::Data, EMemAccessType::Read, thread_id);
    AddressReuseMode addr_reuse_mode;
    addr_reuse_mode.EnableReuseType(EAddressReuseType::ReadAfterRead);
    addr_reuse_mode.EnableReuseType(EAddressReuseType::WriteAfterWrite);

    SECTION( "Test updating virtual constraint" ) {
      std::unique_ptr<MemoryConstraint> virt_constr(new SingleThreadMemoryConstraint(thread_id));
      virt_constr->Initialize(ConstraintSet(0x0, 0xFFFFFFFFFFFF));
      mark_used_for_type_update.UpdateVirtualConstraint(0x1572A00, 0x157721C, virt_constr.get());

      ConstraintSet read_constr_set(0x1572000, 0x1572FFF);
      virt_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Read, thread_id, addr_reuse_mode, &read_constr_set);
      EXPECT(read_constr_set.ToSimpleString() == "0x1572000-0x1572fff");

      ConstraintSet write_constr_set(0x1574C00, 0x1574E00);
      virt_constr->ApplyToConstraintSet(EMemDataType::Data, EMemAccessType::Write, thread_id, addr_reuse_mode, &write_constr_set);
      EXPECT(write_constr_set.IsEmpty());
    }
  }
},

CASE( "Test UnmarkUsedUpdate" ) {

  SETUP( "Setup UnmarkUsedUpdate" )  {
    UnmarkUsedUpdate unmark_used_update(0x9B2580, 0x9C3500);

    SECTION( "Test updating virtual constraint" ) {
      std::unique_ptr<MemoryConstraint> virt_constr(new SingleThreadMemoryConstraint(0));
      virt_constr->Initialize(ConstraintSet(0x0, 0xFFFFFFFFFFFF));
      virt_constr->MarkUsed(0x992D000, 0x993F000);
      unmark_used_update.UpdateVirtualConstraint(0x992C580, 0x993D500, virt_constr.get());

      const ConstraintSet* usable = virt_constr->Usable();
      EXPECT(usable->ToSimpleString() == "0x0-0x993d500,0x993f001-0xffffffffffff");
    }
  }
},

CASE( "Test SharedUpdate" ) {

  SETUP( "Setup SharedUpdate" )  {
    SharedUpdate shared_update(0x2468, 0x48C0);

    SECTION( "Test updating virtual constraint" ) {
      std::unique_ptr<MemoryConstraint> virt_constr(new SingleThreadMemoryConstraint(0));
      virt_constr->Initialize(ConstraintSet(0x0, 0xFFFFFFFFFFFF));
      shared_update.UpdateVirtualConstraint(0x6468, 0x88C0, virt_constr.get());

      const ConstraintSet* shared = virt_constr->Shared();
      EXPECT(shared->ToSimpleString() == "0x6468-0x88c0");
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
