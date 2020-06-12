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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from DV.riscv.trees.instruction_tree import ALU_Int_All_instructions
from DV.riscv.trees.instruction_tree import LDST_All_instructions



class  Bunch_of_ALU_Int(Sequence):

    def generate(self, **kwargs):

        self.notice("Generating in 'Bunch_of_ALU_Int'")
        for _ in range(self.random32(5, 20)):
            instr = self.pickWeighted(ALU_Int_All_instructions)
            self.genInstruction(instr)



class  Bunch_of_LDST(Sequence):

    def generate(self, **kwargs):

        self.notice('Generating in "Bunch_of_LDST"')
        for _ in range(self.random32(5, 20)):
            instr = self.pickWeighted(LDST_All_instructions)
            instr_rec_id = self.genInstruction(instr)



