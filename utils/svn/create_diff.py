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
import os.path

def usage():
    usage_str = """Create svn diff files to sync between different builds.
  -r, --revision specify the revision range to create diff.
  -k, --keep indicates to keep the diff directory or not.
  -f, --full also tar in full source tree.
  -h, --help print this help message.
  -d, --dir provide the code directory to be compared (version is given in -r)
Example:
%s -z green -r 1:11
""" % sys.argv[0]
    print(usage_str)

def create_diff():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hz:r:kfd:", ["help", "revision=", "keep", "full", "dir="])
    except getopt.GetoptError as err:
        print (err)
        usage()
        sys.exit(1)

    rev_str = None
    keep_dir = False
    full_tree = False
    compare_dir = None
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-r", "--revision"):
            rev_str = a
        elif o in ("-k", "--keep"):
            keep_dir = True
        elif o in ("-f", "--full"):
            full_tree = True
        elif o in ("-d", "--dir"):
            compare_dir = a.rstrip("/")            
            print(compare_dir)
        else:
            assert False, "Unhandled option."

    if not rev_str:
        usage()
        sys.exit()

    from svn_utils import create_diff_file, create_diff_dir, check_out_revision, get_svn_path, get_versions, get_svn_log, tar_up, remove_dot_svn_dirs, create_special_file_lists, get_latest_version, validate_versions, copy_code_from_dir
    svn_path = get_svn_path()
    ver1_str, ver2_str = get_versions(rev_str)    
    latest_ver = get_latest_version(svn_path)
    ver1, ver2 = validate_versions (ver1_str, ver2_str, latest_ver)
       
    if compare_dir is not None and os.path.isdir(compare_dir) == False:
        print("Incorrect given directory to be compared.")
        sys.exit()
    
    if ver1 > ver2:
        print("Expecting version number in ascending order.")
        sys.exit()

    output_basename = "Force-%d-%d" % (ver1, ver2)
    diff_file_name = output_basename + ".diff"
    exe_list_name = output_basename + ".exe"
    bin_list_name = output_basename + ".bin"

    diff_dir = create_diff_dir(ver1, ver2)
    
    cur_dir = os.getcwd()
    os.chdir(diff_dir)
    
    # Note: assume given directory to be compared always refer to ver1
    if compare_dir is not None:
        dir_name1 = copy_code_from_dir (compare_dir, "Force", ver1)
        print("Using copied directory %s for version: %d" % (dir_name1, ver1))
        remove_dot_svn_dirs(dir_name1)        
    else:    
        dir_name1 = check_out_revision(svn_path, ver1, "Force", True)
    dir_name2 = check_out_revision(svn_path, ver2, "Force-%d" % ver2, False)

    # Gather SVN log from the second directory
    log_file_name = output_basename + ".log"
    get_svn_log(ver1, ver2, svn_path, log_file_name)

    remove_dot_svn_dirs(dir_name2)

    create_diff_file(dir_name1, dir_name2, diff_file_name)
    create_special_file_lists(diff_file_name, exe_list_name, bin_list_name)

    if full_tree:
        tar_up(output_basename, dir_name2)
    else:
        tar_up(output_basename)

    os.chdir(cur_dir)

    if not keep_dir:
        import shutil
        shutil.rmtree(diff_dir)

if __name__ == "__main__":
    create_diff()
