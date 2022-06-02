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
#include "OperandRequest.h"

#include <sstream>

#include "Constraint.h"
#include "Log.h"

using namespace std;

namespace Force {

  OperandRequest::OperandRequest(const string& name, uint64 value)
    : Object(), mName(name), mpValueConstraint(nullptr), mApplied(false), mIgnored(false)
  {
    SetValueRequest(value);
  }

  OperandRequest::OperandRequest(const string& name, const string& valueStr)
    : Object(), mName(name), mpValueConstraint(nullptr), mApplied(false), mIgnored(false)
  {
    SetValueRequest(valueStr);
  }

  OperandRequest::OperandRequest(const OperandRequest& rOther)
    : Object(rOther), mName(rOther.mName), mpValueConstraint(nullptr), mApplied(false), mIgnored(false)
  {
    if (nullptr != rOther.mpValueConstraint) {
      mpValueConstraint = rOther.mpValueConstraint->Clone();
    }
  }

  OperandRequest::~OperandRequest()
  {
    delete mpValueConstraint;
  }

  Object* OperandRequest::Clone() const
  {
    return new OperandRequest(*this);
  }

  const std::string OperandRequest::ToString() const
  {
    stringstream out_stream;

    out_stream << Type() << ": " << Name();
    if (nullptr != mpValueConstraint) {
      out_stream << "<=" << mpValueConstraint->ToSimpleString() << endl;
    }

    return out_stream.str();
  }

  void OperandRequest::SetValueRequest(uint64 value)
  {
    if (nullptr != mpValueConstraint) {
      delete mpValueConstraint;
    }

    mpValueConstraint = new ConstraintSet(value);
  }

  void OperandRequest::SetValueRequest(const std::string& valueStr)
  {
    if (nullptr != mpValueConstraint) {
      delete mpValueConstraint;
    }

    mpValueConstraint = new ConstraintSet(valueStr);
  }

}
