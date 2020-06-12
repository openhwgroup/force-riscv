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

class IGroupByFormat(object):

    def __init__(self, format_name):
        super(IGroupByFormat, self).__init__()

        self.name = format_name
        self.instructions = list()

    def add_instruction(self, instr, instr_format):
        if instr_format != self.name:
            raise BuilderException("Adding instruction \"%s\" with format \"%s\" to group \"%s\"." % (instr.name, instr_format, self.name))
        self.instructions.append(instr)

    def write(self, file_handle):
        file_handle.write("Format group \"%s\"\n" % self.name)
        for instr in self.instructions:
            file_handle.write(instr.get_full_ID() + "\n")

    def print_test(self):
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
            from shared.basic_test_template import basic_template_str
            test_str = basic_template_str % instr_names
            output_handle.write(test_str)

        return file_name

class InstructionFormatGroup(object):

    def __init__(self):
        self.IDict = dict()

    def add_instruction(self, instr):
        instr_format = instr.get_format()
        if instr_format in self.IDict:
            igrp = self.IDict[instr_format]
        else:
            igrp = IGroupByFormat(instr_format)
            self.IDict[instr_format] = igrp

        igrp.add_instruction(instr, instr_format)

    def write(self, file_handle):
        for key, igrp in sorted(self.IDict.items()):
            igrp.write(file_handle)

    def print_tests(self):
        all_tests_file = open("all_tests.txt", "a")
        for key, igrp in sorted(self.IDict.items()):
            file_name = igrp.print_test()
            all_tests_file.write(file_name + "\n")
        
        all_tests_file.close()
