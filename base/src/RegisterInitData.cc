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
#include <Generator.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <Data.h>
#include <Generator.h>
#include <Register.h>
#include <OperandDataRequest.h>
#include <RegisterInitData.h>
#include <Log.h>

using namespace std;
namespace Force {
  void RegisterInitData::Setup(const Generator& gen, const Register *pRegister , const OperandDataRequest *pDataRequest)
  {
    mpDataRequest = pDataRequest;
    if (mpDataRequest != nullptr) {
      auto valueStr = mpDataRequest->GetDataString();
      mpData = DataFactory::Instance()->BuildData(valueStr, gen);
    }
    else {
      mpData = DataFactory::Instance()->BuildDataPattern(pRegister, gen);
    }
  }

  void RegisterInitData::Commit(const Generator& gen, Register* pRegister)
  {
    auto reg_file = gen.GetRegisterFile();
    if (nullptr == mpDataRequest) {
      if (pRegister->RegisterType() == ERegisterType::SysReg) {
        gen.RandomInitializeRegister(pRegister);
        return;
      }
    }

    ChoicesModerator* choices_mod = nullptr;
    if (pRegister->RegisterType() == ERegisterType::SysReg) {
      choices_mod = gen.GetChoicesModerator(EChoicesType::RegisterFieldValueChoices);
    }

    auto reg_name = pRegister->Name();
    if (pRegister->IsLargeRegister()) {
      vector<uint64> datas;
      mpData->ChooseLargeData(datas);
      reg_file->InitializeRegister(reg_name, datas, choices_mod);
    }
    else {
      auto data = mpData->ChooseData();
      reg_file->InitializeRegister(reg_name, data, choices_mod);
      // << "{RegisterInitData::Commit} Register "<< reg_name << " is initialized to 0x" << std::hex << data << endl;
    }

    if (mpDataRequest != nullptr)
      mpDataRequest->SetApplied();

  }

  void RegisterInitData::Cleanup()
  {
    delete mpData;
  }

}
