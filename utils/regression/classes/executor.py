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
# name: executor.py
# comments: Defines and Implementes the Executor abstract class which serves as the abstration
#           used to spawn a process or execute a process

from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.errors import *

from classes.control_item import ControlItem, CtrlItmKeys

# executor response that comes from SysUtils::exec_process
class ProcessResult:
    # exec process results
    process_retcode = 0
    process_stdout  = 1
    process_stderr  = 2
    process_start   = 3
    process_end     = 4

# Executor abstract class
class Executor( object ):

    def __init__( self ):
        self.ctrl_item = None
        self.frun = None
        self.task_file = None
        self.task_name = None

    def load( self, arg_ctrl_item ):
        # Msg.user( str(arg_ctrl_item ))
        # Msg.trace( "Executor::load()" )
        self.ctrl_item = arg_ctrl_item
        return self.ctrl_item is not None

    def skip(self):
        return False

    def set_frun( self, arg_frun ):
        self.frun = arg_frun

    def set_task_file( self, arg_task_file ):
        self.task_file = arg_task_file

    def set_task_name( self, arg_task_name ):
        self.task_name = arg_task_name

    def addDictOptions(self, aOptDict, aIgnoreKeys):
        ret_str = ""
        for (key, value) in sorted(aOptDict.items()):
            if key in aIgnoreKeys:
                continue

            key_str = str(key)
            value_str = str(value)
            ret_str += " %s" % key_str

            if value is None:
                continue

            if key_str.endswith('='):
                ret_str += value_str
            else:
                ret_str += ' ' + value_str

        Msg.user( "addDictOptions: %s " % ret_str)
        return ret_str

    def addDefaultOptions(self, aCmdLine, aDefOptions):
        ret_str = ""
        for def_opt in aDefOptions:
            if def_opt.isSpecified(aCmdLine):
                #Msg.user("option %s specified. " % def_opt._mName)
                pass
            else:
                ret_str += " " + def_opt.formatDefaultOption()
                #Msg.user("adding option: %s" % ret_str)
        return ret_str

    def execute( self ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Executor::execute() not implemented in descendent [%s]" % ( str( type( self ))))

    def extract_results( self, arg_result, arg_log, arg_elog ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Executor::extract_results(...) not implemented in descendent [%s]" % ( str( type( self ))))

    def open_log_file( self, pFileName, pOpenMode ):
        return open( pFileName, pOpenMode )
    
    def query_logs( self, arg_log, arg_elog ):

        my_errors = None

        with self.open_log_file( arg_log, "r" ) as my_hfile:
            Msg.dbg( "File Open: %s" % arg_log)
            try:
                my_results = self.query_result_log( my_hfile )
            except Exception as arg_ex:
                # NOTE: Determine the possible errors and handle accordingly, for now just keep processing
                Msg.error_trace()
                Msg.err( str( arg_ex ))
                raise
            finally:
                my_hfile.close()

        if arg_elog is not None:
            with self.open_log_file( arg_elog, "r" ) as my_hfile:
                Msg.dbg( "File Open: %s" % arg_elog)
                try:
                    my_errors = self.query_errors( my_hfile )

                except Exception as arg_ex:
                    # NOTE: Determine the possible errors and handle accordingly, for now just keep processing
                    Msg.error_trace()
                    Msg.err( str( arg_ex ))
                    raise
                finally:
                    my_hfile.close()

        return my_results, my_errors

    def query_result_log( self, arg_hfile ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Executor::query_result_log() not implemented in descendent [%s]" % ( str( type( self ))))

    def query_errors( self, arg_hfile, arg_results ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Executor::query_errors() not implemented in descendent [%s]" % ( str( type( self ))))

    def factory( self, arg_ctrl_item ):
        Msg.error_trace()
        raise NotImplementedError( "{{{TODO}}}: Implement a factory method to create one or more executors, Executor::factory() ..." )

