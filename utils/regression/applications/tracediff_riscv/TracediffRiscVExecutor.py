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

from executors.tool_executor import *


class TracediffRiscVExecutor(ToolExecutor):
    def __init__(self):
        super().__init__()
        self.trace_cmd = None
        self.log = None
        self.elog = None

    def load(self, arg_ctrl_item):
        super().load(arg_ctrl_item)

    def skip(self):
        return False

    def execute(self):
        if not PathUtils.check_file("sim.log"):
            Msg.info("[TracediffRiscVExecutor::skip] skipping since no sim.log found")
            return True
        if not PathUtils.check_file("fpix_sim.log"):
            Msg.info("[TracediffRiscVExecutor::skip] skipping since fpx_sim.log not found")
            return True

        my_cmd = (
            'diff -y fpix_sim.log sim.log | grep -ve "---" | grep -vie '
            '"exi" | grep -vie "exe" | grep -ve "_t"'
        )

        self.log = "tracediff_result.log"
        self.elog = "tracediff_result.err"

        my_result = SysUtils.exec_process(
            my_cmd, self.log, self.elog, self.ctrl_item.timeout, True
        )
        my_use_result = None

        vbar_symbol_count = 0
        exception_count = 0
        success = False
        with open(self.log, "r") as results_file:
            for line in results_file:
                vbar_symbol_count += line.count("|")
                if "Excpt ID 0x2" in line:
                    exception_count = exception_count + 1
            if vbar_symbol_count == 0 and exception_count == 0:
                success = True
            my_use_result = list(my_result)
            my_use_result[0] = int(not success)
            # This inversion is necessary because int 0 means success to the
            # Summary class.

        with open(self.log, "a") as out_file:
            if not success:
                out_file.write(
                    "tracediff_riscv.log fail, look for | symbols or "
                    "'Excpt ID 0x2'; "
                    + str(vbar_symbol_count)
                    + " mismatches, and up to "
                    + str(exception_count)
                    + " suspicious exceptions."
                )
            else:
                out_file.write(
                    "tracediff_riscv.log success, only bootcode difference "
                    "between standalone and interactive as expected."
                )

        Msg.info("CMPCommand = " + str({"trace-cmp-cmd": my_cmd}))
        my_extract_results = self.extract_results(my_use_result, "./" + self.log, None)
        Msg.info("CMPResult = " + str(my_extract_results))

        Msg.flush()

        # return SysUtils.success(0) #[0,1,2] #Doesn't seem to really matter
        # what this is, the Summary system needs fixing.
        return SysUtils.success(int(my_result[ToolResult.process_retcode]))

    def query_result_log(self, arg_hfile):
        my_msg = ""
        # for now only the first line is of interest
        for line in arg_hfile:
            pass
        my_msg = line
        return my_msg
