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
import StateTransition
from Enums import (
    EStateElementType,
    EStateTransitionOrderMode,
    EStateTransitionType,
)
from State import State

import state_transition_test_utils
from StateTransitionHandlerTest import StateTransitionHandlerTest
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


#  This test verifies that a basic StateTransition yields the expected State.
class MainSequence(Sequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        StateTransition.registerStateTransitionHandler(
            StateTransitionHandlerTest(self.genThread),
            EStateTransitionType.Explicit,
            (EStateElementType.GPR, EStateElementType.FloatingPointRegister),
        )

        # Test overwriting a StateTransitionHandler assignment
        StateTransition.registerStateTransitionHandler(
            StateTransitionHandlerTest(self.genThread),
            EStateTransitionType.Explicit,
            (EStateElementType.FloatingPointRegister,),
        )

        state_elem_type_order = (
            EStateElementType.PredicateRegister,
            EStateElementType.FloatingPointRegister,
            EStateElementType.Memory,
            EStateElementType.SystemRegister,
            EStateElementType.GPR,
            EStateElementType.VmContext,
            EStateElementType.VectorRegister,
            EStateElementType.PrivilegeLevel,
            EStateElementType.PC,
        )
        StateTransition.setDefaultStateTransitionOrderMode(
            EStateTransitionType.Boot,
            EStateTransitionOrderMode.ByStateElementType,
            state_elem_type_order,
        )
        StateTransition.setDefaultStateTransitionOrderMode(
            EStateTransitionType.Explicit,
            EStateTransitionOrderMode.AsSpecified,
        )

        state = self._createState()
        StateTransition.transitionToState(state)

        state_transition_test_utils.verify_state(
            self, self._mExpectedStateData
        )

    # Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()
        self._mExpectedStateData[
            EStateElementType.Memory
        ] = state_transition_test_utils.add_random_memory_state_elements(
            self, state, RandomUtils.random32(0, 5)
        )
        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
