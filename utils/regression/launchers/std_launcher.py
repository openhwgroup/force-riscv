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
# std_launcher.py
# implements the standard process launcher

from classes.launcher import Launcher
from common.msg_utils import Msg
from common.sys_utils import SysUtils


class StdLauncher(Launcher):
    def __init__(self, aSequenceRunner):
        super().__init__(aSequenceRunner)

        self.process = None

    def launch(self):
        Msg.user("Timeout: %s" % (str(self.timeout)), "STD-LAUNCHER")
        self.process_cmd, self.process_log = self.build()
        Msg.user(
            "Process Cmd: %s, Process Log: %s" % (str(self.process_cmd), str(self.process_log)),
            "STD-LAUNCHER",
        )
        Msg.user("Launcher Id 1: %s" % (str(id(self))), "STD-LAUNCHER")

        # enable timeout but only trigger shutdown of spawned process allow
        # that process to kill the child processes.
        self.process_result = SysUtils.exec_process(
            self.process_cmd,
            self.process_log,
            self.process_log,
            self.timeout,
            False,
            self.set_process,
        )
        Msg.user("Launcher Id 2: %s" % (str(id(self))), "STD-LAUNCHER")
        Msg.user("Process Results: %s" % (str(self.process_result)), "STD-LAUNCHER")

    def validate(self):
        pass

    def set_process(self, arg_process):
        Msg.user("Setting Process: %s" % (str(arg_process)), "STD-LAUNCHER")
        Msg.user("Launcher Id 3: %s" % (str(id(self))), "STD-LAUNCHER")
        self.process = arg_process
        Msg.user("Launcher Id 4: %s" % (str(id(self))), "STD-LAUNCHER")
        Msg.user("Saved Process: %s" % (str(self.process)), "STD-LAUNCHER")

    def terminate(self):
        Msg.user("Terminating Process: %s" % (str(self.process)), "STD-LAUNCHER")
        if self.process is not None:
            self.process.kill()

    def status(self):
        pass
