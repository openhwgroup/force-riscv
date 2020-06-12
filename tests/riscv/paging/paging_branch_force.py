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

class MainSequence(Sequence):

    def generate(self, **kargs):
        random_instructions = ['ADDW##RISCV', 'SRLI#RV64I#RISCV', 'ADDI##RISCV', 'SLLI#RV64I#RISCV', 'LUI##RISCV']
        branch_instr = 'JALR##RISCV'
        for _ in range(10):
            for _ in range(self.random32(0, 5)):
                self.genInstruction(self.choice(random_instructions))

            (opt_value, opt_valid) = self.getOption("FlatMap")

            rand_VA = 0
            if opt_valid:
                rand_VA = self.genVA(Size=48, Align=8, Type="I", Bank=0, FlatMap=opt_value)
            else:
                rand_VA = self.genVA(Size=48, Align=8, Type="I", Bank=0)

            self.notice("gen target VA={:#x}".format(rand_VA))
            self.genInstruction(branch_instr, {'BRTarget':rand_VA})

MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

