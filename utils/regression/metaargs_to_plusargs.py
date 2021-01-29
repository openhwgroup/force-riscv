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

import os
import sys

from common.cmdline_utils import CmdLineParser, AttributeContainer
from common.path_utils import PathUtils
from common.sys_utils import SysUtils


class MetaArgsConversion(object):
    def __init__(self, aRtlRoot, aMakefile):
        self._mRtlRoot = aRtlRoot
        self._mMakefile = aMakefile
        self._mBaseResultSet = (
            None  # Raw result produced with an empty meta args string
        )
        self._mMakeCommand = "make -f %s output_run_opts " % self._mMakefile
        self._mTopSimPath = (
            PathUtils.include_trailing_path_delimiter(self._mRtlRoot)
            + "verif/top/sim"
        )

    def convertRawResult(self, aMetaArgs, aFilter=False):
        current_dir = PathUtils.current_dir()
        PathUtils.chdir(self._mTopSimPath)
        result, is_valid = SysUtils.get_command_output(
            self._mMakeCommand + aMetaArgs
        )
        PathUtils.chdir(current_dir)

        if not is_valid:
            print("Converting meta args: %s to raw result failed." % aMetaArgs)
            sys.exit(1)

        if aFilter:
            args_list = result.split(" ")
            args_list = self.filterList(args_list)
            result = " ".join(args_list)

        return result

    def filterList(self, aArgsList):
        syntax_to_skip = [
            "+UVM_TC_PATH=",
            "+UVM_TC_NAME=",
            "+UVM_TEST_DIR=",
            "+UVM_TC_SEED=",
            "+tbench_elf64=",
            "+ns_mem_init=",
            "+s_mem_init=",
            "+ntb_random_seed=",
            "-reportstats",
            "-assert",
            "nopostproc",
            "+REGR=",
            "+REGR_ID=",
        ]
        filtered_list = []
        for arg in aArgsList:
            skip_arg = False
            for syntax in syntax_to_skip:
                if arg.startswith(syntax):
                    skip_arg = True
                    break
            if skip_arg:
                continue
            else:
                filtered_list.append(arg)
        return filtered_list

    def rawResultSet(self, aRawResult, aFilter=False):
        args_list = aRawResult.split(" ")
        if aFilter:
            args_list = self.filterList(args_list)

        args_set = set(args_list)
        return args_set

    def convertNetResult(self, aMetaArgs):
        raw_result = self.convertRawResult(aMetaArgs)
        raw_result_set = self.rawResultSet(raw_result)

        if self._mBaseResultSet is None:
            self._mBaseResultSet = self.rawResultSet(self.convertRawResult(""))

        # print(" one set: ", str(raw_result_set))
        # print(" base set: ", str(self._mBaseResultSet))
        net_set = raw_result_set - self._mBaseResultSet

        return " ".join(net_set)

    def compareMetaAndPlus(self, aMetaArgs, aPlusArgs):
        result_set1 = self.rawResultSet(self.convertRawResult(aMetaArgs), True)
        result_set2 = self.rawResultSet(aPlusArgs, True)
        only_in_set1 = result_set1 - result_set2
        only_in_set2 = result_set2 - result_set1
        comp_result = "Only in set1: %s\nOnly in set2: %s" % (
            " ".join(only_in_set1),
            " ".join(only_in_set2),
        )
        return comp_result

    def comparePlusAndPlus(self, aPlusArgs1, aPlusArgs2):
        result_set1 = self.rawResultSet(aPlusArgs1, True)
        result_set2 = self.rawResultSet(aPlusArgs2, True)
        only_in_set1 = result_set1 - result_set2
        only_in_set2 = result_set2 - result_set1
        comp_result = "Only in set1: %s\nOnly in set2: %s" % (
            " ".join(only_in_set1),
            " ".join(only_in_set2),
        )
        return comp_result


def convert_meta_args(aConversionParms):
    if aConversionParms.meta_args is None:
        if (aConversionParms.plusargs is not None) and len(
            aConversionParms.cmp_plusargs
        ):
            pass
        else:
            print("Meta args not specified.")
            sys.exit(1)

    meta_args = aConversionParms.meta_args

    if aConversionParms.rtl_root is None:
        rtl_root = os.environ.get("PROJ_ROOT", None)
        if rtl_root is None:
            print("No RTL root defined.")
            sys.exit(1)
    else:
        rtl_root = aConversionParms.rtl_root

    if not PathUtils.check_dir(rtl_root):
        print("RTL root does not exist or is not a directory: %s" % rtl_root)
        sys.exit(1)

    script_dir, script_name = PathUtils.split_path(
        PathUtils.real_path(sys.argv[0])
    )
    make_file_path = (
        PathUtils.include_trailing_path_delimiter(script_dir)
        + "applications/rtl/MetaArgs.make"
    )
    if not PathUtils.check_file(make_file_path):
        print("File not exist: %s" % make_file_path)
        sys.exit(1)

    meta_args_conv = MetaArgsConversion(rtl_root, make_file_path)
    if aConversionParms.net:
        conversion_result = meta_args_conv.convertNetResult(meta_args)
    elif len(aConversionParms.cmp_plusargs):
        if (aConversionParms.plusargs is not None) and len(
            aConversionParms.plusargs
        ) > 0:
            conversion_result = meta_args_conv.comparePlusAndPlus(
                aConversionParms.plusargs, aConversionParms.cmp_plusargs
            )
        else:
            conversion_result = meta_args_conv.compareMetaAndPlus(
                meta_args, aConversionParms.cmp_plusargs
            )
    else:
        conversion_result = meta_args_conv.convertRawResult(meta_args, True)
    print(conversion_result)


class CommandLineParameters(object):
    usage = (
        """
      Convert meta args to plusargs.
    
      Example:
    
        %s -m "mp_hack=on mp_hack_debug=on"
    """
        % sys.argv[0]
    )

    # save and pass on remainder
    pass_remainder = True

    # do not allow abbrev parameters, only in Python >3.5
    # allow_abbrev = False

    parameters = [
        # "short option"     "number of additonal args"
        # |      "long option"    |   "additional specifications"
        # |      |                |   |
        # |      |                |   |
        ["-m", "--meta-args", 1, {}, "specify meta args"],
        ["-r", "--rtl-root", 1, {}, "specify RTL root directory"],
        [
            "-n",
            "--net",
            0,
            {"action": "store_true"},
            "convert meta args to net result.",
        ],
        [
            "-c",
            "--cmp-plusargs",
            1,
            {"default": ""},
            "compare plusargs with meta args or plusargs.",
        ],
        [
            "-p",
            "--plusargs",
            1,
            {},
            "specify plusargs for comparison with plusargs.",
        ],
        # -h and --help is not needed, proved by default.
    ]


if __name__ == "__main__":
    cmd_line_parser = CmdLineParser(CommandLineParameters, add_help=True)
    args = cmd_line_parser.parse_args(sys.argv[1:])
    conversion_parms = AttributeContainer()
    cmd_line_parser.set_parameters(conversion_parms)
    convert_meta_args(conversion_parms)
