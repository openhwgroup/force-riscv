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
#include "VmConstraint.h"

#include <sstream>

#include "Constraint.h"
#include "Log.h"

using namespace std;

/*!
  \file VmConstraint.cc
  \brief Code managing VmConstraint objects.
*/

namespace Force {

  string VmConstraint::ToString() const
  {
    stringstream out_str;

    out_str << "VmConstraint " << EVmConstraintType_to_string(Type()) << " requiring " << Requiring() << ": " << mpConstraint->ToSimpleString();

    return out_str.str();
  }

  void VmInConstraint::ApplyOn(ConstraintSet& rConstrSet) const
  {
    rConstrSet.ApplyLargeConstraintSet(*mpConstraint);
  }

  bool VmInConstraint::Allows(uint64 value) const
  {
    return mpConstraint->ContainsValue(value);
  }

  void VmNotInConstraint::ApplyOn(ConstraintSet& rConstrSet) const
  {
    rConstrSet.SubConstraintSet(*mpConstraint);
  }

  bool VmNotInConstraint::Allows(uint64 value) const
  {
    return not mpConstraint->ContainsValue(value);
  }

}
