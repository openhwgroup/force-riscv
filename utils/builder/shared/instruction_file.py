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
import sys

class IGroupByName(object):

    def __init__(self, iname):
        super(IGroupByName, self).__init__()

        self.name = iname
        self.instructions = list()

    def add_instruction(self, instr):
        if instr.name != self.name:
            raise BuilderException("Adding instruction \"%s\" to group \"%s\"." % (instr.name, self.name))
        self.instructions.append(instr)

    def adjust_instructions(self, instr_adjustor):
        self.instructions.sort(key = lambda x: x.get_full_ID())
        extra_instrs = list()
        for instr in self.instructions:
            instr_adjustor.adjust_instruction(instr, extra_instrs)
        for extra_i in extra_instrs:
            self.add_instruction(extra_i)

    def sort_instructions(self):
        self.instructions.sort(key=lambda x: x.get_full_ID())

    def write(self, file_handle):
        for instr in self.instructions:
            instr.write(file_handle)

class InstructionFile(object):

    def __init__(self):
        self.IDict = dict()

    def add_instruction(self, instr):
        if instr.name is None:
            print("WARNING [InstructionFile::add_instruction] instr.name = 'None'???")
            return

        #print("[add_instruction] instr.name = '%s'" % instr.name)
        if instr.name in self.IDict:
            igrp = self.IDict[instr.name]
        else:
            igrp = IGroupByName(instr.name)
            self.IDict[instr.name] = igrp

        igrp.add_instruction(instr)

    def adjust_instructions(self, instr_adjustor):
        for key, igrp in sorted(self.IDict.items()):
            igrp.adjust_instructions(instr_adjustor)

    def write(self, file_handle, license_str = None):
        file_handle.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
        if license_str is not None:
            file_handle.write(license_str)
        file_handle.write("<instruction_file>\n")
        for key, igrp in sorted(self.IDict.items()):
            igrp.sort_instructions()
            igrp.write(file_handle)
        file_handle.write("</instruction_file>\n")

    def instruction_iterator(self):
        for key, igrp in sorted(self.IDict.items()):
            for instr in igrp.instructions:
                yield instr
