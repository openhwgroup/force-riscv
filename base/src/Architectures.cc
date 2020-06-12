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
#include <Architectures.h>
#include <Log.h>

using namespace std;

/*!
  \file Architectures.cc
  \brief Code for managing multiple ArchInfo objects.
*/

namespace Force {

  ArchInfo::ArchInfo(const std::string& name)
    : mName(name), mpGeneratorTemplate(nullptr), mpInstructionSet(nullptr), mpPagingInfo(nullptr), mpSimAPI(nullptr), mpSimApiModule(nullptr), mChoicesSets(), mVariableSets(), mMemoryBanks(), mInstructionFiles(), mRegisterFiles(), mChoicesFiles(), mPagingFiles(), mVariableFiles(),
      mDefaultIClass(), mDefaultPteClass(), mDefaultPteAttributeClass(), mDefaultOperandClasses(), mRegisterClasses(), mSimulatorApiModule(), mSimulatorDLL(), mSimulatorStandalone()
  {

  }

  /*!
    The object pointed to by mpGeneratorTemplate is owned by Scheduler, merely set the pointer to nullptr.
    The object pointed to by mpInstructionSet and mpPagingInfo are managed by derived class.
   */
  ArchInfo::~ArchInfo()
  {
    mpGeneratorTemplate = nullptr;
    mpInstructionSet = nullptr;
    mpPagingInfo = nullptr;
    mpSimAPI = nullptr;
    mpSimApiModule = nullptr;
  }

  void ArchInfo::AddInstructionFile(const string& iFile)
  {
    mInstructionFiles.push_back(iFile);
  }

  void ArchInfo::AddRegisterFile(const string& iFile)
  {
    mRegisterFiles.push_back(iFile);
  }

  void ArchInfo::AddChoicesFile(const std::string& iFile)
  {
    mChoicesFiles.push_back(iFile);
  }

  void ArchInfo::AddPagingFile(const std::string& iFile)
  {
    mPagingFiles.push_back(iFile);
  }

  void ArchInfo::AddVariableFile(const std::string& iFile)
  {
    mVariableFiles.push_back(iFile);
  }

  const string ArchInfo::DefaultOperandClass(const string& operandType) const
  {
    const auto opr_finder = mDefaultOperandClasses.find(operandType);
    if (opr_finder != mDefaultOperandClasses.end()) {
      return opr_finder->second;
    } else {
      LOG(fail) << "Unsupported operand type \'" << operandType << "\'." << endl;
      FAIL("unsupported-operand-type");
    }
    return "";
  }


  Architectures * Architectures::mspArchitectures = nullptr;

  void Architectures::Initialize()
  {
    if (nullptr == mspArchitectures) {
      mspArchitectures = new Architectures();
    }
  }

  void Architectures::Destroy()
  {
    delete mspArchitectures;
    mspArchitectures = nullptr;
  }

  Architectures::Architectures()
    : mArchInfoObjects(), mpDefaultArchInfo(nullptr)
  {

  }

  Architectures::~Architectures()
  {
    for (auto & map_item : mArchInfoObjects) {
      delete map_item.second;
    }
  }

  void Architectures::AssignArchInfoObjects(vector<ArchInfo*>& archInfoObjs)
  {
    if (mArchInfoObjects.size()) {
      LOG(fail) << "{Architectures::AssignArchInfoObjects} Not expecting to be called multiple times." << endl;
      FAIL("Re-assign-arch-info-objects");
    }

    for (auto item_ptr : archInfoObjs) {
      item_ptr->Setup();
      const string& arch_name = item_ptr->Name();
      if (mArchInfoObjects.find(arch_name) != mArchInfoObjects.end()) {
        LOG(fail) << "Adding ArchInfo object with duplicated name \"" << arch_name << "\"." << endl;
        FAIL("duplicated-arch-info-name");
      }
      mArchInfoObjects[arch_name] = item_ptr;
    }

    if (archInfoObjs.size() == 1) {
      mpDefaultArchInfo = archInfoObjs.front();
    }
  }

  void Architectures::SetupSimAPIs()
  {
    for (auto & map_item : mArchInfoObjects) {
      map_item.second->SetupSimAPIs();
    }
  }

}
