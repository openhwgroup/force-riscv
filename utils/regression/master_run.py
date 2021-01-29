#!/usr/bin/env python3
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

## PYTHON3 UP
# /software/public/python/3.4.1/bin/python3

# File: master_run.py
# Comment: MasterRun main module
# Contributors: Howard Maler, Noah Sherrill, Amit Kumar, Jingliang(Leo) Wang, Venkat Srinivasan

# General imports
import signal
import sys
import traceback

import common.cmdline_utils as CmdLineUtils
from classes.performance_summary import PerformanceSummary
from classes.regression_summary import RegressionSummary
from classes.summary import SummaryLevel
from common.datetime_utils import DateTime
from common.kernel_objs import HiCriticalSection
from common.msg_utils import Msg
# general shared utility and data structure classes
from common.path_utils import PathUtils
from common.sys_utils import SysUtils
# from force_init import the_force_root
from master_init import Modes, Formats, CmdLine, Defaults, EnVars, Expire, CommandLineParameters

# since there will be a significant amount of directory manipulation import the path utils
# ancestor class

try:
    from classes.file_controller import FileController
    from classes.task_controller import TaskController
    from classes.control_item import ControlItem, CtrlItmKeys, CtrlItmDefs
    from classes.process_queue import ProcessQueue
    from classes.cleanup_rules import CleanUpRules
    from classes.module_run import ModuleRun
except ImportError:
    print('Please run \'make tests\'')
    traceback.print_exc(file=sys.stdout)
    sys.exit(1)

from classes.ApplicationsSetup import ApplicationsSetup
from classes.launcher import LauncherType

# Parallel Run imports
from common.threads import HiOldThread, HiEvent, workers_done_event, summary_done_event

shutdown_proc = None


class LoadError( Exception ):
    pass


class MasterRun( ModuleRun ):

    cLsfWaitTime = 30  # seconds

    def __init__(self):
        try:
            self.mCmdLineParms = CommandLineParameters
            self.mConfigArgs = retrieveConfigArgument(sys.argv[1:])

        except SystemExit as aSysExit:
            sys.exit(int(str(aSysExit)))
        except:
            print("[ERROR] - An Unhandled Error has Occurred during applications setup of " + str(sys.argv[0]))
            traceback.print_exc(file=sys.stdout)
            sys.exit(44)

        super().__init__(CmdLine.Switches[CmdLine.msg_lev], Defaults.msg_level)

        self.fctrl_dir = None
        self.mode = None
        self.summary = None
        self.process_queue = None
        self.client_lev = False
        self.default_root = ""

        self.ctrl_item = ControlItem()

        # no propogate values
        self.num_runs = None
        self.sum_level = None

        self.item_data = {}
        self.options = {}

        self.max_fails = 0
        self.limit_fails = False
        self.crit_sec = HiCriticalSection()
        self.processor_name = None
        self.fctrl = None

        self.terminated = False

        # this is a proc that will decrement the fails counter until it is 0
        # if the proc is None then this will not occur
        self.on_fail_proc = None
        self.is_term_proc = None

        # if an external terminate event is captured then this call back will shutdown
        # master run and all associated threads gracefully
        global shutdown_proc
        shutdown_proc = self.trigger_shutdown

        # Msg.lout( CmdLine.Switches, "dbg", "Allowed Command Line Switches" ) # Labels.cmd_line_switches_allowed  )

    def init_app_setup(self):
        if not self.m_app_setup:
            self.m_app_setup = ApplicationsSetup(self.mCmdLineParms, sys.argv, self.mConfigArgs, True, True)
            self.m_app_info = self.m_app_setup.getApplicationsInfo()
            self.m_app_info.mMainAppPath = PathUtils.include_trailing_path_delimiter(self.module_dir) + "../.."  # TODO still keep main app path for now.
            self.m_app_info.mProcessMax = self.option_def(CmdLine.Switches[CmdLine.process_max], None, self.to_int)
            self.m_app_info.mTestBaseDir = None
            self.m_app_info.mToolPath = self.module_dir

    def load(self):
        # Msg.user( "MasterRun::load" )
        self.init_all()
        # create the top level FileController
        self.fctrl = FileController(self.process_queue, self.m_app_info)

        # Msg.lout( self.options, "user", "Initial Option Values" )
        self.item_data[CtrlItmKeys.fname]   = self.fctrl_name
        self.item_data[CtrlItmKeys.options] = self.options

        if self.rtl is not None:
            self.item_data['rtl'] = self.rtl

        try:
            self.ctrl_item.load(self.m_app_info, self.item_data)
        except:
            Msg.err("Unable to load initial control item.")
            raise

        # populate the controller
        if not self.fctrl.load( self.ctrl_item ):
            raise LoadError( "Unable to load initial Control File .... " )

        # initialize the callbacks, if these are None then these are ignored
        self.fctrl.set_on_fail_proc( self.on_fail_proc )
        self.fctrl.set_is_term_proc( self.is_term_proc )

    def run(self):
        Msg.dbg( "MasterRun::run()" )

        # Run single run applications here before anything else is done
        for app_cfg in self.m_app_info.mSingleRunApps:
            app_executor = app_cfg.createExecutor()
            app_executor.load(self.ctrl_item)
            if not app_executor.skip():
                # TODO: need to add some proper logging here (so we can find information in the output folder)
                Msg.info('Currently executing %s app' % app_cfg.name())
                app_executor.pre()
                app_executor.execute()
                app_executor.post()
                Msg.info('Finished executing %s app' % app_cfg.name())

        for my_ndx in range( self.num_runs ):
            if self.terminated:
                break
            # my_usr_lbl = Msg.set_label( "user", "NUMRUNS" )
            Msg.info( Formats.exec_num_runs % ( my_ndx + 1, self.num_runs ))
            # Msg.set_label( "user", my_usr_lbl )
            Msg.blank()
            self.fctrl.process()

        # Wait until all the threads are done
        self.process_queue.fully_loaded = True
        workers_done_event.wait()
        summary_done_event.wait()

        if self.summary:
            Msg.dbg( Formats.summ_level % ( self.sum_level ))
            self.summary.process_summary( self.sum_level )

            # TODO: total_cycle_count and total_instruction_count are specific to RegressionSummary and
            # are used in the rtl application's reporter (PerformanceSummary does not contain either).
            # Do the rtl application and performance mode ever get run at the same time?

            # make cycle count and instruction count available in the shared object
            if isinstance(self.summary, RegressionSummary):
                self.m_app_info.mTagToReportInfo.update({"master_run":{"total_cycle_count":self.summary.total_cycle_count,
                                                                       "total_instruction_count":self.summary.total_instruction_count,
                                                                       "initial_control_file":self.fctrl_name,
                                                                       "output_dir":self.output_dir}})

        if self.mode == "count":
            Msg.info("Total tasks counted in control file tree: " + str(self.m_app_info.mNumTestsCount) + "\n")

        if self.terminated:
            Msg.info("####\n#### Reached max fails limit before test was completed.\n####")

        self.writeVersionInfo()
        self.modulesReport()

    # callback for the various points of failure
    def handle_on_fail( self, arg_sender ):

        Msg.user( "Fail from: %s" % ( str( arg_sender )))
        with self.crit_sec:
            if self.terminated:
                return
            self.max_fails -= 1
            if self.max_fails > 0:
                return
            Msg.info("####\n#### Reached max fails limit, setting terminated status flag and triggering regression shutdown.\n####\n")
            self.terminated = True

    def query_terminated( self ):
        with self.crit_sec:
            return self.terminated

    def trigger_shutdown( self ):
        with self.crit_sec:
            self.terminated = True
            self.shutdown()

    # add a callback that will execute and shutdown all threads when executed
    def shutdown( self ):
        # {{{TODO}}}
        pass
        # while not length( process_list ) > 0:
        #     process_list[0].process_terminate()
        #     process_list.remove( 0 )

    # initializes everything
    def init_all(self):
        # load options and initialize

        # == Old ==>> self.resolve_initial_dirs()
        self.check_config()
        self.initialize_directories()
        self.initialize_output()
        # self.initialize_tools( )
        Msg.dbg( " ------------------------------------------------------------------------------" )
        self.initialize_processor_cmd()

        self.populate_options()
        self.process_general_options()

        self.initialize_summary()

        # finally create the process queue
        self.initialize_process_queue()

    def check_config( self ):
        my_config_file = self.option_def( CmdLine.Switches[CmdLine.config], None)

        if my_config_file is None:
            self.iss         = { }
            self.generator   = { }
            self.rtl         = { }
            self.performance = CtrlItmDefs.performance
            self.regression  = CtrlItmDefs.regression
            Msg.user( "iss        : %s" % ( str( self.iss         )))
            Msg.user( "generator  : %s" % ( str( self.generator   )))
            Msg.user( "rtl        : %s" % ( str( self.rtl         )))
            Msg.user( "performance: %s" % ( str( self.performance )))
            Msg.user( "regression : %s" % ( str( self.regression  )))
            return True

        return self.load_config(my_config_file)

    def load_config( self, arg_cfg_file ):

        # ok there is a config file to load, there are two possibilities, neither require path manipulation
        # 1. the config file is specified as a relative path from the launch directory
        # 2. the config file is specified as a fully qualified path

        try:
            # load the config file if it exists
            my_content = open( arg_cfg_file ).read()

        except Exception as arg_ex:
            Msg.err( "Unable to open Config File: %s" % ( str( arg_cfg_file )))
            raise

        try:
            my_glb, my_loc = SysUtils.exec_content( my_content )
            self.load_config_data( my_loc )

        except Exception as arg_ex:
            Msg.err( "Unable to Process Config File, Message: %s" % ( str( arg_ex )))
            raise

        return True

    def load_config_data( self, arg_config_data ):

        self.master_config = arg_config_data["master_config"]

        self.iss         = self.master_config.get( 'iss'        , { } )
        self.generator   = self.master_config.get( 'generator'  , { } )
        self.rtl         = self.master_config.get( 'rtl'        , { } )
        self.performance = self.master_config.get( CtrlItmKeys.performance, CtrlItmDefs.performance )
        self.regression  = self.master_config.get( CtrlItmKeys.regression , CtrlItmDefs.regression  )

        Msg.user( "iss        : %s" % ( str( self.iss         )))
        Msg.user( "generator  : %s" % ( str( self.generator   )))
        Msg.user( "rtl        : %s" % ( str( self.rtl         )))
        Msg.user( "performance: %s" % ( str( self.performance )))
        Msg.user( "regression : %s" % ( str( self.regression  )))
        return True

    # resolves the the test_base, initial control directory, and populates options dictionay populated with these three values
    # if failure an Exception is raised
    def initialize_directories(self):

        # extract the initial control file information
        # self.fctrl_dir is now the fully qualified path to the first control file, thus
        self.fctrl_dir, self.fctrl_name = self.locate_control_file()
        self.m_app_info.mTestBaseDir  = self.locate_directory(CmdLine.Switches[ CmdLine.test_base], EnVars.test_base, self.fctrl_dir if self.fctrl_dir is not None else Defaults.test_base)

        Msg.user( "Module Path      : %s" % ( str( self.module_dir )), "INITIAL_DIRS")
        Msg.user( "Main Control File: %s" % ( str( self.fctrl_name )), "INITIAL_DIRS")
        Msg.user( "Main Control Dir : %s" % ( str( self.fctrl_dir  )), "INITIAL_DIRS")
        Msg.user( "Test Root        : %s" % (str(self.m_app_info.mTestBaseDir)), "INITIAL_DIRS")

    def locate_control_file( self ):
        # populate the initial control file, if none is specified then use the default
        my_fctrl_path = self.option_def( CmdLine.Switches[ CmdLine.control_name], CtrlItmDefs.fctrl_name )
        Msg.user( "Control Path: %s (1)" % ( str( my_fctrl_path )), "INITIAL_DIRS" )

        # if the control file contains a path then split that into the directory and the file
        my_fctrl_dir, my_fctrl_file = PathUtils.split_path( my_fctrl_path )
        Msg.user( "Control File Split, Directory: %s, FileName: %s" % ( str(my_fctrl_dir), str( my_fctrl_file  )), "INITIAL_DIRS" )

        # important to realize that if the default control file is used, it is necessary to find the right file
        if my_fctrl_dir is None:
            # if the control file does not contain a path then need to assume the default path of
            my_fctrl_dir = self.locate_directory( CmdLine.Switches[CmdLine.control_dir], EnVars.test_base, Defaults.test_base )
        else:
            my_fctrl_dir = PathUtils.include_trailing_path_delimiter( PathUtils.real_path( my_fctrl_dir ))

        return my_fctrl_dir, my_fctrl_file

    def locate_directory( self, arg_cmd_switch, arg_envar, arg_default ):

        Msg.dbg( "arg_cmd_switch[%s], arg_envar[%s], arg_default[%s]" % (str(arg_cmd_switch), str(arg_envar), arg_default))
        my_tmp = self.option_def( arg_cmd_switch, None)
        # Msg.user( "Result Path: %s" % ( str( my_tmp )))

        if my_tmp is None:
            if arg_envar is not None:
                # Not passed on the command line check for envar and the default
                my_tmp = SysUtils.envar( arg_envar, arg_default, False )
            else:
                my_tmp = arg_default
            # If a relative path has been provided either in the environmental var or as the default that path needs
            # to be appended to the module path. Since all full paths begin with a path delimiter this is a valid test
            if my_tmp[0] != "/":
                my_tmp = PathUtils.include_trailing_path_delimiter( self.module_dir ) + my_tmp

        # OK here is where it gets a bit tricky, when passed on the command line the path is calculated from the
        # current directory, in all other cases from the module path. Since the app should be in the initial directory
        # calculating the real path should resolve to a fully qualified path. To remove all indirection use real path
        my_tmp = PathUtils.real_path( my_tmp )

        # At this point the path should be a fully qualified path
        # Msg.user( "Result Path: %s" % ( str( my_tmp )))
        #
        Msg.user( "Result Path: %s" % ( str( my_tmp )))
        if not PathUtils.valid_path( my_tmp ):
            raise FileNotFoundError( "Initial Directory for %s Resolution Failed[%s] could not be located" % ( arg_cmd_switch, my_tmp ))

        if not PathUtils.check_exe( my_tmp ):
            my_tmp = PathUtils.include_trailing_path_delimiter( my_tmp )

        return my_tmp

    # set up director and archive new directories
    def initialize_output(self):

        self.mode = self.option_def( CmdLine.Switches[CmdLine.mode], None )
        self.output_root = PathUtils.exclude_trailing_path_delimiter( self.option_def( CmdLine.Switches[CmdLine.target_dir], PathUtils.current_dir() ))
        Msg.user( "Output Root: %s" % ( str(self.output_root )) )

        # check launcher type here since we need to know if we are running with LSF
        Msg.user( "Before Launcher Type", "MASTERRUN" )
        self.launcher_type = self.option_def( CmdLine.Switches[CmdLine.run_launcher], Defaults.run_launcher )
        Msg.user( "Launcher Type: %s" % ( str( self.launcher_type  ) ), "MASTERRUN" )

        # ok the root output directory has been established.
        # next check to see if there is an expiration if there is handle that and exit the session
        my_expire = self.option_def( CmdLine.Switches[CmdLine.expire], None )

        my_session_type = Formats.perf_output_dir if SysUtils.found( self.mode.find( Modes.perf )) else Formats.regress_output_dir

        if my_expire is not None:
            self.handle_expire( my_expire, my_session_type )
            raise Exception( "Problem with handle_expire, should have terminated ....." )

        # Continuing create the full output directory which if exists should be archived or removed
        my_output_base = Formats.main_output_dir % self.output_root
        self.output_dir = "%s/%s/" % ( PathUtils.exclude_trailing_path_delimiter( my_output_base ), PathUtils.exclude_trailing_path_delimiter( my_session_type ))

        Msg.user( "Target Output Dir: %s" % ( str(self.output_dir )) )

        mod_time = None
        # if there is no expire setting then
        if PathUtils.check_dir( self.output_dir ):
            # check modification time of the directory, if it is created very recently, delay a bit when running on LSF.
            # since client machines might hold a stale directory handler still.
            mod_time = PathUtils.time_modified(self.output_dir)
            if self.option_def( CmdLine.Switches[CmdLine.no_archive ], Defaults.no_archive ):
                PathUtils.rmdir( self.output_dir, True ) # remove output directory tree
                # Msg.user( "[1] - Target Output Dir: %s" % ( str(self.output_dir )) )
            else:
                PathUtils.archive_dir( self.output_dir )
                # Msg.user( "[2] - Target Output Dir: %s" % ( str(self.output_dir )) )

        PathUtils.mkdir( self.output_dir )

        if mod_time is not None:
            self.waitForLfs(mod_time)

        return True

    # Wait a bit for LSF to expire stale file handle for regression directory if running with LSF
    def waitForLfs(self, aModTime):
        if self.launcher_type == LauncherType.Lsf:
            time_diff = int ( DateTime.Time() - aModTime )
            if time_diff < MasterRun.cLsfWaitTime:
                sec_delay = MasterRun.cLsfWaitTime - time_diff
                Msg.info ("Using LSF, delaying %d seconds so that stale output/regression file handle will expire..." % sec_delay)
                SysUtils.sleep_seconds_with_progress(sec_delay)
                Msg.info ("Waiting done, resumed master run")

    def writeVersionInfo(self):
        out_line_fmt = "{}, scm_system: {}, revision number: {}, location: {}, url: {}\n"
        version_info = ""
        for app_tag, app_config in self.m_app_info.mTagToApp.items():
            Msg.user('app_tag: %s, app_config: %s' % (app_tag, app_config))
            version_data = app_config.parameter("version")
            for item in version_data:
                if item['status']:
                    version_info += out_line_fmt.format(app_config.name(),
                                                        item['scm_type'],
                                                        str(item['version']),
                                                        item["folder"],
                                                        item['url'])
        with open(self.output_dir + "version_info.txt", "w+") as outfile:
            if version_info:
                outfile.write(version_info)
            else:
                outfile.write("No version information found")

    # Call the report methods from each of the sequence apps. Some apps report, others pass through
    def modulesReport(self):
        for app_cfg in self.m_app_info.mSequenceApps:
            reporter = app_cfg.createReporter()
            reporter.report(self.m_app_info, app_cfg.tag())

    @staticmethod
    def to_int(a_value):
        return int(a_value)

    @staticmethod
    def to_hex(a_value):
        return hex(int(a_value, 0))

    # populate the general options
    def populate_options(self):
        self.options[CtrlItmKeys.fdir        ] = self.fctrl_dir
        self.options[CtrlItmKeys.no_sim      ] = self.option_def( CmdLine.Switches[CmdLine.no_sim     ], CtrlItmDefs.no_sim      )
        self.options[CtrlItmKeys.num_chips   ] = self.option_def( CmdLine.Switches[CmdLine.num_chips  ], CtrlItmDefs.num_chips   , self.to_int)
        self.options[CtrlItmKeys.num_cores   ] = self.option_def( CmdLine.Switches[CmdLine.num_cores  ], CtrlItmDefs.num_cores   , self.to_int)
        self.options[CtrlItmKeys.num_threads ] = self.option_def( CmdLine.Switches[CmdLine.num_threads], CtrlItmDefs.num_threads , self.to_int)
        self.options[CtrlItmKeys.min_instr   ] = self.option_def( CmdLine.Switches[CmdLine.min_instr  ], CtrlItmDefs.min_instr   , self.to_int)
        self.options[CtrlItmKeys.max_instr   ] = self.option_def( CmdLine.Switches[CmdLine.max_instr  ], CtrlItmDefs.max_instr   , self.to_int)
        self.options[CtrlItmKeys.timeout     ] = self.option_def( CmdLine.Switches[CmdLine.timeout    ], CtrlItmDefs.timeout     , self.to_int)
        self.options[CtrlItmKeys.seed        ] = self.option_def( CmdLine.Switches[CmdLine.seed       ], CtrlItmDefs.seed        , self.to_hex)
        self.options[CtrlItmKeys.suffix      ] = Defaults.suffix

        self.max_fails                         = self.option_def( CmdLine.Switches[CmdLine.max_fails  ], Defaults.max_fails      , self.to_int)

        if self.max_fails > 0:
            # {{{TODO}}} expand this to use the terminated proc as a way to determine whether or not to stop
            # processing more items instead of the insane way it works now.
            self.is_term_proc = self.query_terminated
            self.on_fail_proc = self.handle_on_fail

    def process_general_options(self):
        # run options
        self.num_runs  = self.option_def( CmdLine.Switches[ CmdLine.num_runs  ], Defaults.num_runs, self.to_int )
        self.sum_level = self.option_def( CmdLine.Switches[ CmdLine.sum_level ], SummaryLevel.Fail, self.to_int )
        #my_usr_lbl = Msg.set_label( "user", "PROCESS" )
        Msg.user( "process-max: %d" % (self.m_app_info.mProcessMax), "MASTER")
        #Msg.set_label( "user", my_usr_lbl )

    # create the proper summary
    def initialize_summary(self):

        my_keep = self.option_def( CmdLine.Switches[CmdLine.keep], Defaults.keep )
        clean_up_rules = CleanUpRules(my_keep)

        if SysUtils.found( self.mode.find( Modes.perf )):
            self.mode = Modes.perf
            self.options[CtrlItmKeys.no_sim] = True
            self.summary = PerformanceSummary( self.output_dir, clean_up_rules )
        elif SysUtils.found( self.mode.find( Modes.regress )):
            self.mode = Modes.regress
            self.summary = RegressionSummary( self.output_dir, clean_up_rules )
        else:
            self.mode = Modes.count
            self.m_app_info.mMode = "count"
            self.summary = RegressionSummary( self.output_dir, clean_up_rules )

        if self.summary is not None:

            self.summary.set_on_fail_proc( self.on_fail_proc )
            # {{{TODO}}} expand this to use the terminated proc as a way to determine whether or not to stop
            # processing more items instead of the insane way it works now.
            self.summary.set_is_term_proc( self.is_term_proc )

        # self.options[CtrlItmKeys.summary] = self.summary

    def initialize_process_queue( self ):

        # {{{TODO}}} - Clean this up
        global workers_done_event

        self.process_queue = ProcessQueue()

        self.process_queue.process_cmd  = self.process_cmd
        self.process_queue.processor_name = self.processor_name
        self.process_queue.summary      = self.summary
        self.process_queue.process_max  = self.m_app_info.mProcessMax

        self.process_queue.launcher_type = self.launcher_type
        Msg.user( "Done Event: %s" % (str( workers_done_event)), "MAIN")

        self.process_queue.done_event = workers_done_event
        # Msg.user( "Done Event: %s" % (str( workers_done_event)), "MAIN")

        self.process_queue.open_queue()

        # {{{TODO}}} Replace the open_queue call with a simplified version above
        #self.process_queue.open_queue( self.process_cmd, self.summary, self.process_max, workers_done_event, self.process_launcher )

    def initialize_processor_cmd( self ):

        # the default task processor is "forrest_run.py"
        # the default directory is the same directory as the master_run
        # the processor can be replaced with a command line argument which may or may not contain a path
        # if it does not contain a path then the default path will be used
        # if a directory is passed on the command line in all cases that will be the location of the processor
        my_run_dir  = None
        my_run_name = None
        my_tmp_name = None
        my_tmp_path = None

        my_run_path = self.option_def( CmdLine.Switches[CmdLine.run_name], None )

        if my_run_path is not None:
            my_run_dir, my_run_name = PathUtils.split_path( my_run_path )
            Msg.user( "Client Dir: %s, Client Name: %s (1)" % (str(my_run_dir), str(my_run_name)), "PROCESS_CMD" )

        if my_run_dir is None:
            my_tmp_path = self.locate_directory( CmdLine.Switches[CmdLine.run_dir], EnVars.run_path, self.module_dir)
            # Msg.user( "Temp Path: [%s] (1)" % (str(my_tmp_path)), "PROCESS_CMD" )

            if PathUtils.check_exe( my_tmp_path ):
                my_run_dir, my_tmp_name = PathUtils.split_path( my_tmp_path )
                # Msg.user( "Client Dir: %s, Client Name: %s (2)" % (str(my_run_dir), str(my_tmp_name)), "PROCESS_CMD" )
            else:
                my_run_dir = my_tmp_path
                # Msg.user( "Client Dir: %s, Client Name: %s (3)" % (str(my_run_dir), str(my_run_name)), "PROCESS_CMD" )

        if my_run_name is None:
            my_run_name = my_tmp_name if my_tmp_name is not None else Defaults.run_name

        # Msg.user( "Client Dir: %s, Client Name: %s (4)" % (str(my_run_dir), str(my_run_name)), "PROCESS_CMD" )
        my_process_cmd = PathUtils.real_path( PathUtils.append_path( PathUtils.include_trailing_path_delimiter( my_run_dir ), my_run_name  ))
        Msg.user( "Process Cmd: %s (1)" % (str(my_process_cmd)), "PROCESS_CMD" )

        my_msg_lev = self.option_def( CmdLine.Switches[CmdLine.client_lev], None)

        if my_msg_lev is not None :
            if my_msg_lev == True:
                my_process_cmd += Msg.get_level_as_str()
                # Msg.user( "Process Cmd: %s (2)" % (str(my_process_cmd)), "PROCESS_CMD" )

            else :
                my_process_cmd += " -l " + my_msg_lev
                Msg.user( "Process Cmd: %s" % (str(my_process_cmd)), "PROCESS_CMD" )

        if self.m_app_info.mConfigPath is not None:
            my_process_cmd += " -w %s" % self.m_app_info.mConfigPath
        my_process_cmd += " -f %s"

        self.processor_name = my_run_name.replace( ".py", "" ).replace( "_run", "" )
        self.process_cmd = my_process_cmd

        Msg.dbg( "Process Cmd: %s" % (str(self.process_cmd)))

        # Msg.dbg( "Process Cmd: %s" % (self.process_cmd))

    # An expire value has been found
    # There are several possibilities
    # 1. "clean" was pass on the command line, in this case remove the output directory which should
    #     the last directory that received output for that mode
    # 2. "purge" was passed on the command line, in this case purge all output directories for that mode
    # 3. a zero (0) was passed on the command line
    # 4. a none zero integer was passed on the command line, purge all directories related to the mode
    # if none of these are found and exception is raised in all cases master_run terminates immediately
    def handle_expire(self, arg_expire, arg_mask):

        try:
            Msg.user( "Expire: %s, Mask: %s" % ( str( arg_expire), str( arg_mask )), "EXPIRE" )
            my_output_base = Formats.main_output_dir % self.output_root
            Msg.user( "Expire [1], Output Base: %s" % ( str( my_output_base )), "EXPIRE" )
            Msg.info( "Output Directories Cleanup, Please wait ..." )

            if int( arg_expire ) == Expire.all:

                Msg.info( "Building Directory List, [%s]" % ( my_output_base ))
                my_dirs = PathUtils.list_dirs( my_output_base )
                Msg.dbg( "All Dirs: %s" % ( str( my_dirs  )), "EXPIRE" )

                for my_dir in my_dirs:
                    # Msg.user( "Current Dir: %s" % ( str( my_dir )), "EXPIRE" )
                    if my_dir.startswith( arg_mask ):
                        my_full_dir = "%s%s" % ( PathUtils.include_trailing_path_delimiter(my_output_base), my_dir )
                        Msg.info( "Removing: %s" % ( my_full_dir ))
                        PathUtils.rmdir( my_full_dir, True )
                    # end if my_dir.startswith( arg_mask ):
                # end for my_dir in my_dirs:

            else:

                Msg.info( "Checking for Expired Directories: %s" % ( my_full_dir ))
                my_expiredate = DateTime.DateDelta( int( my_expire ))
                PathUtils.expire( my_full_dir, my_expiredate )

        except Exception as ex:
            Msg.error_trace()
            Msg.err( str( ex ))

        finally:
            Msg.info( "Operation Complete, Restart Master Run to continue ..." )
            sys.exit(1)

    def run_mode(self):
        return self.mode


def sig_interrupt_handler(signal, frame):
    # Allows us to catch SIGINTs and inform child threads to quit at next chance.
    # Since the child threads are daemons, this should not be needed, but we can remove this at a later date.

    global shutdown_proc
    if shutdown_proc is not None:
        shutdown_proc()

    sys.exit(-1)


def retrieveConfigArgument(aArguments):
    config_argument = CmdLineUtils.basicCommandLineArgumentRetrieval(aArguments, '-c', '--config', str, 1)

    # PathUtils.resolvePath() lets us retrieve the entire file path from a string
    return PathUtils.resolvePath(config_argument.config[0]) if config_argument.config is not None else None


def main():
    signal.signal(signal.SIGINT, sig_interrupt_handler)
    my_pwd = None

    print( "\n=======================================\n\tInitializing ....\n=======================================\n" )
    try:
        my_module = MasterRun()

        # save current working directory
        my_pwd = PathUtils.current_dir()

        Msg.dbg( "Original Directory: " + my_pwd )
        Msg.dbg( "Processing Command Line and Loading Control File" )
        my_module.load()

        if not PathUtils.chdir( my_module.output_dir, True ):
            Msg.dbg( "Unable to change into: " + my_module.output_dir + ", using the current directory for output" )

        Msg.info( "\nConcurrent Operations: %s" % (str(my_module.m_app_info.mProcessMax)))
        my_module.run()

        Msg.info( "Test Completed ....\n" )

    except FileNotFoundError as arg_ex:
        # Msg.error_trace("[ERROR] -  " + str(arg_ex) )
        Msg.err( str(arg_ex) )
        Msg.blank()

    except LoadError as arg_ex:
        # Msg.error_trace("[ERROR] -  " + str(arg_ex) )
        Msg.err( str(arg_ex) )
        Msg.blank()

    except Exception:
        Msg.err( "[ERROR] - An Unhandled Error has Occurred during run of " + str( sys.argv[0] ))
        traceback.print_exc( file=sys.stdout )

    finally:
        if my_pwd is not None:
            # Msg.dbg( "Restoring Original Directory: " + my_pwd )
            PathUtils.chdir( my_pwd )


if __name__ == "__main__":
    main()
