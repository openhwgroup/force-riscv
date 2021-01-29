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
# mutex_tests.py

from unit_test import UnitTest
from common.kernel_objs import HiMutex

from common.path_utils import PathUtils
from common.sys_utils import SysUtils
from common.msg_utils import Msg

from common.errors import *
from test_classes import *

# need two threads to test mutex.

class MutexWrapper( object ):

    def run_test( self ):

        Msg.info( "HiThread(NoLoop): Start Unit Test ... " )

        myMutex = HiMutex( "wrapper_mutex" )

        myThreadProcs = { "on-start"   : self.thread_start          # start thread sequence (outside thread space)
                        , "on-execute" : self.thread_execute        # thread termination handler (inside thread space)
                        , "on-finished": self.thread_finished           # thread before finished handler (inside thread space)
                        , "on-done"    : self.thread_done           # thread terminated handler (outside thread space)
                        }

        Msg.info( "UnitTest_NoLoopThread >> Creating NoLoop Thread ... " )
        myThread = ThreadFactory( "NoLoopThread", True, myThreadProcs )

        Msg.info( "UnitTest_NoLoopThread >> Initializing Thread ... " )
        myThread.start_thread()




        # wait for thread to terminate
        myThread.wait_for()
        Msg.info( "UnitTest_NoLoopThread >> Thread Completed ... " )


    def process_result( self ):
        Msg.info( "HiThread(NoLoop): Process Test Results... " )


    def thread_start( self ):
        Msg.info( "UnitTest_NoLoopThread >> Started .... " )


    def thread_execute( self ):
        Msg.info( "UnitTest_NoLoopThread << Executing .... " )

    def thread_done ( self ):
        Msg.info( "UnitTest_NoLoopThread << Execute Done .... " )

    def thread_finished( self ):
        Msg.info( "UnitTest_NoLoopThread >> Exiting .... " )






class UnitTest_HiMutex( UnitTest ):

    def run_test( self ):
        Msg.info( "HiMutex: Start Unit Test ..." )

        # test with construct!
        self.with_test()


    def process_result( self ):
        Msg.info( "HiMutex: Process Test Result ..." )


    def with_test( self ):

        # create a mutex
        my_mutex = HiMutex( "with_mutex" )
        # Msg.info( "Before With: Mutex[%s] is %s" % ( my_mutex.name, SysUtils.ifthen( my_mutex.locked(), "Locked", "Unlocked" )))
        Msg.info( "Before With: Mutex[%s]" % ( my_mutex.name ))

        with my_mutex :
            Msg.info( "In With: Mutex[%s]" % ( my_mutex.name ))
            # Msg.info( "In With: Mutex[%s] is %s" % ( my_mutex.name, SysUtils.ifthen( my_mutex.locked(), "Locked", "Unlocked" )))

        Msg.info( "After With: Mutex[%s]" % ( my_mutex.name ))
        # Msg.info( "After With: Mutex[%s] is %s" % ( my_mutex.name, SysUtils.ifthen( my_mutex.locked(), "Locked", "Unlocked" )))

