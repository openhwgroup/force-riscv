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
import copy
import re
from abc import ABC, abstractmethod

from instruction_grouping import *
from shared.instruction_file import InstructionFile


class RiscV_InstructionAdjustor(object):
    def __init__(self, aAdjustByFormatFunc):
        self.mSupportedInstrFile = InstructionFile()
        self.mAdjustByFormat = aAdjustByFormatFunc

    def setAdjustorByFormatFunction(self, aFunc):
        self.mAdjustByFormat = aFunc

    def adjust_instruction(self, instr, extra_instrs):
        adjust_instruction_group(instr)

        if self.mAdjustByFormat(instr):
            supported_instr = copy.deepcopy(instr)
            self.mSupportedInstrFile.add_instruction(supported_instr)
        else:
            print("unsupported instr: {} using format: {}".format(instr.get_full_ID(), instr.get_format()))

    def get_supported_instructions(self):
        return self.mSupportedInstrFile
