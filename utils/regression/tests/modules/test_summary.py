#!/usr/bin/env python3
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
import sys

from classes.summary import SummaryItem, SummaryQueueItem
from common.msg_utils import Msg
from common.path_utils import PathUtils
from common.sys_utils import SysUtils


#  MAIN FUNCTION of the module
#
def test_summary(aParameters):
    forrest_log = aParameters.forrest_log
    msg_level = aParameters.msg_lev
    if msg_level is not None:
        Msg.set_level(Msg.translate_levelstr(msg_level))

    print("Forrest log file is: %s" % forrest_log)

    work_dir, my_tmp = PathUtils.split_path(forrest_log)
    frun_path = PathUtils.append_path(
        PathUtils.include_trailing_path_delimiter(work_dir), "_def_frun.py"
    )

    # test - timeout
    summary_queue_args = {
        "frun-path": frun_path,
        "process-log": forrest_log,
        "process-result": (
            0,
            None,
            "Process Timeout Occurred",
            1555792446.313606,
            1555792446.313606,
            SysUtils.PROCESS_TIMEOUT,
        ),
    }
    summary_queue_item = SummaryQueueItem(summary_queue_args)
    summary_item = SummaryItem({})
    summary_item.load(summary_queue_item)


class CommandLineParameters(object):
    usage = (
        """
      Test summary module of master_run.

      Example:

        %s -f /path/to/regression/output/forrest.log
    """
        % sys.argv[0]
    )

    # save and pass on remainder
    pass_remainder = True

    # do not allow abbrev parameters, only in Python >3.5
    # allow_abbrev = False

    parameters = [
        # "short option"     "number of additonal args"
        # |      "long option"    |   "additional specifications"
        # |      |                |   |
        # |      |                |   |
        [
            "-f",
            "--forrest-log",
            1,
            {"required": True},
            "path to the forrest log file",
        ],
        ["-m", "--msg-lev", 1, {}, "debug message level"],
        # -h and --help is not needed, provided by default.
    ]


if __name__ == "__main__":
    from common.cmdline_utils import CmdLineParser, AttributeContainer

    cmd_line_parser = CmdLineParser(CommandLineParameters, add_help=True)
    args = cmd_line_parser.parse_args(sys.argv[1:])
    test_summary_parms = AttributeContainer()
    cmd_line_parser.set_parameters(test_summary_parms)

    test_summary(test_summary_parms)
