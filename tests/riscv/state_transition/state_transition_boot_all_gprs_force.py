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
from riscv.Utils import LoadGPR64
import RandomUtils

## This test forces all GPRs to be loaded during the boot sequence. It also causes some floating
# point registers to be loaded as well, which will utilize GPRs during the boot sequence. The
# purpose is to ensure GPRs aren't reloaded with their final state values after being loaded with
# their initial state values by the StateTransition process. The final state values are set to
# invalid addresses in order to potentially trigger simulation failures when executing load and
# store instructions if the boot process does not work as expected.
class MainSequence(Sequence):

    def generate(self, **kargs):
        self.randomInitializeRegister('D0')
        for reg_index in range(1, 32):
            self.randomInitializeRegister('D%d' % reg_index)
            self.initializeRegister('x%d' % reg_index, RandomUtils.random64(0x0, 0x7FFFFFFFFFFF))

        instructions = ('LD##RISCV', 'SD##RISCV')
        for _ in range(RandomUtils.random32(100, 200)):
            self.genInstruction(self.choice(instructions), {'NoPreamble': 1})

        load_gpr64_seq = LoadGPR64(self.genThread)
        for reg_index in range(1, 32):
            load_gpr64_seq.load(reg_index, RandomUtils.random64(0x800000000000, 0x7FFFFFFFFFFFFFFF))


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
