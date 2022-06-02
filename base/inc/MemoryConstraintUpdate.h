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
#ifndef Force_MemoryConstraintUpdate_H
#define Force_MemoryConstraintUpdate_H

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class MemoryConstraint;

  /*!
    \class MemoryConstraintUpdate
    \brief Class to help propagate physical memory constraint updates to virtual memory constraints.
  */
  class MemoryConstraintUpdate {
  public:
    MemoryConstraintUpdate(cuint64 paStart, cuint64 paEnd); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(MemoryConstraintUpdate);
    SUPERCLASS_DESTRUCTOR_DEFAULT(MemoryConstraintUpdate); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(MemoryConstraintUpdate);

    virtual void UpdateVirtualConstraint(cuint64 vaStart, cuint64 vaEnd, MemoryConstraint* memConstr) const = 0; //!< Propagate the physical memory constraint update using the specified translated addresses and virtual memory constraint.
    inline uint64 GetPhysicalStartAddress() const { return mPaStart; } //!< Return the starting physical address of update.
    inline uint64 GetPhysicalEndAddress() const { return mPaEnd; } //!< Return the ending physical address of update.
  private:
    cuint64 mPaStart; //!< Starting physical address of update
    cuint64 mPaEnd; //!< Ending physical address of update
  };

  /*!
    \class MarkUsedUpdate
    \brief Class to help propagate physical memory constraint updates to virtual memory constraints when marking addresses as used.
  */
  class MarkUsedUpdate : public MemoryConstraintUpdate {
  public:
    MarkUsedUpdate(cuint64 paStart, cuint64 paEnd); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(MarkUsedUpdate);
    SUBCLASS_DESTRUCTOR_DEFAULT(MarkUsedUpdate); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(MarkUsedUpdate);

    void UpdateVirtualConstraint(cuint64 vaStart, cuint64 vaEnd, MemoryConstraint* memConstr) const override; //!< Propagate the physical memory constraint update using the specified translated addresses and virtual memory constraint.
  };

  /*!
    \class MarkUsedForTypeUpdate
    \brief Class to help propagate physical memory constraint updates to virtual memory constraints when marking addresses as used for a specified access type.
  */
  class MarkUsedForTypeUpdate : public MemoryConstraintUpdate {
  public:
    MarkUsedForTypeUpdate(cuint64 paStart, cuint64 paEnd, const EMemDataType memDataType, const EMemAccessType memAccessType, cuint32 threadId); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(MarkUsedForTypeUpdate);
    SUBCLASS_DESTRUCTOR_DEFAULT(MarkUsedForTypeUpdate); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(MarkUsedForTypeUpdate);

    void UpdateVirtualConstraint(cuint64 vaStart, cuint64 vaEnd, MemoryConstraint* memConstr) const override; //!< Propagate the physical memory constraint update using the specified translated addresses and virtual memory constraint.
  private:
    const EMemDataType mMemDataType; //!< Memory data type of update
    const EMemAccessType mMemAccessType; //!< Memory access type of update
    cuint32 mThreadId; //!< Thread ID associated with the update
  };

  /*!
    \class UnmarkUsedUpdate
    \brief Class to help propagate physical memory constraint updates to virtual memory constraints when unmarking addresses as used.
  */
  class UnmarkUsedUpdate : public MemoryConstraintUpdate {
  public:
    UnmarkUsedUpdate(cuint64 paStart, cuint64 paEnd); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(UnmarkUsedUpdate);
    SUBCLASS_DESTRUCTOR_DEFAULT(UnmarkUsedUpdate); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(UnmarkUsedUpdate);

    void UpdateVirtualConstraint(cuint64 vaStart, cuint64 vaEnd, MemoryConstraint* memConstr) const override; //!< Propagate the physical memory constraint update using the specified translated addresses and virtual memory constraint.
  };

  /*!
    \class SharedUpdate
    \brief Class to help propagate physical memory constraint updates to virtual memory constraints when marking addresses as shared.
  */
  class SharedUpdate : public MemoryConstraintUpdate {
  public:
    SharedUpdate(cuint64 paStart, cuint64 paEnd); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(SharedUpdate);
    SUBCLASS_DESTRUCTOR_DEFAULT(SharedUpdate); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(SharedUpdate);

    void UpdateVirtualConstraint(cuint64 vaStart, cuint64 vaEnd, MemoryConstraint* memConstr) const override; //!< Propagate the physical memory constraint update using the specified translated addresses and virtual memory constraint.
  };

}

#endif  // Force_MemoryConstraintUpdate_H
