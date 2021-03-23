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
from DV.riscv.trees.instruction_tree import LDST32_All_instructions
from DV.riscv.trees.instruction_tree import LDST_All_instructions
from DV.riscv.trees.instruction_tree import LDST_Byte_instructions
from DV.riscv.trees.instruction_tree import LDST_Double_instructions
from DV.riscv.trees.instruction_tree import LDST_Half_instructions
from DV.riscv.trees.instruction_tree import LDST_Word_instructions
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


class MyMainSequence(Sequence):
    """"""

    def generate(self, **kargs):

        for _ in range(100):
            if self.getGlobalState("AppRegisterWidth") == 32:
                instr = self.pickWeighted(LDST32_All_instructions)
            else:
                instr = self.pickWeighted(LDST_All_instructions)

            # Get two adjacent 4K pages.
            page_addr = self.genVA(Size=0x2000, Align=0x1000)

            min_addr_offset = self._get_min_address_offset(instr)
            target_addr = page_addr + self.random32(min_addr_offset, 0xFFF)

            self.genInstruction(instr, {"LSTarget": target_addr})

            self.notice(
                ">>>>>  Instruction: {}   Target addr: {:012x}".format(
                    instr, target_addr
                )
            )

    def _get_min_address_offset(self, instr):
        # To generate page crossings, for the target address to be at the
        # end of the first page setting the page offset to some value close
        # to the end of the page based on the size of the target operand of
        # the selected instruction.  The code below will generated some
        # page crossings and some accesses that are at the end of the page,
        # but not crossing the page.
        min_addr_offset = 0
        if (instr in LDST_Byte_instructions) or (
            instr in LDST_Half_instructions
        ):
            min_addr_offset = 0xFFC
        elif instr in LDST_Word_instructions:
            min_addr_offset = 0xFFA
        elif instr in LDST_Double_instructions:
            min_addr_offset = 0xFF6
        else:
            self.error(
                ">>>>>  Hmmm...  {} is an unexpected instruction.".format(
                    instr
                )
            )

        return min_addr_offset


MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
