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
import glob
import os
import os.path
import re
import shlex
import shutil
import stat
import subprocess
import sys
import tarfile


def get_svn_path():
    if "FORCE_SVN_PATH" not in os.environ:
        print("ERROR: FORCE_SVN_PATH environment variable not specified.")
        sys.exit(1)

    svn_path = os.environ["FORCE_SVN_PATH"]
    print("FORCE SVN path is: %s" % svn_path)
    return svn_path


def get_versions(rev_str):
    vers = rev_str.split(":")
    return (vers[0], vers[1])


def validate_versions(ver1_str, ver2_str, latest_ver):
    ver1, ver2 = 0, 0
    try:
        if ver2_str == "top":
            ver2 = latest_ver
        else:
            ver2 = int(ver2_str)
        ver1 = int(ver1_str)
    except ValueError:
        print("ERROR: Invalid version inputs")
        print("-r usage: num1:num2 or num1:top")
        sys.exit()

    return ver1, ver2


def get_latest_version(svn_path):
    version = None
    result = subprocess.check_output(["svn", "info", svn_path]).decode("utf-8")
    for line in result.splitlines():
        if "Revision" in line:
            version = line.split(":")[1]

    return int(version)


def create_diff_dir(ver1, ver2):
    diff_dir = "diff-%d-%d" % (ver1, ver2)
    if os.path.exists(diff_dir):
        print("Diff directory %s already exists." % diff_dir)
        sys.exit()

    os.mkdir(diff_dir)

    return diff_dir


def remove_dot_svn_dirs(dir_name):
    for (root, subdir_names, file_names) in os.walk(dir_name):
        for subdir_name in subdir_names:
            if subdir_name == ".svn":
                shutil.rmtree(os.path.join(root, subdir_name))


def copy_code_from_dir(source_dir, base_name, ver):
    dest_dir = "%s-%d" % (base_name, ver)
    shutil.copy2(source_dir, dest_dir)
    return dest_dir


def check_out_revision(svn_path, ver, base_name, clean_up=False):
    svn_command = "svn co " + svn_path
    if ver:
        svn_command += " -r %d" % ver
        if clean_up:
            dir_name = "%s-%d" % (base_name, ver)
        else:
            dir_name = base_name
    else:
        dir_name = base_name

    svn_command += " %s" % dir_name

    subprocess.run(shlex.split(svn_command))

    # remove .svn directories, to facilitate diff file making
    if clean_up:
        remove_dot_svn_dirs(dir_name)

    return dir_name


def create_diff_file(dir_name1, dir_name2, diff_file_name):
    diff_cmd = "diff -ruN %s %s > %s" % (dir_name1, dir_name2, diff_file_name)
    print(diff_cmd)
    subprocess.run(shlex.split(diff_cmd))
    print("Created diff file: %s." % diff_file_name)


def create_merge_dir(base_name, rev):
    merge_dir = "%s-merge" % (base_name)
    if rev:
        merge_dir += "-%d" % rev

    if os.path.exists(merge_dir):
        print("Merge directory %s already exists." % merge_dir)
        sys.exit()

    os.mkdir(merge_dir)

    return merge_dir


def apply_patch_file(diff_file):
    patch_cmd = "patch -p1 < %s" % diff_file
    print("Executing: %s" % patch_cmd)
    subprocess.run(shlex.split(patch_cmd))


def is_svn_separator_line(line):
    for char in line:
        if char != "-":
            return False

    if len(line) < 3:
        return False

    return True


def parse_svn_author_line(line):
    parts = line.split("|")
    if len(parts) != 4:
        print("Not auther line: %s" % line)
        sys.exit(1)

    # parse out svn version
    version_raw = parts[0].strip()
    match = re.match(r"r([0-9]+)", version_raw)
    if match is None:
        print(
            'svn authro line mal-formed: %s, not finding version in "%s"'
            % (line, version_raw)
        )
        sys.exit()
    svn_version = int(match.group(1))

    # parse out author and time
    author = parts[1].strip()
    time_whole = parts[2].strip()
    time_pieces = time_whole.split(" ")
    time_str = time_pieces[0]
    return svn_version, author, time_str


def get_svn_log(ver1, ver2, svn_path, log_file_name):
    log_file_handle = open(log_file_name, "w")
    log_file_handle.write("[MERGE] versions %d:%d\n" % (ver1, ver2))
    log_cmd = ["svn", "log", svn_path, ("-r%d:%d" % (ver2, ver1))]
    ps = subprocess.Popen(
        log_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    output = ps.communicate()[0]
    log_str = str(output, "utf-8")
    lines = log_str.split("\n")
    last_is_separator = False

    from collections import OrderedDict

    log_dict = OrderedDict()
    tmp_key = ""
    tmp_line = ""
    # print("version 1 %d, version 2 %d" % (ver1, ver2))

    for line in lines:
        # print ("line is %s" % line)
        if len(line) == 0:
            continue
        if is_svn_separator_line(line):
            last_is_separator = True
            # add previous item to the list
            if tmp_key:
                if tmp_key in log_dict:
                    log_dict[tmp_key] += tmp_line
                else:
                    log_dict[tmp_key] = tmp_line
            tmp_key = ""
            tmp_line = ""
            continue
        if last_is_separator:
            svn_version, author, time = parse_svn_author_line(line)
            last_is_separator = False
            if svn_version <= ver1:
                break  # svn log done
            item = author + "@" + time
            tmp_key = item
            continue
        if not tmp_line:
            tmp_line = line + "\n"
        else:
            tmp_line += line + "\n"
    for k, v in log_dict.items():
        log_file_handle.write("%s\n" % k)
        log_file_handle.write("%s" % v)

    log_file_handle.close()


def separate_path_top(full_file_name):
    slash_pos = full_file_name.find("/")
    if slash_pos == -1:
        print(
            'File name in diff file not in expected format: "%s".'
            % full_file_name
        )
        sys.exit()

    dir_name = full_file_name[:slash_pos]
    file_name = full_file_name[slash_pos + 1 :]

    return dir_name, file_name


def parse_diff_minus_plus_line(line):
    line_parts = line.split(" ")
    full_file_name = line_parts[1]
    tab_pos = full_file_name.find("\t")
    full_file_name = full_file_name[:tab_pos]
    full_file_name = full_file_name.strip()

    return separate_path_top(full_file_name)


def create_special_file_lists(diff_file_name, exe_list_name, bin_list_name):
    diff_handle = open(diff_file_name, "r", encoding="utf-8")
    exe_handle = open(exe_list_name, "w")
    bin_handle = open(bin_list_name, "w")

    last_line_minus = False
    minus_dir_name = None
    minus_file_name = None
    last_line_plus = False
    plus_dir_name = None
    plus_file_name = None
    binary_count = 0
    line_num = 0

    for line in diff_handle:
        # print ("gotten line %d" % line_num)
        line_num += 1
        line = line[:-1]
        if line.find("--- ") == 0:
            minus_dir_name, minus_file_name = parse_diff_minus_plus_line(line)
            last_line_minus = True
            continue
        if last_line_minus:
            last_line_minus = False
            if line.find("+++ ") != 0:
                print(
                    "Expecting to see plus line after minus line in "
                    'diff file, but getting "%s".' % line
                )
                sys.exit()
            plus_dir_name, plus_file_name = parse_diff_minus_plus_line(line)
            if plus_file_name != minus_file_name:
                print(
                    "Expecting minus and plus line has the same file name, "
                    'but getting "%s" and "%s".'
                    % (minus_file_name, plus_file_name)
                )
                sys.exit()
            last_line_plus = True
            continue
        if last_line_plus:
            last_line_plus = False
            if line.find("@@") != 0:
                print(
                    "Expect line count difference after plus line in diff "
                    'file, but getting "%s".' % line
                )
                sys.exit()
            if line.find("@@ -0,0 ") == 0:
                # new file
                file_path = plus_dir_name + "/" + plus_file_name
                if os.access(file_path, os.X_OK):
                    exe_handle.write(plus_file_name + "\n")
            continue
        if line.find("Binary files ") == 0:
            bin_line_parts = line.split(" ")
            if len(bin_line_parts) != 6:
                print(
                    "Expecting binary file diff line to have 6 parts, the "
                    'line is "%s".' % line
                )
                sys.exit()
            bin_file_to_copy = bin_line_parts[-2]
            bin_dir, bin_file_name = separate_path_top(bin_file_to_copy)
            bin_handle.write(bin_file_name + "\n")
            bin_name = bin_list_name + ".binary%d" % binary_count
            binary_count += 1
            shutil.move(bin_file_to_copy, bin_name)

    diff_handle.close()
    exe_handle.close()
    bin_handle.close()


def tar_up(base_name, dir_name=None):
    tar_file = base_name + ".tar.gz"
    with tarfile.open(tar_file, "w:gz") as tar:
        for file_name in glob.glob("%s.*" % base_name):
            tar.add(file_name)

        if dir_name:
            tar.add(dir_name)

    shutil.move(tar_file, "..")


def chmod_exe_files(exe_list):
    with open(exe_list) as list_handle:
        for line in list_handle:
            if len(line) == 0:
                continue
            line = line[:-1]

            # Add user execute permission
            stat_info = os.stat(line)
            os.chmod(line, (stat_info.st_mode | stat.S.IXUSR))


def copy_source_tree(from_path, to_path):
    try:
        if os.path.exists(to_path):
            shutil.rmtree(to_path)
        shutil.copytree(from_path, to_path)
    except BaseException:
        print(
            "ERROR - Copy Failed, source[%s], Remove the dest[%s], and "
            "confirm source then retry ....",
            (from_path, to_path),
        )
        sys.exit()


def copy_binary_files(binary_files_list):
    binary_count = 0
    with open(binary_files_list) as list_handle:
        for line in list_handle:
            if len(line) == 0:
                continue
            line = line[:-1]
            binary_file_source = binary_files_list + ".binary%d" % binary_count
            shutil.move(binary_file_source, line)
            binary_count += 1
