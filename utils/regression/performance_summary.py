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
import re

from shared.datetime_utils import DateTime
from shared.msg_utils import Msg
from shared.path_utils import PathUtils
from shared.summary_core import (
    Summary,
    SummaryItem,
    SummaryLevel,
)
from shared.sys_utils import SysUtils


class PerformanceInstructionType:
    Total = 0
    Secondary = 1
    Default = 2

    performance_instruction_type_strs = ["Total", "Secondary", "Default"]

    @classmethod
    def instruction_type(cls, arg_str):
        if arg_str == "Total":
            return PerformanceInstructionType.Total

        if arg_str == "Secondary":
            return PerformanceInstructionType.Secondary

        if arg_str == "Default":
            return PerformanceInstructionType.Default

        raise Exception("Unable to indentify the instruction type")


class PerformanceSummaryItem(SummaryItem):
    def unpack(self, arg_queue_item):
        super().unpack(arg_queue_item)

    # save the generation results for this task item
    def commit_generate(self):

        self.force_result = SysUtils.ifthen(
            SysUtils.success(self.force_retcode), "PASS", "FAIL"
        )
        self.force_level = SummaryLevel.Any

        if SysUtils.failed(self.force_retcode):
            self.force_level = SummaryLevel.Fail

        Msg.lout(self, "user", "Performance Summary Item Commit Generate")
        if SysUtils.success(self.force_retcode):
            self.instruction_counts()
            Msg.info(
                "Instructions: %d, Default: %d, Secondary: %d, "
                "Elapsed Time: %0.5f Seconds\n\n"
                % (
                    self.count,
                    self.default,
                    self.secondary,
                    self.force_end - self.force_start,
                )
            )
            return 1

        return 0

    def commit_simulate(self):

        self.instr_count, self.iss_message = self.extract_iss_info()
        Msg.user(
            "Instr Count: %d, Message: %s"
            % (self.instr_count, self.iss_message)
        )
        try:

            # check the return code for error
            if SysUtils.success(self.iss_retcode):
                # Check to see for instruction overrun
                if int(self.instr_count) < self.max_instr:
                    if int(self.instr_count) >= self.min_instr:
                        self.iss_result = "PASS"
                        self.iss_level = SummaryLevel.Any
                        return 1

            self.iss_result = "FAIL"
            self.iss_level = SummaryLevel.Fail
            return 0
        finally:
            pass
        #     Msg.user( "Iss Result: %s" % ( self.iss_result ))

    def instruction_counts(self):
        my_lines = None
        my_glog = "%s%s" % (
            PathUtils.include_trailing_path_delimiter(self.work_dir),
            self.force_log,
        )

        Msg.user("Path: %s" % my_glog)

        with open(my_glog, "r") as my_log:
            my_lines = my_log.readlines()
            Msg.dbg("Line %d: %s" % (len(my_lines), my_lines[-1]))
            my_log.close()
        try:
            my_results = [
                my_tmp
                for my_tmp in my_lines
                if re.search(" Instructions Generated", my_tmp)
            ]
            Msg.lout(my_results, "dbg")
            if not my_results:
                raise Exception('Instruction Count Not Found in "gen.log"')

            # ok there are instruction counts located
            for my_line in my_results:
                my_line = my_line.strip()

                # find the instruction type (Total, Default, Secondary)
                my_lpos = my_line.find("]")
                my_rpos = my_line.find("Instr")
                my_type = PerformanceInstructionType.instruction_type(
                    (my_line[my_lpos + 1 : my_rpos - 1]).strip()
                )

                # get the count for this instruction type
                my_lpos = my_line.find(":")
                my_count = int(my_line[my_lpos + 2 :].strip())

                if my_type == PerformanceInstructionType.Total:
                    self.count = my_count
                elif my_type == PerformanceInstructionType.Secondary:
                    self.secondary = my_count
                elif my_type == PerformanceInstructionType.Default:
                    self.default = my_count

        except ValueError:
            Msg.error_trace()
            Msg.err(
                "Unable to extract instruction count from %s"
                % (int(my_lines[-1]))
            )

        return 0


class PerformanceSummary(Summary):
    def __init__(self, arg_summary_path):
        super().__init__(arg_summary_path)
        self.gen_total = 0
        self.gen_passed = 0
        self.iss_total = 0
        self.iss_passed = 0
        self.task_total = 0
        self.start_time = DateTime.Time()

    def create_summary_item(self):
        return PerformanceSummaryItem()

    def commit_item(self, arg_item):

        self.task_total += 1

        # replaced tasks dict with defaultdict(list) in parent
        self.tasks[arg_item.task_id].append(arg_item)
        self.groups.add_item(arg_item)
        my_results = arg_item.commit()
        self.gen_total += my_results[0]
        self.gen_passed += my_results[1]
        self.iss_total += my_results[2]
        self.iss_passed += my_results[3]

    def process_summary(self, sum_level=SummaryLevel.Fail):

        my_file_name = "%sperformance_summary.log" % (
            PathUtils().include_trailing_path_delimiter(self.summary_dir)
        )
        Msg.dbg("Master Log File: %s" % (my_file_name))
        my_utcdt = DateTime.UTCNow()
        my_ofile = None
        try:
            # First try to open file
            with open(my_file_name, "w") as my_ofile:

                my_ofile.write("Date: %s\n" % (DateTime.DateAsStr(my_utcdt)))
                my_ofile.write("Time: %s\n" % (DateTime.TimeAsStr(my_utcdt)))

                self.process_errors(my_ofile)
                my_total_count, my_total_elapsed = self.process_groups(
                    my_ofile
                )

                Msg.blank("info")
                my_line = "Total Instructions Generated: %3d\n" % (
                    my_total_count
                )
                my_line += "Total Elapsed Time:  %0.3f\n" % (my_total_elapsed)
                my_line += "Overall Instructions per Second:  %0.3f\n" % (
                    SysUtils.ifthen(
                        bool(my_total_elapsed),
                        my_total_count / my_total_elapsed,
                        0,
                    )
                )

                Msg.info(my_line)
                my_ofile.write(my_line)

        except Exception as arg_ex:
            Msg.error_trace()
            Msg.err("Error Processing Summary, " + str(arg_ex))

        finally:
            my_ofile.close()

    def process_groups(self, arg_ofile):

        my_total_count = 0
        my_total_elapsed = 0

        my_groups = self.groups.task_groups()

        # Msg.trace("PerformanceSummaryItem::process_groups")
        for my_group, my_items in my_groups.items():
            try:
                my_str = "\nBegin Group: %s\n" % (my_group)
                arg_ofile.write(my_str)
                Msg.blank("info")
                Msg.info(my_str)

                my_grp_count, my_grp_elapsed = self.process_group_items(
                    arg_ofile, my_items
                )

                my_total_count += my_grp_count
                my_total_elapsed += my_grp_elapsed

                my_line = "\nGroup Instructions: %3d\n" % (my_grp_count)
                my_line += "Group Elapsed Time: %0.3f\n" % (my_grp_elapsed)
                my_line += "Group Instructions per Second:  %0.3f\n" % (
                    SysUtils.ifthen(
                        bool(my_grp_elapsed), my_grp_count / my_grp_elapsed, 0
                    )
                )
                my_line += "End Group: %s\n" % (my_group)

                Msg.info(my_line)
                arg_ofile.write(my_line)

            except Exception as arg_ex:

                Msg.error_trace()
                Msg.err(
                    "Unable to process, Group: %s, Reason: %s"
                    % (my_group, type(arg_ex))
                )

        return my_total_count, my_total_elapsed

    def process_group_items(self, arg_ofile, arg_items):

        my_grp_count = 0
        my_grp_elapsed = 0
        try:
            for my_item in arg_items:
                my_item_elapsed = my_item.force_end - my_item.force_start
                my_item_count = my_item.total

                my_grp_elapsed += my_item_elapsed
                my_grp_count += my_item_count

                my_line = "\nTask: %s, Instructions: %d, Elapsed: %0.3f\n" % (
                    my_item.task_id,
                    my_item_count,
                    my_item_elapsed,
                )
                arg_ofile.write(my_line)
                # Msg.dbg( my_line )
                # end: for my_task in my_group["tasks"]:

        except Exception as arg_ex:
            Msg.error_trace()
            Msg.err("Error Processing Summary, Reason: %s" % (str(arg_ex)))

        return my_grp_count, my_grp_elapsed
