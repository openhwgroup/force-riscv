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
from LoopTestSequence import LoopTestSequence

## This test executes standard loop with branch instructions in the loop body. The branch
# instructions can take the execution to a wide variety of possible addresses, so this test
# exercises the various mechanisms for generating loop control instructions depending on the
# distance from the end of the loop to the beginning.
class MainSequence(LoopTestSequence):

    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread, name)

        self._mInstructionWeights = {
            'ADDI##RISCV': 10,
            'ADDW##RISCV': 10,
            'JAL##RISCV': 2,
            'JALR##RISCV': 2,
            'LUI##RISCV': 10,
            'SLLI#RV64I#RISCV': 10,
            'SRLI#RV64I#RISCV': 10,
        }

        if self.getGlobalState('AppRegisterWidth') == 32:
            self._mInstructionWeights = {
                'ADDI##RISCV': 10,
                'JAL##RISCV': 2,
                'JALR##RISCV': 2,
                'LUI##RISCV': 10,
                'SLLI#RV32I#RISCV': 10,
                'SRLI#RV32I#RISCV': 10,
            }
            
    ## Return a dictionary of names of instructions to generate in the loop body with their
    # corresponding weights.
    def getInstructionWeights(self):
        return self._mInstructionWeights


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

