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
from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.errors import *

from executors.app_executor import *
from classes.control_item import ControlItem, CtrlItmKeys


class FrunToCtrlExecutor(AppExecutor):
    def __init__(self):
        super().__init__()
        self.mFrunToCtrlCmd = None
        self.log = None
        self.elog = None

    def load(self, aCtrlItem):
        super().load(aCtrlItem)
        self.mFrunToCtrlCmd = self.ctrl_item.fruntoctrl.get("path", None)

    def skip(self):
        if not self.ctrl_item.fruntoctrl.get("run", False):
            Msg.user(
                "[FrunToCtrlExecutor::skip] skipping - run is not True..."
            )
            return True

        Msg.user("[FrunToCtrlExecutor::skip] not skipping")
        return False

    def execute(self):

        if not PathUtils.check_file("./_def_frun.py"):
            Msg.user(
                "[FrunToCtrlExecutor::skip] skipping - no _def_frun.py found"
            )
            return True

        my_cmd = self.mFrunToCtrlCmd
        if my_cmd is None:
            Msg.user("[FrunToCtrlExecutor::skip] skipping - no path was given")
            return True

        Msg.user("FrunToCtrlCommand = " + str({"frun-to-ctrl-cmd": my_cmd}))
        Msg.flush()

        self.log = "frun_to_ctrl_result.log"
        self.elog = "frun_to_ctrl_result.err"

        my_result = SysUtils.exec_process(
            my_cmd, self.log, self.elog, self.ctrl_item.timeout, True
        )
        Msg.user("FrunToCtrlResult = " + str(my_result))
        Msg.flush()

        return SysUtils.success(int(my_result[0]))
