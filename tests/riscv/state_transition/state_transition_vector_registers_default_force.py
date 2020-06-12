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

## This test verifies that the default vector register StateTransitionHandler alters the vector
# register State as expected.
class MainSequence(Sequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        state = self._createState()
        StateTransition.transitionToState(state)

        # TODO(Noah): Verify the vector register State when the VL1R.V instruction can be generated
        # and simulated successfully.
        #state_transition_test_utils.verifyState(self, self._mExpectedStateData)

    ## Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()
        self._mExpectedStateData[EStateElementType.VectorRegister] = state_transition_test_utils.addRandomVectorRegisterStateElements(self, state, RandomUtils.random32(0, 15))
        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

