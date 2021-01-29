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

# import sys, traceback

# since there will be a significant amount of directory manipulation import the path utils
from shared.path_utils import PathUtils
from shared.sys_utils import SysUtils
from shared.msg_utils import Msg
from shared.control_item import ControlItem

# class forwards
class Controller(object): pass
class TaskController(Controller): pass
class FileController(Controller): pass

class Controller(object):

    def __init__( self):

        Msg.dbg( "Controller::__init__()" )
        self.ctrl_item = None


    def load( self, arg_ctrl_item ):
        self.ctrl_item = arg_ctrl_item

    def process(self):
        pass


