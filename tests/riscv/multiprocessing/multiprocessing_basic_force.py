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
import RandomUtils

from DV.riscv.trees.instruction_tree import RV64I_map, RV32I_map
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


# This test is intended to be run with mulitple threads to verify that
# multiprocess generation and simulation are basically functional.
class MainSequence(Sequence):
    def generate(self, **kargs):
        for _ in range(RandomUtils.random32(250, 300)):
            if self.getGlobalState("AppRegisterWidth") == 32:
                instr = RV32I_map.pick(self.genThread)
            else:
                instr = RV64I_map.pick(self.genThread)
            self.genInstruction(instr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
