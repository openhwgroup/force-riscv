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
# kernel_objs.py

# implementes several kernel objects
# HiSemaphore - derived from BoundedSemaphore
# HiMutex     - wraps a Lock object and impementes bool lock, void unlock
#               and bool locked
# HiCriticalSection - mimics the Windows CriticalSection by providing a
#                     wrapper for the Lock
# HiEvent - implements a wrapper

import threading


class HiCriticalSection(object):
    def __init__(self):
        self.lock = threading.Lock()

    def enter(self):
        self.lock.acquire()

    def leave(self):
        self.lock.release()

    def __enter__(self):
        self.enter()

    def __exit__(self, type, value, traceback):
        self.leave()


class HiMutex(object):
    def __init__(self, aName=None):
        self.name = aName
        self.lock = threading.Lock()

    def acquire(self):
        self.lock.acquire()

    def release(self):
        self.lock.release()

    def __enter__(self):
        self.acquire()

    def __exit__(self, type, value, traceback):
        self.release()
