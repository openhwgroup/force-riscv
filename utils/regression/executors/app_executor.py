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
# file: app_executor.py
# comment: implements AppExecutor Abstract Class for executing processes in client processing apps

from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.errors import *

from classes.executor import *
from classes.control_item import ControlItem, CtrlItmKeys

class AppExecutor( Executor ):

    def __init__( self ):
        super().__init__()

    def pre(self):
        pass

    def post(self):
        pass

    # def load( self, arg_ctrl_item ):
    #     return super().load(arg_ctrl_item )
    #
    # def execute( self ):
    #     return super().execute()
    #
    # def extract_results( self, arg_result, arg_log ):
    #     return super().extract_results( arg_result, arg_log )
    #
    # def query_log( self, arg_log ):
    #     return super().query_log( self, arg_log )
    #
    # def query_result_log\all *( self, arg_hlog ):
    #     return super().query_result_log( arg_hlog )

