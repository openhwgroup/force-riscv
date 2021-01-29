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
##################################################################################
# file: launcher                                                                 #
# summary: Base process launcher class                                           #
##################################################################################

from common.sys_utils import SysUtils
from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.datetime_utils import DateTime
from common.errors import *

from classes.summary import SummaryQueueItem

class LauncherType( object ):
    Lsf = "lsf"
    Local = "local"

class Launcher( object ):

    def __init__( self, aSequenceRunner ):

        self.frun_path   = None   # the path to the frun control file
        self.frun_dir    = None   # the actual output directory
        self.timeout     = 0      # timeout passed from the control item

        self.process_result = None   # the result of the process
        self.process_cmd    = None   # full path of command without decorations
        self.process_log    = None   # the path to the log that will be produced
                                     # by process client to be consumed by summary

        self.process_id     = None   # Id of the process when launched
        self.process_log = "%s.log" % str( aSequenceRunner)

        # self.options   = {}          # options data

    def setLaunchParameters( self, aFrunPath, aSeqRunnerCommand, aTimeout):
        self.frun_path = aFrunPath
        self.frun_dir  = self.frun_path.replace( "_def_frun.py", "" )
        self.process_cmd = aSeqRunnerCommand
        self.timeout = aTimeout

        # Msg.user( "F-Run Control File: %s " % (str( self.frun_path )), "WORK-THREAD")
        # Msg.user( "F-Run Directory: %s" % ( str(self.frun_dir )), "WORK-THREAD")
        # Msg.user( "Processor Log: %s" % ( str(self.process_log )), "WORK-THREAD")
        # Msg.user( "Processor Command: %s" % ( str(self.process_cmd )), "WORK-THREAD")
        
    # populate launcher with initial values
    # def populate( self, arg_options ):
    #     self.options = None           # should prevent duplicates
    #     self.options = arg_options

    # build the command line launcher with initial values
    def build( self ):
        my_cmd = self.process_cmd % ( self.frun_path )
        Msg.user( "Process Command: %s" % ( str( my_cmd )), "LAUNCHER" )
        my_log = PathUtils.include_trailing_path_delimiter( self.frun_dir) + self.process_log
        return my_cmd, my_log

    def launch():
        raise AbstractionError( "Abstract Method Error: Launcher::launch() not implemented in descendent [%s]" % ( str( type( self ))))

    def validate():
        raise AbstractionError( "Abstract Method Error: Launcher::validate() not implemented in descendent [%s]" % ( str( type( self ))))

    def terminate():
        raise AbstractionError( "Abstract Method Error: Launcher::terminate() not implemented in descendent [%s]" % ( str( type( self ))))

    def extract_results( self ):
        my_result = { "process-cmd"   : self.process_cmd
                    , "process-log"   : self.process_log
                    , "frun-path"     : self.frun_path
                    , "parent-fctrl"  : self.parent_fctrl
                    , "fctrl-item"    : self.fctrl_item
                    , "item-group"    : self.item_group
                    , "content"       : self.content
                    , "process-result": self.process_result
                    }

        # Msg.lout( my_result, "user", "Process Result" )
        return my_result

    def do_on_launch( self, arg_process_id ):
        self.process_id = arg_process_id

