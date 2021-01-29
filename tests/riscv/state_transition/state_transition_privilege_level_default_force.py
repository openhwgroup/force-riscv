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

## This test verifies that the default privilege level StateTransitionHandler alters the privilege
# level State as expected.
class MainSequence(Sequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        priv_level_names = ('U', 'S', None, 'M')
        for priv_level in (1, 0, 1, 3):
            state = State()

            if RandomUtils.random32(0, 1) == 1:
                state.addPrivilegeLevelStateElementByName(priv_level_names[priv_level])
            else:
                state.addPrivilegeLevelStateElement(priv_level)

            self._mExpectedStateData[EStateElementType.PrivilegeLevel] = priv_level

            StateTransition.transitionToState(state)
            for _ in range(10):
                self.genInstruction('ADDI##RISCV')

            state_transition_test_utils.verifyState(self, self._mExpectedStateData)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
