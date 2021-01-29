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

import getopt, sys
import os

def usage():
    usage_str = """Starting a plugin source directory.  Run the script inside Force/fpix/plugins/template directory.
Example:
%s -n PLUGIN_NAME
A directory called PLUGIN_NAME will be created under Force/fpix/plugins/src.""" % sys.argv[0]
    print(usage_str)

def start_plugin():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hn:", ["help", "name="])
    except getopt.GetoptError as err:
        print (err)
        usage()
        sys.exit(1)

    plugin_name = None
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-n", "--name"):
            plugin_name = a
        else:
            assert False, "Unhandled option."

    if not plugin_name:
        usage()
        sys.exit()

    check_location()
    start_plugin_with_name(plugin_name)

def check_dir(dir_name):
    if (not os.path.exists(dir_name)) or (not os.path.isdir(dir_name)):
        print ("Not in Force/fpix/plugins/template directory.")
        sys.exit(1)

def check_location():
    check_dir("../../plugins")
    check_dir("../template")
    

def start_plugin_with_name(plugin_name):
    plugin_path = "../src/" + plugin_name
    if os.path.exists(plugin_path):
        print ("Test path already exist: %s." % plugin_path)
        sys.exit(1)

    os.mkdir(plugin_path)
    os.system("cp Makefile.template %s/Makefile" % plugin_path)
    os.system("cp plugin.cc.template %s/%s.cc" % (plugin_path, plugin_name))
    os.system("cp Makefile.target.template %s/Makefile.target" % plugin_path)
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
    os.system("mv %s.bak %s" % (file_path, file_path))

if __name__ == "__main__":
    start_plugin()
