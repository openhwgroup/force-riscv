#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Threading
################################################################################
# file: process_queue                                                          #
# summary: Implemennts a Thread Safe Queue that us used as a marshall for      #
#          the ExecuteProcess workers                                          #
#                                                                              #
################################################################################

from shared.collections import HiThreadedProducerConsumerQueue, HiAtomicInteger
from shared.msg_utils import Msg
from shared.sys_utils import SysUtils
from shared.path_utils import PathUtils
from shared.summary_core import SummaryQueueItem
import threading
from shared.threads import HiThread, HiSemaphore, HiEvent
from shared.datetime_utils import DateTime

class ProcessAction():

    WriteOnly=0   # write control files but do not execute
    Immediate=1   # run as soon as there are items in the process queue
    Delay=2       # delay run until all components have been pre processed
    NoWrite=3     # execute without write to control file

    @classmethod
    def translate( arg_class, arg_str ):
        if arg_str == "write-only": return ProcessAction.WriteOnly
        if arg_str == "immediate" : return ProcessAction.Immediate
        if arg_str == "delay"     : return ProcessAction.Delay
        if arg_str == "no-write"  : return ProcessAction.NoWrite
        raise Execption( "Unable to translate string value: %s, to ProcessAction" % ( arg_str ))

    @classmethod
    def asstr( arg_class, arg_val ):
        if arg_val == ProcessAction.WriteOnly : return "write-only"
        if arg_val == ProcessAction.Immediate : return "immediate"
        if arg_val == ProcessAction.Delay     : return "delay"
        if arg_val == ProcessAction.NoWrite   : return "no-write"
        raise Execption( "Unable to translate value to string  %s, to ProcessAction" % (str( arg_val ) ))

class ProcessWorkerThread( HiThread ):
    def __init__( self, arg_process_queue, arg_summary, done_semaphore, peer_count, arg_queue_item ):
        self.done_semaphore = done_semaphore
        self.queue_item = arg_queue_item
        self.summary = arg_summary
        self.thread_count = peer_count
        self.process_queue = arg_process_queue
        # We do not want the thread to launch until we've loaded all the properties
        super().__init__(True)

    def extract_results( self, arg_result, arg_cmd, arg_log, arg_process_item, arg_start_time, arg_end_time ):
        Msg.dbg( "Return Code %s, Command Line: |%s|, Log File: %s" % ( str( arg_result ), arg_cmd, arg_log ))

        #Msg.fout( arg_log, "info" )
        my_summary_item = SummaryQueueItem( { "process-cmd"  : arg_cmd
                                            , "process-log"   : arg_log
                                            , "process-result": arg_result
                                            , "frun-path"     : arg_process_item.frun_path
                                            , "parent-fctrl"  : arg_process_item.parent_fctrl
                                            , "fctrl-item"    : arg_process_item.fctrl_item
                                            , "item-group"    : arg_process_item.item_group
                                            , "content"       : arg_process_item.content
                                            , "start-time"    : arg_start_time
                                            , "end-time"      : arg_end_time
                                            } )
        while (self.summary.queue.enqueue( my_summary_item ) == False):
            self.HeartBeat()
            pass

    def run( self ):
        # Record the starting and ending of the run for performance metrics
        start = DateTime.Time()

        my_cmd = self.process_queue.process_cmd % ( self.queue_item.frun_path )
        # Msg.dbg( "Process Command: %s" % ( str( my_cmd )))
        my_log = "%s/forrest.log" % (self.queue_item.work_dir)
        # my_retcode = SysUtils.exec_process( my_cmd, my_log, my_log, self.my_queue_item.ctrl_item.timeout )

        # Msg.dbg( "Process Timeout: %s" % ( str( self.queue_item.ctrl_item.timeout )))  # self.queue_item.ctrl_item.timeout )))
        my_retcode = SysUtils.exec_process( my_cmd, my_log, my_log, self.queue_item.ctrl_item.timeout )

        end = DateTime.Time()

        self.extract_results( my_retcode, my_cmd, my_log, self.queue_item, start, end )
        self.done_semaphore.release()
        self.thread_count.delta(-1)

class ProcessThread( HiThread ):
    def __init__( self, arg_process_queue, arg_summary, max_process_count, arg_done_signal):
        self.max_process_count = max_process_count
        self.semaphore = threading.BoundedSemaphore(max_process_count)
        self.summary = arg_summary
        self.thread_count = HiAtomicInteger(0)
        self.process_queue = arg_process_queue
        self.done_signal = arg_done_signal
        # We do not want the thread to launch until we've loaded all the properties
        super().__init__(True)

    def run( self ):
        while ((not self.process_queue.fully_loaded) or (self.process_queue.fully_loaded and (self.thread_count.value() > 0 or self.process_queue.size() > 0))) :
            try:
                # Pop off the top of the process queue (should block if the queue is empty)
                my_queue_item = self.process_queue.dequeue(1)
                # Launch a thread with the item. Sempahore will block if we've reached max workers
                self.semaphore.acquire(True)
                my_work_thread = ProcessWorkerThread(self.process_queue, self.summary, self.semaphore, self.thread_count, my_queue_item)
                self.thread_count.delta(1)
            except TimeoutError:
                pass
            self.HeartBeat()
        self.done_signal.Signal()

class ProcessQueueItem( object ):
    def __init__( self, arg_frun_path, arg_ctrl_item, arg_content  ): #  parent_fctrl, arg_fctrl_item, arg_item_group ):
    # def __init__( self, arg_frun_path, arg_parent_fctrl, arg_fctrl_item, arg_item_group, arg_content ):
        super().__init__()
        self.frun_path    = arg_frun_path
        self.work_dir, my_tmp = PathUtils.split_path( arg_frun_path )
        self.ctrl_item = arg_ctrl_item

        self.parent_fctrl = arg_ctrl_item.parent_fctrl
        self.fctrl_item   = arg_ctrl_item.fctrl_item
        self.item_group   = arg_ctrl_item.group
        # self.frun_str     = arg_frun_str
        self.content      = arg_content

    # def __str__( self ):
    #     return "{\"\":%s, \"\":%s, \"\":%s, \"\":%s" % ( self.frun_path, self.parent_fctrl, self.fctrl_item, self.item_group )


class ProcessQueue( HiThreadedProducerConsumerQueue ):

    def __init__( self, arg_process_cmd, arg_summary, arg_max_process_count, arg_done_signal  ):
        super().__init__(blocking = True)
        self.process_cmd = arg_process_cmd
        self.summary = arg_summary
        # Msg.dbg( "F-Run Command Line: %s" % (self.process_cmd ))
        self.threadList = list()
        self.fully_loaded = False
        # Create the Process Thread
        self.mutex = threading.Lock
        self.process_thread = ProcessThread(self, self.summary, arg_max_process_count, arg_done_signal)
