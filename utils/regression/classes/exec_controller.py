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
# file: execute_controller.py
# comment: executes the processes necessary to

from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.errors import *

from classes.controller import Controller
from classes.control_item import ControlItem, CtrlItmKeys

class ExecuteController( Controller ):

    def __init__( self, aAppsInfo ):
        super().__init__(aAppsInfo )
        self.executors = []
        self.frun = None
        self.task_file = None
        self.task_name = None
        self.task_ndx  = None

    def load(self, arg_ctrl_item ):
        super().load( arg_ctrl_item )
        self.task_file, self.task_name, self.task_ndx = self.initialize_task()
        self.load_executors()

    def process( self ):
        # super().process()
        try:
            self.process_executors()
        except Exception as arg_ex:
            self.report_error( arg_ex )
            raise
        finally:
            return True

    def set_frun( self, arg_frun ):
        self.frun = arg_frun

    def load_executors( self ):
        try:
            # for app_cfg in self.mAppsInfo.mSequenceApps:
            for seq_app_cfg in self.mAppsInfo.mSequenceApps:
                my_executor = seq_app_cfg.createExecutor()
                Msg.user( "ExecuteController::load_executors( 2 )" )
                my_executor.load( self.ctrl_item )
                Msg.user( "ExecuteController::load_executors( 1 )" )
                if not my_executor.skip():
                    Msg.user( "ExecuteController::load_executors( 3 )" )
                    my_executor.set_frun( self.frun )
                    # Msg.user( "ExecuteController::load_executors( 4 )" )
                    my_executor.set_task_file( self.task_file )
                    # Msg.user( "ExecuteController::load_executors( 5 )" )
                    my_executor.set_task_name( self.task_name )
                    # Msg.user( "ExecuteController::load_executors( 5.1 )" )
                    self.executors.append(my_executor)
        except Exception as arg_ex:
            Msg.user( "ExecuteController::load_executors( 14 )" )
            Msg.error_trace()
            self.report_error( arg_ex )

        finally:
            Msg.user( "ExecuteController::load_executors( 13 )" )
            return True

    def process_executors( self ):

        my_ret_val = False

        try:
            for my_executor in self.executors:
                Msg.user( "ExecuteController::process_executors( my_executor: %s )" % str( type( my_executor )), "EXE-CTRL" )
                my_executor.pre()
                if not my_executor.execute():
                    Msg.user("Executor returning False", "EXE-CTRL")
                    break
                my_executor.post()

            my_ret_val = True

        except Exception as arg_ex:
            Msg.error_trace()
            self.report_error( arg_ex )

        finally:
            return my_ret_val

    def initialize_task( self ):
        #Msg.user( "ExecuteController::initialize_task(1)" )
        my_task_file = PathUtils.append_path( self.ctrl_item.fctrl_dir, self.ctrl_item.fctrl_name )
        #Msg.user( "ExecuteController::initialize_task(2)" )
        my_tmp, my_task_ndx = PathUtils.split_path( self.ctrl_item.fctrl_dir )
        #Msg.user( "ExecuteController::initialize_task(3)" )
        my_task_name = self.ctrl_item.fctrl_name.replace( ".py", "")
        #Msg.user( "ExecuteController::initialize_task(5)" )
        Msg.user( "Task File: %s, Task Name: %s, Task Index: %s" % ( my_task_file, my_task_name, my_task_ndx )  )
        return ( my_task_file, my_task_name, my_task_ndx )

    # arg_ex is an exception class
    def report_error( self, arg_ex ):
        Msg.err( "%s: %s" %( str( type( self ), str( arg_ex ) )))
        raise

