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
import RandomUtils

## This test verifies that add-with-carry and subtract-with-borrow instructions don't use v0 as the
# destination register.
class MainSequence(Sequence):

    def generate(self, **kargs):
        instructions = [
            'VADC.VIM##RISCV',
            'VADC.VVM##RISCV',
            'VADC.VXM##RISCV',
            'VSBC.VVM##RISCV',
            'VSBC.VXM##RISCV',
        ]

        for _ in range(RandomUtils.random32(50, 100)):
            instr = self.choice(instructions)
            instr_id = self.genInstruction(instr)

            instr_record = self.queryInstructionRecord(instr_id)
            if instr_record is None:
                self.error('Instruction %s did not generate correctly' % instr)

            vd_val = instr_record['Dests']['vd']
            if vd_val == 0:
                self.error('Instruction %s was generated with v0 as the destination register' % instr)

            illegal_instr_count = self.queryExceptionRecordsCount(0x2)
            if illegal_instr_count != 0:
                self.error('Instruction %s did not execute correctly' % instr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
