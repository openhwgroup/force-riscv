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
#include "RegisterInitPolicy.h"

#include "Generator.h"
#include "Log.h"
#include "PageTable.h"
#include "Register.h"
#include "UtilityFunctions.h"

using namespace std;

namespace Force {

  void InitPolicyTuple::Setup(const RegisterFile* pRegisterFile)
  {
    mpInitPolicy = pRegisterFile->GetInitPolicy(mPolicyName);
  }

  void RegisterInitPolicy::Setup(const Generator* pGen)
  {
    mpGenerator = pGen;
    mpRegisterFile = pGen->GetRegisterFile();
  }

  void RegisterInitPolicy::InitializeRegister(Register* pRegister) const
  {
    REPORT_UNIMPLEMENTED_METHOD;
  }

  void RegisterInitPolicy::InitializeRegisterField(RegisterField* pRegField, const ChoiceTree* pChoiceTree) const
  {
    REPORT_UNIMPLEMENTED_METHOD;
  }

  uint64 RegisterInitPolicy::RegisterReloadValue(Register* pRegister) const
  {
    REPORT_UNIMPLEMENTED_METHOD;
    return 0;
  }

  uint64 RegisterInitPolicy::RegisterFieldReloadValue(RegisterField* pRegField) const
  {
    REPORT_UNIMPLEMENTED_METHOD;
    return 0;
  }

  void RegisterInitPolicy::SetRegisterFieldInitialValue(RegisterField* pRegField, uint64 value) const
  {
    // Directly call RegisterField::InitializeField, since the RegisterInitPolicy is called from inside RegisterField::InitializeRandomly.
    pRegField->InitializeField(value);
  }

  void RegisterInitPolicy::SetRegisterInitialValue(Register* pRegister, uint64 value) const
  {
    pRegister->Initialize(value);
  }

}
