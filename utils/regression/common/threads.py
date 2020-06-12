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

import threading
import sys


import threading
import sys

from threading import Thread
from common.sys_utils import SysUtils
from common.msg_utils import Msg

class HiThread( threading.Thread ):

    def __init__( self, arg_options ):

        # the HiThread has two states state 1 has no thread loop thus there is no need for control to be halted
        # to allow other thread processes to continue, State 2 executes a loop until the shutdown sequence is invoked
        # which determines whether or not the thread a) has more work to do, or b) will wait for another process to
        # finished. A Thread will remain in the shutdown sequence until the terminate flag is set. When the terminate
        # flag is set then the thread will execute the terminate sequence.
        my_target = SysUtils.ifthen( arg_options.get( "noloop", False ), self.run_once, self.run_loop )
        super().__init__( name = arg_options.get( "name", None ), target = my_target )
        Msg.dbg( "Created Thread[%s] Processing...." % ( self.name ))

        # useful Thread events
        self.on_start    = arg_options.get( "on-start"   , None )
        self.on_execute  = arg_options.get( "on-execute" , None )
        self.on_done     = arg_options.get( "on-done"    , None )
        self.on_shutdown = arg_options.get( "on-shutdown", None )
        self.on_finished = arg_options.get( "on-finished", None )

        # thread flags
        self.shutdown = False  # shutdown request has been received
        self.finished = False  # thread has completed all work and has exited

        # the thread heartbeat is set to write a debug message every 30 seconds if the thread
        # is waiting. I do not like the term blocking because waiting is a more controlled event.
        # The waiting thread asks for permission to procees rather than the thread that is busy having
        # to create a block. Blocks are notorious for causing deadlock requiring less than a graceful
        # shutdown
        self.heartbeat_rate = arg_options.get( "heartbeat-rate", 30 )
        self.sleep_period   = arg_options.get( "sleep-period"  , 1000 )
        self.current_tick = 0

        # not a good idea to use daemons as a blanket rule, makes bad designs seem to function
        # but there are always underlying issues with these. The daemon attribute exists in the
        # abstract as does the handler. Setting it here will ensure the thread acts as we expect
        self.daemon = arg_options.get( "daemon", False )
        if arg_options.get( "active", False ):
            self.start_thread()

    # perform any remaining initialization outside the thread space
    # using a callback handler if initialized then starts the thread
    def start_thread( self ):
        if self.on_start is not None:
            self.on_start( )
        self.start()

    # waits for the thread to exit then executes a notify callback if initialized
    def wait_for( self ):

        Msg.dbg( "Before Thread[%s] Join" % ( self.name ))
        self.join()
        Msg.dbg( "After Thread[%s] Join" % ( self.name ))

        # thread has finishedd its work, trigger notify thread done
        if self.on_finished is not None:
            self.on_finished( )

    # general thread loop
    def run_loop( self ):

        Msg.info( "Entering HIThread[%s] Loop Processing...." % ( self.name )  )
        while not self.terminated():
            # initialize iteration for work performed in execute
            Msg.dbg( "HiThread[%s]::run_loop(1)" % ( self.name )  )
            if self.on_execute is not None:
                Msg.dbg( "HiThread[%s]::run_loop(2)" % ( self.name )  )
                self.on_execute( )
            # perform the iteration work
            Msg.dbg( "HiThread[%s]::run_loop(3)" % ( self.name )  )
            self.execute()
            # finish work prior the next iteratation
            Msg.dbg( "HiThread[%s]::run_loop(4)" % ( self.name )  )
            if self.on_done is not None:
                Msg.dbg( "HiThread[%s]::run_loop(5)" % ( self.name )  )
                self.on_done( )
            Msg.dbg( "HiThread[%s]::run_loop(6)"  % ( self.name ) )

        Msg.info( "Leaving HIThread[%s] Loop Processing...." % ( self.name )  )


    # general thread execute
    def run_once( self ):

        Msg.dbg( "Entering Thread[%s] Processing...." % ( self.name ))
        # initialize thread for work performed in execute
        if self.on_execute is not None:
           self.on_execute( )
        # perform the thread work
        self.execute()
        # perform any remaining work prior to exit thread space
        if self.on_done is not None:
            self.on_done( )
        Msg.dbg( "Leaving Thread[%s] Processing...." % ( self.name ))


    # returns True once Thread has exited
    def terminated( self ):
        # Msg.info( "HiThread::terminated() - self.finished: %s, self.is_alive(): %s, returns: [%s]" % (str(self.finished),str(self.is_alive())),str(self.finished or ( not self.is_alive() )))
        Msg.info( "HiThread[%s]::terminated() - self.finished: %s, self.is_alive(): %s" % (self.name, str(self.finished),str(self.is_alive())) )
        my_retval = self.finished or ( not self.is_alive())
        Msg.info( "HiThread[%s]::terminated() - returns: [%s]"  % ( self.name, str( my_retval )))

        return self.finished or ( not self.is_alive() )


    def heartbeat( self ):

        # increment the heartbeat and when debug messages are enabled then a heartbeat message will be
        # posted every self.heartbeat-interval ticks. Whenever the heartbeat method is called the current_tick
        # is updated
        self.current_tick += 1

        if not bool( self.current_tick % self.heartbeat_rate ):
            Msg.dbg( "HiThread[%s] Still Working" % ( self.name ) )

        # the sleep in SysUtils uses milliseconds as does the rest of the computing world instead of
        # fractions of a second. Thus this will pause this thread for 10 seconds allowing the process
        # thread some processor time
        SysUtils.sleep(self.sleep_period)
        return False

    def trigger_shutdown( self ):

        Msg.dbg( "HiThread[%s]::trigger_shutdown() - enter " % ( self.name ))
        if self.on_shutdown is not None:
            self.on_shutdown(self)
        else:
            self.shutdown = True
        Msg.dbg( "HiThread[%s]::trigger_shutdown() - exit " % ( self.name ))

    def execute( self ):
        raise NotImplementedError( "Thread::execute() must be implemented" )



class HiOldThread( threading.Thread ):
    def __init__( self, arg_create_active = False ):
        super().__init__( name = "HiThread-01" )
        # Ensure that all threads are killed when master thread is exited for any reason by marking it as a daemon
        self.daemon = True
        if arg_create_active:
            self.start()
        #pass

    def run( self ):
        pass

    def HeartBeat( self ):
        # Enable this if it's a good idea to have a periodic printing heartbeat
        #Msg.dbg("[Thread %s]: Heartbeat" % (self.threadName))
        pass


class HiEvent( threading.Event ):
    def __init__( self, arg_options ):
        super().__init__()
        # default to return immediately
        self.timeout         = arg_options.get( "timeout"      , None )
        self.before_signal   = arg_options.get( "before-sig"   , None )  # use this to perform some action prior to setting event
        self.after_signal    = arg_options.get( "after-sig"    , None )  # use this to perform some action after to setting event
        self.before_unsignal = arg_options.get( "before-unsig" , None )  # use this to perform some action prior to unsetting event
        self.after_unsignal  = arg_options.get( "after-unsig"  , None )  # use this to perform some action after to unsetting event

    def Signal( self, arg_sender = None ):

        # perform any activities prior to notification, this could include finishing some work
        # that could make the system unstable. This is a callback that is part of the dictionary
        # used to initialize
        if self.before_signal is not None:
           self.before_signal( arg_stat )

        # signal the event
        self.set()

        # perform any activities once notification has been dispatched, this can be used to notify the caller the even has
        # been signaled
        # This is a callback that is part of the dictionary used to initialize
        if self.after_signal is not None:
           self.after_signal( arg_stat )

    def Unsignal( self,  arg_sender = None ):

        # perform any activities prior to notification the event has been cleared and will block, this could include initializing to
        # prevent system instability. This is a callback that is part of the dictionary used to initialize the Event
        if self.before_unsignal is not None:
           self.before_unsignal( self )

        self.clear( )

        if self.after_unsignal is not None:
           self.after_unsignal( self )

    def Signaled( self,  arg_sender = None ):
        return self.isSet()

    def Reset( self,  arg_sender = None ):
        self.clear()


##   {{{{   TTTTTTTT  OOOOOO   DDDDDD     OOOOOO  }}}}
##  {{      TTTTTTTT OOOOOOOO  DDDDDDD   OOOOOOOO    }}
##  {{         TT    OO    OO  DD   DDD  OO    OO    }}
## {{          TT    OO    OO  DD    DD  OO    OO     }}
##  {{         TT    OO    OO  DD   DDD  OO    OO    }}
##  {{         TT    OOOOOOOO  DDDDDDD   OOOOOOOO    }}
##   {{{{      TT     OOOOOO   DDDDDD     OOOOOO  }}}}
##
##  GET RID OF THIS INSANITY, replace with proper thread management

# This event is signalled when all the worker threads are completed.
workers_done_event = HiEvent({})

# Summary thread signals master thread that it's done
summary_done_event = HiEvent({})

class HiSemaphore( threading.Semaphore ):

    def test( self ):
        my_lock = threading.Lock()


# class HiMutex( threading.Lock ):
#     def test( self ):
#         pass
#
#
# class HiCriticalSection( threading.Lock ):
#     pass
#
#

