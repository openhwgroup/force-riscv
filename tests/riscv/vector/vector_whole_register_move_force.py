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
import UtilityFunctions

## This test verifies that whole register move instructions are generated with the operands aligned
# appropriately for the instruction.
class MainSequence(Sequence):

    def generate(self, **kargs):
        instructions = [
            'VMV1R.V##RISCV',
            'VMV2R.V##RISCV',
            'VMV4R.V##RISCV',
            'VMV8R.V##RISCV',
        ]

        for _ in range(RandomUtils.random32(50, 100)):
            instr = self.choice(instructions)
            instr_id = self.genInstruction(instr)

            instr_record = self.queryInstructionRecord(instr_id)
            if instr_record is None:
                self.error('Instruction %s did not generate correctly' % instr)

            vd_val = instr_record['Dests']['vd']
            vs2_val = instr_record['Srcs']['vs2']
            align_mask = UtilityFunctions.getAlignMask(int(instr[3]))
            if ((vd_val & align_mask) != vd_val) or ((vs2_val & align_mask) != vs2_val):
                self.error('Instruction %s was generated with an improperly aligned operand' % instr)

            illegal_instr_count = self.queryExceptionRecordsCount(0x2)
            if illegal_instr_count != 0:
                self.error('Instruction %s did not execute correctly' % instr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
