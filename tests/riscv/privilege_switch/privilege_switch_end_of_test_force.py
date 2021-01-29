#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0
#  (the "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
# OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from DV.riscv.trees.instruction_tree import RV_G_map, RV32_G_map
import RandomUtils

## This test intentionally returns to an address that will generate a page fault on instruction
# fetch at the end of the test. The purpose is to verify that Force can recover from this scenario.
class MainSequence(Sequence):

   def generate(self, **kargs):
        for _ in range(RandomUtils.random32(5, 10)):
            if (self.getGlobalState('AppRegisterWidth') == 32):
                instr = RV32_G_map.pick(self.genThread)
            else:
                instr = RV_G_map.pick(self.genThread)
            self.genInstruction(instr)

        self.genInstruction('MRET##RISCV', {'NoSkip': 1, 'priv': 1})

        for _ in range(RandomUtils.random32(5, 10)):
            if (self.getGlobalState('AppRegisterWidth') == 32):
                instr = RV32_G_map.pick(self.genThread)
            else:
                instr = RV_G_map.pick(self.genThread)
            self.genInstruction(instr)

        target_addr = self.genVA(Size=4, Align=4, Type='I')
        self.genInstruction('SRET##RISCV', {'NoSkip': 1, 'epc': target_addr, 'priv': 0})


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
