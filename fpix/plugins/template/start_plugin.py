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

import argparse
import getopt
import os
import shutil
import sys


def start_plugin():
    parser = argparse.ArgumentParser(
        description="Create a plugin source directory called PLUGIN_NAME "
        "within the fpix/plugins/src directory. Run the script inside the "
        "fpix/plugins/template directory."
    )
    parser.add_argument("-n", "--plugin_name", required=True)
    args = parser.parse_args()

    check_location()
    start_plugin_with_name(args.plugin_name)


def check_dir(dir_name):
    if (not os.path.exists(dir_name)) or (not os.path.isdir(dir_name)):
        print("Not in Force/fpix/plugins/template directory.")
        sys.exit(1)


def check_location():
    check_dir("../../plugins")
    check_dir("../template")


def start_plugin_with_name(plugin_name):
    plugin_path = "../src/" + plugin_name
    if os.path.exists(plugin_path):
        print("Test path already exist: %s." % plugin_path)
        sys.exit(1)

    os.mkdir(plugin_path)
    shutil.copyfile("Makefile.template", ("%s/Makefile" % plugin_path))
    shutil.copyfile(
        "plugin.cc.template", ("%s/%s.cc" % (plugin_path, plugin_name))
    )
    shutil.copyfile(
        "Makefile.target.template", ("%s/Makefile.target" % plugin_path)
    )
    replace_file_var("%s/Makefile.target" % plugin_path, plugin_name)


def replace_file_var(file_path, varval):
    file_handle = open(file_path, "r")
    bak_handle = open(file_path + ".bak", "w")

    for line in file_handle:
        if line.find("$PLUGIN_NAME") != -1:
            line = line.replace("$PLUGIN_NAME", varval)
        bak_handle.write(line)

    file_handle.close()
    bak_handle.close()
    shutil.move(("%s.bak" % file_path), file_path)


if __name__ == "__main__":
    start_plugin()
