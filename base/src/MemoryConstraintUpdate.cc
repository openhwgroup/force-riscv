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
#include <MemoryConstraintUpdate.h>
#include <MemoryConstraint.h>

using namespace std;

/*!
  \file MemoryConstraintUpdate.cc
  \brief Code supporting the propagation of physical memory constraint updates to virtual memory constraints.
*/

namespace Force {

  MemoryConstraintUpdate::MemoryConstraintUpdate(cuint64 paStart, cuint64 paEnd)
    : mPaStart(paStart), mPaEnd(paEnd)
  {
  }

  MarkUsedUpdate::MarkUsedUpdate(cuint64 paStart, cuint64 paEnd)
    : MemoryConstraintUpdate(paStart, paEnd)
  {
  }

  void MarkUsedUpdate::UpdateVirtualConstraint(cuint64 vaStart, cuint64 vaEnd, MemoryConstraint* memConstr) const
  {
    memConstr->MarkUsed(vaStart, vaEnd);
  }

  MarkUsedForTypeUpdate::MarkUsedForTypeUpdate(cuint64 paStart, cuint64 paEnd, const EMemDataType memDataType, const EMemAccessType memAccessType, cuint32 threadId)
    : MemoryConstraintUpdate(paStart, paEnd), mMemDataType(memDataType), mMemAccessType(memAccessType), mThreadId(threadId)
  {
  }

  void MarkUsedForTypeUpdate::UpdateVirtualConstraint(cuint64 vaStart, cuint64 vaEnd, MemoryConstraint* memConstr) const
  {
    memConstr->MarkUsedForType(vaStart, vaEnd, mMemDataType, mMemAccessType, mThreadId);
  }

  UnmarkUsedUpdate::UnmarkUsedUpdate(cuint64 paStart, cuint64 paEnd)
    : MemoryConstraintUpdate(paStart, paEnd)
  {
  }

  void UnmarkUsedUpdate::UpdateVirtualConstraint(cuint64 vaStart, cuint64 vaEnd, MemoryConstraint* memConstr) const
  {
    memConstr->UnmarkUsed(vaStart, vaEnd);
  }

  SharedUpdate::SharedUpdate(cuint64 paStart, cuint64 paEnd)
    : MemoryConstraintUpdate(paStart, paEnd)
  {
  }

  void SharedUpdate::UpdateVirtualConstraint(cuint64 vaStart, cuint64 vaEnd, MemoryConstraint* memConstr) const
  {
    memConstr->MarkShared(vaStart, vaEnd);
  }

}
