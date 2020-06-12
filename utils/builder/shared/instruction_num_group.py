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
from shared.builder_exception import BuilderException

class IGroupByNumber(object):

    def __init__(self, grpSize, format_name):
        super(IGroupByNumber, self).__init__()

        # The name of the group should follow formatting "Grouping_X"
        self.name = format_name
        # The max number of instructions that this group will contain. Will show up in its name.
        self.size = grpSize
        # Instructions added to this group
        self.instructions = list()

    def add_instruction(self, instr):
        # If we try to add more instructions than we can handle, just raise an exception
        if ((len(self.instructions) + 1) > self.size):
            raise BuilderException("Adding instruction \"%s\" to full group \"%s\, with size \"%d\"." % (instr.name, self.name, self.size))
        # If there's still space, append the instruction to the end of the list
        self.instructions.append(instr)

    def sizeUsed(self):
        # Utilized by encapsulating class (InstructionNumGroup) to measure how full this grouping is
        return len(self.instructions)

    def maxSize(self):
        # Utilized by encapsulating class (InstructionNumGroup) to get this group's maximum capacity
        return self.size

    def write(self, file_handle):
        # Same logic as the corresponding function in InstructionFormatGroup in instruction_format_group.py
        file_handle.write("Size group \"%s\"\n" % self.name)
        for instr in self.instructions:
            file_handle.write(instr.get_full_ID() + "\n")

    def print_test(self, use_standard_template = True):
        # Same logic as the corresponding function in InstructionFormatGroup in instruction_format_group.py
        file_name = "T%d-" % len(self.instructions) + self.name
        file_name = file_name.replace("|", "Or") # | will be mistaken as file pipe in bash
        file_name = file_name.replace("[", "+") # [] will be mistaken as regular expression in file name
        file_name = file_name.replace("]", "+") # [] will be mistaken as regular expression in file name
        file_name = file_name.replace(".", "+") # . will be mistaken as module delimitor when the file is imported
        file_name += "_force.py"
        instrs_list = list()
        for instr in self.instructions:
            instrs_list.append("\"" + instr.get_full_ID() + "\"")

        instr_names = ",\n                      ".join(instrs_list)
        with open(file_name, "w") as output_handle:
            if (use_standard_template):
                from shared.basic_test_template import basic_template_str
                test_str = basic_template_str % instr_names
            else:
                from shared.basic_test_template import basic_non_standard_template_str
                test_str = basic_non_standard_template_str % instr_names

            output_handle.write(test_str)

        return file_name

# Allows for user to group instructions by a set count
# The class groups instructions that are added to it into a single file, until it hits this count.
# Once it does so, it creates a new group and starts adding instructions into that. This cycle countinues.
# Other than that, this function acts very similar to the InstructionFormatGroup class in instruction_format_group.py.
class InstructionNumGroup(object):

    # Some instructions use X17. If we're also using X17 as a system reg, we run into an issue.
    # Use a non-basic template to get around it.
    def __init__(self, grpSize, use_standard_template = True):
        # List allows us to keep track of the various groups, encapsulated by the class IGroupByNumber
        self.IList = list()
        # Similar list for generate-only instructions
        self.IGenList = list()
        # Similar list for atomic instructions
        self.IAtomicList = list()
        # Create the first empty group -- the group uses "Grouping_X" naming convention.
        self.IList.append(IGroupByNumber(grpSize, "Grouping_0" if use_standard_template else "Grouping_NonStandard_0"))
        self.IGenList.append(IGroupByNumber(grpSize, "GenOnly_Grouping_0" if use_standard_template else "GenOnly_Grouping_NonStandard_0"))
        self.IAtomicList.append(IGroupByNumber(grpSize, "Atomic_Grouping_0" if use_standard_template else "Atomic_Grouping_NonStandard_0"))

        self.use_standard_template = use_standard_template

    def add_instruction(self, instr, gen_only, atomic):
        # Depending on what kind of instruction this is, we need to group it accordingly
        if (gen_only == True):
            activeIList = self.IGenList
            groupingPrefix = "GenOnly_Grouping_" if self.use_standard_template else "GenOnly_Grouping_NonStandard_"
        elif (atomic == True):
            activeIList = self.IAtomicList
            groupingPrefix = "Atomic_Grouping_" if self.use_standard_template else "Atomic_Grouping_NonStandard_"
        else:
            activeIList = self.IList
            groupingPrefix = "Grouping_" if self.use_standard_template else "Grouping_NonStandard_"
        igrp = activeIList[len(activeIList) - 1]
        # If the newest group of instructions is full, then create a new group w/ the same naming convention
        if (igrp.sizeUsed() >= igrp.maxSize()):
            activeIList.append(IGroupByNumber(igrp.maxSize(), groupingPrefix + str(len(activeIList))))
            # Update reference to the newest group of instructions that we just added
            igrp = activeIList[len(activeIList) - 1]
        igrp.add_instruction(instr)

    def write(self, file_handle):
        all_list = self.IList + self.IGenList + self.IAtomicList
        for igrp in all_list:
            if (igrp.sizeUsed() == 0):
                continue
            igrp.write(file_handle)

    def print_tests(self):
        all_list = self.IList + self.IGenList + self.IAtomicList
        all_tests_file = open("all_tests.txt", "a")
        for igrp in all_list:
            if (igrp.sizeUsed() == 0):
                continue
            file_name = igrp.print_test(self.use_standard_template)
            all_tests_file.write(file_name + "\n")
        all_tests_file.close()
