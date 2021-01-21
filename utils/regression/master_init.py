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
## @package master_init.py
#    master_init.py
#
#    Contains definition of parameters used by argparse in master_run.
#
import argparse
import sys 
import os

## 
# Purpose: used to configure an argparse argument parser instantiated in module_run a superclass to master_run
# Caveats: setting defaults still requires the user to change the defaults in the Defaults class as well as in this CommandLineParameters class.
#


class CommandLineParameters(object):
    usage = """
  Master Regression and Performance Utility

  Example:

    %s -f control_file_fctrl.py

""" % sys.argv[0]

    # save and pass on remainder
    pass_remainder = False

    # Choosing the RawTextHelpFormatter means the endline characters in the descriptions are not ignored.
    formatter_class = argparse.RawTextHelpFormatter

    # do not allow abbrev parameters, only in Python >3.5 
    # allow_abbrev = False 

    #command line options are grouped into option groups a concept in argparse
    _group_1_name = "General options"
    _group_1_description = "basic, common, high-level options" 
    _parameters_general_options = [
        # -h and --help is not needed, provided by default.
        # "short option"                      "number of additonal args"          "help text
        # |                 "long option"     |  "additional specifications"      |
        # |                 |                 |  |                                |
        # |                 |                 |  |                                |
        ["-f"            , "--control-name=", 1, {"default":"_def_fctrl.py",
                                                  "metavar":""}                , "- When present, overrides the default control file name, \"_def_fctrl.py\", in\n"
                                                                                 "  the default   initial control directory which can be specified as a full path\n"
                                                                                 "  or a simple filename.   If only a simple file name is specified then the\n"
                                                                                 "  control directory will be used.   If a path is specified that path will become\n"
                                                                                 "  the control directory in all cases.   A control file must exist in the resolved\n"
                                                                                 "  control directory."], 
        ["-d"            , "--control-dir=" , 1, {"metavar":""}                , "- When present, overrides the default control path, \"<test-base>/tests\". This\n"
                                                                                 "  option can   be a full path or a relative path from the master run directory.\n"
                                                                                 "  If a path was specified as part of the control file name on the command line this\n"
                                                                                 "  option is ignored."],
        ["-c"            , "--config="      , 1, {"metavar":""}                , "- When present, overrides the default config file name, \"_riscv_rv64_fcfg.py\",\n"
                                                                                 "  located in the config directory which is found in the module directory which\n"
                                                                                 "  equates to the location of Master Run. A custom config file may be substituted by\n"
                                                                                 "  specifying a new file name or providing a full path to the desired config file.\n"
                                                                                 "  If a config file is not located, then the default config file is used. If that\n"
                                                                                 "  file does not exist, then default values are used. NOTE: Due to the complex\n"
                                                                                 "  configurations possible, relative paths are not supported at this time."],
        ["-r"            , "--num-runs="    , 1, {"default":1, "metavar":""}   , "- Number of times to repeat the entire process control file or a control\n"
                                                                                 "  template, this cannot be specified   in the control file, to repeat a control\n"
                                                                                 "  item line specify iterations in the options tag"],
        ["-z"            , "--seed="        , 1, {"default":None, "metavar":""}, "- Number to be used as a random seed for all test this run"],
        ["--no-archive"  , "--no-archive"   , 0, {"action":"store_true"}       , "- previous output directories are removed when this option is present"],
    ]

    _group_2_name = "Persistent, no override"
    _group_2_description = "options which are persistent and cannot be overridden in a control-file"
    _parameters_persisting_no_override = [
        # "short option"                      "number of additonal args"          "help text
        # |                 "long option"     |  "additional specifications"      |
        # |                 |                 |  |                                |
        # |                 |                 |  |                                |
        ["-o"            , "--test-base="   , 1, {"metavar":""}                , "- When present, overrides the default test base, calculated based using the\n"
                                                                                 "  master run location as a starting point, \"/../../tests\" must exist, along with\n"
                                                                                 "  the specified or defaut control file named if the test-base is to be used to\n"
                                                                                 "  determine the initial control directory."],
        ["-s"            , "--run-dir="     , 1, {"metavar":""}                , "- When present, overrides the default location of any client application used to\n"
                                                                                 "  process individual tasks, the default location is determined by using the\n"
                                                                                 "  directory where the executing master run is located."],
        ["-n"            , "--run-name="    , 1, {"metavar":""}                , "- When present, overrides the default client application, \"forrest_run.py\", used\n"
                                                                                 "  to process individual tasks. The run name must reference an executable file in\n"
                                                                                 "  the default or specified directory.  If a path is specified that path can be\n"
                                                                                 "  either a relative path having the same rules as the run directory or a full\n"
                                                                                 "  path to the executable."],
        ["-j"            , "--run-launcher=", 1, {"default":"local", 
                                                  "metavar":""}                , "- lsf or local, defaults to local"],
        ["-x"            , "--process-max=" , 1, {"default":16, "metavar":""}  , "- When present, overrides the default number of concurrent processes which is\n"
                                                                                 "  currently 16"],
        ["--target-dir=" , "--target-dir="  , 1, {"default":os.getcwd(), 
                                                  "metavar":""}                , "- When present, overrides the default output base directory, \"<current-dir>/output\"\n"
                                                                                 "  If a relative path is specified that path will be calculated from the current\n"
                                                                                 "  directory and converted to a full path."],
        ["--sum-level="  , "--sum-level="   , 1, {"default":1, 
                                                  "choices":['0','1','2','3'], 
                                                  "metavar":""}                , "- overrides the default level to display only fails and errors in the summary.\n"
                                                                                 "  Allowed values:\n"
                                                                                 "  \t0 - Silent, 1 - instruction overrun, 2 - all fails, 3 - everything"],
        ["-m"            , "--mode="        , 1, {"default":"regress", 
                                                  "choices":["mock", "regress", 
                                                             "perf", "count"], 
                                                  "metavar":""}                , "- When present, overrides the default, \"regress\" mode,\n"
                                                                                 "  Allowed values:\n"
                                                                                 "  \t\"mock\", \"regress\", \"perf\""],
    ]
    _group_3_name = "Persistent, yes override"
    _group_3_description = "options which are persistent yet may be overridden in a control-file.\n" \
                           "NOTE: The following command line options can be overridden using the options tag\n" \
                           "in each control item line in any control file, unless overridden the default or\n" \
                           "specified values are persistent until changed"
    _parameters_persisting_yes_override = [
        # "short option"                      "number of additional args"         "help text
        # |                 "long option"     |  "additional specifications"      |
        # |                 |                 |  |                                |
        # |                 |                 |  |                                |
        ["-t"            , "--timeout="     , 1, {"default":600, "metavar":""} , "- When present, overrides the default (600) maximum time in seconds that a\n"
                                                                                 "  process is allowed to run prior to a forced termination and reported error of\n"
                                                                                 "  that process."],
        ["--num-chips="  , "--num-chips="   , 1, {"default":1, 
                                                  "choices":[str(x) for x in 
                                                             range(1,33)], 
                                                  "metavar":""}                , "- When present, overrides the default number of chips (1) to use in specified\n"
                                                                                 "  named test"],
        ["--num-cores="  , "--num-cores="   , 1, {"default":1, 
                                                  "choices":['1','2','3','4'], 
                                                  "metavar":""}                , "- When present, overrides the default number of cores per chip (1) to use in\n"
                                                                                 "  specified named test"],
        ["--num-threads=", "--num-threads=" , 1, {"default":1, 
                                                  "choices":['1','2','3','4'], 
                                                  "metavar":""}                , "- When present, overrides the default number of threads per core (1) to use in\n"
                                                                                 "  specified named test"],
        ["--max-instr="  , "--max-instr="   , 1, {"default":10000, 
                                                  "metavar":""}                , "- When present, overrides the default maximum number (10000) of instructions for\n"
                                                                                 "  a specified test"],
        ["--min-instr="  , "--min-instr="   , 1, {"default":1, "metavar":""}   , "- When present, overrides the default minimum number (1) of instructions that\n"
                                                                                 "  must be executed for success"],
        ["--no-sim"      , "--no-sim"       , 0, {"action":"store_true"}       , "- When present, the simulator does not run"],
        ["--max-fails="  , "--max-fails="   , 1, {"default":10, "metavar":""}  , "- When present, sets the number of fails before the run is abandoned"],
        ["-k"            , "--keep="        , 1, {"default":"", "metavar":""}  , "- When present, \"all\" is specified, does not remove pass files in the output\n"
                                                                                 "  directory for that task; if a comma seperated file-extension string is\n"
                                                                                 "  specified, then the type of files will be preserved; otherwise the only two\n"
                                                                                 "  files for task that successfully complete is the _def_frun.py and the process\n"
                                                                                 "  log which is usually forrst.log"], 
        ]

    _group_4_name = "Non persistent"
    _group_4_description = "notably includes generator options, and others that are not preserved between tasks"
    _parameters_non_persistent = [
        # "short option"                      "number of additonal args"          "help text
        # |                 "long option"     |  "additional specifications"      |
        # |                 |                 |  |                                |
        # |                 |                 |  |                                |
        ["--iss="        , "--iss="         , 1, {"metavar":""}                , "- When present, sets iss options to pass as a dictionary for a specific set of\n"
                                                                                 "  instructions for any test processed in the first control file, after that it\n"
                                                                                 "  must be specified or it will revert to the default which is empty.\n"
                                                                                 "  Example:\n"
                                                                                 "  \t--iss= {\"path\":\"bla/bla\"} - will override the default simulator with bla/bla\n"
                                                                                 "  \t--iss= {\"--cpu_actlr=1\":None} would add \"--cpu_actlr=1\""],
        ["--generator="  , "--generator="   , 1, {"metavar":""}                , "- When present, sets the generator options for all tests processed in the first\n"
                                                                                 "  control file only, nested control files will need to provided this\n"
                                                                                 "  Example:\n"
                                                                                 "  \t--generator={\"--options\":\"option-tag\":\"option-value\", ...}\n"
                                                                                 "  \t--generator={\"--options\":\"op1-tag=op1-val op2-tag=op2-val\",\"tag = val\"::None ...}"],
    ]

    _group_5_name = "Others"
    _group_5_description = "Misc operations"
    _parameters_others = [
        # "short option"                      "number of additonal args"          "help text
        # |                 "long option"     |  "additional specifications"      |
        # |                 |                 |  |                                |
        # |                 |                 |  |                                |
        ["-e"            , "--expire="      , 1, {"metavar":""}                , "- When present, specifies number of days to keep output directory, once the files\n"
                                                                                 "  are removed then the utility will exit and not process further. If a mode was\n"
                                                                                 "  not specified then the expire will operate on regression only, otherwise it\n"
                                                                                 "  will follow the value in mode except when mock is passed in. In that case\n"
                                                                                 "  expire will have no effect.\n"
                                                                                 "  NOTE: archive and clean can be grouped with regress or perf to clean or archive\n"
                                                                                 "  the specific directories.\n"
                                                                                 "  Example:\n"
                                                                                 "  \t--expire 4 will preserve output that is 4 or less days old,\n"
                                                                                 "  \t--expire 0 will preserve nothing"],
        ["-l"            , "--msg-lev="     , 1, {"default":"crit+err+warn+"
                                                            "info+noinfo", 
                                                  "metavar":""}                , "- When present, Updates or replaces the default output level, which include:\n"
                                                                                 "  \t\"crit\"    [0x0001]- Critical Errors Messages,\n"
                                                                                 "  \t\"err\"     [0x0002]- Non Critical Errors Messages,\n"
                                                                                 "  \t\"warn\"    [0x0004]- Include Warning Messages,\n"
                                                                                 "  \t\"info\"    [0x0008]- Run Info Messages,\n"
                                                                                 "  \t\"noinfo\"  [0x0080]- supresses the info label, Additional Levels are:\n"
                                                                                 "  \t\"dbg\"     [0x0010]- Debug Information,\n"
                                                                                 "  \t\"user\"    [0x0020]- Special Messages,\n"
                                                                                 "  \t\"trace\"   [0x0040]- provides call stack trace info,\n"
                                                                                 "  \t\"all\"     [0xFFFF]- displays all supported messages,\n"
                                                                                 "  \t\"nomsg\"   [0x0000]- displays only non managed messages.\n"
                                                                                 "  To turn on a message type use the (+) operator. To turn off a message type\n"
                                                                                 "  use the (-) operator.\n"
                                                                                 "  Examples:\n"
                                                                                 "  \t-l +dbg-warn - will turn on debug messages and turn off warning,\n"
                                                                                 "  \t-l dbg+info  - will display only debug and info messages with the INFO label,\n"
                                                                                 "  \t-l all       - will display all message types,\n"
                                                                                 "  \t-l nomsg     - will display only system level messages and critical errors"],
        ["--client-lev=" , "--client-lev="  , 1, {"metavar":""}                , "- When present, sets or updates the message level for the client application,\n"
                                                                                 "  this output is written to the client log all options are the same except\n"
                                                                                 "  \t\"crit\", \"err\", \"warn\", \"info\" and \"noinfo\"\n"
                                                                                 "  are always enabled in the client and cannot be disabled otherwise everything is\n"
                                                                                 "  exactly the same as the -l option. If True is passed the message level is the\n"
                                                                                 "  same as that of master run respecting the required message types.\n"
                                                                                 "  Example:\n"
                                                                                 "  \t--client-lev all  - will enable all message types.\n"
                                                                                 "  \t--client-lev True - will enable the master run level in addition to the\n"
                                                                                 "  \t                    required levels in the client."],
        ]

    # These three lists are used as arguments to module_run
    group_names = [_group_1_name, _group_2_name, _group_3_name, _group_4_name, _group_5_name]
    group_descriptions = [_group_1_description, _group_2_description, _group_3_description, _group_4_description, _group_5_description]
    group_parameters = [_parameters_general_options, _parameters_persisting_no_override, _parameters_persisting_yes_override, _parameters_non_persistent, _parameters_others]

    # the following variable is for compatibility with the single argument constructor in cmdline_utils.py
    parameters = []
    for param_list in group_parameters:
        parameters.extend(param_list)


class Modes(object):
    # mode strings
    mock = "mock"
    perf = "perf"
    regress = "regress"
    count = "count"


class Expire(object):
    none = None
    clean = "clean"
    purge = "purge"
    all = int(0)


class Defaults(object):
    expire = Expire.none
    mode = Modes.regress
    msg_level = "crit+err+warn+info+noinfo"
    help = False
    num_runs = 1
    process = "write-only"
    fctrl_name = "_def_fctrl.py"      # default control file
    fctrl_dir = "../../tests"         # location of the default control file relative to master run
    fcfg_name = "_riscv_rv64_fcfg.py" # default config file
    run_name = "forrest_run.py"       # default processing client
    run_dir = "."                     # default processing client location
    run_launcher = "local"            # default launch mode for the client processor
    run_fctrl = "_def_frun.py"        # default processing client control file
    test_base = "../../tests"         # default tests root directory
    process_max = 16
    timeout = 600
    keep = ""
    max_fails = 10
    client_lev = None
    no_archive = False
    suffix = None
    iss = {}


class Formats(object):
    # output directories
    main_output_dir = "%s/output"
    perf_output_dir = "performance"
    regress_output_dir = "regression"
    msg_level = "crit+err+warn+info+noinfo%s"
    summ_level = "Summary Level: %d"
    exec_num_runs = "Executing %d of %d num-runs"
    out_dir = "Output Directory: %s"


class EnVars( object ):
    test_base    = "TEST_BASE"
    run_path     = "RUN_PATH"        # Client Processor Path
    run_launcher = "RUN_LAUNCHER"    # Client Processor Launcher

class Labels(object):
    CmdLineSwitchesAllowed = "Allowed Command Line Switches"

class CmdLine(object):
    # Allowed Command Line Switches
    Switches = [ "help"            #  0
               , "no-sim"          #  1     Do Not run simulations
               , "control-name="   #  2     Name of initial Control File
               , "control-dir="    #  3     Directory of initial Control File
               , "test-base="      #  4     Root of Test directories
               , "num-cores="      #  5     Number of cores to use in run
               , "max-instr="      #  6     Maximun number of instructions to be simulated
               , "min-instr="      #  7     Minimum number of instructions to need to be simulated
               , "sum-level="      #  8     Verbosity of the report at the end of the run
               , "target-dir="     #  9     Output directory override
               , "iss="            # 10     Iss Options
               , "generator="      # 11     Generator Options
               , "mode="           # 12     Master Mode
               , "msg-lev="        # 13     Message Levels
               , "num-runs="       # 14     Number of times to repeat full run
               , "expire="         # 15     Number of days to keep output
               , "inter-sim"       # 16
               , "run-dir="        # 17     Client Processor Home Directory
               , "run-name="       # 18     Client Processor Name
               , "run-launcher="   # 19     Type launcher for client processor. lsf or local
               , "process-max="    # 20     Max number of concurrent operations
               , "timeout="        # 21     Max number of seconds that can elapse on any single task before error
               , "client-lev="     # 22     Log Level in the Client, saved in the client log
               , "keep="           # 23
               , "no-archive"      # 24
               , "max-fails="      # 25
               , "num-chips="      # 26     Number of chips to use in run
               , "num-threads="    # 27     Number of threads per core to use in run
               , "config="         # 28     config file name
               , "seed="           # 29     global seed value
               ]

    # command Switch Index
    help         = 0
    no_sim       = 1
    control_name = 2
    control_dir  = 3
    test_base    = 4
    num_cores    = 5
    max_instr    = 6
    min_instr    = 7
    sum_level    = 8
    target_dir   = 9
    iss          = 10
    generator    = 11
    mode         = 12
    msg_lev      = 13
    num_runs     = 14
    expire       = 15
    inter_sim    = 16
    run_dir      = 17
    run_name     = 18
    run_launcher = 19
    process_max  = 20
    timeout      = 21
    client_lev   = 22
    keep         = 23
    no_archive   = 24
    max_fails    = 25
    num_chips    = 26
    num_threads  = 27
    config       = 28
    seed         = 29

# end: class CmdOpts(object):




