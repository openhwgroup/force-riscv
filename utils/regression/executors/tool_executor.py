#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# file: tool_executor.py
# comment: implements IssExecutor which serves as an Abstract Class
#          for executing tool utility applications in client processing apps

from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.errors import *

from classes.executor import *
from executors.app_executor import *
from classes.control_item import ControlItem


class ToolResult( ProcessResult ):
    tool_msg       = 0


class ToolKeys( object ):
    tool_retcode   = "retcode"
    tool_stdout    = "stdout"
    tool_stderr    = "stderr"
    tool_start     = "start"
    tool_end       = "end"
    tool_msg       = "msg"
    tool_log       = "log"


class ToolExecutor( AppExecutor ):

    # def __init__( self ):
    #     super().__init__()
    #
    # def load( self, arg_ctrl_item ):
    #     super().load(arg_ctrl_item )
    #
    # def execute( self ):
    #     super().execute()
    #
    def extract_results( self, arg_result, arg_log, arg_elog ):

        # extract information from the result log
        my_result = self.query_logs( arg_log, arg_elog  )
        my_res_dict = { "trace-cmp-retcode": int(arg_result[ToolResult.process_retcode ])
                      , "trace-cmp-stdout" : str(arg_result[ToolResult.process_stdout  ])
                      , "trace-cmp-stderr" : str(arg_result[ToolResult.process_stderr  ])
                      , "trace-cmp-start"  : str(arg_result[ToolResult.process_start   ])
                      , "trace-cmp-end"    : str(arg_result[ToolResult.process_end     ])
                      , "trace-cmp-log"    : arg_log
                      , "trace-cmp-msg"    : my_result
                      }
        return my_res_dict


    def query_errors( self, arg_hfile ):
        return None

