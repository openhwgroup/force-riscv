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
#  @package forrest_init.py
#   forrest_init.py
#
#   Contains definition of parameters used by argparse in forrest_run
#
import argparse


#
# Purpose: used to configure an argparse argument parser instantiated in
# module_run a superclass to forrest_run Caveats: setting defaults still
# requires the user to change the defaults in the Defaults class as weall as
# in this CommandLineParameters class.
#
class CommandLineParameters(object):
    # required variable for use with argparse, forms part of the help message
    usage = "Single Task Regression And Performance Execute Utility"

    # required variable for use with argparse, save and pass on remaider
    # (enabling prevents detecting unknown arguments)
    pass_remainder = False

    # required variable for use with argparse, choosing the
    # RawTextHelpFormatter means the endline characters in the descriptions
    # are not ignored.
    formatter_class = argparse.RawTextHelpFormatter

    # command line options are grouped into option groups a concept in argparse
    _group_1_name = "General options"
    _group_1_description = "basic, common, high-level options"
    _parameters_general_options = [
        # "short option"     "number of additonal args"
        # |      "long option"    |   "additional specifications"
        # |      |                |   |
        # |      |                |   |
        [
            "-f",
            "--control-name=",
            1,
            {},
            "- (Optional) Control File: Optional to override the default "
            "control file name in the specified\ndirectory, if not specified "
            'the default "_def_fctrl.py" will be used.',
        ],
        [
            "-d",
            "--control-dir=",
            1,
            {},
            '- (Optional) Location the_force_root + "tests/"of initial '
            "Control File, Relative path to a directory containing the\n"
            "Initial Control File, if not specified the relative path will be "
            '"tests/basic"',
        ],
        [
            "-w",
            "--workflow=",
            1,
            {},
            "- When present, specifies a particular workflow module to use, "
            "otherwise uses the default workflow.",
        ],
        [
            "-t",
            "--test-base=",
            1,
            {},
            "- (Optional) Absolute Path to alternate test base directory: "
            "Overrides the Force path, Needs\nto be an absolute path",
        ],
        [
            "-m",
            "--mode=",
            1,
            {"default": "regress", "choices": ["mock", "regress", "perf"]},
            "-mode\n\tmock - dry run\n\tregress (default) - actually "
            "generate and/or run simulations\n\tperf - performance tests, "
            "generate only",
        ],
        [
            "-o",
            "--logfile=",
            1,
            {},
            "- when present redirects stdout into specified file",
        ],
        [
            "-l",
            "--msg-lev=",
            1,
            {"default": "crit+err+warn+info+noinfo"},
            "- Set the output level, implemented as a bit mask example if you "
            "want warnings and debug pass 20, 4 + 16\n\t"
            '"nomsg"   [0x0000]- supresses all message\n\t'
            '"crit"    [0x0001]- Critical Errors Messages\n\t'
            '"err"     [0x0002]- Non Critical Errors Messages\n\t'
            '"warn"    [0x0004]- Include Warning Messages\n\t'
            '"info"    [0x0008]- Run Info Messages\n\t'
            '"dbg"     [0x0010]- Debug Information\n\t'
            '"user"    [0x0020]- Special Messages\n\t'
            '"trace"   [0x0040]- provides call stack trace info\n\t'
            '"noinfo"  [0x0080]- supresses the info label\n'
            'Example: "err"+"warn"+"dbg" will post messages\n\t'
            "[ERROR] - Shows Non Critical Error Messages     and\n\t"
            "[WARN]  - shows warning messages                and\n\t"
            "[DEBUG] - Shows debug messages                  and nothing else"
            '\n\n Default: "crit+err+warn+info+noinfo"\n'
            "To use the default log level and add levels prepend that level "
            "with a (+)\nTo use the default log level and remove levels "
            "prepend that level with a (-)\n"
            "Example:\n\t"
            "[-l +trace-crit+user] will create an output level "
            "err+warn+info+noinfo+trace+user",
        ],
    ]

    # These three lists are used as arguments to module_run
    group_names = [_group_1_name]
    group_descriptions = [_group_1_description]
    group_parameters = [_parameters_general_options]

    parameters = _parameters_general_options
    # for param_list in group_parameters:
    #    parameters.extend(param_list)


class Modes(object):
    # mode strings
    mock = "mock"
    perf = "perf"
    regress = "regress"


class Defaults(object):
    msg_level = "crit+err+warn+info+noinfo"
    fctrl_name = "_def_frun.py"
    help = False


class CmdLine(object):
    # Allowed Command Line Switches
    Switches = [
        "help",  # 0
        "control-name=",  # 1
        "control-dir=",  # 2
        "test-base=",  # 3
        "mode=",  # 4
        "msg-lev=",  # 5
        "logfile=",  # 6
    ]

    # command Switch Index
    help = 0
    control_name = 1
    control_dir = 2
    test_base = 3
    mode = 4
    msg_lev = 5
    logfile = 6
