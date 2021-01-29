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
from Enums import EStateElementType, EStateTransitionType
from State import State

from base.Sequence import Sequence
from base.StateTransitionHandler import StateTransitionHandler
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


#  A test StateTransitionHandler that verifies getArbitraryGprs() returns the
#  expected results.
class StateTransitionHandlerTest(StateTransitionHandler):
    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self.mSpecGprIndices = None

    def processStateElement(self, aStateElem):
        # It's uncertain how many system-reserved registers there are, but the
        # allowance below should be plenty
        max_arbitrary_gpr_count = 32 - len(self.mSpecGprIndices) - 5
        for _ in range(10):
            arbitrary_gprs = self.getArbitraryGprs(
                RandomUtils.random32(0, max_arbitrary_gpr_count)
            )

            if self.mSpecGprIndices.intersection(arbitrary_gprs):
                self.error(
                    "GPR indices %s were incorrectly designed as arbitrary"
                    % self.mSpecGprIndices.intersection(arbitrary_gprs)
                )

        return True


#  This test verifies that only GPRs that have not been specified as part of
#  the State are designated as arbitrary.
class MainSequence(Sequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mSpecGprIndices = set()

    def generate(self, **kargs):
        # Reserved registers should not be treated as arbitrary
        reserved_gpr_indices = self.getRandomGPRs(RandomUtils.random32(0, 5))
        for gpr_index in reserved_gpr_indices:
            self.reserveRegister("x%d" % gpr_index)

        self._mSpecGprIndices = self._mSpecGprIndices.union(
            reserved_gpr_indices
        )

        state_trans_handler = StateTransitionHandlerTest(self.genThread)
        StateTransition.registerStateTransitionHandler(
            state_trans_handler,
            EStateTransitionType.Explicit,
            (EStateElementType.GPR,),
        )

        state = self._createState()

        state_trans_handler.mSpecGprIndices = self._mSpecGprIndices

        StateTransition.transitionToState(state)

    # Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()

        gpr_indices = self.getRandomGPRs(
            RandomUtils.random32(0, 15), exclude="0"
        )
        for gpr_index in gpr_indices:
            state.addRegisterStateElement(
                ("x%d" % gpr_index), (RandomUtils.random64(),)
            )

        self._mSpecGprIndices = self._mSpecGprIndices.union(gpr_indices)

        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
