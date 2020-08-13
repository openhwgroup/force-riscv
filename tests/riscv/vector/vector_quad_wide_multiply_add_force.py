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

## This test verifies that vector register operands with different layouts in quad-widening
# instructions don't overlap.
class MainSequence(Sequence):

    def generate(self, **kargs):
        instructions = [
            'VQMACC.VV##RISCV',
            'VQMACC.VX##RISCV',
            'VQMACCSU.VV##RISCV',
            'VQMACCSU.VX##RISCV',
            'VQMACCU.VV##RISCV',
            'VQMACCU.VX##RISCV',
            'VQMACCUS.VX##RISCV',
        ]

        for _ in range(RandomUtils.random32(50, 100)):
            instr = self.choice(instructions)
            instr_id = self.genInstruction(instr)

            instr_record = self.queryInstructionRecord(instr_id)
            if instr_record is None:
                self.error('Instruction %s did not generate correctly' % instr)

            vd_val = instr_record['Dests']['vd']
            vs1_val = instr_record['Srcs'].get('vs1')
            vs2_val = instr_record['Srcs']['vs2']
            if vs1_val and (vd_val == (vs1_val & 0x1C)):
                self.error('Instruction %s used overlapping source and destination registers of different formats' % instr)

            if vd_val == (vs2_val & 0x1C):
                self.error('Instruction %s used overlapping source and destination registers of different formats' % instr)

            illegal_instr_count = self.queryExceptionRecordsCount(0x2)
            if illegal_instr_count != 0:
                self.error('Instruction %s did not execute correctly' % instr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
