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
import os
import os.path
import sys
import subprocess

def verify_dir(dir_path):
    if not os.path.isdir(dir_path):
        print("Expecting \"%s\" to be a directory." % dir_path)
        sys.exit()

def verify_file(file_path):
    if not os.path.exists(file_path):
        print("File %s does not exist." % file_path)
        sys.exit()

    if not os.path.isfile(file_path):
        print("This is not a file \"%s\"." % file_path)
        sys.exit()

def verify_executable(exe_file):
    verify_file(exe_file)
    if not os.access(exe_file, os.X_OK):
        print("This is not an executable \"%s\"." % file_path)
        sys.exit()

def verify_force_path(force_path):
    verify_dir(force_path)

    force_exe = force_path + "/bin/friscv"
    verify_executable(force_exe)

    unit_path = force_path + "/unit_tests/tests"
    verify_dir(unit_path)
    return force_exe, unit_path

def verify_dir_writable(dir_path):
    if not os.access(dir_path, os.W_OK):
        print("This path is not writable \"%s\"." % dir_path)
        sys.exit()

def execute_command(cmd, print_it=False, test_summ = None):
    if print_it:
        print("Executing: %s" % cmd)
    ret_code = os.system(cmd)
    # test_summ will exist if rumming in suite mode
    if test_summ:
        test_summ.add_result( ret_code )
        # print ("Failed executing %s, Continue to Next Test" % cmd)

    elif ret_code != 0:
        print ("Failed executing %s, Terminating ..." % cmd)
        sys.exit()

def run_one_unit_test(upath, module_name):
    cur_path = os.getcwd()
    os.chdir(upath)
    execute_command("make clean")
    execute_command("make")
    execute_command("bin/%s_test" % module_name)
    os.chdir(cur_path)

def run_unit_tests(unit_path):
    verify_dir_writable(unit_path)
    for sub_path in os.listdir(unit_path):
        if sub_path != ".svn":
            unit_test_path = unit_path + "/" + sub_path
            run_one_unit_test(unit_test_path, sub_path)

def get_unsupported_list(suite_path):
    unsupported_file = suite_path + "/unsupported_tests.txt"
    unsupported_list = list()
    if os.path.isfile(unsupported_file):
        with open(unsupported_file, "r") as unsupported_handle:
            for line in unsupported_handle:
                line = line[:-1]
                unsupported_list.append(line)

    return unsupported_list

def is_green_zone():
    output = subprocess.check_output(["uname", "-n"])
    log_str = str(output, "utf-8")
    if log_str.find("SAN") == 0:
        print ("Running on green zone machine %s" % log_str)
        return True
    return False

def adjust_ld_library_path():
    if True:
        gcc_std_path = "/software/public/gcc/5.1/centos6.6/lib64"
        try:
            existing_ld_library_path = os.environ["LD_LIBRARY_PATH"]
            new_ld_library_path = existing_ld_library_path
            if existing_ld_library_path.find(gcc_std_path) == -1:
                new_ld_library_path = gcc_std_path + ":" + new_ld_library_path
        except KeyError as ke:
            new_ld_library_path = gcc_std_path
        os.environ["LD_LIBRARY_PATH"] = new_ld_library_path

def run_test_suite(suite_path, force_exe, asm_output, num_instr=1000, test_summ = None ):
    verify_dir(suite_path)
    cur_dir = os.getcwd()

    suite_path = os.path.abspath(suite_path)
    os.chdir(suite_path)
    unsupported_list = get_unsupported_list(suite_path)
    for file_name in os.listdir(suite_path):
        if file_name.endswith("_force.py"):
            if not file_name in unsupported_list:
                test_file = suite_path + "/" + file_name
                force_cmd = force_exe + " -t " + test_file
                if not asm_output:
                    force_cmd += " --noasm"
                gen_log_name = file_name.replace(".py", ".gen.log")
                force_cmd += " >& " + gen_log_name
                execute_command(force_cmd, True)

    os.chdir(cur_dir)

def run_direct_test(test_case, force_exe, asm_output, num_instr=1000):
    force_cmd = force_exe + " -t " + test_case
    if not asm_output:
        force_cmd += " --noasm"
    gen_log_name = test_case.replace(".py", ".gen.log")
    force_cmd += " >& " + gen_log_name

    execute_command(force_cmd)


