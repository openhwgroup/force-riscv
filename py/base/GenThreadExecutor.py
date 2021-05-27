#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0
#  (the "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
# OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import abc
import threading
import traceback

import Log
from ThreadDispatcher import (
    ThreadDispatcher,
    SingleThreadDispatcher,
    MultiThreadDispatcher,
)


#  @package GenThreadExecutor
#
#  Mainly two versions of generator threads, one for single-thread test
#  generation, one for multi-thread test generation.

#  Abstract base class for GenThreadExecutor
#
class GenThreadExecutor(abc.ABC):

    # Common base __init__ method.
    def __init__(self):
        self.mSingleThreadDispatcher = SingleThreadDispatcher()

    def getDispatcher(self):
        return self.mSingleThreadDispatcher

    # Notify the exeuctor of the main generator thread ID for the before and
    # after main test
    # processing.
    def setMainThreadId(self, aThreadId):
        self.mSingleThreadDispatcher.addThreadId(aThreadId)

    # Interface method to execute generator threads.
    #
    @abc.abstractmethod
    def executeGenThreads(self, aGenThreads):
        raise NotImplementedError

    # Static method used by base classes to execute a generator thread.
    #
    @staticmethod
    def executeGenThread(aGenThread):
        try:
            aGenThread.setup()
            aGenThread.generate()
            aGenThread.cleanUp()
        except Exception as exc:
            err_str = traceback.format_exception_only(type(exc), exc)[-1]
            err_str += "".join(traceback.format_tb(exc.__traceback__))
            Log.fail(err_str)


#  Generator executor class for single-thread test generation.
#
class SingleGenThreadExecutor(GenThreadExecutor):
    def __init__(self):
        super().__init__()

    # Execute single generator thread
    #
    def executeGenThreads(self, aGenThreads):
        # To avoid unnecessarily impairing performance for the case where
        # there is only one GenThread, execute it on
        # the main thread
        if len(aGenThreads) != 1:
            raise AssertionError(
                "SingleGenThreadExecutor was used to execute more than one GenThread!"
            )

        return GenThreadExecutor.executeGenThread(aGenThreads[0])


#  Generator executor class for multi-thread test generation.
#
class MultiGenThreadExecutor(GenThreadExecutor):

    # Setup multi-thread dispatcher.
    #
    def __init__(self):
        super().__init__()
        self.mMultiThreadDispatcher = MultiThreadDispatcher()
        self.mGenThreads = None

    # Execute the generator threads in pseudo random concurrent fashion.
    #
    def executeGenThreads(self, aGenThreads):
        ex_threads = []
        self.mGenThreads = aGenThreads
        start_barrier = threading.Barrier(len(aGenThreads))
        for gen_thread in aGenThreads:
            ex_thread = threading.Thread(
                target=MultiGenThreadExecutor.executeGenThreadControlledStart,
                args=[gen_thread, self.mMultiThreadDispatcher, start_barrier],
            )
            ex_threads.append(ex_thread)

        with ThreadingEnableContextManager(self):
            for ex_thread in ex_threads:
                ex_thread.start()

            for ex_thread in ex_threads:
                ex_thread.join()

    @staticmethod
    def executeGenThreadControlledStart(aGenThread, aDispatcher, aStartBarrier):
        with ExecutionContextManager(aGenThread.genThreadID, aDispatcher, aStartBarrier):
            GenThreadExecutor.executeGenThread(aGenThread)


#  Use this context manager to control swapping in and out the multi-threading
#  API call dispatcher.
#
class ThreadingEnableContextManager:
    def __init__(self, executor):
        self.mExecutor = executor

    # Iterate through all GenThread objects to set their dispatcher to the
    # multi-thread one.
    def __enter__(self):
        ThreadDispatcher.setCurrentDispatcher(self.mExecutor.mMultiThreadDispatcher)
        for gen_thread in self.mExecutor.mGenThreads:
            self.mExecutor.mMultiThreadDispatcher.addThreadId(gen_thread.genThreadID)

        self.mExecutor.mMultiThreadDispatcher.start()
        Log.noticeNoBlock("Multi-threading phase entered.")
        return self

    # Iterate through all GenThread objects to set their dispatcher back to
    # the single-thread one.
    def __exit__(self, *unused):
        self.mExecutor.mMultiThreadDispatcher.stop()
        Log.noticeNoBlock("Multi-threading phase exited.")
        ThreadDispatcher.setCurrentDispatcher(self.mExecutor.mSingleThreadDispatcher)
        return False


#  Put in barrier at the thread starting phase so all threads will start
#  orderly. Remove the thread ID from dispatcher so that all threads will
#  finish orderly.
class ExecutionContextManager:
    def __init__(self, aThreadId, aDispatcher, aStartBarrier):
        self.mThreadId = aThreadId
        self.mDispatcher = aDispatcher
        self.mStartBarrier = aStartBarrier

    # Add thread ID to dispatcher.
    #
    def __enter__(self):
        self.mDispatcher.registerExecutionThread(self.mThreadId)
        self.mStartBarrier.wait()
        return self

    # Remove thread ID from dispatcher
    #
    def __exit__(self, *aUnused):
        self.mDispatcher.reportThreadDone()
        return False


#  Factory class to return the correct GenThreadExecutor object.
#
class GenThreadExecutorFactory:
    @staticmethod
    def createGenThreadExecutor(genThreadCount):
        if genThreadCount == 1:
            executor = SingleGenThreadExecutor()
        else:
            executor = MultiGenThreadExecutor()

        ThreadDispatcher.setCurrentDispatcher(executor.mSingleThreadDispatcher)
        return executor
