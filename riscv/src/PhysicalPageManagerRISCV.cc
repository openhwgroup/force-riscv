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
#include "PhysicalPageManagerRISCV.h"

using namespace std;

namespace Force {

  PhysicalPageManagerRISCV::PhysicalPageManagerRISCV(EMemBankType bankType, MemoryTraitsManager* pMemTraitsManager)
    : PhysicalPageManager(bankType, pMemTraitsManager), mPteTypes()
  {
    mPteTypes.push_back(EPteType::P4K);
    mPteTypes.push_back(EPteType::P2M);
    mPteTypes.push_back(EPteType::P4M);
    mPteTypes.push_back(EPteType::P1G);
    mPteTypes.push_back(EPteType::P512G);
  }

}
