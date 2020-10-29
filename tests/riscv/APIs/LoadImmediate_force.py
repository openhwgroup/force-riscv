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

## This test verifies that a 64-bit immediate value can be loaded into a general purpose register.
class MainSequence(Sequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mLoadGpr = LoadGPR64(aGenThread)

    def generate(self, **kargs):
        boundary_values = (0x1, 0x7FFFF000, 0xFFFFF000, 0xFFFFFFFF000, 0xFFFFFFFFFFF000, 0xFFFFFFFFFFFFEFFF)

        if self.getGlobalState('AppRegisterWidth') == 32:
            boundary_values = (0x1, 0x7FFFF000, 0xFFFFF000, 0xFFFFF000, 0xFFFFEFFF)
            
        for boundary_value in boundary_values:
            for bottom_bits in (0x0, 0x7FF, 0xFFF):
                for adjustment in (-1, 0, 1):
                    value = boundary_value + bottom_bits + adjustment
                    if self.getGlobalState('AppRegisterWidth') == 32:
                        value &= 0xffffffff
                    self._testLoadGpr(value)

        for _ in range(50):
            self._testLoadGpr(self.random32())

        if self.getGlobalState('AppRegisterWidth') == 32:
            pass
        else:
            for _ in range(50):
                self._testLoadGpr(self.random64())

    ## Load an immediate value into a random general purpose register; then verify the result.
    #
    #  @param aValue A 64-bit value.
    def _testLoadGpr(self, aValue):
        gpr_index = self.getRandomGPR(exclude='0')
        self._mLoadGpr.load(gpr_index, aValue)

        gpr_name = 'x%d' % gpr_index
        (gpr_val, valid) = self.readRegister(gpr_name)
        if not valid:
            self.error('Unable to read register %s' % gpr_name)

        if gpr_val != aValue:
            self.error('Register %s has an unexpected value: Expected = 0x%x; Actual = 0x%x' % (gpr_name, aValue, gpr_val))


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

