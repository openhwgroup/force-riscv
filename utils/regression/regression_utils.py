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

def get_unsupported_list(suite_path):
    unsupported_file = suite_path + "/unsupported_tests.txt"
    unsupported_list = list()
    if os.path.isfile(unsupported_file):
        with open(unsupported_file, "r") as unsupported_handle:
            for line in unsupported_handle:
                line = line[:-1]
                unsupported_list.append(line)

    return unsupported_list

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
