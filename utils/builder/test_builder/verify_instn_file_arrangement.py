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

import ast as arrayParser
import os
import subprocess
from glob import glob

from builder_init import the_force_path


# PURPOSE:

# This file can be used to verify any changes made to how test_builder.py
# arranges instructions within the various *_force.py files (inside
# Force/tests/instructions). For example, there are currently two modes of
# arrangements supported -- arranging by instruction format, and arranging
# instructions in batches of certain sizes. This file currently runs both
# arrangements, and then iterates down every created directory, reads all
# instructions assigned to each *_force.py file, and compares them. This
# ensures that the specific instructions themselves are categorized properly
# for both arrangments (i.e. gen-only instructions are put under the
# generate/subfolder, etc.), even if they're now associated with differently
#  named *_force.py files, or shuffled between various *_force.py files. So, if
# any changes are made to the test_builder.py script -- to either introduce a
# new arrangment scheme or to change any existing schemes -- this file can be
# used to ensure that the specific instructions themselves are categorized
# the exact same under both schemes.

# TWEAKING:

# -CHANGING COMPARISIONS-
# If you'd like to make changes to how this file approves comparisions, please
# make changes to the __eq__ operator for ParseFiles class. The rest of the
# class just deals with parsing the specific instructions out of the various
# *_force.py files while still keeping track of what subdirectory path they
# were present in.

# -CHANGING OTHER CORE FUNCTIONS-
# If you'd like to make changes to that behavior, you'd probably have to dive
# into the rest of the class. To understand what's happening, I'd recommend
# starting with main, and then diving into the run() function. The rest should
# easily branch out of there.

# -REFLECTING MODIFICATIONS TO *_FORCE.PY FILES-
# If the structure of the *_force.py files has changed, such that the lexer
# logic needs to be tweaked. Please modify the sentinel variables to identify
# where the start of the array is, and where the end of the array is.

# Parses all the instruction files and returns a tuple of three items.
# 1. The first item in the tuple is the list of directories present under
#    tests/instructions.
# 2. The second item is the dict of force files under each directory. This is
#    a dictionary; please use the various values in the first list (i.e. the
#    list of directories) to get back the list of force files in that
#    directory.
# 3. The third item is the dict of instructions found within each force file.
#    This is a dictionary. Please use the various values in the first list
#    (i.e. list of directories ) to get back the list of force files, and use
#    the various force file names in the second dict to access the instructions
#    stored in that force file.

# This structure allows us to map the path where exactly a specific
# instruction has fallen under in a flexible fashion.

# Comparision __eq__ operator compares if instructions filed under the various
# directories are the same. Ignores force file
# naming conventions.
class ParseFiles:
    # Constants
    INSTRUCTIONS_DIR_PATH = "tests/instructions/"
    FORCE_FILE_START_SENTINEL = "for instr in "
    FORCE_FILE_END_SENTINEL = "]:"

    # We need to use this very often, so cache it to avoid recalculating
    # length each time
    FORCE_FILE_START_SENTINEL_LEN = len(FORCE_FILE_START_SENTINEL)
    FORCE_FILE_END_SENTINEL_LEN = len(FORCE_FILE_END_SENTINEL) - 1

    run_completed = False

    def __init__(self, path_to_force_dir):
        isntn_path = self.include_trailing_path_delimiter(path_to_force_dir)

        # Navigate into the tests directory
        os.chdir(isntn_path + self.INSTRUCTIONS_DIR_PATH)

        # Build the directory mappings
        self.test_dirs = [x[0] for x in os.walk(os.getcwd())]
        self.master_instns_in_force_file_dict = None
        self.master_force_files_dict = None

    def run(self):
        # Build the list hierarchy to load the instruction names into
        self.constructDirMappings()

        # Parse all the _force.py files in each directory and load it into the
        # respective dictionary, associating it with its parent directory.
        self.buildListInformation()

        # Parse all the files in the list, and get the instruction
        # information, associating it with its parent _force.py file.
        self.parseAllFiles()

        # For comparisions, we set the flag to indicate that this object is
        # ready to be compared
        self.run_completed = True

        # Return the data in pre-established format
        # 1. self.test_dirs contains all the directory names present in the
        #    instruction file
        # 2. self.master_force_files_dict uses a full directory path as a key
        #    to return all *_force.py files in that dict. ideally, we'd use the
        #    various values in self.test_dirs to index into this dict.
        # 3. self.master_instns_in_force_file_dict uses the full force file
        #    path to return all instructions in that force file. ideally, we'd
        #    use the various values in self.test_dirs to get all the
        #    force_files in that directory, and use each of those force file
        #    paths to get the list of instructions in that force file.
        return (
            self.test_dirs,
            self.master_force_files_dict,
            self.master_instns_in_force_file_dict,
        )

    # Utility Functions
    def parseAllFiles(self):
        self.master_instns_in_force_file_dict = dict()

        for dirName in self.test_dirs:
            force_files_in_this_dir = self.master_force_files_dict[dirName]

            for force_file in force_files_in_this_dir:
                # Parse the force file and get just the instructions
                try:
                    all_instns = self.parseFileIntoArray(open(force_file, "r"))
                    self.master_instns_in_force_file_dict[
                        force_file
                    ] = all_instns
                except LookupError:
                    print(
                        "ALERT: %s was improperly formatted. Ignoring this "
                        "file. " % (force_file)
                    )
                except IOError:
                    print(
                        "ALERT: %s could not be accessed. Please check "
                        "permissions." % (force_file)
                    )

    def parseFileIntoArray(self, hFile):
        # Read entire file into string
        file_data = hFile.read().replace("\n", "")

        # Find the location of the array of instructions
        instn_array_index = file_data.find(self.FORCE_FILE_START_SENTINEL)
        if instn_array_index == -1:
            raise LookupError("Force file improperly formatted.")

        # Skip over to the actual instn array
        instn_array_start_index = (
            instn_array_index + self.FORCE_FILE_START_SENTINEL_LEN
        )

        # Find the end of the instn array
        file_data = file_data[instn_array_start_index:]
        instn_array_index = file_data.find(self.FORCE_FILE_END_SENTINEL)
        if instn_array_index == -1:
            raise LookupError("Force file improperly formatted.")

        # Get the instn array only
        instn_array_end_index = (
            instn_array_index + self.FORCE_FILE_END_SENTINEL_LEN
        )
        instn_array_str = file_data[:instn_array_end_index]

        instn_array_str = instn_array_str.replace(" ", "").replace("\t", "")

        return arrayParser.literal_eval(instn_array_str)

    def buildListInformation(self):
        for dirName in self.test_dirs:
            # Gather all the files ending with _force.py in this folder
            os.chdir("%s" % (dirName))
            print("Parsing %s" % dirName)

            self.master_force_files_dict[dirName] = glob(
                os.getcwd() + "/*_force.py"
            )

    def constructDirMappings(self):
        self.master_force_files_dict = dict()
        for dirName in self.test_dirs:
            self.master_force_files_dict[dirName] = list()

    def include_trailing_path_delimiter(self, arg_path):
        arg_path = str(arg_path)
        if not arg_path.endswith("/"):
            arg_path += "/"
        return arg_path

    def __eq__(self, other):
        print("Running comparisions:")
        if not self.run_completed or not other.run_completed:
            raise ValueError(
                "ERROR: run() was not executed on this object. Please run "
                ".run() before trying to compare."
            )

        if sorted(self.test_dirs) != sorted(other.test_dirs):
            return False

        print("Directory organization looks correct.")
        # We can't compare file names of force files since the naming
        # convention is different. Instead, we can just append all instns in
        # every file into one array and compare for every directory level.
        for dirName in self.test_dirs:
            all_instns_in_this_dir_for_this = list()
            all_instns_in_this_dir_for_other = list()

            force_files_in_this_dir = self.master_force_files_dict[dirName]
            for force_file in force_files_in_this_dir:
                all_instns_in_this_dir_for_this.extend(
                    self.master_instns_in_force_file_dict[force_file]
                )

            force_files_in_other_dir = other.master_force_files_dict[dirName]
            for force_file in force_files_in_other_dir:
                all_instns_in_this_dir_for_other.extend(
                    other.master_instns_in_force_file_dict[force_file]
                )

            if sorted(all_instns_in_this_dir_for_this) != sorted(
                all_instns_in_this_dir_for_other
            ):
                return False

        print(
            "Organization of instructions within the directory structure "
            "looks correct."
        )
        return True


if __name__ == "__main__":
    # Do make new
    os.chdir(the_force_path)
    subprocess.run(["make", "tests_old"])

    # Parse the results
    parseNew = ParseFiles(the_force_path)
    parseNew.run()

    # Do make old
    os.chdir(the_force_path)
    subprocess.run(["make", "tests"])

    # Parse the results
    parseOld = ParseFiles(the_force_path)
    parseOld.run()

    # Compare instructions
    if parseNew == parseOld:
        print("All looks correct.")
    else:
        print("Error. Instructions are not the same.")
