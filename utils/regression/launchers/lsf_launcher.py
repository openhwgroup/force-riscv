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
## LSF Launcher
##################################################################################
# file: lsf_launcher                                                             #
# summary: This implements utility function to build LSF commands                #
#          and submit to the Queue                                               #
# Note: i) By default LSF command constructed with default setting               #
#          but command options like Group, Queue can be provided                 #
#      ii) There is no LSF support in Green Zone, command that is built is not   #
#          LSF command                                                           #
##################################################################################

from common.sys_utils import SysUtils
from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.datetime_utils import DateTime
from common.errors import *
from common.kernel_objs import *

from classes.launcher  import Launcher

class LsfDefaults(object):
    CoreFileSize = 1

class LsfLauncher( Launcher ):

    def __init__( self, aSequenceRunner, aLsfInfo ):
        super().__init__(aSequenceRunner)

        self.group = aLsfInfo.parameter('group')
        self.sla = aLsfInfo.parameter('sla')
        self.queue = aLsfInfo.parameter('queue')
        self.core_file_size = LsfDefaults.CoreFileSize
        self.red_zone = bool( not SysUtils.check_host( "SAN" ))
        self.shell_log = "%s.lsf" % str( aSequenceRunner )
        self.lsf_log   = "lsf.%J"

    def build( self ):

        # First get the raw process command
        my_process_cmd, my_process_log = super().build()

        # check to see if operating in the red zone
        if self.red_zone:
            # Red Zone with LSF Enabled requires some extra work
            my_lsf_log = PathUtils.include_trailing_path_delimiter( self.frun_dir) + self.lsf_log
            Msg.user( "lsf.log: %s" % ( my_lsf_log ))

            # first build the bsub command line
            my_bsub = "bsub -K"

            # Apply SLA if applicable
            if len(self.sla):
                my_bsub += " -sla %s" % self.sla

            my_bsub += " -q %s -G %s -C %d -o %%s \"%%s\"" % ( self.queue, self.group, self.core_file_size )
            
            Msg.user( "BSub Command Line: %s" % ( str( my_bsub )), "LSF-LAUNCHER" )

            # next append the processor log file as path of the processor command
            my_cmd = "%s --logfile %s" % ( my_process_cmd, my_process_log )

            my_process_cmd = my_bsub % ( my_lsf_log, my_cmd )
            self.process_log =  my_process_log
            my_process_log = PathUtils.include_trailing_path_delimiter( self.frun_dir) + self.shell_log

        Msg.user( "Process Log: %s" % ( str( my_process_log )), "LSF-LAUNCHER" )
        Msg.user( "Process Command Line: %s" % ( str( my_process_cmd )), "LSF-LAUNCHER" )

        return my_process_cmd, my_process_log


    def launch( self ):

        self.process_cmd, my_process_log = self.build()
        my_timeout = self.timeout # cif not self.red_zone else None
        self.process_result = SysUtils.exec_process( self.process_cmd, my_process_log, my_process_log, my_timeout )
        Msg.user( "Process Results: %s" % ( str( self.process_result )), "LSF-LAUNCHER" )

        # insert the message:


    def terminate( self ):

        # if self.process is not None:
        #     self.process.kill()

        pass


    def validate( self ):
        pass

    def status(self):
        pass

