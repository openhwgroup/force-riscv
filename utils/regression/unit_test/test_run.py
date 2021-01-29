#!/usr/bin/env python3
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

#
#  class_test.py
#  summary - implements a series of tests to examine if various classes
#            function as expected.
#
#

print(
    "\n=======================\n\tInitializing ....\n=======================\n"
)

import getopt
import sys
import traceback

from force_init import the_force_root
from shared.module_run import ModuleRun
from shared.msg_utils import Msg
from shared.path_utils import PathUtils
from test_core import UsageStr, CmdLine, Defaults
from test_event import UnitTest_HiEvent
from test_mutex import UnitTest_HiMutex
from test_thread import UnitTest_HiThread


# from test_semaphore import UnitTest_HiSemaphore


class UnitTestRun(ModuleRun):
    def __init__(self, arg_force_dir):
        super().__init__(
            arg_force_dir,
            CmdLine.Switches,
            CmdLine.ShortOpts,
            CmdLine.Switches[CmdLine.msg_lev],
            Defaults.msg_level,
            CmdLine.MsgLevel,
        )

    def load(self):
        Msg.dbg("ForrestRun::load()")
        self.check_usage()
        self.process_cmdline()

    def process_cmdline(self):
        self.process_max = int(
            self.option_def(
                CmdLine.Switches[CmdLine.process_max],
                Defaults.process_max,
                CmdLine.ProcessMax,
            )
        )

    def run(self):
        Msg.dbg("ThreadTestRun::run()")
        with UnitTest_HiThread():
            pass
        with UnitTest_HiEvent():
            pass
        with UnitTest_HiMutex():
            pass

    # checks for usage requests and performs the initial validation of the
    # command line
    def check_usage(self):
        # validate arguments
        if self.option_def(
            CmdLine.Switches[CmdLine.help], False, CmdLine.Help
        ):
            from force_init import force_usage

            force_usage(UsageStr)


if __name__ == "__main__":

    # save current working directory
    my_pwd = PathUtils.current_dir()

    try:
        my_module = UnitTestRun(the_force_root)

        Msg.info("\nForce Path: %s" % (str(the_force_root)))
        Msg.info("Original Directory: " + my_pwd)

        Msg.dbg("Processing Command Line and Loading  Control File")
        my_module.load()

        Msg.dbg("Directory set to %s" % (PathUtils.current_dir()))
        if not PathUtils.chdir(the_force_root, False):
            Msg.dbg(
                "Directory Unchanged, using the current directory for output"
            )

        my_module.run()
        Msg.dbg("Test Completed ....\n")
        Msg.blank()
        sys.exit(40)

    except getopt.GetoptError as arg_ex:

        from force_init import force_usage

        Msg.error_trace("[ERROR] - " + str(arg_ex))
        force_usage(UsageStr)
        sys.exit(41)

    except Exception as arg_ex:

        from force_init import force_usage

        Msg.err(
            "[ERROR] - An Unhandled Error has Occurred during run of "
            + str(sys.argv[0])
        )
        traceback.print_exc(file=sys.stdout)
        Msg.error_trace(str(arg_ex))
        force_usage(UsageStr)
        sys.exit(42)

    finally:

        if my_pwd is not None:
            PathUtils.chdir(my_pwd)
            Msg.dbg("Returned To: %s" % (PathUtils.current_dir()))

        Msg.info("Done with Unit Test of Regression")
