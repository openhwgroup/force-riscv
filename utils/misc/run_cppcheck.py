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
import os
import subprocess
import sys


def setupArguments():
    parser = argparse.ArgumentParser(
        description="A script to run CppCheck",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument(
        "path",
        nargs="*",
        help="Usage: 'FORCE_ROOT/utils/misc/run_cppcheck.py [path to file]'\n"
        "Defaults to ./\nCan also be 'svn' to run against each added, "
        "replaced, or modified file",
        default=["."],
    )
    # parser.add_argument('--addon_path', nargs='*', help='path(s) to
    # script(s) to run on Cppcheck dump file')
    parser.add_argument(
        "--xml",
        nargs="?",
        help="Displays XML output\nDefaults to 'No'\nUseful for getting "
        "error id (for suppression)",
        default="No",
    )
    parser.add_argument(
        "--inline",
        nargs="?",
        help="Suppresses findings for things marked with '// "
        "cppcheck-suppress' in source\nDefaults to 'Yes'",
        default="Yes",
    )
    return parser


def parseSvnStatus():
    paths = []

    # subprocess.check_output returns a byte string: b'some text here'
    # therefore it needs a little formatting before processing
    raw_svn_status = subprocess.check_output(["svn", "status"])
    raw_svn_status = raw_svn_status.decode()
    raw_svn_status = raw_svn_status[
        : len(raw_svn_status) - 1
    ]  # stripping away last \n character

    for line in raw_svn_status.split("\n"):
        contents = line.split()
        # only check files that are [A]dded, [M]odified, or [R]eplaced
        if contents[0] == "A" or contents[0] == "M" or contents[0] == "R":
            if "unit_tests" not in contents[1] and "py" not in contents[1]:
                paths.append(contents[1])

    return (".", paths)


def runCppcheck(args):
    # build list for subprocess.call
    cppcheck_path = os.environ["CPPCHECK_BIN"]
    cmd_line = [cppcheck_path, "--language=c++", "--std=c++11"]

    # root is force root directory
    # paths is list of files (or the two src/ folders)
    root = args.path[0]
    paths = [
        os.path.join(args.path[0], "base", "src"),
        os.path.join(args.path[0], "riscv", "src"),
    ]

    if args.path[0].casefold() == "svn".casefold():  # svn
        # from CppCheck manual: For historical reasons, --enable=style enables
        # warning, performance, portability, and style messages.
        cmd_line.append("--enable=style")
        (root, paths) = parseSvnStatus()
    elif args.path[0].endswith(".cc") or args.path[0].endswith(
        ".h"
    ):  # file(s)
        # from CppCheck manual: For historical reasons, --enable=style enables
        # warning, performance, portability, and style messages.
        cmd_line.append("--enable=style")
        if ("base" + os.path.sep) in args.path[0]:
            root = args.path[0][: args.path[0].find("base" + os.path.sep)]
        elif ("riscv" + os.path.sep) in args.path[0]:
            root = args.path[0][: args.path[0].find("riscv" + os.path.sep)]
        paths = []
        for file_path in args.path:
            paths.append(file_path)
    else:  # FORCE path
        cmd_line.append("--enable=all")

    # adding include directories
    cmd_line.extend(["-I", os.path.join(root, "base", "inc")])
    cmd_line.extend(["-I", os.path.join(root, "riscv", "inc")])

    # suppression file (for generated files and the like)
    cmd_line.append(
        "--suppress-xml="
        + os.path.join(root, "utils", "misc", "cppcheck_suppress.xml")
    )

    # additional minor arguments
    if args.xml != "No":
        cmd_line.append("--xml")
    if args.inline == "Yes":
        cmd_line.append("--inline-suppr")

    # adding paths (or files) to check
    for path in paths:
        cmd_line.append(path)

    subprocess.call(cmd_line)


if __name__ == "__main__":
    if "CPPCHECK_BIN" not in os.environ:
        print(
            "Please set the CPPCHECK_BIN environment variable to point to "
            "Cppcheck."
        )
        sys.exit(1)

    parser = setupArguments()
    args = parser.parse_args()
    runCppcheck(args)
