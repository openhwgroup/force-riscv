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


class HiThread(threading.Thread):
    def __init__(self, arg_create_active=False):
        super().__init__(name="HiThread-01")
        # Ensure that all threads are killed when master thread is exited for
        # any reason by marking it as a daemon
        self.daemon = True
        if arg_create_active:
            self.start()
        # pass

    def run(self):
        pass

    def HeartBeat(self):
        # Enable this if it's a good idea to have a periodic printing heartbeat
        # Msg.dbg("[Thread %s]: Heartbeat" % (self.threadName))
        pass


class HiEvent(threading.Event):
    def __init__(self, arg_options):
        super().__init__()
        # default to return immediately
        self.timeout = arg_options.get("timeout", None)
        self.pre_signal = arg_options.get("pre-sig", None)
        self.post_signal = arg_options.get("post-sig", None)
        self.pre_unsignal = arg_options.get("pre-unsig", None)
        self.post_unsignal = arg_options.get("post-unsig", None)

    def Signal(self, arg_stat=None):

        # perform any activities prior to notification, this could include
        # finishing some work that could make the system unstable. This is a
        # callback that is part of the dictionary used to initialize
        if self.pre_signal is not None:
            self.pre_signal(arg_stat)

        # signal the event
        self.set()

        # perform any activities once notification has been dispatched, this
        # can be used to notify the caller the even has been signaled
        # This is a callback that is part of the dictionary used to initialize
        if self.post_signal is not None:
            self.post_signal(arg_stat)

    def Unsignal(self, arg_stat=None):

        # perform any activities prior to notification the event has been
        # cleared and will block, this could include initializing to prevent
        # system instability. This is a callback that is part of the dictionary
        # used to initialize the Event
        if self.pre_unsignal is not None:
            self.pre_unsignal(arg_stat)

        self.clear()

        if self.post_unsignal is not None:
            self.post_unsignal(arg_stat)


# This event is signalled when all the worker threads are completed.
workers_done_event = HiEvent({})

# Summary thread signals master thread that it's done
summary_done_event = HiEvent({})


class HiSemaphore(threading.Semaphore):
    def test(self):
        my_lock = threading.Lock()
