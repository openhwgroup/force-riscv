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

import getopt
import os
import sys
import time
from regression_utils import verify_force_path, verify_dir_writable

from multiprocessing import Pool, Manager
from subprocess import Popen, PIPE, STDOUT
from functools import partial

def usage(extra=None):
    usage_str = """Run quick regression
  -h, --help print this help message
  --clean Run "make clean" in each unit test directory before running "make".
  --force-path point to force path.
  --nopicky disable -Weffc++ compiler flag.
  -x, --process-max The maximum number of concurrent execution threads.
  -z, --print-failures Print the stdout and stderr of each failing test case to the console.

Example:
%s --force-path force-gen-path
""" % sys.argv[0]
    print(usage_str)
    if extra:
        print(extra)


class ExecutionTrace:
    def __init__(self, arg_stdout, arg_retcode, arg_cmd, arg_time):
        self.stdout = arg_stdout
        self.retcode = arg_retcode
        self.cmd = arg_cmd
        self.execution_duration = arg_time


def unit_tests():

    try:
        my_oplist = [
            "help",
            "clean",
            "force-path=",
            "nopicky",
            "process-max=",
            "print-failures",
        ]

        (opts, args) = getopt.getopt(sys.argv[1:], "hx:z", my_oplist)

    except getopt.GetoptError as err:
        print ("\nERROR: " + str(err) + "\n")
        print ( "Please Run \"" + sys.argv[0] + " -h or --help\" for usage information\n")
        sys.exit(1)

    clean_build = False
    force_path = None
    process_max = 16
    print_fails = False
    nopicky = False

    for (o, a) in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit(0)
        elif o == "--clean":
            clean_build = True
        elif o == "--force-path":
            force_path = a
        elif o == "--nopicky":
            nopicky = True
        elif o in ["-x", "--process-max"]:
            process_max = int(a)
        elif o in ["-z", "--print-failures"]:
            print_fails = True
        else:
            assert False, "Unhandled option."

    if not force_path:
        try:
            force_path = os.environ["FORCE_PATH"]
        except KeyError as ke:
            force_path = "."
            print("Default FORCE_PATH=\".\".")

    force_path = os.path.abspath(force_path)
    (_, unit_tests_path) = verify_force_path(force_path)

    run_unit_tests(unit_tests_path, process_max, print_fails, nopicky, clean_build)

    sys.exit(0)


def run_unit_tests(unit_tests_path, num_parallel_workers, print_fails, nopicky, clean_build):
    verify_dir_writable(unit_tests_path)

    p = Pool(num_parallel_workers)

    arg_array = list()
    sharing_manager = Manager()
    job_output_queue = sharing_manager.list()

    for test_layer_dirname in os.listdir(unit_tests_path):
        test_layer_path = os.path.join(unit_tests_path, test_layer_dirname)

        if os.path.isdir(test_layer_path):
            for test_dirname in os.listdir(test_layer_path):
                test_path = os.path.join(test_layer_path, test_dirname)

                if os.path.isdir(test_path) and (test_dirname != ".svn"):
                    arg_tuple = (test_path, test_dirname, job_output_queue)
                    arg_array.append(arg_tuple)

    run_one_test = partial(run_one_unit_test, nopicky, clean_build)
    p.map(run_one_test, arg_array)
    p.close()
    p.join()

    num_fails = 0
    bufp = list()
    bufp.append ("The following tests have failed: \n")
    for (test_dirname, process_return) in job_output_queue:
        if (process_return.retcode != 0):
          num_fails += 1
          bufp.append("[%s]: Test: %s. Command: %s. Duration: %s seconds." % (str(num_fails), test_dirname, process_return.cmd, str(process_return.execution_duration)))
          if (print_fails):
            bufp.append(process_return.stdout)
            bufp.append("\n=======\n")

    if (num_fails == 0):
        print ("=====ALL SUCCESS!=====")
    else:
        print ("\n=====FAILURES:=====")
        for output in bufp:
            print (output)
        print ("%s failures.\n" % (num_fails))


def run_one_unit_test(nopicky, clean_build, arg_tuple):
    (test_path, test_dirname, execution_queue) = arg_tuple
    cur_path = os.getcwd()
    os.chdir(test_path)

    success = True
    if clean_build:
        cmd = "make clean"
        process_return = execute_command(cmd)
        success = (process_return.retcode == 0)

    make_duration = 0.0
    if success:
        cmd = "make OPTIMIZATION=-O0"
        if nopicky:
            cmd += " PICKY="
        process_return = execute_command(cmd)
        success = (process_return.retcode == 0)
        make_duration = process_return.execution_duration
        log_output(process_return.stdout, test_path, "make.log")

    run_duration = 0.0
    if success:
        cmd = "bin/%s_test" % test_dirname
        process_return = execute_command(cmd)
        success = (process_return.retcode == 0)
        run_duration = process_return.execution_duration
        log_output(process_return.stdout, test_path, "run.log")

    result_string = "SUCCESS." if success else "FAILED."

    print ("%s: %s (Make Duration: %0.4f sec; Run Duration: %0.4f sec)" % (test_dirname, result_string, make_duration, run_duration))

    # Whole append call is done in one GIL lock; list appends are thread safe, no need for special threaded queues
    execution_queue.append((test_dirname, process_return))
    os.chdir(cur_path)


def execute_command(cmd):
    arg = cmd.split(" ")
    # Measure the amount of time it took to run this test
    start_time = time.time()
    process = Popen(arg, stdout=PIPE, stderr=STDOUT)
    result = process.communicate()
    end_time = time.time()
    process_stdout = result[0].decode("utf-8")
    ret_code = process.returncode

    trace = ExecutionTrace(process_stdout, ret_code, cmd, end_time - start_time)
    return trace


def log_output(output, test_path, file_name):
    with open(os.path.join(test_path, "bin", file_name), "w") as log_file:
        log_file.write(output)


if __name__ == "__main__":
    unit_tests()
