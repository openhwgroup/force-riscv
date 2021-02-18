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
# Threading Base
from collections import defaultdict
from common.collections import HiThreadedProducerConsumerQueue
from common.kernel_objs import HiCriticalSection
from common.msg_utils import Msg
from common.path_utils import PathUtils
from common.sys_utils import SysUtils
from common.threads import HiOldThread, workers_done_event, summary_done_event


class SummaryNdx(object):
    RetCode = 0
    StdOut = 1
    StdErr = 2
    StartTime = 3
    EndTime = 4


class SummaryLevel:
    Silent = 0  # only show summary info, log all results to summary log
    Fail = 1  # show only the fails,
    Any = 2


class SummaryDetail:
    Nothing = 0x0000
    GenCmd = 0x0001
    GenResult = 0x0002
    IssCmd = 0x0004
    IssResult = 0x0008
    RtlCmd = 0x0010
    RtlResult = 0x0020
    TraceCmpCmd = 0x0040
    TraceCmpResult = 0x0080
    Signaled = 0x0100
    AnyDetail = 0x01FF


class SummaryQueueItem(object):
    def __init__(
        self, arg_process_info
    ):  # arg_frun_path, arg_parent_fctrl, arg_fctrl_item, arg_group ):
        Msg.user("Results Info: %s" % (str(arg_process_info)), "SUM-QITEM")
        # Msg.lout( arg_process_info, "user", "SUM-QITEM" )
        self.process_info = arg_process_info


class SummaryErrorQueueItem(object):
    def __init__(self, arg_error_info):
        self.error_info = arg_error_info


class SummaryErrorItem(object):
    def __init__(self):
        self.error_item = None
        self.error_item = None

    def load(self, arg_error_qitem):
        self.extract_error(arg_error_qitem)
        self.report()

    def extract_error(self, arg_error_qitem):
        self.error_item = arg_error_qitem.error_info

    def get_err_line(self):
        my_err_type = (
            str(self.error_item["type"])
            .replace("<class '", "")
            .replace("'>", "")
        )

        return '[ERROR] Type: %s, Error: %s, Path: "%s", Message: %s' % (
            my_err_type,
            str(self.error_item["error"]),
            str(self.error_item["path"]),
            str(self.error_item["message"]),
        )

    def report(self):
        pass


class SummaryQueue(HiThreadedProducerConsumerQueue):
    def __init__(self):
        super().__init__(True)


class SummaryItem(object):
    def __init__(self, arg_summary):
        self.summary = arg_summary

        self.generate_extractor = None
        self.iss_extractor = None
        self.tool_extractor = None

        self.force_elog = None
        self.force_stderr = None
        self.force_stdout = None
        self.force_level = SummaryLevel.Any
        self.force_start = 0.00
        self.force_end = 0.00

        self.force_cmd = None
        self.force_log = None
        self.force_retcode = None

        self.iss_cmd = None
        self.iss_log = None
        self.iss_retcode = None
        self.instr_count = None

        self.rtl_cmd = None
        self.rtl_log = None
        self.rtl_retcode = None
        self.cycle_count = None

        self.trace_cmp_cmd = None
        self.trace_cmp_log = None
        self.trace_cmp_retcode = None

        self.signal_id = None
        self.signal_message = None

        self.max_instr = None
        self.min_instr = None

        self.detail_flags = 0

        self.default = None
        self.secondary = None
        self.total = None

        self.sum_tups = [
            (
                "GenCmd",
                self.load_gen_info,
                SummaryDetail.GenCmd,
            ),  # Contains a Gen Command Line
            (
                "GenResult",
                self.load_gen_result,
                SummaryDetail.GenResult,
            ),  # Contains a Gen Result Line
            (
                "ISSCommand",
                self.load_iss_info,
                SummaryDetail.IssCmd,
            ),  # Contains a ISS Command Line
            (
                "ISSResult",
                self.load_iss_result,
                SummaryDetail.IssResult,
            ),  # Contains a ISS Result Line
            (
                "RTLCommand",
                self.load_rtl_info,
                SummaryDetail.RtlCmd,
            ),  # Contains a RTL Command Line
            (
                "RTLResult",
                self.load_rtl_result,
                SummaryDetail.RtlResult,
            ),  # Contains a RTL Result Line
            (
                "CMPCommand",
                self.load_trace_cmp_info,
                SummaryDetail.TraceCmpCmd,
            ),
            (
                "CMPResult",
                self.load_trace_cmp_result,
                SummaryDetail.TraceCmpResult,
            ),
            ("Signal", self.load_signaled, SummaryDetail.Signaled),
        ]

        self.process_cmd = None
        self.process_log = None
        self.process_result = None
        self.frun_path = None
        self.parent_fctrl = None
        self.fctrl_item = None
        self.item_group = None
        self.fctrl_content = None
        self.work_dir = None
        self.task_id = None
        self.task_index = None
        self.task_path = None
        self.force_msg = None
        self.seed = None
        self.iss_message = None
        self.rtl_message = None
        self.trace_cmp_msg = None
        self.passed = None

    def load(self, arg_queue_item):

        try:
            self.load_process_info(arg_queue_item.process_info)
            self.load_task_info()
            self.load_process_log()
            self.report()

        except Exception as arg_ex:
            Msg.error_trace()
            Msg.err(str(arg_ex))

        except BaseException:
            Msg.error_trace()

    def report(self):

        if self.default is None:
            self.default = 0
        if self.secondary is None:
            self.secondary = 0
        if self.total is None:
            self.total = 0

        Msg.info(
            "Task Id: %s, Task Index: %d" % (self.task_id, self.task_index)
        )

        my_msg = "Process Log Contains "
        if (
            self.detail_flags & SummaryDetail.AnyDetail
            == SummaryDetail.Nothing
        ):
            my_msg = "Process No Found or is Empty"
        else:
            if (
                self.detail_flags & SummaryDetail.GenCmd
                == SummaryDetail.GenCmd
            ):
                my_msg += "GenCmd, "
            if (
                self.detail_flags & SummaryDetail.GenResult
                == SummaryDetail.GenResult
            ):
                my_msg += "GenResult, "
            if (
                self.detail_flags & SummaryDetail.IssCmd
                == SummaryDetail.IssCmd
            ):
                my_msg += "IssCommand, "
            if (
                self.detail_flags & SummaryDetail.IssResult
                == SummaryDetail.IssResult
            ):
                my_msg += "IssResult, "
            if (
                self.detail_flags & SummaryDetail.TraceCmpCmd
                == SummaryDetail.TraceCmpCmd
            ):
                my_msg += "TraceCmpCommand, "
            if (
                self.detail_flags & SummaryDetail.TraceCmpResult
                == SummaryDetail.TraceCmpResult
            ):
                my_msg += "TraceCmpResult, "

            # remove the trailing comma
            my_msg = my_msg[:-2]

        Msg.user(my_msg, "SUM-RPT")
        Msg.user(
            "Force Return Code: %s" % (str(self.force_retcode)), "SUM-RPT"
        )
        # Msg.trace()

        # Msg.info( "Originating Control File: %s"  % ( self.parent_fctrl ))
        # Msg.info( "Control File Item: %s"  % ( str(  self.fctrl_item )))  2
        # Msg.info( "F-Run Control File: %s" % ( self.frun_path ))
        Msg.dbg("Force Ret Code: %s" % (str(self.force_retcode)))

        if SysUtils.success(self.force_retcode):
            Msg.info("[SUCCESS] Generate Command: %s" % (str(self.force_cmd)))
            Msg.info(
                "Instructions Generated - "
                "Default: %d, Secondary: %d, Total: %d"
                % (self.default, self.secondary, self.total)
            )
        elif self.signal_id is not None:
            Msg.info(
                "[INCOMPLETE] Generate Command: %s" % (str(self.force_cmd))
            )
            Msg.info("No Instructions Generated")
        else:
            Msg.info("[FAILED] Generate Command: %s" % (str(self.force_cmd)))
            Msg.info("No Instructions Generated")

        if self.iss_cmd is not None:
            if self.iss_success():
                Msg.info("[SUCCESS] ISS Command: %s" % (str(self.iss_cmd)))
            elif self.signal_id is not None:
                Msg.info("[INCOMPLETE] ISS Command: %s" % (str(self.iss_cmd)))
            else:
                Msg.info("[FAILED] ISS Command: %s" % (str(self.iss_cmd)))
            # Msg.fout( self.iss_log, "user" )

        if self.rtl_cmd is not None:
            if SysUtils.success(self.rtl_retcode):
                Msg.info("[SUCCESS] RTL Command: %s" % (str(self.rtl_cmd)))
                Msg.info("Cycle count : %d" % (self.cycle_count))
            elif self.signal_id is not None:
                Msg.info("[INCOMPLETE] RTL Command: %s" % (str(self.rtl_cmd)))
            else:
                Msg.info("[FAILED] RTL Command: %s" % (str(self.rtl_cmd)))

        if (
            self.detail_flags & SummaryDetail.TraceCmpCmd
            == SummaryDetail.TraceCmpCmd
        ):
            Msg.info("Comparing Simulation output .... ")
            if SysUtils.success(self.trace_cmp_retcode):
                Msg.info(
                    "[SUCCESS] Trace Compare Command: %s"
                    % (str(self.trace_cmp_cmd))
                )
            elif self.signal_id is not None:
                Msg.info(
                    "[INCOMPLETE] Trace Compare Command: %s"
                    % (str(self.trace_cmp_cmd))
                )
            else:
                Msg.info(
                    "[FAILED] Trace Compare Command: %s"
                    % (str(self.trace_cmp_cmd))
                )

        Msg.blank()

    def clean_up(self):

        clean_up_rules = self.summary.cleanUpRules
        if self.passed and not clean_up_rules.shouldKeepAll():
            # list all the files and delete them except for _def_frun.py and
            # the processor log (forrest.log)

            my_dir = PathUtils.include_trailing_path_delimiter(self.work_dir)
            Msg.user("Dir: %s" % (str(my_dir)), "FILES-TO-REMOVE")
            my_file_path = str(my_dir) + "*"
            Msg.user("Path: %s" % (my_file_path), "FILES-TO-REMOVE")

            my_file_list = PathUtils.list_files(my_file_path)
            Msg.user("Files: %s" % (str(my_file_list)), "FILES-TO-REMOVE")

            process_log_base = PathUtils.base_name(self.process_log)
            clean_up_rules.setBaseNamesToKeep(
                ["_def_frun.py", "PASS", process_log_base]
            )

            for my_file in my_file_list:
                if clean_up_rules.shouldKeepFile(my_file):
                    Msg.user("File: %s KEPT" % clean_up_rules.lastBaseName())
                else:
                    Msg.user(
                        "File: %s REMOVED" % clean_up_rules.lastBaseName()
                    )
                    PathUtils.remove(my_file)

    def iss_success(self):
        return True

    def has_generate(self):
        return self.force_cmd is not None

    def has_simulate(self):
        return self.iss_cmd is not None

    def has_rtl(self):
        return self.rtl_cmd is not None

    def has_trace_cmp(self):
        return self.trace_cmp_cmd is not None

    # extract the client process information
    def load_process_info(self, arg_process_info):

        Msg.dbg(
            "self.process_result: (%s)"
            % (str(arg_process_info.get("process-result", None)))
        )

        self.process_cmd = arg_process_info.get("process-cmd", None)
        self.process_log = arg_process_info.get("process-log", None)
        self.process_result = arg_process_info.get("process-result", None)
        self.frun_path = arg_process_info.get("frun-path", None)
        self.parent_fctrl = arg_process_info.get("parent-fctrl", None)
        self.fctrl_item = arg_process_info.get("fctrl-item", None)
        self.item_group = arg_process_info.get("item-group", None)
        self.fctrl_content = arg_process_info.get("content", None)

        self.detail_flags = SummaryDetail.Nothing

    # extract information used to execute the task
    def load_task_info(self):

        self.task_id = None
        self.task_index = None
        self.work_dir, my_tmp = PathUtils.split_path(self.frun_path)
        my_tmp, my_index = PathUtils.split_dir(self.work_dir)
        my_tmp, self.task_id = PathUtils.split_dir(my_tmp)
        self.task_index = int(my_index)
        self.task_path = PathUtils.include_trailing_path_delimiter(
            str(self.work_dir)
        )

    def check_missing_result(self, aProcessResult, aModuleName):
        message = aProcessResult[2]
        known_abnormal = False
        if aProcessResult[5] != SysUtils.NORMAL:
            retcode = aProcessResult[5]
            known_abnormal = True
        else:
            retcode = aProcessResult[0]
            if retcode == 0:
                retcode = -1
        if len(message) == 0:
            message = " due to unknown reasons."

        message = "Missing %s result, " % aModuleName + message
        if (self.signal_id is None) or known_abnormal:
            self.signal_id = retcode
            self.signal_message = message

        return retcode, message

    # parse process log
    def load_process_log(self):

        Msg.fout(self.process_log, "dbg")
        with open(self.process_log, "r") as my_flog:
            try:
                for my_line in my_flog:
                    Msg.user("Load: %s" % my_line, "PROCESS-LINE")
                    self.load_process_line(my_line)

            except Exception as arg_ex:
                Msg.error_trace(arg_ex)
                Msg.err(str(arg_ex))
            finally:
                my_flog.close()

        # here is the lowdown, if a command line exists then a result line must
        # also exist, check for generate pair
        if (self.detail_flags & SummaryDetail.GenCmd) and not (
            self.detail_flags & SummaryDetail.GenResult
        ):
            retcode, message = self.check_missing_result(
                self.process_result, "generator"
            )
            self.load_gen_result(
                {
                    "retcode": retcode,
                    "stdout": self.process_result[1],
                    "stderr": self.process_result[2],
                    "start": self.process_result[3],
                    "end": self.process_result[4],
                    "message": message,
                }
            )

        # check for summary pair
        elif (self.detail_flags & SummaryDetail.IssCmd) and not (
            self.detail_flags & SummaryDetail.IssResult
        ):
            retcode, message = self.check_missing_result(
                self.process_result, "iss"
            )
            self.load_iss_result(
                {"retcode": retcode, "log": None, "message": message}
            )

        # check for compare pair
        elif (self.detail_flags & SummaryDetail.TraceCmpCmd) and not (
            self.detail_flags & SummaryDetail.TraceCmpResult
        ):
            retcode, message = self.check_missing_result(
                self.process_result, "trace-cmp"
            )
            self.load_trace_cmp_result(
                {
                    "trace-cmp-retcode": retcode,
                    "trace-cmp-log": None,
                    "message": message,
                }
            )

        # check for RTL pair
        elif (self.detail_flags & SummaryDetail.RtlCmd) and not (
            self.detail_flags & SummaryDetail.RtlResult
        ):
            retcode, message = self.check_missing_result(
                self.process_result, "rtl"
            )
            self.load_rtl_result(
                {"retcode": retcode, "log": None, "message": message}
            )

    def load_process_line(self, arg_line):
        Msg.user("Process Result Line: %s" % (str(arg_line)), "SUM-TUPLE")
        if arg_line[0] == "[":
            return
        my_val = None
        try:
            my_glb, my_loc = SysUtils.exec_content(arg_line, True)
        except (SyntaxError, TypeError) as arg_ex:
            return
        except BaseException:
            raise

        # summary tuples are initialized in the __init__ as are the element
        # indexes the idea is to the tuple list for a match on the key. When
        # one is found the callback proc that is referenced is executed with
        # the line dictionary retrieved and the flags are updated as to what
        # this summary item contains

        for (my_key, my_proc, my_mask) in self.sum_tups:

            my_result = my_loc.get(my_key, None)

            if my_result is None:
                # nothing was returned continue causes the next element to be
                # checked if there is a next element
                continue

            my_proc(my_result)

            self.detail_flags |= int(my_mask)
            break

        return True

        # raise Exception ????

    def load_gen_info(self, arg_dict):

        self.force_cmd = arg_dict["command"]
        self.force_log = arg_dict["log"]
        self.force_elog = arg_dict["elog"]
        self.max_instr = arg_dict["max-instr"]
        self.min_instr = arg_dict["min-instr"]

    # load the force generate results
    def load_gen_result(self, arg_dict):

        try:

            my_retcode = str(arg_dict["retcode"]).strip()
            Msg.user("Return Code: %s" % (my_retcode))

            if my_retcode is None:
                self.force_retcode = -1
                raise Exception("Generate Return Code Not Found")

            self.force_retcode = int(str(arg_dict["retcode"]).strip())

        except BaseException:
            self.force_retcode = -1
            Msg.err("Generate Return Code in unrecognizable format")

        self.force_stdout = arg_dict["stdout"]
        self.force_stderr = arg_dict["stderr"]

        if SysUtils.failed(self.force_retcode):
            self.force_level = SummaryLevel.Fail

        self.force_start = float(arg_dict.get("start", 0.00))
        self.force_end = float(arg_dict.get("end", 0.00))

        self.secondary = int(arg_dict.get("secondary", 0))
        self.default = int(arg_dict.get("default", 0))
        self.total = int(arg_dict.get("total", 0))

        if self.signal_id is None:
            self.force_msg = arg_dict.get("message", None)
            self.seed = str(arg_dict.get("seed", "Seed Not Found"))
        else:
            self.force_msg = "Incomplete, Signal Id: %s, %s " % (
                str(self.signal_id),
                str(self.signal_message),
            )
            self.seed = None
        # Msg.lout( self, "dbg", "General Summary Item ... " )

    # load the iss execution information
    def load_iss_info(self, arg_dict):
        # Msg.lout( arg_dict, "user", "ISS Info Dictionary ... " )
        self.iss_cmd = arg_dict["command"]

    # load the iss execution results
    def load_iss_result(self, arg_dict):
        # Msg.lout( arg_dict, "user", "ISS Results Dictionary ... " )
        self.iss_log = arg_dict["log"]
        self.instr_count = int(arg_dict.get("count", 0))
        self.iss_retcode = int(arg_dict["retcode"])

        if self.signal_id is None:
            self.iss_message = str(arg_dict.get("message", None))
        else:
            self.iss_message = "Incomplete, Signal Id: %s, %s " % (
                str(self.signal_id),
                str(self.signal_message),
            )

    # load the rtl execution information
    def load_rtl_info(self, arg_dict):
        # Msg.lout( arg_dict, "user", "RTL Info Dictionary ... " )
        self.rtl_cmd = arg_dict["rtl-command"]

    # load the rtl execution results
    def load_rtl_result(self, arg_dict):
        # Msg.lout( arg_dict, "user", "ISS Results Dictionary ... " )
        self.rtl_log = arg_dict["log"]
        self.cycle_count = int(arg_dict.get("count", 0))
        self.rtl_retcode = int(arg_dict["retcode"])

        if self.signal_id is None:
            self.rtl_message = str(arg_dict.get("message", None))
        else:
            self.rtl_message = "Incomplete, Signal Id: %s, %s " % (
                str(self.signal_id),
                str(self.signal_message),
            )

    # load the cmp execution information
    def load_trace_cmp_info(self, arg_dict):
        # Msg.lout( arg_dict, "user", "CMP Info Dictionary ... " )
        self.trace_cmp_cmd = arg_dict["trace-cmp-cmd"]

    # load the cmp execution results
    def load_trace_cmp_result(self, arg_dict):

        # Msg.lout( arg_dict, "user", "CMP Results Dictionary ... " )
        self.trace_cmp_log = arg_dict["trace-cmp-log"]
        try:
            self.trace_cmp_retcode = int(arg_dict["trace-cmp-retcode"])
            if self.signal_id is None:
                self.trace_cmp_msg = str(arg_dict["trace-cmp-msg"])
            else:
                self.trace_cmp_msg = "Incomplete, Signal Id: %s, %s " % (
                    str(self.signal_id),
                    str(self.signal_message),
                )

        except BaseException:
            self.trace_cmp_retcode = -1
            Msg.err("CMP Return Code in unrecognizable format")

    def load_signaled(self, arg_dict):
        Msg.user(str(arg_dict), "SIGNALED")

        try:
            self.signal_id = arg_dict["retcode"]
            self.signal_message = arg_dict["message"]
        except BaseException:
            self.signal_id = -1
            Msg.err("Signal Info Corrupt")

    # load the force generate information
    def commit(self):

        my_gen_cnt = 0
        my_gen_ret = 0
        my_sim_cnt = 0
        my_sim_ret = 0
        my_rtl_cnt = 0
        my_rtl_ret = 0
        my_trace_cmp_ret = 0
        my_trace_cmp_cnt = 0
        my_tgt_name = ""
        self.passed = True

        if self.has_generate():
            # Msg.user( "if self.has_generate(): True" )
            my_gen_cnt = 1
            my_gen_ret = self.commit_generate()
            self.passed = self.passed and bool(my_gen_ret)

        if self.has_simulate():
            # Msg.user( "if self.has_simulate(): True" )
            my_sim_cnt = 1
            my_sim_ret = self.commit_simulate()
            self.passed = self.passed and bool(my_sim_ret)

        if self.has_rtl():
            # Msg.user( "if self.has_rtl(): True" )
            my_rtl_cnt = 1
            my_rtl_ret = self.commit_rtl()
            self.passed = self.passed and bool(my_rtl_ret)

        # Msg.user( "SummaryItem::commit - [20]", "GOT HERE" )
        if self.has_trace_cmp():
            # Msg.user( "SummaryItem::commit - [21]", "GOT HERE"  )
            my_trace_cmp_cnt = 1
            # Msg.user( "SummaryItem::commit - [22]", "GOT HERE"  )
            my_trace_cmp_ret = self.commit_trace_cmp()
            self.passed = self.passed and bool(my_trace_cmp_ret)

        # Msg.user( "SummaryItem::commit - [24]", "GOT HERE"  )
        my_src_name = "%s%s" % (self.task_path, "STARTED")
        my_tgt_name = "%s%s" % (
            self.task_path,
            SysUtils.ifthen(
                self.passed,
                "PASS",
                SysUtils.ifthen(self.signal_id is None, "FAIL", "INCOMPLETE"),
            ),
        )

        PathUtils.move(my_src_name, my_tgt_name)

        if not self.passed:
            self.summary.do_on_fail(self)

        return (
            my_gen_cnt,
            my_gen_ret,
            my_sim_cnt,
            my_sim_ret,
            my_rtl_cnt,
            my_rtl_ret,
            my_trace_cmp_cnt,
            my_trace_cmp_ret,
        )


class SummaryGroups(object):
    def __init__(self):
        self.groups = {}
        self.queue = SummaryQueue()

    def update_groups(self, arg_group):
        if arg_group not in self.groups:
            self.groups[arg_group] = []

    # adds an item to a group list if the group does not exist the list is
    # created
    def add_item(self, arg_item):
        # Msg.dbg( "Item Group: %s"% ( arg_item.item_group ))
        if arg_item.item_group not in self.groups:
            self.groups[arg_item.item_group] = []
        self.groups[arg_item.item_group].append(arg_item)

    # returns the item list associated with the group passed as argument
    def group_items(self, arg_group):
        return self.groups[arg_group]

    # return the list of groups
    def task_groups(self):
        return self.groups


# class SummaryThread( HiThread ):
class SummaryThread(HiOldThread):
    def __init__(self, sq, summary):
        self.summary_queue = sq
        self.summary = summary
        # We do not want the thread to launch until we've loaded all the
        # properties
        super().__init__(True)

    def commit_item(self, arg_item):

        if arg_item.task_id not in self.summary.tasks:
            self.summary.tasks[arg_item.task_id] = []
            self.summary.task_lookup.append(arg_item.task_id)
        self.summary.groups.update_groups(arg_item.item_group)
        return 0

    def run(self):
        # Block on process queue while we have threads running and stuff to do

        try:
            while True:  # not workers_done_event.isSet():

                # Pop off the top of the process queue (should block if the
                # queue is empty)
                try:
                    my_qitem = self.summary_queue.dequeue(0)
                    if isinstance(my_qitem, SummaryErrorQueueItem):
                        Msg.user(str(my_qitem.error_info), "SUMMARY_ERROR")
                        my_eitem = SummaryErrorItem()
                        my_eitem.load(my_qitem)
                        self.summary.commit_error_item(my_eitem)
                    else:
                        my_item = self.summary.create_summary_item()
                        my_item.load(my_qitem)
                        self.summary.commit_item(my_item)
                        my_item.clean_up()

                    my_qitem = None

                except TimeoutError as arg_ex:
                    # {{{TODO}}} Implement proper heartbeat
                    # Msg.dbg( str( arg_ex ) )
                    if workers_done_event.isSet():
                        break
                    else:
                        self.HeartBeat()
                        continue

                except Exception as arg_ex:
                    Msg.error_trace()
                    Msg.err(str(arg_ex))
                    raise

                except BaseException:
                    Msg.error_trace()
                    raise
        finally:
            summary_done_event.Signal()


class Summary(object):
    def __init__(self, arg_summary_dir, arg_cleanUpRules):

        self.summary_dir = arg_summary_dir
        self.tasks = defaultdict(list)
        self.errors = []
        self.groups = SummaryGroups()
        self.queue = SummaryQueue()
        self.cleanUpRules = arg_cleanUpRules

        # common code protection
        self.crit_sec = HiCriticalSection()
        # failed callback
        self.on_fail_proc = None
        # check terminated callback
        self.is_term_proc = None

        # Launch a Summary Thread
        self.summary_thread = SummaryThread(self.queue, self)

    def lock(self):
        return True

    def unlock(self):
        pass

    def commit_error_item(self, arg_error):
        self.errors.append(arg_error)

    def process_errors(self, arg_ofile):

        if len(self.errors) > 0:
            arg_ofile.write("\n")
            Msg.blank()
            for my_eitem in self.errors:
                my_err_line = str(my_eitem.get_err_line())
                Msg.info(my_err_line)
                arg_ofile.write("%s\n" % (my_err_line))

    def set_on_fail_proc(self, arg_on_fail_proc):
        self.on_fail_proc = arg_on_fail_proc

    def do_on_fail(self, arg_sender):
        # if a fail proc exists then it will be called in a protected mode
        with self.crit_sec:
            if self.on_fail_proc is not None:
                self.on_fail_proc(arg_sender)

    def set_is_term_proc(self, arg_is_term_proc):
        self.is_term_proc = arg_is_term_proc

    def is_terminated(self):

        # if a terminate proc exists then the return from that proc will be
        # returned otherwise it will be False and should not have any effect
        # and other methods of determining termination should be used
        with self.crit_sec:
            if self.is_term_proc is not None:
                return self.is_term_proc()
        return False

    # abstracts
    def create_summary_item(self):
        pass

    def commit_item(self, arg_item):
        pass

    def process_summary(self, sum_level=SummaryLevel.Fail):
        pass
