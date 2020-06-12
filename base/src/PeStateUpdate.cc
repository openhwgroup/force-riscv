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
#include <PeStateUpdate.h>
#include <Log.h>
#include <GenRequest.h>

#include <sstream>

using namespace std;

/*!
  \file PeStateUpdate.cc
  \brief Code for updaing PE state
*/

namespace Force {

  PeStateUpdate::PeStateUpdate() : mRequestList()
  {
  }

  PeStateUpdate::PeStateUpdate(const PeStateUpdate& rOther)
    : Object(rOther), mRequestList()
  {
  }

  PeStateUpdate::~PeStateUpdate()
  {
    // Note: mRequestList contains a list GenRequest* which should be cleared by GenAgent!
  }

  Object* PeStateUpdate::Clone() const
  {
    return new PeStateUpdate(*this);
  }

  const std::string PeStateUpdate::ToString() const
  {
    stringstream out_stream;
    for (auto item_ptr : mRequestList) {
      out_stream << item_ptr->ToString() << endl;
    }

    return out_stream.str();
  }

  void PeStateUpdate::AddRequest(GenRequest* request)
  {
    mRequestList.push_back(request);
  }

  void PeStateUpdate::UpdateState(const std::string& stateName, const std::string& actionName, uint64 value)
  {
    GenStateRequest* gen_req = GenStateRequest::GenStateRequestInstance(stateName);
    gen_req->SetAction(actionName);
    gen_req->SetPrimaryValue(value);
    AddRequest(gen_req);
  }

  void PeStateUpdate::UpdateRegisterField(const std::string& regName, const std::string& fieldName, uint64 value)
  {
    GenRequest* register_req = GenSequenceRequest::GenSequenceRequestInstance("UpdateRegisterField");
    register_req->AddDetail("RegName", regName);
    register_req->AddDetail("FieldName", fieldName);
    register_req->AddDetail("FieldValue", value);
    AddRequest(register_req);
  }

}
