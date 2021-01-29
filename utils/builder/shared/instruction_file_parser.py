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
import defusedxml.defusedxml.sax
import sys
from shared.instruction import Instruction, Operand, GroupOperand, Asm
import shared.builder_utils as builder_utils
import copy
from shared.builder_exception import BuilderException

# Needed to create class to interpret parsed XML data
from xml.sax.handler import ContentHandler


def asm_op_index(op):
    if op.find("op") == 0:
        return int(op[2:])
    else:
        return 0


class InstructionFileHandler(ContentHandler, object):
    def __init__(self, file_path, instr_file):
        super().__init__()
        self.instructionFile = instr_file
        self.filePath = file_path
        self.nameStack = list()
        self.currentInstruction = None
        self.currentGroupOperand = None
        self.currentOperand = None
        self.currentChars = None
        self.currentAsm = None

    def startDocument(self):
        print('Starting parsing "%s" ...' % self.filePath)

    def endDocument(self):
        print("End parsing.")

    def startElement(self, name, attrs):
        self.nameStack.append(name)
        self.currentChars = ""

        if name == "instruction_file":
            pass
        elif name in ["I", "O", "asm"]:
            self.__getattribute__("start_" + name)(attrs)

    def endElement(self, name):
        self.nameStack.pop()

        if name == "instruction_file":
            pass
        elif name in ["I", "O", "asm"]:
            self.__getattribute__("end_" + name)()

    def characters(self, content):
        self.currentChars += content

    def start_I(self, attrs):
        self.currentInstruction = Instruction()
        names = attrs.getNames()
        for name in names:
            if name in [
                "name",
                "form",
                "isa",
                "group",
                "aliasing",
                "extension",
            ]:
                setattr(self.currentInstruction, name, attrs[name])
            elif name == "class":
                self.currentInstruction.iclass = attrs[name]
            else:
                print("WARNING: not handled instruction attribute : %s" % name)

    def end_I(self):
        if self.currentInstruction:
            self.instructionFile.add_instruction(self.currentInstruction)

    def create_operand(self, opr_type):
        if opr_type in [
            "Group",
            "Branch",
            "AuthBranch",
            "LoadStore",
            "AuthLoadStore",
            "ALU",
            "DataProcessing",
            "CacheOp",
            "SystemOp",
        ]:
            self.currentGroupOperand = GroupOperand()
            return self.currentGroupOperand
        else:
            return Operand()

    def start_O(self, attrs):
        self.currentOperand = self.create_operand(attrs["type"])
        names = attrs.getNames()
        my_attr_list = [
            "name",
            "type",
            "bits",
            "value",
            "reserved",
            "access",
            "choices",
            "choices2",
            "choices3",
            "exclude",
            "differ",
            "slave",
            "layout-multiple",
            "reg-count",
            "reg-index-alignment",
            "elem-width",
            "uop-param-type",
            "sizeType",
        ]
        for name in names:
            if name in my_attr_list:
                setattr(self.currentOperand, name, attrs[name])
            elif name == "class":
                self.currentOperand.oclass = attrs[name]
            else:
                self.currentOperand.set_extra_attribute(name, attrs[name])

        if not self.currentOperand.bits:
            self.currentOperand.width = 0
        else:
            self.currentOperand.width = builder_utils.get_bits_size(
                self.currentOperand.bits
            )

    def end_O(self):
        if self.currentOperand:
            if self.currentGroupOperand:
                if self.currentGroupOperand == self.currentOperand:
                    self.currentInstruction.add_operand(
                        self.currentGroupOperand
                    )
                    self.currentGroupOperand = None
                else:
                    self.currentGroupOperand.add_operand(self.currentOperand)
            else:
                self.currentInstruction.add_operand(self.currentOperand)
        elif self.currentGroupOperand:
            self.currentInstruction.add_operand(self.currentGroupOperand)
            self.currentGroupOperand = None

        self.currentOperand = None

    def start_asm(self, attrs):
        self.currentAsm = Asm()
        names = sorted(attrs.getNames(), key=lambda x: asm_op_index(x))
        for name in names:
            if name.find("op") == 0:
                self.currentAsm.ops.append(attrs[name])
            elif name == "format":
                self.currentAsm.format = attrs["format"]
            else:
                print("WARNING: not handled asm attribute : %s" % name)

    def end_asm(self):
        self.currentInstruction.asm = self.currentAsm
        self.currentAsm = None


class InstructionFileParser(object):
    def __init__(self, instr_file):
        self.instrFile = instr_file

    def parse(self, file_path):
        ifile_handler = InstructionFileHandler(file_path, self.instrFile)
        # import traceback
        # print( "File Path: " + str( file_path ))
        # traceback.print_stack()
        try:
            defusedxml.defusedxml.sax.parse(file_path, ifile_handler)
        except BaseException:
            e_type, e_value, e_tb = sys.exc_info()
            import traceback

            traceback.print_exception(e_type, e_value, e_tb)
            loc = ifile_handler._locator
            print(
                'Parsing error, file "%s", line %d, column %d'
                % (file_path, loc.getLineNumber(), loc.getColumnNumber())
            )
            sys.exit(1)
