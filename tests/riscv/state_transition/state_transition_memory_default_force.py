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
import state_transition_test_utils
from Enums import EStateElementType
from State import State
import RandomUtils
import StateTransition

## This test verifies that the default MemoryStateElement StateTransitionHandler alters the memory
# State as expected.
class MainSequence(Sequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mMemBlockStartAddr = None
        self._mMemBlockEndAddr = None
        self._mExpectedStateData = {}

    def generate(self, **kargs):
        state = self._createState()

        target_addr_range = '%d-%d' % (self._mMemBlockStartAddr, self._mMemBlockEndAddr)
        instructions = ('LD##RISCV', 'SD##RISCV')
        for _ in range(RandomUtils.random32(2, 5)):
            for _ in range(RandomUtils.random32(50, 100)):
                self.genInstruction(self.choice(instructions), {'LSTarget': target_addr_range})

            StateTransition.transitionToState(state)
            state_transition_test_utils.verifyState(self, self._mExpectedStateData)

    ## Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()

        expected_mem_state_data = []
        mem_block_size = RandomUtils.random32(0x8, 0x20) * 16
        self._mMemBlockStartAddr = self.genVA(Size=mem_block_size, Align=16, Type='D')
        self._mMemBlockEndAddr = self._mMemBlockStartAddr + mem_block_size - 1
        cur_addr = self._mMemBlockStartAddr
        while cur_addr <= self._mMemBlockEndAddr:
            mem_val = RandomUtils.random64()
            state.addMemoryStateElement(cur_addr, 8, mem_val)
            expected_mem_state_data.append((cur_addr, mem_val))
            cur_addr += 8

        self._mExpectedStateData[EStateElementType.Memory] = expected_mem_state_data

        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

