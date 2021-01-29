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
# file: force_executor.py
# comment: implements IssExecutor which serves as a Class Wrapper for
#          for executing force in client processing apps

from executors.generate_executor import *


class ForceExecutor(GenerateExecutor):
    def __init__(self):
        super().__init__()

    def load(self, arg_ctrl_item):
        super().load(arg_ctrl_item)

        # Msg.dbg("ForceProcessor::load()")
        my_cmd = self.ctrl_item.generator["path"]

        my_cmd += " -t %s" + " --max-instr %d" % self.ctrl_item.max_instr
        my_cmd += " --num-chips %d --num-cores %d --num-threads %d" % (
            self.ctrl_item.num_chips,
            self.ctrl_item.num_cores,
            self.ctrl_item.num_threads,
        )
        my_cmd += SysUtils.ifthen(
            self.ctrl_item.seed is None, "", (" -s %s" % (self.ctrl_item.seed))
        )
        # Msg.dbg("my_cmd: %s" %  (str(my_cmd)))

        if isinstance(self.ctrl_item.generator, dict):
            for my_key in self.ctrl_item.generator.keys():
                if my_key not in ["path"]:
                    my_cmd += " %s %s " % (
                        str(my_key),
                        SysUtils.ifthen(
                            self.ctrl_item.generator[my_key] is None,
                            "",
                            str(self.ctrl_item.generator[my_key]),
                        ),
                    )
        self.force_cmd = my_cmd.strip()

    def execute(self):

        # super().execute()
        # Msg.dbg("ExecuteController::exec_gen(%s)" % (arg_task_file))

        # NOTE: Do not change force_cmd may need to reuse!!
        my_log = "gen.log"  # arg_testname.replace(".py", ".gen.log")
        my_elog = "gen.err"  # arg_testname.replace(".py", ".gen.log")

        my_cmd = self.force_cmd % (self.task_file)  # , my_log)

        Msg.info(
            "GenCmd = "
            + str(
                {
                    GenerateKeys.gen_app: "force",
                    GenerateKeys.gen_cmd: my_cmd,
                    GenerateKeys.gen_log: my_log,
                    GenerateKeys.gen_elog: my_elog,
                    "max-instr": self.ctrl_item.max_instr,
                    "min-instr": self.ctrl_item.min_instr,
                }
            ),
            True,
        )
        my_return = SysUtils.exec_process(
            my_cmd, my_log, my_elog, self.ctrl_item.timeout, True
        )
        # the return from exec_process is a tuple, see generate_executor.py,
        # retcode, stdout, stderr, start-time, end-time
        my_results = self.extract_results(
            my_return, "./" + my_log, "./" + my_elog
        )
        Msg.info("GenResult = " + str(my_results))
        Msg.flush()

        return SysUtils.success(int(my_return[GenerateResult.process_retcode]))

    # extract information from logs methods
    def query_result_log(self, arg_hfile):

        my_seed = None
        my_total = 0
        my_secondary = 0
        my_default = 0

        my_results = []

        def get_colon_value(arg_line):
            """Get value from colon to EOL and return as int"""
            idx = arg_line.find(":")
            return int(arg_line[idx + 1 :])

        for my_line in arg_hfile:
            Msg.dbg("Process Line: %s" % my_line)

            if "]Secondary Instructions Generated" in my_line:
                my_secondary = get_colon_value(my_line)
                Msg.user("Secondary Instructions: %d" % my_secondary)

            elif "]Default Instructions Generated" in my_line:
                my_default = get_colon_value(my_line)
                Msg.user("Default Instructions: %d" % my_default)

            if "]Total Instructions Generated" in my_line:
                my_total = get_colon_value(my_line)
                Msg.user("Total Instructions: %d" % my_total)

            if "Initial seed" in my_line:
                my_seed = (
                    my_line.replace("[notice]", "")
                    .replace("Initial seed = ", "")
                    .strip()
                )

            if (
                my_seed is None
                or my_total == 0
                or my_secondary == 0
                or my_default == 0
            ):
                continue

        return my_seed, my_total, my_secondary, my_default

    def query_errors(self, arg_hfile):
        my_error = None
        for my_line in arg_hfile:
            if SysUtils.found(my_line.find("[fail]")):
                return my_line.replace("[fail]", "").strip()
