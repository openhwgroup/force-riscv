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

## This test verifies instructions with implied base register operands can be generated without
# preamble.
class MainSequence(Sequence):

    def generate(self, **kargs):
        instructions = ('C.FLDSP##RISCV', 'C.FSDSP##RISCV', 'C.LDSP##RISCV', 'C.LWSP##RISCV', 'C.SDSP##RISCV', 'C.SWSP##RISCV')

        if self.getGlobalState('AppRegisterWidth') == 32:
            instructions = ('C.FLDSP##RISCV', 'C.FSDSP##RISCV', 'C.LWSP##RISCV', 'C.SWSP##RISCV')
            
        for _ in range(100):
            instr_id = self.genInstruction(self.choice(instructions), {'NoPreamble': 1})

            if instr_id:
                instr_record = self.queryInstructionRecord(instr_id)

                if instr_record['Addressing']['Base'][0] != 2:
                    self.error('Unexpected base register; Expected=x2, Actual=x%d' % instr_record['Addressing']['Base'][0])


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
