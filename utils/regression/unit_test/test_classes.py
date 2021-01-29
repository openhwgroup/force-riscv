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
# test_classes.py
# creates a library of common classes used in the unit test run

from classes.module_run import ModuleRun
from common.path_utils import PathUtils
from common.sys_utils import SysUtils
from common.msg_utils import Msg

# =============================================================================
# == HiThread Test Classes ====================================================
# =============================================================================

from common.threads import HiThread

class NoLoopThread( HiThread ):

    def execute( self ):
        Msg.info( "Begin: NoLoopThread::execute()" )
        for i in range( 5 ): Msg.info( "Thread Work: %d" % ( i ))
        Msg.info( "End: NoLoopThread::execute()" )

class LoopThread( HiThread ):

    def __init__( self, arg_options ):
        super().__init__( arg_options )
        self.max_loops = 15

    def execute( self ):
        Msg.info( "LoopThread::execute() => Enter " )
        for i in range( 3 ): Msg.info( "Thread Work: %d:%d" % ( self.max_loops, i + 1 ))
        self.check_finished()
        self.heartbeat()
        Msg.info( "LoopThread::execute() => Leave" )

    def check_finished( self ):
        # Msg.info( "LoopThread::check_finished( %d )" % ( self.max_loops) )
        self.max_loops -= 1
        if not self.max_loops > 0:
            self.finished = True

def ThreadFactory( argName, argNoLoop, argProcs = {}, argOptions = {}):

    my_thread_opts = {}

    my_thread_opts["noloop"] = argNoLoop  # Does the thread have a processing loop
    my_thread_opts["name"]   = argName    # name the thread

    my_thread_opts["on-start"]    = argProcs.get( "on-start"   , None ) # start thread sequence (outside thread space)
    my_thread_opts["on-execute"]  = argProcs.get( "on-execute" , None ) # thread termination handler (inside thread space)
    my_thread_opts["on-finished"] = argProcs.get( "on-finished", None ) # thread before finished handler (inside thread space)
    my_thread_opts["on-done"]     = argProcs.get( "on-done"    , None ) # thread terminated handler (outside thread space)

    if argNoLoop:
        return NoLoopThread( my_thread_opts )

    # loop threads require additional information
    my_thread_opts ["heartbeat-rate"] = argOptions.get( "heartbeat-rate" , None )
    my_thread_opts ["sleep-period"]   = argOptions.get( "sleep-period"   , None )
    my_thread_opts ["daemon"]         = argOptions.get( "daemon"         , None )   # run once threads should not be daemons
    my_thread_opts ["active"]         = argOptions.get( "active"         , None )   # do not start the thread until ready
    return  LoopThread( my_thread_opts )

