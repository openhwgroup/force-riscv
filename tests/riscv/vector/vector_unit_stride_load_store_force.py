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

## This test verifies that unit-stride load and store instructions can be generated and executed
# successfully.
class MainSequence(Sequence):

    def generate(self, **kargs):
        instructions = [
            'VLE16.V##RISCV',
            'VLE32.V##RISCV',
            'VLE64.V##RISCV',
            'VLE8.V##RISCV',
            'VSE16.V##RISCV',
            'VSE32.V##RISCV',
            'VSE64.V##RISCV',
            'VSE8.V##RISCV',
        ]

        for _ in range(RandomUtils.random32(50, 100)):
            instr = self.choice(instructions)

            instr_params = {}
            if RandomUtils.random32(0, 1) == 1:
                instr_params['NoPreamble'] = 1

            instr_id = self.genInstruction(instr, instr_params)

            # Instructions disallowing preamble may legitimately be skipped sometimes if no solution
            # can be determined, but instructions that permit preamble should always generate
            if 'NoPreamble' not in instr_params:
                instr_record = self.queryInstructionRecord(instr_id)

                if instr_record is None:
                    self.error('Instruction %s did not generate correctly' % instr)

            except_count = 0
            for except_code in (0x2, 0x4, 0x5, 0x6, 0x7, 0xD, 0xF):
                except_count += self.queryExceptionRecordsCount(except_code)

            if except_count != 0:
                self.error('Instruction %s did not execute correctly' % instr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
