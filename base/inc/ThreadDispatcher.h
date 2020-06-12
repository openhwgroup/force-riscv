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
#ifndef Force_ThreadDispatcher_H
#define Force_ThreadDispatcher_H

#include <Defines.h>

#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>

namespace Force {

  /*!
    \class CountingMutex
    \brief Reentrant mutex with an interface to retreive the lock count.
  */
  class CountingMutex {
  public:
    CountingMutex(); //!< Default constructor
    COPY_CONSTRUCTOR_ABSENT(CountingMutex);
    DESTRUCTOR_DEFAULT(CountingMutex); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(CountingMutex);

    void lock(); //!< Lock the mutex and increment the lock count. Block if the mutex is not available
    bool try_lock(); //!< Try to lock the mutex. Increment the lock count and return true if the mutex is available; return false otherwise.
    void unlock(); //!< Unlock the mutex and decrement the lock count.
    uint32 get_lock_count(); //!< Return the lock count.
  private:
    std::recursive_mutex mMutex; //!< Underlying mutex object
    uint32 mLockCount; //!< Lock count
  };

  /*!
    \class ThreadDispatcher
    \brief Class to coerce threads into executing in a prescribed, repeatable order.
  */
  class ThreadDispatcher {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(ThreadDispatcher); //!< Default constructor
    COPY_CONSTRUCTOR_ABSENT(ThreadDispatcher);
    SUPERCLASS_DESTRUCTOR_DEFAULT(ThreadDispatcher); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(ThreadDispatcher);

    static ThreadDispatcher* GetCurrentDispatcher() { return mspDispatcher; } //!< Return current dispatcher instance.
    static void SetCurrentDispatcher(ThreadDispatcher* pDispatcher) { mspDispatcher = pDispatcher; } //!< Set current dispatcher instance.

    virtual void Start() = 0; //!< Start scheduling and dispatching threads.
    virtual void Stop() = 0; //!< Ensure all threads have been unscheduled.
    virtual void Request() = 0; //!< Request to progress. If the calling thread is not the currently scheduled thread, it will block until it is the currently scheduled thread.
    virtual void Finish() = 0; //!< Finish current thread progress; notify other threads to progress.
    virtual void FinishNoAdvance() = 0; //!< Finish current thread request; do not notify other threads to progress.
    virtual void AddThreadId(cuint32 threadId) = 0; //!< Add thread ID to the scheduler.
    virtual uint32 GetThreadId() = 0; //!< Return the thread ID for the calling execution thread.
  private:
    static ThreadDispatcher* mspDispatcher; //!< Current dispatcher instance
  };

  class Scheduler;

  /*!
    \class MultiThreadDispatcher
    \brief Class to coerce threads into executing in a prescribed, repeatable order when multiple threads are executing.
  */
  class MultiThreadDispatcher : public ThreadDispatcher {
  public:
    explicit MultiThreadDispatcher(Scheduler* pScheduler); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(MultiThreadDispatcher);
    SUBCLASS_DESTRUCTOR_DEFAULT(MultiThreadDispatcher); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(MultiThreadDispatcher);

    void Start() override; //!< Start scheduling and dispatching threads.
    void Stop() override; //!< Ensure all threads have been unscheduled.
    void Request() override; //!< Request to progress. If the calling thread is not the currently scheduled thread, it will block until it is the currently scheduled thread.
    void Finish() override; //!< Finish current thread progress; notify other threads to progress.
    void FinishNoAdvance() override; //!< Finish current thread request; do not notify other threads to progress.
    void AddThreadId(cuint32 threadId) override; //!< Add thread ID to the scheduler.
    uint32 GetThreadId() override; //!< Return the thread ID for the calling execution thread.
    void RegisterExecutionThread(cuint32 threadId); //!< Establish link between the execution thread and its thread ID; this method must be called on the execution thread for the specified thread ID.
    void ReportThreadDone(); //!< Notify dispatcher that the execution thread is about to terminate.
  private:
    void UnlockMutex(); //!< Unlock the mutex and notify other threads to progress.
    void UnlockMutexNoAdvance(); //!< Unlock the mutex; do not notify other threads to progress.
  private:
    Scheduler* mpScheduler; //!< Scheduler
    CountingMutex mDispatchMutex; //!< Reentrant mutex with lock count
    std::condition_variable_any mCondVar; //!< Condition variable to ensure thread is current thread
    std::map<std::thread::id, uint32> mThreadIds; //!< Mapping between internal execution thread ID and application thread ID
  };

  /*!
    \class SingleThreadDispatcher
    \brief Class to allow fast execution when only a single thread is executing.
  */
  class SingleThreadDispatcher : public ThreadDispatcher {
  public:
    SingleThreadDispatcher(); //!< Default constructor
    COPY_CONSTRUCTOR_ABSENT(SingleThreadDispatcher);
    SUBCLASS_DESTRUCTOR_DEFAULT(SingleThreadDispatcher); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(SingleThreadDispatcher);

    void Start() override { } //!< Start scheduling and dispatching threads.
    void Stop() override { } //!< Ensure all threads have been unscheduled.
    void Request() override { } //!< Request to progress. If the calling thread is not the currently scheduled thread, it will block until it is the currently scheduled thread.
    void Finish() override { } //!< Finish current thread progress; notify other threads to progress.
    void FinishNoAdvance() override { } //!< Finish current thread request; do not notify other threads to progress.
    void AddThreadId(cuint32 threadId) override; //!< Add thread ID to the scheduler.
    uint32 GetThreadId() override { return mThreadId; } //!< Return the thread ID for the calling execution thread.
  private:
    uint32 mThreadId;
  };

}

#endif  // Force_ThreadDispatcher_H
