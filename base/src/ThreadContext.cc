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
#include <ThreadContext.h>

#include <ThreadDispatcher.h>

#include <pybind11/pybind11.h>

/*!
  \file ThreadContext.cc
  \brief Code supporting providing a scoped call guard for multithreaded access.
*/

namespace py = pybind11;

namespace Force {

  ThreadContext::ThreadContext()
  {
    // Must release the GIL prior to calling a C++ method that may block such as the call to
    // Request() below
    py::gil_scoped_release release;

    ThreadDispatcher* dispatcher = ThreadDispatcher::GetCurrentDispatcher();
    dispatcher->Request();
  }

  ThreadContext::~ThreadContext()
  {
    ThreadDispatcher* dispatcher = ThreadDispatcher::GetCurrentDispatcher();
    dispatcher->Finish();
  }

  uint32 ThreadContext::GetThreadId() const
  {
    ThreadDispatcher* dispatcher = ThreadDispatcher::GetCurrentDispatcher();
    return dispatcher->GetThreadId();
  }

  ThreadContextNoAdvance::ThreadContextNoAdvance()
  {
    // Must release the GIL prior to calling a C++ method that may block such as the call to
    // Request() below
    py::gil_scoped_release release;

    ThreadDispatcher* dispatcher = ThreadDispatcher::GetCurrentDispatcher();
    dispatcher->Request();
  }

  // The call to FinishNoAdvance() will maintain the current thread, avoiding a forced context
  // switch. This should be far more performant than the Finish() call in ThreadContext, but isn't
  // appropriate for cases where we want to advance to the next thread.
  ThreadContextNoAdvance::~ThreadContextNoAdvance()
  {
    ThreadDispatcher* dispatcher = ThreadDispatcher::GetCurrentDispatcher();
    dispatcher->FinishNoAdvance();
  }

  uint32 ThreadContextNoAdvance::GetThreadId() const
  {
    ThreadDispatcher* dispatcher = ThreadDispatcher::GetCurrentDispatcher();
    return dispatcher->GetThreadId();
  }

}
