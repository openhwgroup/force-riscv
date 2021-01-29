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
# Since there will be a significant amount of directory manipulation import the path utils

import sys

from common.path_utils import PathUtils
from common.sys_utils import SysUtils
from common.msg_utils import Msg

from classes.controller import Controller
from classes.task_controller import TaskController
from classes.summary import SummaryErrorQueueItem
from classes.control_item import ControlItem, CtrlItmKeys, ControlItemType

class FileController(Controller):

    def __init__( self, aProcessQueue, aAppsInfo, aParentLocals = None):
        super().__init__(aAppsInfo )
        # Msg.dbg( "FileController::__init__( )" )
        self.mProcessQueue = aProcessQueue
        self.mControlFileLocals = {}
        if isinstance(aParentLocals, dict):
            import copy
            self.mControlFileLocals = copy.deepcopy(aParentLocals)

    def load(self, arg_ctrl_item ):
        super().load( arg_ctrl_item )
        # Msg.user( "FileController::load(1)" )

        self.parent_fctrl = self.ctrl_item.file_path()
        # Msg.user( "FileController::load(2)" )
        Msg.user( "File Path: %s" % ( self.parent_fctrl ), "FCTRL-LOADER")

        try:
            my_content = open( self.parent_fctrl ).read()

            # Msg.user( "FileController::load(3)" )
        except Exception as arg_ex:
            # Msg.user( "FileController::load(4)" )
            Msg.err( "Message: %s, Control File Path: %s" % ( str( arg_ex ), self.parent_fctrl ))
            my_err_queue_item = SummaryErrorQueueItem( { "error"  : arg_ex
                                                       , "message": "Control File Not Found ..."
                                                       , "path"   : self.ctrl_item.file_path()
                                                       , "type"   : str(type( arg_ex ))
                                                       } )

            ## Msg.user( "FileController::load(5)" )
            if self.mProcessQueue.summary is not None:
                self.mProcessQueue.summary.queue.enqueue(my_err_queue_item)
            return False
        finally:
            # Msg.user( "FileController::load(6)" )
            pass

        try:
            # Msg.user( "FileController::load(7)" )
            my_glb, my_loc = SysUtils.exec_content(my_content, False, self.mControlFileLocals)
            # Msg.user( "FileController::load(8)" )

        except Exception as arg_ex:
            # Msg.user( "FileController::load(9)" )
            my_exc_type, my_exc_val, my_exc_tb = sys.exc_info()
            my_ex = arg_ex

            Msg.err( "Message: %s, Control File Path: %s" % ( str( arg_ex ), self.parent_fctrl))
            Msg.blank()

            ## Msg.user( "FileController::load(10)" )
            my_err_queue_item = SummaryErrorQueueItem( { "error"  : arg_ex
                                                       , "message": "Control File not processed..."
                                                       , "path"   : self.ctrl_item.file_path()
                                                       , "type"   : str( my_exc_type )
                                                       } )

            if self.mProcessQueue.summary is not None:
                self.mProcessQueue.summary.queue.enqueue(my_err_queue_item)
            return False

        finally:
            # Msg.user( "FileController::load(12)" )
            pass

        self.fcontrol = my_loc["control_items"]

        #propagate local variables
        for key in my_loc.keys():
            self.mControlFileLocals.update({key: my_loc[key]})

        # Msg.user( "FileController::load(13)" )
        return True


    def process(self):

        # Msg.dbg( "FileController::process()" )
        # Msg.dbg( "FileController Contents: \n\"" + str( self.fcontrol ) + "\n" )

        try:
            # Msg.lout( my_options["work-dir"], MsgLevel.dbg, "Work Director Stack" )
            # a control file may iterate through to completion according to the amount set in num_runs
            for my_ndx in range( self.ctrl_item.iterations ):

                # Msg.user( "Executing %d of %d Iterations, Control File: %s" % ( my_ndx + 1, self.ctrl_item.iterations, self.ctrl_item.file_path()) , "FILE-ITERATION")
                if self.is_terminated():
                    break

                try:
                    my_item_ndx = 0
                    # each item in the control set exists as a dictionary
                    for my_item_dict in self.fcontrol:
                        try:
                            if self.is_terminated():
                                break

                            my_item_ndx += 1
                            # Msg.user( "Processing Line: %s" % (str( my_item_dict )), "CTRL-FILE"  )  )

                            my_ctrl_item = ControlItem()
                            my_ctrl_item.parent_fctrl = self.parent_fctrl
                            my_ctrl_item.fctrl_item   = str( my_item_dict )

                            try:
                                my_ctrl_item.load( self.mAppsInfo, my_item_dict, self.ctrl_item )
                            except:
                                raise

                            my_item_type = my_ctrl_item.item_type()
                            my_controller = None

                            if my_item_type == ControlItemType.TaskItem:
                                # Msg.dbg( "\nControl Item is a Control Task ..." )
                                my_controller = TaskController(self.mProcessQueue, self.mAppsInfo)

                            elif my_item_type == ControlItemType.FileItem:
                                # Msg.dbg( "\nControl Item is a Control File ..." )
                                my_controller = FileController(self.mProcessQueue, self.mAppsInfo, self.mControlFileLocals)

                            else:
                                raise Exception( "\"" + my_fctrl_name + "\": Unknown Item Type ...\nUnable to Process ... " )

                            if my_controller.load( my_ctrl_item ):
                                my_controller.set_on_fail_proc( self.on_fail_proc )
                                my_controller.set_is_term_proc( self.is_term_proc )
                                my_controller.process()
                            # Msg.dbg( "%s: Controller Creation Complete" % ( my_ctrl_item.fctrl_name ))

                        except TypeError as arg_ex:

                            Msg.err( str(  arg_ex  ))
                            my_err_queue_item = SummaryErrorQueueItem( { "error"  : "Item #%s Contains an Invalid Type" % ( str( my_item_ndx ))
                                                                       , "message": arg_ex
                                                                       , "path"   : self.ctrl_item.file_path()
                                                                       , "type"   : str(type( arg_ex ))
                                                                       } )

                            if self.mProcessQueue.summary is not None:
                                self.mProcessQueue.summary.queue.enqueue(my_err_queue_item)
                            Msg.blank()

                        except FileNotFoundError as arg_ex:

                            Msg.err( str(  arg_ex  ))
                            my_err_queue_item = SummaryErrorQueueItem( { "error"  : arg_ex
                                                                       , "message": "Control File Not Found ..."
                                                                       , "path"   : self.ctrl_item.file_path()
                                                                       , "type"   : str(type( arg_ex ))
                                                                       } )

                            if self.mProcessQueue.summary is not None:
                                self.mProcessQueue.summary.queue.enqueue(my_err_queue_item)
                            Msg.blank()

                        except Exception as arg_ex:
                            Msg.error_trace( str(arg_ex) )
                            Msg.err( str(arg_ex))
                            Msg.blank()

                        finally:
                            my_controller = None
                            my_item_dict = None

                except Exception as arg_ex:
                    Msg.error_trace( "[ERROR] - " + str(arg_ex) )
                    Msg.err( str(arg_ex))
                finally:
                    pass

        finally:
            pass
            # Msg.dbg()

        return True

