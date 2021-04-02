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
# file: iss_executor.py
# comment: implements IssExecutor which serves as an Abstract Class
#          for executing simulation applications in client processing apps

from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.errors import *

from executors.app_executor import *
from classes.control_item import ControlItem


class IssResult(ProcessResult):
    iss_instr_count = 0
    iss_message = 1
    iss_log = 2


class IssKeys(object):
    iss_retcode = "retcode"
    iss_stdout = "stdout"
    iss_stderr = "stderr"
    iss_start = "start"
    iss_end = "end"
    iss_count = "count"
    iss_message = "message"
    iss_log = "log"


class IssExecutor(AppExecutor):
    def __init__(self):
        super().__init__()
        self.task_name = None

    def extract_results(self, arg_result, arg_log, arg_elog):

        # extract information from the generate log
        my_result, my_error = self.query_logs(arg_log, arg_elog)
        Msg.user("Process: %s" % (str(arg_result)), "SIMULATE")
        Msg.user("Log[%s]: []" % (str(arg_log)), "SIMULATE")

        my_process_data = {
            IssKeys.iss_retcode: int(arg_result[IssResult.process_retcode]),
            IssKeys.iss_stdout: str(arg_result[IssResult.process_stdout]),
            IssKeys.iss_stderr: str(arg_result[IssResult.process_stderr]),
            IssKeys.iss_start: str(arg_result[IssResult.process_start]),
            IssKeys.iss_end: str(arg_result[IssResult.process_end]),
            IssKeys.iss_count: int(my_result[IssResult.iss_instr_count]),
            IssKeys.iss_message: str(my_result[IssResult.iss_message]),
            IssKeys.iss_log: arg_log,
        }

        return my_process_data

    def locate_test_case(self, arg_extension, arg_test_name):

        Msg.user(
            "IssExecutor::locate_test_case( Extension: %s, Test Name: %s )"
            % (str(arg_extension), str(arg_test_name))
        )
        my_match_files = PathUtils.list_files(arg_extension)
        Msg.lout(my_match_files, "user", "Simulate File List")
        if arg_test_name.endswith("_force"):
            arg_test_name = arg_test_name.replace("_force", "")

        for my_file in my_match_files:
            Msg.user(
                "Match File: %s, Test Name: %s"
                % (str(my_file), str(arg_test_name))
            )
            if my_file.find(arg_test_name) != -1:
                return my_file

        raise Exception(
            "Type %s test case for base test: %s not found."
            % (arg_extension, arg_test_name)
        )

    def query_errors(self, arg_hfile, arg_results=None):
        return None
