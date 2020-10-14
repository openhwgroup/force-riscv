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
################################################################################
# file: process_queue                                                          #
# summary: Implemennts a Thread Safe Queue that us used as a marshall for      #
#          the ExecuteProcess workers                                          #
#                                                                              #
################################################################################

from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.datetime_utils import DateTime

from common.collections import HiThreadList, HiThreadedProducerConsumerQueue, HiAtomicInteger
from common.threads import HiOldThread, HiSemaphore, HiEvent, HiThread
from common.kernel_objs import HiCriticalSection, HiMutex

from classes.launcher import Launcher, LauncherType
from classes.summary import SummaryQueueItem, SummaryErrorQueueItem

from launchers.lsf_launcher import LsfLauncher
from launchers.std_launcher import StdLauncher

import threading

class ProcessAction():

    WriteOnly=0   # write control files but do not execute
    Immediate=1   # run as soon as there are items in the process queue
    Delay=2       # delay run until all components have been pre processed
    NoWrite=3     # execute without write to control file

    @classmethod
    def translate( cls, arg_str ):
        if arg_str == "write-only": return ProcessAction.WriteOnly
        if arg_str == "immediate" : return ProcessAction.Immediate
        if arg_str == "delay"     : return ProcessAction.Delay
        if arg_str == "no-write"  : return ProcessAction.NoWrite
        raise Execption( "Unable to translate string value: %s, to ProcessAction" % ( arg_str ))

    @classmethod
    def asstr( cls, arg_val ):
        if arg_val == ProcessAction.WriteOnly : return "write-only"
        if arg_val == ProcessAction.Immediate : return "immediate"
        if arg_val == ProcessAction.Delay     : return "delay"
        if arg_val == ProcessAction.NoWrite   : return "no-write"
        raise Execption( "Unable to translate value to string  %s, to ProcessAction" % (str( arg_val ) ))


class ProcessWorkerThread( HiOldThread ):
    def __init__( self, arg_summary, done_semaphore, peer_count, arg_queue_item, aProcessorName, aProcessCmd, aUseLsf):
        #acquire the semaphore here
        done_semaphore.acquire(True)
        #once acquired, increment the active peer thread count
        peer_count.add(1)

        self.done_semaphore = done_semaphore
        self.queue_item     = arg_queue_item
        self.thread_count   = peer_count
        self.mProcessorName = aProcessorName
        self.mProcessCmd = aProcessCmd
        self.mUseLsf = aUseLsf
        self.summary = arg_summary
        # We do not want the thread to launch until we've loaded all the properties
        Msg.user( "Thread Id: %s __init__" % ( str( id( self ))), "WORK-THREAD" )

        super().__init__(True)
        self.on_done = None
        self._launcher = None

    def __del__(self):
        Msg.user( "Thread Id: %s __del__" % ( str( id( self ))), "WORK-THREAD" )

    def setupWorkDir( self ):
        work_dir = self.queue_item.work_dir
        PathUtils.mkdir( work_dir )
        # write out the control file
        PathUtils.touch( "%sSTARTED" % PathUtils.include_trailing_path_delimiter( work_dir ))
        if not PathUtils.write_file( self.queue_item.frun_path, self.queue_item.content, "Frun Control" ):
            raise Exception("Failed to write frun file: %s" % self.queue_item.frun_path)

    def run( self ):

        # Msg.user( "Worker Thread Instance Id: %s, (1)" % ( str( id( self ))) , "WORK-THREAD" )
        my_sum_qitem = None
        try:
            self.setupWorkDir()

            my_launcher = self.create_launcher()
            Msg.user( "Launcher Id 1: %s" % ( str( id( my_launcher  ))), "WORK-THREAD" )
            # Msg.user( "Worker Thread Instance Id: %s, (2)" % ( str( id( self ))) , "WORK-THREAD" )
            my_launcher.launch()
            Msg.user( "Launcher Id 2: %s" % ( str( id( my_launcher ))), "WORK-THREAD" )
            # Msg.user( "Worker Thread Instance Id: %s, (3)" % ( str( id( self ))) , "WORK-THREAD" )
            my_process_result = my_launcher.extract_results()
            Msg.user( "Process Result: %s" % ( my_process_result ), "WORK-THREAD" )
            Msg.user( "Launcher Id 3: %s" % ( str( id( my_launcher  ))), "WORK-THREAD" )
            #
            self.launcher = my_launcher

            Msg.user( "Process Result: %s" % ( my_process_result ), "WORK-THREAD" )
            my_sum_qitem  = SummaryQueueItem( my_process_result )
            Msg.user( "Created Summary Queue Item", "WORK-THREAD" )


        except Exception as arg_ex:

            Msg.error_trace( str(arg_ex) )
            Msg.err( "Message: %s, Control File Path: %s" % ( str( arg_ex ), self.queue_item.work_dir ))
            my_sum_qitem = SummaryErrorQueueItem( { "error"  : arg_ex
                                                  , "message": "Error Processing Task ..."
                                                  , "path"   : self.queue_item.ctrl_item.file_path()
                                                  , "type"   : str(type( arg_ex ))
                                                  } )
        finally:

            # my_launcher = None
            my_attempt = 0
            while ( self.summary.queue.enqueue( my_sum_qitem ) == False):
                SysUtils.sleep( 100 )
                #heartbeat
                my_attempt += 1
                if ( my_attempt % 10 ) == 0:
                    Msg.dbg( "Attempt %d to insert into summary queue" % ( my_attempt ))

            self.thread_count.add(-1)
            Msg.user( "Thread Count Decremented", "WORK-THREAD")

            self.done_semaphore.release()
            Msg.user( "Semaphore Released", "WORK-THREAD")
            Msg.user( "Launcher Id 5: %s" % ( str( id( self.launcher  ))), "WORK-THREAD" )


    # Basically a psudeo launcher factory
    def create_launcher( self ):

        my_launcher = None
        if self.mUseLsf:
            my_launcher = LsfLauncher(self.mProcessorName, self.queue_item.mAppsInfo.mLsfInfo);
        else:
            my_launcher = StdLauncher(self.mProcessorName)

        my_launcher.setLaunchParameters(self.queue_item.frun_path, self.mProcessCmd, self.queue_item.ctrl_item.timeout)

        my_launcher.parent_fctrl = self.queue_item.parent_fctrl
        my_launcher.fctrl_item   = self.queue_item.fctrl_item
        my_launcher.item_group   = self.queue_item.item_group
        my_launcher.content      = self.queue_item.content

        return my_launcher
        # self.launcher = my_launcher

    @property
    def launcher(self):
        Msg.user( "Get Launcher, Returning Id: %s" % ( str( id( self._launcher))), "WORK-THREAD" )
        return self._launcher

    @launcher.setter
    def launcher(self, arg_launcher ):
        self._launcher = arg_launcher
        Msg.user( "Set Launcher, Populated with Id: %s" % ( str( id( self._launcher))), "WORK-THREAD" )


class ProcessMonitorThread( HiThread ):

    def __init__( self ):

        my_thread_opts = {}
        my_thread_opts["noloop"] = False                     # Thread has a processing loop
        my_thread_opts["name"]   = "ProcessMonitorThread"    # name the thread
        my_thread_opts["on-start"]    = do_on_start     # thread initalize sequence     (outside thread space)
        my_thread_opts["on-execute"]  = do_on_execute   # thread before execute handler (inside thread space)
        my_thread_opts["on-done"]     = do_on_done      # thread after execute handler  (inside thread space)
        my_thread_opts["on-finished"] = do_on_finished  # thread done cleanup handler   (outside thread space)
        my_thread_opts["daemon"]      = False           # monitors should not be daemons, EVER!!!!!!
        my_thread_opts["active"]      = False           # Wait to start thread

        self.thread_list = ThreadList()

        super().__init__(my_thread_opts )

    def execute( self ):



        pass

    # Thread Events Unknown if they are needed at this time
    def do_on_start( self, arg_sender = None ):
        pass

    def do_on_execute( self, arg_sender = None ):
        pass

    def do_on_done( self, arg_sender = None ):
        pass

    def do_on_finished( self, arg_sender ):
        pass


class ProcessThread( HiOldThread ):

    def __init__( self, arg_process_queue, arg_summary, max_process_count, arg_done_event):

        self.max_process_count = max_process_count
        self.semaphore = threading.BoundedSemaphore(max_process_count)
        self.summary = arg_summary

        self.thread_count = HiAtomicInteger(0)
        self.process_queue = arg_process_queue
        self.done_event = arg_done_event

        Msg.user( "Done Event: %s" % str( self.done_event ), "PROCESS-THREAD" )

        # We do not want the thread to launch until we've loaded all the properties
        super().__init__(True)


    def run( self ):

        try:
            while ((not self.process_queue.fully_loaded) or (self.process_queue.fully_loaded and (self.thread_count.value() > 0 or self.process_queue.size() > 0))) :
                try:
                    # Msg.user( "(1) Thread Count: %d" % ( self.thread_count.value()), "PROCESS-THREAD")
                    # if something tiggered a shutdown then exit the processing loop and stop processing more items.
                    if self.process_queue.summary.is_terminated():
                        Msg.user( "Terminated", "PROCESS-THREAD")
                        break
                    # Pop off the top of the process queue (should block if the queue is empty)
                    my_queue_item = self.process_queue.dequeue(1)
                    # Launch a thread with the item. Sempahore will block if we've reached max workers
                    Msg.user( "Waiting for Semaphore ....", "PROCESS-THREAD")

                    if my_queue_item.mAppsInfo.mMode == "count":
                        Msg.user("Counting %s" % my_queue_item.work_dir, "PROCESS-THREAD")
                        my_queue_item.mAppsInfo.incrementTestCount()
                    else:
                        my_work_thread = ProcessWorkerThread(self.summary, self.semaphore, self.thread_count, my_queue_item, self.process_queue.processor_name, self.process_queue.process_cmd, self.process_queue.use_lsf() )
                    #Msg.info( "(3) Thread Count: %d" % ( self.thread_count.value()), "PROCESS-THREAD")
                except TimeoutError:
                    pass

                finally:
                    pass

            # need to wait until all worker threads are done before continuing
            while self.thread_count.value() > 0:
                Msg.user( "Thread Count: %d" % ( self.thread_count.value()), "PROCESS-THREAD")
                SysUtils.sleep( 100 )

        finally:
            # Msg.user( "(4) Thread Count: %d" % ( self.thread_count.value()), "PROCESS-THREAD")

            self.done_event.Signal()


class ProcessQueueItem( object ):

    def __init__( self, aWorkDir, arg_ctrl_item, aAppsInfo, arg_content  ):
        super().__init__()
        self.work_dir = aWorkDir
        self.frun_path = PathUtils.append_path(PathUtils.include_trailing_path_delimiter(self.work_dir), "_def_frun.py")
        
        self.ctrl_item = arg_ctrl_item
        self.mAppsInfo = aAppsInfo

        self.parent_fctrl = arg_ctrl_item.parent_fctrl
        self.fctrl_item   = arg_ctrl_item.fctrl_item
        self.item_group   = arg_ctrl_item.group
        self.content      = arg_content


class ProcessQueue( HiThreadedProducerConsumerQueue ):

    def __init__( self ):
        super().__init__(blocking = True)

        self.process_cmd    = None
        self.processor_name = None
        self.summary        = None

        self.fully_loaded  = False
        self.process_max   = None
        self.done_event    = None
        self.launcher_type = None
        self.process_thread = None

    def open_queue( self ):

        # Create the Process Thread and lock ???
        Msg.user( "Done Event: %s" %str( self.done_event ), "PROCESS-QUEUE" )

        self.process_thread = ProcessThread( self, self.summary, self.process_max, self.done_event )
        Msg.user( "F-Run Command Line: %s" % (self.process_cmd ), "PROCESS-QUEUE")


    def use_lsf( self ):
        return str( self.launcher_type ).strip() == LauncherType.Lsf
        


