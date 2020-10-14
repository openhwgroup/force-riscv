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

from shared.controller import Controller
from shared.msg_utils import Msg
from shared.control_item import ControlItem, CtrlItmKeys
from shared.sys_utils import SysUtils
from shared.path_utils import PathUtils

class ExecuteController( Controller ):

    def __init__( self ):
        super().__init__( )
        # Msg.dbg( "ExecuteController::__init__( )" )


    def load(self, arg_ctrl_item ):
        super().load( arg_ctrl_item )
        # Msg.dbg( "ExecuteController::load()" )

        self.initialize_gen()
        if not self.ctrl_item.no_sim:
            self.initialize_sim()

        # Msg.dbg( "Directory Check: %s" % ( PathUtils.current_dir()))

    def process(self):

        # Msg.dbg( "ExecuteController::process()")

        # my_usr_lbl = Msg.set_label( "user", "EXEC-VALS" )
        # Msg.lout( self.ctrl_item.catalougs(), "user", "Control Item Cataloug" )
        my_task_file =  PathUtils.include_trailing_path_delimiter( self.ctrl_item.parent_data.test_root )
        my_task_file += PathUtils.include_trailing_path_delimiter( self.ctrl_item.fctrl_dir )
        my_task_file += self.ctrl_item.fctrl_name

        my_tmp, my_task_ndx = PathUtils.split_path( self.ctrl_item.fctrl_dir )
        my_task_name = self.ctrl_item.fctrl_name.replace( ".py", "")

        # Msg.user( "Task File: %s, Task Name: %s, Task Index: %s" % ( my_task_file, my_task_name, my_task_ndx )  )

        self.process_task_file( my_task_file, my_task_name, my_task_ndx )

        # Msg.set_label( "user", my_usr_lbl )


    # process initialization
    def initialize_gen(self):

        # Msg.dbg( "ExecuteController::initialize_gen()")

        my_cmd = self.ctrl_item.parent_data.force_cmd

        # Msg.user( "ControlItem.no-asm: %s" %  ( str( self.ctrl_item.no_asm )) )
        # Msg.dbg( "ControlItem.max_instr: %s" %  ( str( self.ctrl_item.max_instr )) )


        my_cmd += " -t %s" + SysUtils.ifthen( self.ctrl_item.no_asm == False, "", " --noasm" ) + " --max-instr %d" % self.ctrl_item.max_instr

        # Msg.dbg( "my_cmd: %s" %  ( str( my_cmd )) )

        if isinstance( self.ctrl_item.generator, dict):
            for my_key in self.ctrl_item.generator.keys():
                my_cmd += " %s %s " % ( str( my_key ), SysUtils.ifthen( self.ctrl_item.generator[ my_key ] is None, "", str( self.ctrl_item.generator[ my_key ] )))

        self.force_cmd =  my_cmd.strip()
        # self.force_cmd += " >& %s"
        # Msg.user( "force_cmd: " + str( self.force_cmd ))



    def initialize_sim(self):

        # Msg.dbg( "ExecuteController::initialize_sim()")
        if not self.ctrl_item.no_sim:
            my_tmlog =  "iss.railhouse" #arg_testname.replace( ".py", ".log" )
            self.sim_log = "iss_sim.log"

            self.sim_cmd = "%s -T %s -C %s -i %d --exit_loop=%d" % ( self.ctrl_item.parent_data.iss_path
                                                                   , my_tmlog
                                                                   , self.ctrl_item.num_cores
                                                                   , self.ctrl_item.max_instr
                                                                   , self.ctrl_item.exit_loop )

            if isinstance( self.ctrl_item.iss, dict ):
                for my_key in self.ctrl_item.iss.keys():
                    if not my_key == CtrlItmKeys.iss_path:
                        Msg.user( "sim_cmd: %s, key: %s " % ( str( self.sim_cmd ), str( my_key )))
                        self.sim_cmd += " %s %s" % ( str( my_key ), SysUtils.ifthen( self.ctrl_item.iss[ my_key ] is None, "", str( self.ctrl_item.iss[ my_key ] )))

            self.sim_cmd += "%s%s"    #
            # self.sim_cmd += " >& %s" % ( self.sim_log )
            # Msg.user( "sim_cmd: " + str( self.sim_cmd ))


    def process_task_file( self, arg_task_file, arg_task_name, arg_task_ndx ): # , arg_task_name, arg_ndx ):
        # Msg.dbg( "ExecuteController::exec_task( Task File: %s )"% (arg_task_file ))

        if not self.exec_gen( arg_task_file ):
            return False

        if not self.ctrl_item.no_sim:
            self.exec_iss( arg_task_name )

        # if self.ctrl_item.mode != "mock" and self.ctrl_item.summary is not None:
        #     # create a new node in the summary that is keyed with the test name
        #     my_summary_item = self.ctrl_item.summary.register_task( arg_task_name, arg_ndx, self.ctrl_item.group )

        # if self.ctrl_item.mode != "mock" and self.ctrl_item.summary is not None:
            # create a new node in the summary that is keyed with the test name
        #    self.ctrl_item.summary.commit_task( arg_task_name, arg_ndx )

        return True


    def exec_gen( self, arg_task_file ):

        # Msg.dbg( "ExecuteController::exec_gen( %s )" % ( arg_task_file ) )

        # NOTE: Do not change force_cmd may need to reuse!!
        # my_task_file = self.ctrl_item.file_path()

        # my_cmd = self.force_cmd % ( my_testfile, arg_testname.replace( ".py", ".gen.log" ))
        my_log  = "gen.log" # arg_testname.replace( ".py", ".gen.log" )
        my_elog = "gen.err" # arg_testname.replace( ".py", ".gen.log" )

        my_cmd = self.force_cmd % ( arg_task_file ) # , my_log )
        # Msg.dbg( "Generate Command Line: %s" % ( my_cmd ))

        # if self.ctrl_item.mode == "mock":
        #     Msg.info( "Mock Generate: " +  my_cmd )
        # else:
        Msg.info( "ForceCommand = "  + str( { "force-command": my_cmd, "force-log": my_log, "force-elog": my_elog, "max-instr":self.ctrl_item.max_instr, "min-instr": self.ctrl_item.min_instr } ), True )
        Msg.flush()
        #     if self.ctrl_item.summary is not None:
        #         # update the current test with the generate cmd and the generate log file.
        #         self.ctrl_item.summary.set_gen_info( arg_task_name, arg_ndx, my_cmd, my_log )
        my_result = SysUtils.exec_process( my_cmd, my_log, my_elog, self.ctrl_item.timeout, True )

        # Msg.user( str( my_result[0] ) + ", type: " + str( type( my_result[0] )))
        my_ret_code = int(my_result[0])
        my_std_out  = str(my_result[1])
        my_std_err  = str(my_result[2])
        my_start    = str(my_result[3])
        my_end      = str(my_result[4])

        my_time_elapsed = SysUtils.ifthen( my_result[3] is not None, float(my_result[3]) , 0.0 )

        Msg.info( "ForceResult = " + str( { "force-retcode": my_ret_code, "force-stdout": my_std_out, "force-stderr":my_std_err, "force-start":my_start, "force-end": my_end } ))

        return SysUtils.success( my_result[0] )

        #  Msg.info( "Force Result, ReturnCode: %d, StdOut: %s, StdErr: %s" % ( my_result[0], str( my_result[1] ), str( my_result[2] )))
        #  if self.ctrl_item.summary is not None:
        #      self.ctrl_item.summary.set_gen_results( arg_task_name, arg_ndx, my_result[0], my_result[1], my_result[2] )
        #
        #  if bool(my_result[0]):
        #      # Msg.err( "Force Generation Failed, Return Code: %d" % ( my_result[0] ))
        #      #raise Exception( "Force Generation Failed, Return Code: %d" % ( my_result[0] ))
        #      return False
        #
        #  return True



    def exec_iss( self, arg_task_name ):

        # Msg.dbg( "ExecuteController::exec_iss( %s )" % ( arg_task_name ) )
        # build rest of command for iss
        my_elf = "%s.Default.ELF" % ( arg_task_name )
        # Msg.dbg( "Looking For: %s" % ( my_elf ))
        my_elf = SysUtils.ifthen( PathUtils.check_file( my_elf ), " %s" % my_elf, "" )
        # Msg.dbg( "Using ELF: %s" % ( my_elf ))

        my_elfns =  "%s.Secondary.ELF" % ( arg_task_name )
        # Msg.dbg( "Looking For: %s" % ( my_elfns ))
        my_elfns = SysUtils.ifthen( PathUtils.check_file( my_elfns ), " %s" % my_elfns, "" )
        my_cmd = self.sim_cmd % (my_elf, my_elfns)

        try:
            # execute the simulation
            Msg.info( "ISSCommand = " + str( { "iss-command": my_cmd } ))
            my_log  = self.sim_log
            my_elog = self.sim_log

            #my_ret_code = int( SysUtils.exec_cmd( my_cmd ) )
            my_result = SysUtils.exec_process( my_cmd, my_log, my_elog, self.ctrl_item.timeout, True )

            my_ret_code = int(my_result[0])
            my_std_out  = str(my_result[1])
            my_std_err  = str(my_result[2])
            my_start    = str(my_result[3])
            my_end      = str(my_result[4])

            Msg.info( "ISSResult = "  + str( { "iss-retcode": my_ret_code, "iss-log": self.sim_log, "iss-stdout": my_std_out, "iss-stderr":my_std_err, "iss-start":my_start, "iss-end": my_end } ))


        except Exception as arg_ex:
            Msg.error_trace( "ISS Execute Failure" )
            Msg.err( "ISS did not properly execute, Reason: %s" % ( str( arg_ex )))
            return False

        return True



