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
#include "GenAgent.h"

#include "Log.h"

using namespace std;

/*!
  \file GenAgent.cc
  \brief Code for GenAgent base class, provide default implementation for necessary methods.
*/

namespace Force {

  void GenAgent::UnimplementedMethod(const std::string& methodName) const
  {
    LOG(fail) << "{GenAgent::UnimplementedMethod} method: " << methodName << " not implemented." << endl;
    FAIL("GenAgent-method-not-implemented");
  }

  void GenAgent::SetGenRequest(GenRequest* genRequest)
  {
    UnimplementedMethod("SetGenRequest");
  }

  void GenAgent::SetGenQuery(const GenQuery* genQuery) const
  {
    UnimplementedMethod("SetGenQuery");
  }

}
