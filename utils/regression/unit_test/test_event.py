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
# test_event.py

from common.threads import HiEvent
from test_classes import *
from unit_test import UnitTest


class EventWrapper(object):
    def __init__(self, arg_name):
        self.name = arg_name
        self.timeout = 1000  # milliseconds
        self.event = HiEvent(
            {
                "name": arg_name,
                "timeout": self.timeout,
                "pre-sig": self.on_before_signal,
                "post-sig": self.on_after_signal,
                "pre-unsig": self.on_before_unsignal,
                "post-unsig": self.on_after_unsignal,
            }
        )
        self.event.Unsignal()  # confirm that event is in an unsignaled stat

    def on_before_signal(self, arg_sender):

        if not isinstance(arg_sender, HiEvent):
            raise EIncompatableTypeError(
                "Event Notification Contained Incompatable Type"
            )
        Msg.info("Event[%s] is about to be signaled ... " % (arg_sender.name))

    def on_after_signal(self, arg_sender):
        if not isinstance(arg_sender, HiEvent):
            raise EIncompatableTypeError(
                "Event Notification Contained Incompatable Type"
            )
        Msg.info("Event[%s] has been signaled ... " % (arg_sender.name))

    def on_before_unsignal(self, arg_sender):
        if not isinstance(arg_sender, HiEvent):
            raise EIncompatableTypeError(
                "Event Notification Contained Incompatable Type"
            )
        Msg.info(
            "Event[%s] is about to be unsignaled ... " % (arg_sender.name)
        )

    def on_after_unsignal(self, arg_sender):
        if not isinstance(arg_sender, HiEvent):
            raise EIncompatableTypeError(
                "Event Notification Contained Incompatable Type"
            )
        Msg.info("Event[%s] has been unsignaled ... " % (arg_sender.name))


# HiEvent methods
#   Signal()   - set the state of the event to signaled
#   Unsignal() - set the state of the event to unsignaled
#   Signaled() - check to see if Event is signaled
#   WaitFor()  - Pause thread until this returns. Returns True if Signaled,
#                returns False is a timeout was specified and the time
#                specified elapsed
#
# HiEvent event callback procss
#   "pre-sig"    - callback triggered before state is changed to signaled
#   "post-sig"   - callback triggered after state is changed to signaled
#   "pre-unsig"  - callback triggered before state is changed to unsignaled
#   "post-unsig" - callback triggered after state is changed to unsignaled
#
# HiEvent default option
#    self.timeout = arg_options.get( "timeout", None )


class ThreadWrapper(object):
    def __init__(self):
        self.start_event = EventWrapper("start-event").event
        self.execute_event = EventWrapper("execute-event").event
        self.done_event = EventWrapper("done-event").event
        self.finished_event = EventWrapper("finished-event").event
        self.thread = None
        self.init_thread()

    def init_thread(self):
        Msg.info("HiEvent: Creating Test Thread ...")
        myThreadProcs = {
            # start thread sequence (outside thread space)
            "on-start": self.thread_start,
            # thread termination handler (inside thread space)
            "on-execute": self.thread_execute,
            # thread terminated handler (outside thread space)
            "on-done": self.thread_done,
            # thread before finished handler (inside thread space)
            "on-finished": self.thread_finished,
        }
        self.thread = thread_factory("EventTestThread", True, myThreadProcs)

    def run_thread(self):
        Msg.info("UnitTest_HiEvent >> Initializing Thread ... ")
        self.thread.start_thread()
        # wait for thread to terminate
        self.thread.wait_for()
        Msg.info("UnitTest_HiEvent >> Thread Completed ... ")

    def thread_start(self):
        Msg.info("UnitTest_NoLoopThread >> Started .... ")
        if self.start_event is not None:
            self.start_event.Signaled()
            self.start_event.Signal()
            self.start_event.Signaled(True)

    def thread_execute(self):
        Msg.info("UnitTest_NoLoopThread << Executing .... ")
        if self.execute_event is not None:
            self.execute_event.Signaled()
            self.execute_event.Signal()
            self.execute_event.Signaled(True)

    def thread_done(self):
        Msg.info("UnitTest_NoLoopThread << Execute Done .... ")
        if self.done_event is not None:
            self.done_event.Signaled()
            self.done_event.Signal()
            self.done_event.Signaled(True)

    def thread_finished(self):
        Msg.info("UnitTest_NoLoopThread >> Exiting .... ")
        if self.finished_event is not None:
            self.finished_event.Signaled()
            self.finished_event.Signal()
            self.finished_event.Signaled(True)


class UnitTest_HiEvent(UnitTest):

    # need a seperate
    def run_test(self):
        Msg.info("HiEvent: Start Unit Test ...")
        my_wrapper = ThreadWrapper()
        my_wrapper.run_thread()

    def process_result(self):
        Msg.info("HiThread(NoLoop): Process Test Results... ")
