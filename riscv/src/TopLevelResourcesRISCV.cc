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
#include <TopLevelResourcesRISCV.h>

#include <Config.h>
#include <Log.h>
#include <ObjectRegistry.h>
#include <Operand.h>

#include <ArchInfoRISCV.h>
#include <InstructionRISCV.h>
#include <RetOperandRISCV.h>
#include <OperandRISCV.h>
#include <PteAttributeRISCV.h>
#include <PyEnvironmentRISCV.h>

using namespace std;

namespace Force {

  /*!
    Top level resources should implement Initialize and Destroy interface methods.

    The method calling order (in terms of modules involved) in initialize_top_level_resources() and destroy_top_level_resources() should be in opsite order.

    The base level initialize_top_level_resources should be called first and the destroy_top_level_resources should be called last
  */

  void initialize_top_level_resources_RISCV(int argc, char* argv[])
  {
    vector<ArchInfo*> arch_info_objs;
    arch_info_objs.push_back(new ArchInfoRISCV("RiscV"));

    // The architecture-level Python modules must be initialized prior to calling the base
    // initialize_top_level_resources() function
    PyEnvironmentRISCV::initialize_python_modules();

    initialize_top_level_resources(argc, argv, arch_info_objs);

    // Register various RISCV objects.
    ObjectRegistry* obj_registry = ObjectRegistry::Instance();

    // Register Operand based objects.
    obj_registry->RegisterObject(new VsetvlVtypeImmediateOperand());
    obj_registry->RegisterObject(new VectorMaskOperand());
    obj_registry->RegisterObject(new BaseOffsetBranchOperand());
    obj_registry->RegisterObject(new RetOperand());
    obj_registry->RegisterObject(new ConditionalBranchOperandRISCV());
    obj_registry->RegisterObject(new CompressedConditionalBranchOperandRISCV());
    obj_registry->RegisterObject(new CompressedRegisterOperandRISCV());
    obj_registry->RegisterObject(new VsetvlAvlRegisterOperand());
    obj_registry->RegisterObject(new VsetvlVtypeRegisterOperand());
    obj_registry->RegisterObject(new VtypeLayoutOperand());
    obj_registry->RegisterObject(new WholeRegisterLayoutOperand());
    obj_registry->RegisterObject(new CustomLayoutOperand());
    obj_registry->RegisterObject(new VectorStridedLoadStoreOperandRISCV());
    obj_registry->RegisterObject(new VectorIndexedLoadStoreOperandRISCV());
    obj_registry->RegisterObject(new MultiVectorRegisterOperandRISCV());
    obj_registry->RegisterObject(new VectorIndexRegisterOperand());
    obj_registry->RegisterObject(new VectorDataRegisterOperand());
    obj_registry->RegisterObject(new VectorIndexedDataRegisterOperand());

    // Register Paging related objects.
    obj_registry->RegisterObject(new AddressPteAttributeRISCV());
    obj_registry->RegisterObject(new DAPteAttributeRISCV());
    obj_registry->RegisterObject(new GPteAttributeRISCV());
    obj_registry->RegisterObject(new UPteAttributeRISCV());
    obj_registry->RegisterObject(new XPteAttributeRISCV());
    obj_registry->RegisterObject(new WRPteAttributeRISCV());
    obj_registry->RegisterObject(new VPteAttributeRISCV());

    // Register Instruction related objects.
    obj_registry->RegisterObject(new RetInstruction());
    obj_registry->RegisterObject(new VectorLoadStoreInstruction());
    obj_registry->RegisterObject(new VectorAMOInstructionRISCV());
    obj_registry->RegisterObject(new VsetvlInstruction());
  }

  void destroy_top_level_resources_RISCV()
  {
    destroy_top_level_resources();
  }

}
