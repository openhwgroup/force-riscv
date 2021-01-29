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

import getopt, sys
import os
import os.path

def usage(extra=None):
    usage_str = """Run quick regression
  --no-asm indicate that FORCE should not print out assembly code output.
  --unit-only indicate only running unit tests.
  --test-only indicate only running one force test
  --suite-only indicate only running force test suite.
  --force-path point to force path.
  -d, --dir test directory path
  -h, --help print this help message
  -n, --num-instr instruction count where fail occurs
  -s, --sum-level summary level for simulations, 0 = silent, 1 = instruction overrun only, 2 - all fails, 3 - everything
  -t, --test test case

Example:
%s --force-path force-gen-path 
""" % sys.argv[0]
    print(usage_str)
    if extra:
        print(extra)

def quick_run():

    try:
        my_oplist = [ "help"        \
                    , "no-asm"      \
                    , "unit-only"   \
                    , "test-only"   \
                    , "suite-only"  \
                    , "force-path=" \
                    , "dir="        \
                    , "test="       \
                    , "num-instr="  \
                    , "sum-level="  \
                    ]

        opts, args = getopt.getopt(sys.argv[1:], "hd:t:n:s:", my_oplist)

    except getopt.GetoptError as err:
        print ("\nERROR: " + str(err) + "\n")
        # usage()
        print ( "Please Run \"" + sys.argv[0] + " -h or --help\" for usage information\n")
        sys.exit(1)

    # change according to the desired default, if not passed will use this
    def_sum_level = 1

    force_path = None
    test_dir = None
    test_case = None
    run_unit = True
    run_test = False
    run_suite = False
    asm_output = True
    num_instr = 1000
    sum_level = def_sum_level

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o == "--no-asm":
            asm_output = False
        elif o == "--unit-only":
            run_sim = False
        elif o == "--test-only":
            run_unit = False
            run_test = True
        elif o == "--suite-only":
            run_unit = False
            run_suite = True
        elif o == "--force-path":
            force_path = a
        elif o in ["-d", "--dir"]:
            test_dir = a
        elif o in ["-t", "--test"]:
            test_case = a
        elif o in ["-n", "--num-instr"]:
            num_instr = a
        elif o in ["-s", "--sum-level"]:
            sum_level = a
        else:
            assert False, "Unhandled option."

    if not force_path:
        try:
            force_path = os.environ["FORCE_PATH"]
        except KeyError as ke:
            force_path = "."
            print("Default FORCE_PATH=\".\".")

    force_path = os.path.abspath(force_path)
    from regression_utils import verify_force_path, run_unit_tests, run_test_suite, run_direct_test
    force_exe, unit_path = verify_force_path(force_path)

    if run_unit:
        run_unit_tests(unit_path)

    if run_test:
        if not test_case:
            test_case = "tests/basic/all_instructions_force.py"
            print("Default test case = \"%s\"" % test_case)
        if test_dir is None:
            test_dir = ""
        else:
            if not test_dir.endswith("/"):
                test_dir += "/"
        run_direct_test(test_dir + test_case, force_exe, asm_output)

    if run_suite:

        # Verbosity of output to screen during summerize operations
        if sum_level not in [ 0, 1, 2 ]:
            sum_level = def_sum_level

        print ( sys.path )

        if not test_dir:
            test_dir = "tests/basic"
            print("Default test dir = \"%s\"" % test_dir)

        sys.path.append( force_path + "/utils/" )
        from regression.quick_summary import QuickSummary
        test_summ = QuickSummary( test_dir, num_instr - 1 )

        run_test_suite(test_dir, force_exe, asm_output, num_instr, test_summ )
        test_summ.summarize( sum_level )

if __name__ == "__main__":
    quick_run()



