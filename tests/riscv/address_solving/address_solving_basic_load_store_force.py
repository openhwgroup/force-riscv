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

## This test verifies that basic load and store instructions can be executed successfully.
class MainSequence(Sequence):

    def generate(self, **kargs):
        instructions = ('LD##RISCV', 'SD##RISCV')
        if self.getGlobalState('AppRegisterWidth') == 32:
            instructions = ('LW##RISCV', 'SW##RISCV')

        for _ in range(1000):
            instr = self.choice(instructions)
            self.genInstruction(instr, {'NoPreamble': 1})

            except_count = 0
            for except_code in (0x2, 0x4, 0x5, 0x6, 0x7, 0xD, 0xF):
                except_count += self.queryExceptionRecordsCount(except_code)

            if except_count != 0:
                self.error('Instruction %s did not execute correctly' % instr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
