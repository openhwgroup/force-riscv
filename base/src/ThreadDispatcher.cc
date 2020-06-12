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
#include <ThreadDispatcher.h>

#include <Log.h>
#include <Scheduler.h>

/*!
  \file ThreadDispatcher.cc
  \brief Code supporting coercing threads into executing in a prescribed, repeatable order.
*/

using namespace std;

namespace Force {

  CountingMutex::CountingMutex()
    : mMutex(), mLockCount(0)
  {
  }

  void CountingMutex::lock()
  {
    mMutex.lock();
    mLockCount++;
  }

  bool CountingMutex::try_lock()
  {
    bool success = mMutex.try_lock();
    if (success) {
      mLockCount++;
    }

    return success;
  }

  void CountingMutex::unlock()
  {
    mLockCount--;
    mMutex.unlock();
  }

  uint32 CountingMutex::get_lock_count()
  {
    return mLockCount;
  }

  ThreadDispatcher* ThreadDispatcher::mspDispatcher = nullptr;

  MultiThreadDispatcher::MultiThreadDispatcher(Scheduler* pScheduler)
    : ThreadDispatcher(), mpScheduler(pScheduler), mDispatchMutex(), mCondVar(), mThreadIds()
  {
  }

  void MultiThreadDispatcher::Start()
  {
    unique_lock<CountingMutex> lock(mDispatchMutex);

    mpScheduler->RefreshSchedule();
  }

  void MultiThreadDispatcher::Stop()
  {
    unique_lock<CountingMutex> lock(mDispatchMutex);

    uint32 active_thread_count = mpScheduler->ActiveThreadCount();
    if (active_thread_count != 0) {
      LOG(fail) << "{MultiThreadDispatcher::Stop} multi-threading phase is terminating prematurely with active thread count: " << dec << active_thread_count << endl;
      FAIL("thread-dispatch-failure");
    }
  }

  void MultiThreadDispatcher::Request()
  {
    unique_lock<CountingMutex> lock(mDispatchMutex);

    uint32 thread_id = GetThreadId();

    mCondVar.wait(lock,
      [thread_id, this]() { return (thread_id == this->mpScheduler->CurrentThreadId()); });

    // Hold the mutex lock until Finish() is called
    mDispatchMutex.lock();

    LOG(debug) << "{MultiThreadDispatcher::Request} thread 0x" << hex << thread_id << ", locking level=" << dec << mDispatchMutex.get_lock_count() << endl;
  }

  void MultiThreadDispatcher::Finish()
  {
    uint32 thread_id = GetThreadId();

    LOG(debug) << "{MultiThreadDispatcher::Finish} thread 0x" << hex << thread_id << ", starting locking level=" << dec << mDispatchMutex.get_lock_count() << endl;

    UnlockMutex();

    LOG(debug) << "{MultiThreadDispatcher::Finish} thread 0x" << hex << thread_id << ", ending locking level=" << dec << mDispatchMutex.get_lock_count() << endl;
  }

  void MultiThreadDispatcher::FinishNoAdvance()
  {
    uint32 thread_id = GetThreadId();

    LOG(debug) << "{MultiThreadDispatcher::FinishNoAdvance} thread 0x" << hex << thread_id << ", starting locking level=" << dec << mDispatchMutex.get_lock_count() << endl;

    UnlockMutexNoAdvance();

    LOG(debug) << "{MultiThreadDispatcher::FinishNoAdvance} thread 0x" << hex << thread_id << ", ending locking level=" << dec << mDispatchMutex.get_lock_count() << endl;
  }

  void MultiThreadDispatcher::AddThreadId(cuint32 threadId)
  {
    unique_lock<CountingMutex> lock(mDispatchMutex);

    mpScheduler->AddThreadId(threadId);
  }

  uint32 MultiThreadDispatcher::GetThreadId()
  {
    unique_lock<CountingMutex> lock(mDispatchMutex);

    uint32 threadId = MAX_UINT32;
    auto itr = mThreadIds.find(this_thread::get_id());
    if (itr != mThreadIds.end()) {
      threadId = itr->second;
    }
    else {
      LOG(fail) << "{MultiThreadDispatcher::GetThreadId} the calling execution thread could not be identified. It has either not been registered with a call to RegisterExecutionThread() or has terminated with a call to ReportThreadDone()." << endl;
      FAIL("thread-dispatch-failure");
    }

    return threadId;
  }

  void MultiThreadDispatcher::RegisterExecutionThread(cuint32 threadId)
  {
    unique_lock<CountingMutex> lock(mDispatchMutex);

    mThreadIds[this_thread::get_id()] = threadId;
  }

  void MultiThreadDispatcher::ReportThreadDone()
  {
    Request();

    uint32 thread_id = GetThreadId();

    if (mDispatchMutex.get_lock_count() != 1) {
      LOG(fail) << "{MultiThreadDispatcher::NotifyThreadStop} thread 0x" << hex << thread_id << " is terminating prematurely" << endl;
      FAIL("thread-dispatch-failure");
    }

    mpScheduler->RemoveThreadId(thread_id);
    mThreadIds.erase(this_thread::get_id());

    // We avoid calling Finish() directly here because Finish() attempts to get the thread ID for
    // debug logging purposes. This thread's thread ID has just been removed from the map, so
    // attempting to retrieve it will fail. UnlockMutex() does all of the work of Finish() without
    // the logging.
    UnlockMutex();
  }

  void MultiThreadDispatcher::UnlockMutex()
  {
    if (mDispatchMutex.get_lock_count() == 1) {
      mpScheduler->NextThread();

      mDispatchMutex.unlock();
      mCondVar.notify_all();
    } else {
      // This will decrement the lock count, but not release the lock, as it is a nested call
      mDispatchMutex.unlock();
    }
  }

  void MultiThreadDispatcher::UnlockMutexNoAdvance()
  {
    mDispatchMutex.unlock();
  }

  SingleThreadDispatcher::SingleThreadDispatcher()
    : mThreadId(MAX_UINT32)
  {
  }

  void SingleThreadDispatcher::AddThreadId(cuint32 threadId)
  {
    if (mThreadId == MAX_UINT32) {
      mThreadId = threadId;
    }
    else {
      LOG(fail) << "{SingleThreadDispatcher::AddThreadId} unable to add thread 0x" << hex << threadId << "; thread 0x" << mThreadId << " has already been added. SingleThreadDispatcher only supports one thread." << endl;
      FAIL("thread-dispatch-failure");
    }
  }

}
