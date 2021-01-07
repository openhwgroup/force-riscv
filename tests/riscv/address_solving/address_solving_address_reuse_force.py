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
from Constraint import ConstraintSet
import RandomUtils

## This test verifies that load and store instructions can reuse addresses when generating with and
# without preamble.
class MainSequence(Sequence):

    def generate(self, **kargs):
        init_size = 256
        init_addr = self.genVA(Size=init_size, Align=init_size, Type='D')
        addr_constr = ConstraintSet(init_addr, (init_addr + init_size - 1))

        for _ in range(RandomUtils.random32(50, 100)):
            instr_params = {'LSTarget': str(addr_constr), 'NoSkip': 1}

            if RandomUtils.random32(0, 1) == 1:
                instr_params['NoPreamble'] = 1
            else:
                instr_params['UsePreamble'] = 1

            instr_id = self.genInstruction(self.choice(('LD##RISCV', 'SD##RISCV')), instr_params)
            instr_record = self.queryInstructionRecord(instr_id)
            if not addr_constr.containsValue(instr_record['LSTarget']):
                self.error('Target address 0x%x was generated outside of the specified range %s' % (addr, addr_constr))


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
