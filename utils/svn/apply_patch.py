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
    usage_str = """Apply patch (diff) file from one build to another.
  -p, --patch specify the name of the patch file.
  -r, --revision specify the revision to check out
  -s, --source if specified apply patch uses the referenced patch as the source rather than the svn checkout latest
  -h, --help print this help message
  -t, --test test only and remove .svn directories
Example:
%s -z red -p abc.diff
""" % sys.argv[0]
    print(usage_str)

def apply_patch():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "htz:r:p:s:", ["help", "test", "patch=", "revision=", "source_dir="])
    except getopt.GetoptError as err:
        print (err)
        usage()
        sys.exit(1)

    patch_file = None
    source_dir = None
    rev = 0
    test_set = False
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-p", "--patch"):
            patch_file = a
        elif o in ("-r", "--revision"):
            rev = int(a)
        elif o in ("-s","--source"):
            source_dir = a
        elif o in ("-t", "--test"):
            test_set = True
        else:
            assert False, "Unhandled option."

    if not patch_file:
        usage()
        sys.exit()

    patch_file_path = os.path.abspath(patch_file)
    exe_list_path = patch_file_path.replace(".diff", ".exe")
    binary_list_path = patch_file_path.replace(".diff", ".bin")

    from svn_utils import create_merge_dir, check_out_revision, get_svn_path, apply_patch_file, chmod_exe_files, copy_binary_files, remove_dot_svn_dirs,copy_source_tree
    svn_path = get_svn_path()
    merge_dir = create_merge_dir("Force", rev)

    # if the source directory exists then that is used rather than extracting
    # a copy from the repository, otherwise extracts a copy into the merge dir
    if source_dir is None:
        check_out_revision(svn_path, rev, merge_dir)
    else:
        # copy the source directory into the merge directory
        copy_source_tree(source_dir, merge_dir)

    cur_dir = os.getcwd()
    os.chdir(merge_dir)

    apply_patch_file(patch_file_path)
    chmod_exe_files(exe_list_path)
    copy_binary_files(binary_list_path)

    os.chdir(cur_dir)

    if test_set:
        # remove .svn directories for test purpose (to compare the merged dir with target dir)
        remove_dot_svn_dirs(merge_dir)


if __name__ == "__main__":
    apply_patch()
