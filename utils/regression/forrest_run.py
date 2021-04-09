#!/usr/bin/env python3
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
# PYTHON3 UP
#  /software/public/python/3.4.1/bin/python3

#
# module: forrest_run.py
# comments: This module can be run as a part of master_run or can exec as a
#           standalone which will process a special control file which is
#           created by. This frun control file must contain all information
#           necessary to process a template task
#
#

import os
import signal

# Make third-party modules available for import
import sys
import traceback

import common.cmdline_utils as CmdLineUtils
from classes.ApplicationsSetup import ApplicationsSetup
from classes.control_item import ControlItem
from classes.exec_controller import ExecuteController
from classes.module_run import ModuleRun
from common.msg_utils import Msg
from common.path_utils import PathUtils
from common.sys_utils import SysUtils
from force_init import the_force_root
from forrest_init import CmdLine, Defaults, CommandLineParameters

# sys.path.append(PathUtils.real_path("../../3rd_party/py"))


class ForrestRun(ModuleRun):
    def __init__(self):
        super().__init__(CmdLine.Switches[CmdLine.msg_lev], Defaults.msg_level)

        self.frun_name = None
        self.frun_dir = None
        self.fctrl = None

        self.item_data = {}
        self.options = {}

        self.fcontrol = None

    def init_app_setup(self):
        try:
            self.m_app_setup = ApplicationsSetup(
                CommandLineParameters,
                sys.argv,
                CmdLineUtils.basic_command_line_argument_retrieval(
                    sys.argv[1:], "-w", "--workflow", str, 1
                ).workflow[0],
            )
            self.m_app_info = self.m_app_setup.getApplicationsInfo()
        except TypeError:
            # catches error that is thrown when trying to iterate through a
            # None type variable (if workflow argument does not exist)
            self.m_app_setup = ApplicationsSetup(
                CommandLineParameters, sys.argv
            )
            self.m_app_info = self.m_app_setup.getApplicationsInfo()
        except SystemExit as aSysExit:
            sys.exit(int(str(aSysExit)))
        except Exception as ex:
            print(
                "[ERROR] - An Unhandled Error has Occurred during "
                "applications setup of " + str(sys.argv[0])
            )
            traceback.print_exc(file=sys.stdout)
            sys.exit(43)

    def load(self):
        my_frun_path = self.option_def(
            CmdLine.Switches[CmdLine.control_name], None
        )
        if my_frun_path is None:
            raise Exception(
                "F-Run Control File Not Found on the Forrest Run Command "
                "Line: Given Path: %s",
                str((my_frun_path)),
            )

        self.locate_frun(my_frun_path)

        Msg.user("File Path: %s" % (my_frun_path))

        my_content = open(self.frun_name).read()
        my_glb, my_loc = SysUtils.exec_content(my_content)
        Msg.dbg(str(my_loc))

        self.fcontrol = my_loc["control_items"]

        my_ctrl_dict = self.fcontrol[0]

        my_ctrl_item = ControlItem()
        my_ctrl_item.load(self.m_app_info, my_ctrl_dict)

        # Msg.lout( my_ctrl_dict, "user", "Forrest Parent Data ...." )

        self.check_simulator()

        self.fctrl = ExecuteController(self.m_app_info)
        self.fctrl.set_frun(self.frun_name)
        self.fctrl.load(my_ctrl_item)

    def run(self):
        Msg.dbg("ForrestRun::run()")
        self.fctrl.process()

    def locate_frun(self, arg_frun_path):
        Msg.user("Directory set to %s" % (PathUtils.current_dir()))
        # if the control file contains a path then split that into the
        # directory and the file
        my_frun_dir, my_frun_name = PathUtils.split_path(arg_frun_path)
        # always convert to full path
        my_cur_dir = PathUtils.real_path(PathUtils.current_dir())

        # gots to have a source directory as part of the file name
        if my_frun_dir is None:
            my_frun_dir = my_cur_dir

        else:
            # always convert to full path. If the frun was loaded correctly
            # then we can conclude that the path tendered is either a relative
            # path from the starting directory or a full path to that file. If
            # it is not a full path then it will need to be converted to a
            # full path and all links removed
            my_frun_dir = PathUtils.real_path(my_frun_dir)

        # change into the directory to generate and simulate
        if not PathUtils.chdir(my_frun_dir):
            raise Exception(
                "F-Run Directory[%s] Not Found" % (str(my_frun_dir))
            )

        self.frun_name = my_frun_name
        self.frun_dir = my_frun_dir

    def check_simulator(self):
        if SysUtils.check_host("SAN"):
            Msg.dbg("System is in Green Zone .....")
            my_gcc_path = "/project/software/public/gcc/5.1/centos6.6/lib64"
            my_lib_path = SysUtils.envar("LD_LIBRARY_PATH", None)

            if not my_lib_path:
                SysUtils.envar_set("LD_LIBRARY_PATH", my_gcc_path)
            elif my_lib_path.find(my_gcc_path) < 0:
                SysUtils.envar_set(
                    "LD_LIBRARY_PATH", "%s:%s" % (my_gcc_path, my_lib_path)
                )

            Msg.dbg("LD_LIB_PATH: %s " % (str(my_lib_path)))
            Msg.dbg(
                '"LD_LIBRARY_PATH" = %s'
                % (str(SysUtils.envar("LD_LIBRARY_PATH", None)))
            )

        else:
            Msg.dbg("System is Red Zone or Yellow Zone")

        return True


def handle_signal(arg_signal, arg_stackframe):
    # it is necessary to write directly to stdout and not use print which is
    # very unreliable
    if arg_signal == signal.SIGINT:
        sys.stdout.write(
            "Signal = {'retcode': %d, 'message': 'Encountered interrupt, "
            "all processing halted'}\n" % (signal.SIGINT)
        )
    elif arg_signal == signal.SIGTERM:
        sys.stdout.write(
            "Signal = {'retcode': %d, 'message': 'OS Terminated Process, "
            "all processing halted'}\n" % (signal.SIGTERM)
        )
    # Flush the line and release the processor to ensure that the output is
    # fully written
    sys.stdout.flush()
    SysUtils.sleep(1)
    # once the line has been written kill any remaining processes dead dead
    # dead, this will suppress further output
    os.killpg(0, signal.SIGKILL)
    # finally return the signal id as the return code
    sys.exit(int(arg_signal))


def main():
    # set up signal handlers,
    signal.signal(signal.SIGINT, handle_signal)
    signal.signal(signal.SIGTERM, handle_signal)

    # initialize variables
    my_hlog = None
    my_org_stdout = None

    # global the_output_path =

    # Step 1: Save the originating directory
    my_pwd = PathUtils.current_dir()

    # Step 3: Extract Pid Group
    os.setpgid(os.getpid(), os.getpid())

    my_module = ForrestRun()

    try:
        my_module.force_path = the_force_root

        my_logfile = my_module.m_app_info.mCmdLineOpts.option_def(
            CmdLine.Switches[CmdLine.logfile], None
        )

        if my_logfile is not None:
            # print( "Redirecting STDOUT to my_logfile" )
            my_org_stdout = sys.stdout
            my_hlog = open(my_logfile, "w")
            sys.stdout = my_hlog
            Msg.user("Log File: %s" % (str(my_logfile)), "STDLOG")

        Msg.dbg("\nForce Path: %s" % (str(the_force_root)))
        Msg.dbg("Original Directory: " + my_pwd)

        # save current working directory

        Msg.dbg("Processing Command Line and Loading Control File")
        my_module.load()

        Msg.dbg("Directory set to %s" % (PathUtils.current_dir()))
        if not PathUtils.chdir(my_module.frun_dir, False):
            Msg.dbg(
                "Directory Unchanged, using the current directory for output"
            )

        my_module.run()
        Msg.dbg("Test Completed ....\n")
        Msg.blank()
        # sys.exit( 0 )

    except Exception as ex:
        from force_init import force_usage

        Msg.err(
            "An Unhandled Error has Occurred during run of " + str(sys.argv[0])
        )
        traceback.print_exc(file=sys.stdout)
        Msg.error_trace(str(ex))
        my_module.m_app_info.mCmdLineOpts.print_help()
        sys.exit(41)

    except BaseException:
        print(
            "[ERROR] - An Unhandled Error has Occurred during run of "
            + str(sys.argv[0])
        )
        traceback.print_exc(file=sys.stdout)
        sys.exit(42)

    finally:
        if my_logfile is not None:
            my_hlog.close()
            sys.stdout = my_org_stdout
            with open(my_logfile, "r") as my_hlog:
                print(my_hlog.read())

        if my_pwd is not None:
            PathUtils.chdir(my_pwd)
            Msg.dbg("Returned To: %s" % (PathUtils.current_dir()))


if __name__ == "__main__":
    main()
