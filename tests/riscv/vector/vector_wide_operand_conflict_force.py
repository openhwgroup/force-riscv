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

## This test verifies that vector register operands with different layouts don't overlap.
class MainSequence(Sequence):

    def generate(self, **kargs):
        instructions = [
            'VNSRA.WI##RISCV',
            'VNSRA.WV##RISCV',
            'VNSRA.WX##RISCV',
            'VNSRL.WI##RISCV',
            'VNSRL.WV##RISCV',
            'VNSRL.WX##RISCV',
            'VWADD.VV##RISCV',
            'VWADD.VX##RISCV',
            'VWADD.WV##RISCV',
            'VWADD.WX##RISCV',
            'VWADDU.VV##RISCV',
            'VWADDU.VX##RISCV',
            'VWADDU.WV##RISCV',
            'VWADDU.WX##RISCV',
            'VWMACC.VV##RISCV',
            'VWMACC.VX##RISCV',
            'VWMACCSU.VV##RISCV',
            'VWMACCSU.VX##RISCV',
            'VWMACCU.VV##RISCV',
            'VWMACCU.VX##RISCV',
            'VWMACCUS.VX##RISCV',
            'VWMUL.VV##RISCV',
            'VWMUL.VX##RISCV',
            'VWMULSU.VV##RISCV',
            'VWMULSU.VX##RISCV',
            'VWMULU.VV##RISCV',
            'VWMULU.VX##RISCV',
            'VWSUB.VV##RISCV',
            'VWSUB.VX##RISCV',
            'VWSUB.WV##RISCV',
            'VWSUB.WX##RISCV',
            'VWSUBU.VV##RISCV',
            'VWSUBU.VX##RISCV',
            'VWSUBU.WV##RISCV',
            'VWSUBU.WX##RISCV',
        ]

        for _ in range(RandomUtils.random32(500, 1000)):
            instr = self.choice(instructions)
            instr_id = self.genInstruction(instr)

            instr_record = self.queryInstructionRecord(instr_id)
            if instr_record is None:
                self.error('Instruction %s did not generate correctly' % instr)

            vd_val = instr_record['Dests']['vd']
            vs1_val = instr_record['Srcs'].get('vs1')
            vs2_val = instr_record['Srcs']['vs2']
            if instr.startswith('VW'):
                if vs1_val and (vd_val == (vs1_val & 0x1E)):
                    self.error('Instruction %s used overlapping source and destination registers of different formats' % instr)

                if ('.W' not in instr) and (vd_val == (vs2_val & 0x1E)):
                    self.error('Instruction %s used overlapping source and destination registers of different formats' % instr)
            elif instr.startswith('VN'):
                if (vd_val & 0x1E) == vs2_val:
                    self.error('Instruction %s used overlapping source and destination registers of different formats' % instr)
            else:
                self.error('Unexpected instruction %s' % instr)

            illegal_instr_count = self.queryExceptionRecordsCount(0x2)
            if illegal_instr_count != 0:
                self.error('Instruction %s did not execute correctly' % instr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
