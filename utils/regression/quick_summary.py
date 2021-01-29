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
import traceback

from shared.msg_utils import Msg
from shared.path_utils import PathUtils


class SummaryLevel:
    Silent = 0
    Fail = 1
    Any = 2


class QuickSummary(object):
    def __init__(self, arg_summary_dir, arg_num_instr):

        self.summary_dir = arg_summary_dir
        self.results = {}
        self.total_count = 0
        self.failed_count = 0
        self.curr_test_id = None
        self.curr_test_log = None
        self.sim_cmd = None
        self.num_instr = arg_num_instr
        # self.test_path = arg_summary_pathx

    def summary_directory(self):
        return self.summary_dir

    def set_current_test(self, arg_test_id, arg_test_log, arg_sim_cmd):

        Msg.dbg(
            "RegressionSummary:: set_current_test( %s, %s, %s "
            % (str(arg_test_id), str(arg_test_log), str(arg_sim_cmd))
        )
        self.curr_test_id = arg_test_id
        self.curr_test_log = arg_test_log
        self.sim_cmd = arg_sim_cmd

    def add_result(self, arg_ret_code):

        my_tup = None
        self.total_count += 1

        Msg.dbg('Adding Simulation results, "%s"' % (str(self.sim_cmd)))
        if arg_ret_code != 0:
            # only need to create a tuple with the return code
            Msg.dbg("Simulation Failed, return code: " + str(arg_ret_code))
            self.failed_count += 1
            my_tup = (1, arg_ret_code, self.curr_test_log, None, "FAIL")

        else:
            my_lines = None
            try:
                Msg.dbg("Attempting to Open: " + self.curr_test_log)
                with open(self.curr_test_log, "r") as my_log:
                    Msg.dbg("File Open: " + self.curr_test_log)
                    my_lines = my_log.readlines()
                    Msg.dbg("Line %d: %s" % (len(my_lines), my_lines[-1]))
                    my_lastline = my_lines[-1]

                my_segs = my_lastline.split()
                my_num_instr = int(my_segs[0])
                if my_num_instr < self.num_instr:
                    Msg.dbg(
                        "Simulation Success, return code = 0 and the "
                        "instruction limit was not reached"
                    )
                    my_tup = (
                        0,
                        arg_ret_code,
                        self.curr_test_log,
                        None,
                        "PASS",
                    )
                else:
                    #
                    Msg.dbg("Simulation Failed, Instruction Limit Reached ")
                    self.failed_count += 1
                    my_tup = (
                        2,
                        arg_ret_code,
                        self.curr_test_log,
                        "Instruction Limit Reached: Failed at "
                        + str(self.num_instr)
                        + " Instructions ",
                        "FAIL",
                    )
            except BaseException:
                if Msg._debug():
                    traceback.print_exc(file=sys.stdout)
                my_tup = (
                    arg_ret_code,
                    "Unsupported",
                    "Unable to Extract Test Failure Information",
                    "FAIL",
                    " ",
                )
            finally:
                my_log.close()

        self.results[self.curr_test_id] = my_tup
        Msg.dbg("Results Tuple: " + str(my_tup))

        self.curr_test_id = None
        self.curr_test_log = None

    # deprocated use view for new work
    def summarize(self, sum_level=None):
        self.view(sum_level)

    def view(self, sum_level=SummaryLevel.Fail):
        # Instruction Over Flow Failure Count
        # print( "Regression::view() " )
        from datetime import datetime

        my_utcdt = datetime.utcnow()
        my_file_name = (
            PathUtils().include_trailing_path_delimiter(self.summary_dir)
            + "regression_summary_"
            + str(my_utcdt.year)
            + str(my_utcdt.month)
            + str(my_utcdt.day)
            + "-"
            + str(my_utcdt.hour)
            + str(my_utcdt.minute)
            + str(my_utcdt.second)
            + ".log"
        )

        print(my_file_name)

        try:
            my_ofile = None
            myLines = []
            # First try to open file
            with open(my_file_name, "w") as my_ofile:
                my_ofile.write("Date: " + str(my_utcdt.date()) + "\n")
                my_ofile.write("Time: " + str(my_utcdt.time()) + "\n")

                print("\n\n")

                for my_key, my_val in self.results.items():

                    # print( str( my_val ))
                    my_line = (
                        my_val[4]
                        + " - Test Name: "
                        + my_key
                        + ", Return Code: "
                        + str(my_val[1])
                        + ", Log File: "
                        + my_val[2]
                    )

                    if my_val[3]:
                        my_line += my_val[3]

                    my_ofile.write(my_line + "\n")

                    if sum_level > 2:
                        print(my_line)
                    elif sum_level == 1 and my_val[0] in [2]:
                        print(my_line)
                    elif sum_level == 2 and my_val[0] in [1, 2]:
                        print(my_line)

                my_ofile.write(
                    "Total Simulations: " + str(self.total_count) + "\n"
                )
                my_ofile.write(
                    "Total Fails:       " + str(self.failed_count) + "\n"
                )
                if self.total_count > 0:
                    my_ofile.write(
                        "Success Rate:      "
                        + "{0:.2f}".format(
                            100
                            * (self.total_count - self.failed_count)
                            / self.total_count
                        )
                        + "%\n"
                    )
                else:
                    my_ofile.write("Success Rate:      0.00%\n")

                my_ofile.write("Test Suite Complete\n")

                print("Total Simulations: " + str(self.total_count) + "\n")
                print("Total Fails:       " + str(self.failed_count) + "\n")
                if self.total_count > 0:
                    print(
                        "Success Rate:      "
                        + "{0:.2f}".format(
                            100
                            * (self.total_count - self.failed_count)
                            / self.total_count
                        )
                        + "%"
                    )
                else:
                    print("Success Rate:      0.00%")
                print("Test Suite Complete\n")

        except Exception as arg_ex:
            if Msg._debug():
                traceback.print_exception("Exception", arg_ex, None)
            print("Error Processing Summary, " + str(arg_ex))

        finally:
            pass
