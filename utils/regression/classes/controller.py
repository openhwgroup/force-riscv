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
# Controller Classes

from common.path_utils import PathUtils
from common.sys_utils import SysUtils
from common.msg_utils import Msg
from common.errors import *
from common.kernel_objs import HiCriticalSection

from classes.control_item import ControlItem


# class forwards
class Controller(object):
    pass


class TaskController(Controller):
    pass


class FileController(Controller):
    pass


class Controller(object):
    def __init__(self, aAppsInfo):

        Msg.dbg("Controller::__init__()")
        self.ctrl_item = None
        self.on_fail_proc = None
        self.is_term_proc = None
        self.mAppsInfo = aAppsInfo
        self.crit_sec = HiCriticalSection()

    def load(self, arg_ctrl_item):
        Msg.dbg("Controller::load( ... )")
        self.ctrl_item = arg_ctrl_item

    def process(self):
        raise AbstractionError(
            "Abstract Method Error: Controller::process() not implemented in "
            "descendent class"
        )

    # initialize callbacks
    def set_on_fail_proc(self, arg_on_fail_proc):
        # for now there errors in controllers do not count as fails
        # however populating the callback should enable that capability
        self.on_fail_proc = None
        # self.on_fail_proc = arg_on_fail_proc

    def do_fail(self):
        # if a fail proc exists then it will be called in a protected mode
        with self.crit_sec:
            if self.on_fail_proc is not None:
                self.on_fail_proc(self)

    def set_is_term_proc(self, arg_is_term_proc):
        self.is_term_proc = arg_is_term_proc

    def is_terminated(self):
        # if a terminate proc exists then the return from that proc will be
        # returned otherwise it will be False and should not have any effect
        # and other methods of determining termination should be used
        with self.crit_sec:
            if self.is_term_proc is not None:
                return self.is_term_proc()
        return False
