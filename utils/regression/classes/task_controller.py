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
# file: task_controller.py
# summary: Processes a control item that is a control task. If a wild card is
#          used then the task is expanded to include all template files that
#          match the mask of the wild card. This also produces a special
#          control file that is processed by a spawned process that defaults
#          to forrest_run. Once that file is generated a process queue item
#          is created and submitted to the process queue for processing until
#          all files that have been part of the task expansion have been
#          processed.
#
#

# import seedgen.bin.seedgen as sg

from classes.controller import Controller
from classes.process_queue import ProcessQueueItem
from common.msg_utils import Msg
from common.path_utils import PathUtils


class TaskController(Controller):
    # needs to retain value across instances
    iss_ok = None

    def __init__(self, aProcessQueue, aAppsInfo):
        super().__init__(aAppsInfo)
        self.mProcessQueue = aProcessQueue
        # Msg.dbg( "TaskController::__init__( ... )" )

    def load(self, arg_ctrl_item):
        super().load(arg_ctrl_item)
        # Msg.dbg( "TaskController::load()")

        self.task_list = PathUtils.list_files(
            PathUtils.append_path(
                self.ctrl_item.fctrl_dir, self.ctrl_item.fctrl_name
            )
        )
        return True

    def process(self):

        for my_ndx in range(0, self.ctrl_item.iterations):

            # shutdown has been triggered do not do the next iteration
            if self.is_terminated():
                return

            try:
                self.process_task_list()

            except Exception as arg_ex:
                Msg.error_trace()
                Msg.err(str(arg_ex))
                Msg.blank()
                # self.do_fail()
            finally:
                pass

    def process_task_list(self):
        for my_task_file in self.task_list:

            # shutdown has been triggered stop processing tasks
            if self.is_terminated():
                return

            my_curdir = PathUtils.current_dir()
            Msg.user("Task list current dir %s" % my_curdir)

            try:
                # self.process_task_file( my_task_file )
                self.process_task_file(my_task_file, my_curdir)
            except Exception as arg_ex:
                Msg.error_trace()
                Msg.err(str(arg_ex))
                Msg.blank()
                # self.do_fail()
            finally:
                pass

    def process_task_file(self, arg_task_file, aParentDir):
        try:
            # NOTE: a task file can be but is not limited to being an test
            # template file set base task directory and extract the task id and
            # update directory
            my_task_name = PathUtils.base_name(arg_task_file)
            my_task_dir = my_task_name.replace(".py", "")

            # Msg.info("suffix: %s" % ( str( self.ctrl_item.suffix )))
            if self.ctrl_item.suffix is not None:
                my_task_dir = my_task_dir.replace(
                    "_force", "_%s_force" % (str(self.ctrl_item.suffix))
                )

            full_task_dir = PathUtils.append_path(
                PathUtils.include_trailing_path_delimiter(aParentDir),
                my_task_dir,
            )

            # check for exiting sub-directories, and extract the task iteration
            # count to prevent unintended modifications to the original passed
            # on the commafrom common.path_utils import PathUtilsnd line for
            # this task or acquired from a control file item

            self.process_task(arg_task_file, full_task_dir)

        finally:
            pass

    def process_task(self, arg_task_file, aTaskDir):
        try:
            # get the subdirectory index
            my_ndx = self.mAppsInfo.getNextIndex(aTaskDir)

            # form sub task directory
            sub_task_dir = PathUtils.append_path(
                PathUtils.include_trailing_path_delimiter(aTaskDir),
                "%05d" % my_ndx,
            )

            # save the task template file name with path to the control item
            self.ctrl_item.fname = arg_task_file
            # prepare control item content, TODO don't really need it.
            my_content = self.ctrl_item.prepare(self.mAppsInfo, arg_task_file)
            my_queue_item = ProcessQueueItem(
                sub_task_dir, self.ctrl_item, self.mAppsInfo, my_content
            )
            self.mProcessQueue.enqueue(my_queue_item)

        except Exception as arg_ex:
            Msg.error_trace()
            Msg.err(str(arg_ex))
            # reraise to prevent adding to summary instance
            raise

        finally:
            pass
        # end try except finally
