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
# thread_test.py

from unit_test import UnitTest
from test_classes import *

from common.path_utils import PathUtils
from common.msg_utils import Msg

# =============================================================================
# == unit test classes ========================================================
# =============================================================================


class UnitTest_NoLoopThread(UnitTest):
    def run_test(self):
        Msg.info("HiThread(NoLoop): Start Unit Test ... ")

        myThreadProcs = {
            # start thread sequence (outside thread space)
            "on-start": self.thread_start,
            # thread termination handler (inside thread space)
            "on-execute": self.thread_execute,
            # thread before finished handler (inside thread space)
            "on-finished": self.thread_finished,
            # thread terminated handler (outside thread space)
            "on-done": self.thread_done,
        }

        Msg.info("UnitTest_NoLoopThread >> Creating NoLoop Thread ... ")
        myThread = thread_factory("NoLoopThread", True, myThreadProcs)

        Msg.info("UnitTest_NoLoopThread >> Initializing Thread ... ")
        myThread.start_thread()

        # wait for thread to terminate
        myThread.wait_for()
        Msg.info("UnitTest_NoLoopThread >> Thread Completed ... ")

    def process_result(self):
        Msg.info("HiThread(NoLoop): Process Test Results... ")

    def thread_start(self):
        Msg.info("UnitTest_NoLoopThread >> Started .... ")

    def thread_execute(self):
        Msg.info("UnitTest_NoLoopThread << Executing .... ")

    def thread_done(self):
        Msg.info("UnitTest_NoLoopThread << Execute Done .... ")

    def thread_finished(self):
        Msg.info("UnitTest_NoLoopThread >> Exiting .... ")


class UnitTest_LoopThread(UnitTest):
    def run_test(self):
        Msg.info("HiThread(Loop): Start Unit Test ... ")
        myThreadProcs = {
            # start thread sequence (outside thread space)
            "on-start": self.thread_start,
            # thread termination handler (inside thread space)
            "on-execute": self.thread_execute,
            # thread before finished handler (inside thread space)
            "on-finished": self.thread_finished,
            # thread terminated handler (outside thread space)
            "on-done": self.thread_done,
        }

        myThreadOpts = {
            "heartbeat-rate": 2,
            "sleep-period": 100,
            "daemon": False,  # run once threads should not be daemons
            "active": False,  # do not start the thread until ready
        }
        Msg.info("UnitTest_LoopThread << Creating Thread With Loop... ")
        myThread = thread_factory(
            "LoopThread", False, myThreadProcs, myThreadOpts
        )

        Msg.info("UnitTest_LoopThread << Initializing Thread ... ")
        myThread.start_thread()

        # wait for thread to terminate
        myThread.wait_for()
        Msg.info("UnitTest_LoopThread >> Thread Completed ... ")

    def process_result(self):
        Msg.info("HiThread(Loop): Process Test Results... ")

    def thread_start(self):
        Msg.info("UnitTest_LoopThread >> Started .... ")

    def thread_execute(self):
        Msg.info("UnitTest_LoopThread << Executing .... ")

    def thread_done(self):
        Msg.info("UnitTest_LoopThread << Execute Done .... ")
        Msg.blank()

    def thread_finished(self):
        Msg.info("UnitTest_LoopThread >> Exiting .... ")


class UnitTest_HiThread(UnitTest):
    def run_test(self):
        Msg.info("HiThread: Start all Thread Unit Tests ... ")
        with UnitTest_NoLoopThread():
            pass
        with UnitTest_LoopThread():
            pass

    def process_result(self):
        Msg.info("HiThread: Process All Thread Test Results ... ")
