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
#
# file: process_queue
# summary: Implemennts a Thread Safe Queue that us used as a marshall for
#          the ExecuteProcess workers
#
#

from common.path_utils import PathUtils
from common.msg_utils import Msg

from classes.launcher import LauncherType
from classes.summary import SummaryQueueItem, SummaryErrorQueueItem

from launchers.lsf_launcher import LsfLauncher
from launchers.std_launcher import StdLauncher

import concurrent.futures


class ProcessQueueItem(object):
    def __init__(self, arg_frun_path, arg_ctrl_item, arg_content):
        self.frun_path = arg_frun_path
        self.ctrl_item = arg_ctrl_item
        self.content = arg_content
        self.parent_fctrl = arg_ctrl_item.parent_fctrl
        self.fctrl_item = arg_ctrl_item.fctrl_item
        self.item_group = arg_ctrl_item.group
        self.process_queue = None

    def run(self):
        my_sum_qitem = None
        try:
            my_launcher = self.create_launcher()
            my_launcher.launch()
            my_sum_qitem = SummaryQueueItem(my_launcher.extract_results())
        except Exception as arg_ex:
            Msg.error_trace(str(arg_ex))
            Msg.err("Message: %s, Control File Path: %s" % (str(arg_ex), PathUtils.current_dir()))
            my_sum_qitem = SummaryErrorQueueItem(
                {
                    "error": arg_ex,
                    "message": "Error Processing Task ...",
                    "path": self.ctrl_item.file_path(),
                    "type": str(type(arg_ex)),
                }
            )
        finally:
            return my_sum_qitem

    # Basically a pseudo launcher factory
    def create_launcher(self):
        my_launcher = None
        if self.process_queue.use_lsf():
            my_launcher = LsfLauncher()
            my_launcher.shell_log = "%s.lsf" % str(self.process_queue.processor_name)
            my_launcher.lsf_log = "lsf.%P"
        else:
            my_launcher = StdLauncher()

        my_launcher.frun_path = self.frun_path
        my_launcher.ctrl_item = self.ctrl_item
        my_launcher.content = self.content
        my_launcher.parent_fctrl = self.parent_fctrl
        my_launcher.fctrl_item = self.fctrl_item
        my_launcher.item_group = self.item_group

        my_launcher.frun_dir = self.frun_path.replace("_def_frun.py", "")
        my_launcher.process_log = "%s.log" % str(self.process_queue.processor_name)
        my_launcher.process_cmd = self.process_queue.process_cmd

        return my_launcher


class ProcessQueue(object):
    def __init__(self):
        self.process_cmd = None
        self.processor_name = None
        self.summary = None
        self.process_max = None
        self.launcher_type = None
        self.executor = None
        self.futures = []

        self._done_event = None

    @property
    def done_event(self):
        return self._done_event

    @done_event.setter
    def done_event(self, arg_done_event):
        arg_done_event.queue = self
        self._done_event = arg_done_event

    def enqueue(self, arg_item):
        arg_item.process_queue = self
        my_future = self.executor.submit(ProcessQueue.execute_item, arg_item, self.summary.queue)
        self.futures.append(my_future)
        return True

    def open_queue(self):
        self.executor = concurrent.futures.ThreadPoolExecutor(max_workers=self.process_max)

    def wait_for_completion(self):
        # self.executor.shutdown()
        for my_future in self.futures:
            my_future.result()

    def use_lsf(self):
        return str(self.launcher_type).strip() == LauncherType.Lsf

    @staticmethod
    def execute_item(arg_item, arg_summary_queue):
        summary_item = arg_item.run()
        arg_summary_queue.enqueue(summary_item)
