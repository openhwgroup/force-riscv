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
# file: task_controller.py                                                     #
# summary: Processes a control item that is a control task. If a wild card is  #
#          used then the task is expanded to include all template files that   #
#          match the mask of the wild card. This also produces a special       #
#          control file that is processed by a spawned process that defaults   #
#          to forrest_run. Once that file is generated a process queue item    #
#          is created and submitted to the process queue for processing until  #
#          all files that have been part of the task expansion have been       #
#          processed.                                                          #
#                                                                              #
################################################################################
from shared.controller import Controller

from shared.path_utils import PathUtils
from shared.sys_utils import SysUtils
from shared.msg_utils import Msg
from shared.control_item import CtrlItmKeys
from shared.process_queue import ProcessQueueItem

class TaskController(Controller):

    # needs to retain value across instances
    iss_ok = None

    def __init__(self):
        super().__init__()
        # Msg.dbg( "TaskController::__init__( ... )" )


    def load( self, arg_ctrl_item ):
        super().load( arg_ctrl_item )
        # Msg.dbg( "TaskController::load()")

        self.task_list = PathUtils.list_files( self.ctrl_item.parent_data.test_root + self.ctrl_item.fctrl_dir + self.ctrl_item.fctrl_name )
        # Msg.dbg( "File Count: " + str( len( self.task_list )))
        # Msg.lout(self.task_list, "dbg", "File List to Process with this Controller" )
        return True


    def process(self):
        # Msg.dbg( "TaskController::process()")
        for my_ndx in range( 0, self.ctrl_item.iterations ):
            try:
                # my_usr_lbl = Msg.set_label( "user", "TASK-ITERATION" )
                # Msg.user( "Executing Iteration #%d of %d Task: %s " % ( my_ndx + 1, self.ctrl_item.iterations, self.ctrl_item.fctrl_name ))
                self.process_task_list()
            except Exception as arg_ex:
                # pass
                Msg.error_trace()
                Msg.err( str( arg_ex ))
                Msg.blank()
            finally:
                pass
                # Msg.set_label( "user", my_usr_lbl )


    def process_task_list(self):
        for my_task_file in self.task_list:
            # Msg.dbg( "Process Task File: %s" % ( my_task_file ))
            my_curdir = PathUtils.current_dir()
            try:
                # self.process_task_file( my_task_file )
                self.process_task_file( my_task_file )
            except Exception as arg_ex:
                Msg.error_trace()
                Msg.err( str( arg_ex ))
                Msg.blank()
            finally:
                PathUtils.chdir( my_curdir )


    def process_task_file( self,  arg_task_file ):

        try:
            #my_usr_lbl = Msg.set_label( "user", "TASK" )
            #Msg.user( "Old User Lbl: %s" % ( my_usr_lbl ))

            # NOTE: a task file can be but is not limited to being an test template file
            # set base task directory and extract the task id and update directory
            my_task_name = PathUtils.base_name( arg_task_file )
            my_task_dir = my_task_name.replace( ".py", "" )
            PathUtils.chdir( my_task_dir, True )

            # Msg.dbg( "Changed to Base Dir, my_task_name(%s), my_task_dir(%s)" % (my_task_name, my_task_dir))

            # check for exiting sub-directories, and extract the task iteration count to prevent
            # unintended modifications to the original passed on the commafrom shared.path_utils import PathUtilsnd line for this task or
            # acquired from a control file item

            self.process_task( arg_task_file )

        finally:
            PathUtils.chdir( ".." )


    def process_task( self, arg_task_file ):

        try:
            # get the subdirectory index
            my_ndx = int( PathUtils.next_subdir())

            # update the user label and send iteration message to the screen if user is active
            # my_usr_lbl = Msg.set_label( "user", "TEST-ITERATION" )
            # Msg.user( "Executing Iteration #%d of Test File: %s" % ( my_ndx + 1, arg_task_file ))
            # Msg.set_label( "user", my_usr_lbl )

            # create subdirectory and change into it
            PathUtils.chdir( "%05d" % my_ndx, True )

            # save the task template file name with path to the control item
            self.ctrl_item.fname = arg_task_file

            # write out the control file
            # if the write was successful then enqueue the new control file name with real path
            my_ctrl_file = "%s_def_frun.py" % ( PathUtils.include_trailing_path_delimiter( PathUtils.current_dir()))
            PathUtils.touch( "%sSTARTED" % PathUtils.include_trailing_path_delimiter( PathUtils.current_dir()))
            my_content = self.prepare( arg_task_file )
            if self.write_control_file( my_ctrl_file, my_content ):
                # my_queue_item = ProcessQueueItem( my_ctrl_file, self.ctrl_item.parent_fctrl, self.ctrl_item.fctrl_item, self.ctrl_item.group, my_content )
                my_queue_item = ProcessQueueItem( my_ctrl_file, self.ctrl_item, my_content  )  # self.parent_fctrl, self.ctrl_item.fctrl_item, self.ctrl_item.group)
                self.ctrl_item.parent_data.process_queue.enqueue( my_queue_item )

        except Exception as arg_ex:
            Msg.error_trace()
            Msg.err( str( arg_ex ))
            # reraise to prevent adding to summary instance
            raise

        finally:
            PathUtils.chdir( ".." )
        # end try except finally


    def write_control_file( self, arg_frun_file, arg_content ):

        #arg_filename = "_def_frun.py"
        with open( arg_frun_file, "w" ) as my_ofile:
            try:
                return ( my_ofile.write( arg_content ) > 0 )

            except Exception as arg_ex:
                Msg.error_trace()
                Msg.err( "Error Writing Control File, " + str( arg_ex ))
                # reraise exception to prevent creating tasks in queue if the control file cannot be written

            finally:
                my_ofile.close()

        return False


    def prepare( self, arg_task_file ):

         my_catalougs = self.ctrl_item.catalougs()
         my_catalougs[ CtrlItmKeys.fname ] = arg_task_file
         my_contents = "control_items = [\n"
         my_contents += self.ctrl_item.print_vals( my_catalougs, "\t" )
         # my_contents += str( my_catalougs )
         my_contents += "\n]"

         return my_contents

