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
#include "UopInterface.h"

#include "Log.h"

using namespace Force;

/*!
  \file UopInterface.cc
  \brief Placeholder for execution library.
*/

bool execute_uop(uint32_t cpuid, EUop uop, UopParameter* inputParams, uint8_t inputParamCount, UopParameter* outputParams, uint8_t* outputParamCount)
{
  LOG(fail) << "{execute_uop} is not implemented." << std::endl;
  FAIL("function-not-implemented");

  return false;
}
