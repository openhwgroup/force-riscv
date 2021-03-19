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
# file:
# comment: implements FpixExecutor which serves as a Class Wrapper for
#          for executing the simulator family in client processing apps

from executors.iss_executor import IssExecutor, Msg, SysUtils, IssResult
from classes.ApplicationOption import ControlItemOption
import re


class FpixExecutor(IssExecutor):

    cInstrNumPattern = re.compile(
        r"Executed (\d+) instructions, exit status \(([\w: ]+)\)"
    )

    # The default options that will be appended to the command line if they
    # haven't been specified already.
    #
    cDefaultOptions = [
        # Preferred option name to use
        #  | Connector char to use between the option and its parameter
        #  |  | The other option name, if exist
        #  |  |  | Connector char for the other option name
        #  |  |  |  | Default value
        #  |  |  |  |  |
        ControlItemOption(("-w", " "), ("--wfx_nop", " "), 1),
        ControlItemOption(("--exit_loop", "="), ("-X", " "), 1),
    ]

    def __init__(self):
        super().__init__()
        self.mIssSoPath = None
        self.mFpixPath = None
        self.GEN_LOG_FILENAME = "gen.log"
        self.MY_RAILHOUSE_LOG = "fpix_riscv.railhouse"
        self.sim_cmd = None
        self.sim_log = None

    def load(self, arg_ctrl_item):
        super().load(arg_ctrl_item)

        Msg.dbg("ExecuteController::initialize_sim()")
        if not self.ctrl_item.no_sim:
            self.sim_log = "fpix_sim.log"

    def skip(self):
        if self.ctrl_item.no_sim:
            Msg.user("[FpixExecutor::skip] skipping due to no-sim")
            return True

        if "skip" in self.ctrl_item.fpix_riscv.keys():
            if self.ctrl_item.fpix_riscv["skip"]:
                Msg.user(
                    "[FpixExecutor::skip] skipping due to --fpix_riscv.skip "
                    "specified"
                )
            return True

        if "fpix_path" not in self.ctrl_item.fpix_riscv.keys():
            Msg.user(
                "[FpixExecutor::skip] skipping due to no fpix_path specified"
            )
            return True

        return False

    def execute(self):
        my_result = None
        try:
            if self.ctrl_item.suffix is not None:
                my_task_name = self.task_name.replace(
                    "_force", "_%s_force" % (str(self.ctrl_item.suffix))
                )
            else:
                my_task_name = self.task_name

            my_elf = self.locate_test_case("*.Default.ELF", my_task_name)

            my_log = self.sim_log
            my_elog = self.sim_log

            if "cfg" not in self.ctrl_item.fpix_riscv.keys():
                Msg.err(
                    "Fpix_ISS did not properly execute, Reason: Fpix config "
                    "was not specified"
                )
                return False

            if "fpix_path" not in self.ctrl_item.fpix_riscv.keys():
                Msg.err(
                    "FpixExecutor::execute: did not recieve a path to Fpix "
                    "application fpix_path."
                )
                raise
            else:
                self.mFpixPath = self.ctrl_item.fpix_riscv["fpix_path"]

            # build the sim_cmd now that we have full information available
            self.sim_cmd = (
                "%s --railhouse %s --cluster_num %d "
                "--core_num %d --threads_per_cpu %d -i %d "
                "--cfg %s"
                % (
                    self.mFpixPath,
                    self.MY_RAILHOUSE_LOG,
                    self.ctrl_item.num_chips,
                    self.ctrl_item.num_cores,
                    self.ctrl_item.num_threads,
                    self.ctrl_item.max_instr,
                    self.ctrl_item.fpix_riscv["cfg"],
                )
            )

            self.sim_cmd += " %s"

            # initalize the iss summary
            my_cmd = self.sim_cmd % my_elf

            # report the command line
            Msg.info("ISSCommand = " + str({"command": my_cmd}))
            # execute the simulation
            my_result = SysUtils.exec_process(
                my_cmd, my_log, my_elog, self.ctrl_item.timeout, True
            )

            my_extract_results = self.extract_results(
                my_result, my_log, my_elog
            )

            # report the results
            Msg.info("ISSResult = " + str(my_extract_results))

            # Fpix riscv is not made to provide the sort of information that
            # the message system expects. Almost anything here will cause the
            # next app not to run.

        except Exception as arg_ex:
            Msg.error_trace("Fpix_ISS Execute Failure")
            Msg.err(
                "Fpix_ISS did not properly execute, Reason: %s" % (str(arg_ex))
            )
            return False

        return SysUtils.success(int(my_result[IssResult.process_retcode]))

    def open_log_file(self, aFileName, aOpenMode):
        from file_read_backwards import FileReadBackwards

        return FileReadBackwards(aFileName)

    def query_result_log(self, aHfile):
        instr_count = None
        my_message = None
        for line in aHfile:
            if line.find("Executed") != -1:
                search_result = re.search(self.cInstrNumPattern, line)
                if search_result is not None:
                    Msg.user("Matched line: %s" % line, "ISS-LOG")
                    instr_count = int(search_result.group(1))
                    my_message = search_result.group(2)
                    break

        if instr_count is None:
            return 0, "Simulator terminated before completion"

        Msg.user(
            "InstructionCount: %d, Message: %s" % (instr_count, my_message),
            "ISS-LOG",
        )

        return instr_count, my_message
