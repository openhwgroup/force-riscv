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
from base.StateTransitionHandler import StateTransitionHandler
import state_transition_test_utils
from Enums import EStateElementType, EStateTransitionType
from State import State
import RandomUtils
import StateTransition

## A test StateTransitionHandler that defers to the default StateTransitionHandler some of the time.
class PartialStateTransitionHandlerTest(StateTransitionHandler):

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        processed = False

        # Randomly decide whether to process the StateElement or defer to the default implementation
        if RandomUtils.random32(0, 1) == 1:
            (mem_block_ptr_index,) = self.getArbitraryGprs(1, aExclude=(0,))
            self.initializeMemoryBlock(mem_block_ptr_index, (aStateElem,))
            self.genInstruction('FLD##RISCV', {'rd': aStateElem.getRegisterIndex(), 'rs1': mem_block_ptr_index, 'simm12': 0, 'NoRestriction': 1})
            processed = True

        return processed


## This test verifies that a StateTransition handler can process some of the StateElements and defer
# to the default StateTransitionHandler for the remaining StateElements.
class MainSequence(Sequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        state_trans_handler = PartialStateTransitionHandlerTest(self.genThread)
        StateTransition.registerStateTransitionHandler(state_trans_handler, EStateTransitionType.Explicit, (EStateElementType.FloatingPointRegister,))

        state = self._createState()
        StateTransition.transitionToState(state)
        state_transition_test_utils.verifyState(self, self._mExpectedStateData)

    ## Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()
        self._mExpectedStateData[EStateElementType.FloatingPointRegister] = state_transition_test_utils.addRandomFloatingPointRegisterStateElements(self, state, RandomUtils.random32(0, 15))
        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

