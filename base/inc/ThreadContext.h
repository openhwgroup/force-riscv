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
#ifndef Force_ThreadContext_H
#define Force_ThreadContext_H

#include "Defines.h"

namespace Force {

  /*!
    \class ThreadContext
    \brief Scoped call guard for multithreaded access.
  */
  class ThreadContext {
  public:
    ThreadContext(); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(ThreadContext);
    ~ThreadContext(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(ThreadContext);

    uint32 GetThreadId() const; //!< Return the thread ID for the calling execution thread.
  };

  /*!
    \class ThreadContextNoAdvance
    \brief Scoped call guard for multithreaded access that doesn't advance the scheduled thread.
  */
  class ThreadContextNoAdvance {
  public:
    ThreadContextNoAdvance(); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(ThreadContextNoAdvance);
    ~ThreadContextNoAdvance(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(ThreadContextNoAdvance);

    uint32 GetThreadId() const; //!< Return the thread ID for the calling execution thread.
  };

}

#endif  // Force_ThreadContext_H
