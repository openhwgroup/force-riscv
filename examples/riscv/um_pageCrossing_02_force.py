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
    def generate(self, **kargs):

        for _ in range(100):
            if self.getGlobalState("AppRegisterWidth") == 32:
                instr = self.pickWeighted(LDST32_All_instructions)
            else:
                instr = self.pickWeighted(LDST_All_instructions)

            # Get two adjacent 4K pages.
            page_addr = self.genVA(Size=0x2000, Align=0x1000)

            # Generate addresses at the end of the page that are aligned
            # according to the target size.  No page crossings here; every
            # access is right at the end of a page.
            if instr in LDST_Byte_instructions:
                page_addr |= 0xFFF
            elif instr in LDST_Half_instructions:
                page_addr |= 0xFFE
            elif instr in LDST_Word_instructions:
                page_addr |= 0xFFC
            elif instr in LDST_Double_instructions:
                page_addr |= 0xFF8
            else:
                self.error(
                    ">>>>>  Hmmm...  {} is an unexpected instruction.".format(
                        instr
                    )
                )

            self.notice(
                ">>>>>  Instruction: {}   Target addr: {:012x}".format(
                    instr, page_addr
                )
            )
            self.genInstruction(instr, {"LSTarget": page_addr})


MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
