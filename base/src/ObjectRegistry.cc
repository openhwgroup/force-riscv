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
#include <ObjectRegistry.h>
#include <Log.h>

using namespace std;

/*!
  \file ObjectRegistry.cc
  \brief code for the ObjectRegistry class.
*/

namespace Force {

  ObjectRegistry* ObjectRegistry::mspObjectRegistry = nullptr;

  void ObjectRegistry::Initialize()
  {
    if (nullptr == mspObjectRegistry) {
      mspObjectRegistry = new ObjectRegistry();
#ifndef UNIT_TEST
      mspObjectRegistry->Setup();
#endif
    }
  }

  void ObjectRegistry::Destroy()
  {
    delete mspObjectRegistry;
    mspObjectRegistry = nullptr;
  }

  ObjectRegistry::~ObjectRegistry()
  {
    for (auto & map_item : mObjectRegistry) {
      delete map_item.second;
    }
  }

  Object* ObjectRegistry::ObjectInstance(const std::string& objType) const
  {
    const auto map_finder = mObjectRegistry.find(objType);
    if (map_finder == mObjectRegistry.end()) {
      ObjectNotFound(objType);
    }

    return map_finder->second->Clone();
  }

  Object* ObjectRegistry::ObjectInstanceTry(const std::string& objType) const
  {
    const auto map_finder = mObjectRegistry.find(objType);
    if (map_finder == mObjectRegistry.end()) {
      return nullptr;
    }

    return map_finder->second->Clone();
  }


  void ObjectRegistry::ObjectNotFound(const std::string& objType) const
  {
    LOG(fail) << "Object of type \"" << objType << "\" not found." << endl;
    FAIL("object-not-found");
  }

  void ObjectRegistry::RegisterObject(const Object* objPtr)
  {
    const char * obj_type = objPtr->Type();
    const auto map_finder = mObjectRegistry.find(obj_type);
    if (map_finder != mObjectRegistry.end()) {
      LOG(fail) << "Registering duplicated Object type \"" << obj_type << "\"." << endl;
      FAIL("register-duplicated-object");
    }
    mObjectRegistry[obj_type] = objPtr;
  }

}

#ifndef UNIT_TEST

#include <Register.h>
#include <Instruction.h>
#include <Operand.h>
#include <AsmText.h>
#include <Page.h>
#include <PteAttribute.h>

namespace Force {

  void ObjectRegistry::Setup()
  {
    RegisterObject(new PhysicalRegister());
    RegisterObject(new PhysicalRegisterRazwi());
    RegisterObject(new LinkedPhysicalRegister());
    RegisterObject(new ConfigureRegister());
    RegisterObject(new ReadOnlyRegister());
    RegisterObject(new ReadOnlyZeroRegister());
    RegisterObject(new LargeRegister());
    RegisterObject(new BitField());
    RegisterObject(new Register());
    RegisterObject(new RegisterNoAlias());
    RegisterObject(new RegisterField());
    RegisterObject(new RegisterFile());
    RegisterObject(new RegisterFieldRes0());
    RegisterObject(new RegisterFieldRes1());
    RegisterObject(new RegisterFieldRazwi());
    RegisterObject(new RegisterFieldRaowi());
    RegisterObject(new ReadOnlyRegisterField());

    // Instruction based objects
    RegisterObject(new Instruction());
    RegisterObject(new PartialWriteInstruction());
    RegisterObject(new BranchInstruction());
    RegisterObject(new LoadStoreInstruction());
    RegisterObject(new SystemCallInstruction());
    RegisterObject(new UnpredictStoreInstruction());
    RegisterObject(new VectorInstruction());

    // Operand based objects
    RegisterObject(new ImmediateOperand());
    RegisterObject(new ChoicesOperand());
    RegisterObject(new RegisterOperand());
    RegisterObject(new FpRegisterOperand());
    RegisterObject(new VectorRegisterOperand());
    RegisterObject(new ImmediateGe1Operand());
    RegisterObject(new SameValueOperand());
    RegisterObject(new Minus1ValueOperand());
    RegisterObject(new ImmediateExcludeOperand());
    RegisterObject(new RegisterBranchOperand());
    RegisterObject(new BaseOffsetLoadStoreOperand());
    RegisterObject(new BaseIndexLoadStoreOperand());
    RegisterObject(new PcOffsetLoadStoreOperand());
    RegisterObject(new SignedImmediateOperand());
    RegisterObject(new PcRelativeBranchOperand());
    RegisterObject(new ImpliedRegisterOperand());
    RegisterObject(new AluImmediateOperand());
    RegisterObject(new DataProcessingOperand());

    // OperandTextObject based objects
    RegisterObject(new AddressingOperandText());

    // Page table entry related classes
    RegisterObject(new Page());
    RegisterObject(new TablePte());
    RegisterObject(new PteAttribute());
    RegisterObject(new RandomPteAttribute());
    RegisterObject(new ValuePteAttribute());
    RegisterObject(new AddressPteAttribute());
  }
}

#endif
