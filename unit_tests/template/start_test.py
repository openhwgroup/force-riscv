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

def usage():
    usage_str = """Starting a unit test directory.  Run the script inside unit_tests directory.
Example:
%s -d TEST_DIR -t TEST_NAME
A directory called TEST_NAME will be created under unit_tests/tests/TEST_DIR.""" % sys.argv[0]
    print(usage_str)

def start_test():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hd:t:", ["help", "test-dir=", "test="])
    except getopt.GetoptError as err:
        print (err)
        usage()
        sys.exit(1)

    test_name = None
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-d", "--test-dir"):
            test_dir = a
        elif o in ("-t", "--test"):
            test_name = a
        else:
            assert False, "Unhandled option."

    if (not test_name) or (not test_dir):
        usage()
        sys.exit()

    check_location()
    start_test_with_name(test_dir, test_name)

def check_dir(dir_name):
    if (not os.path.exists(dir_name)) or (not os.path.isdir(dir_name)):
        print ("Not in Force/unit_tests directory.")
        sys.exit(1)

def check_location():
    check_dir("tests")
    check_dir("template")
    

def start_test_with_name(test_dir, test_name):
    test_path = os.path.join("tests", test_dir, test_name)
    if os.path.exists(test_path):
        print ("Test path already exist: %s." % test_path)
        sys.exit(1)

    os.mkdir(test_path)
    os.system("cp template/Makefile.template %s/Makefile" % test_path)
    os.system("cp template/test.cc.template %s/%s_test.cc" % (test_path, test_name))
    os.system("cp template/Makefile.target.template %s/Makefile.target" % test_path)
    replace_file_var("%s/Makefile.target" % test_path, test_name)

def replace_file_var(file_path, varval):
    file_handle = open(file_path, "r")
    bak_handle = open(file_path + ".bak", "w")

    for line in file_handle:
        if line.find("$TEST_NAME") != -1:
            line = line.replace("$TEST_NAME", varval)
        bak_handle.write(line)

    file_handle.close()
    bak_handle.close()
    os.system("mv %s.bak %s" % (file_path, file_path))

if __name__ == "__main__":
    start_test()
