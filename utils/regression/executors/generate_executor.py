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
# file: generate_executor.py
# comment: implements GenerateExecutor which serves as an Abstract Class
#          for executing generation applications in client processing apps

from executors.app_executor import *


# tuple index for information extracted from the generation log file
class GenerateResult(ProcessResult):
    gen_seed = 0
    gen_total = 1
    gen_secondary = 2
    gen_default = 3


# dictionary keys for the generate result
class GenerateKeys(object):
    gen_retcode = "retcode"
    gen_stdout = "stdout"
    gen_stderr = "stderr"
    gen_start = "start"
    gen_end = "end"
    gen_seed = "seed"
    gen_total = "total"
    gen_secondary = "secondary"
    gen_default = "default"
    gen_cmd = "command"
    gen_log = "log"
    gen_elog = "elog"
    gen_app = "app"
    gen_message = "message"


# Generate Executor abstract class
class GenerateExecutor(AppExecutor):
    def __init__(self):
        super().__init__()

    def extract_results(self, arg_result, arg_log, arg_elog):

        Msg.user("Suffix: %s" % (str(self.ctrl_item.suffix)), "GENERATE")

        if self.ctrl_item.suffix is not None:
            self.rename_elfs(self.ctrl_item.suffix)

        my_ret_code = int(arg_result[GenerateResult.process_retcode])
        if SysUtils.success(my_ret_code):
            # extract information from the generate log
            arg_elog = None

        my_result, my_error = self.query_logs(arg_log, arg_elog)

        Msg.user("Process: %s" % (str(arg_result)), "GENERATE")
        Msg.user("Log[%s]: %s" % (str(arg_log), str(my_result)), "GENERATE")

        self.ctrl_item.seed = my_result[GenerateResult.gen_seed]

        my_process_data = {
            GenerateKeys.gen_retcode: int(arg_result[GenerateResult.process_retcode]),
            GenerateKeys.gen_stdout: str(arg_result[GenerateResult.process_stdout]),
            GenerateKeys.gen_stderr: str(arg_result[GenerateResult.process_stderr]),
            GenerateKeys.gen_start: str(arg_result[GenerateResult.process_start]),
            GenerateKeys.gen_end: str(arg_result[GenerateResult.process_end]),
            GenerateKeys.gen_seed: self.ctrl_item.seed,
            GenerateKeys.gen_total: my_result[GenerateResult.gen_total],
            GenerateKeys.gen_secondary: my_result[GenerateResult.gen_secondary],
            GenerateKeys.gen_default: my_result[GenerateResult.gen_default],
        }

        if my_error is not None:
            my_process_data[GenerateKeys.gen_message] = my_error

        return my_process_data

    def rename_elfs(self, arg_suffix):

        # before proceeding it is necessary to remove any existing renamed
        # files to eliminate the possibility of causing a chain and messing up
        # the results
        for my_mask in ["*.ELF", "*.S", "*.img"]:
            my_match_files = PathUtils.list_files(my_mask)
            Msg.lout(my_match_files, "user", "Simulate File List")
            for my_src_file in my_match_files:
                Msg.user(
                    "Match File: %s, Suffix: %s" % (str(my_src_file), str(arg_suffix)),
                    "MATCH",
                )
                if SysUtils.found(my_src_file.find("_%s_force" % (str(arg_suffix)))):
                    PathUtils.remove(my_src_file)
                    continue

        # Now rename all the files that are found
        for my_mask in ["*.ELF", "*.S", "*.img"]:
            my_match_files = PathUtils.list_files(my_mask)
            for my_src_file in my_match_files:
                my_tgt_file = my_src_file.replace("_force", "_%s_force" % (str(arg_suffix)))
                PathUtils.rename(my_src_file, my_tgt_file)

        return True
