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
import threading
from common.kernel_objs import HiCriticalSection, HiMutex


class HiCollection(list):
    pass


class HiCollectionItem(object):
    def __init__(self, arg_owner):
        self.owner = arg_owner


class HiCollection(list):
    def empty(self):
        return self == []

    def size(self):
        if self.empty():
            return 0
        return len(self)


# LIFO Collection
class HiStack(HiCollection):

    # pushes an item on the stack
    def push(self, item):
        self.append(item)

    # pops and returns the top item of the stack
    def pop(self):
        if self.empty():
            return None
        return super().pop()

    # returns the top item of the stack
    def peek(self):
        if self.empty():
            return None
        return self[-1]


class HiQueue(HiCollection):
    def __init__(self):
        super().__init__()


class HiThreadList(HiCollection):
    def __init__(self):
        super().__init__()
        self.mutex = HiMutex()

    def append(self, arg_thread):
        with self.mutex:
            self.append(arg_thread)

    def remove(self, arg_thread):
        with self.mutex:
            self.remove(arg_thread)


class HiAtomicInteger(HiCollection):
    def __init__(self, initial_value):
        super().__init__()
        self.val = initial_value
        self.mutex = threading.Lock()

    def add(self, amount):
        try:
            self.mutex.acquire()
            self.val += amount
        finally:
            self.mutex.release()

    def value(self):
        internal = None
        try:
            self.mutex.acquire()
            internal = self.val
        finally:
            self.mutex.release()
        return internal


class HiThreadedQueue(HiQueue):
    def __init__(self):
        super().__init__()
        self.listLock = threading.Lock()

    def enqueue(self, item):
        success = False
        try:
            self.listLock.acquire()
            self.append(item)
            success = True
        finally:
            self.listLock.release()

        return success

    def dequeue(self):
        try:
            self.listLock.acquire()
            item = self.pop(0)
        finally:
            self.listLock.release()

        return item


class HiThreadedProducerConsumerQueue(HiThreadedQueue):
    def __init__(self, blocking=False):
        self.produceAval = threading.Semaphore(0)
        super().__init__()

    def blockUntilAvailable(self, timeout):
        success = self.produceAval.acquire(blocking=True, timeout=timeout)
        return success

    # Returns True if the item was able to be successfully enqueued.
    # Returns False otherwise.
    def enqueue(self, item):
        success = super().enqueue(item)

        self.produceAval.release()

        return success

    # Returns True if the queue is not empty and the item was successfully
    # dequeued. Returns False otherwise. Raises a timeout error if the queue
    # isn't given an element within the timeout specified; default is 1 second.
    def dequeue(self, timeout=1):
        success = self.blockUntilAvailable(timeout)

        if not success:
            raise TimeoutError

        result = super().dequeue()

        return result
