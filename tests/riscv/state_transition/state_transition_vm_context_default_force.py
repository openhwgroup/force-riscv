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

## This test verifies that the default VM context StateTransitionHandler alters the VM context State
# as expected.
class MainSequence(Sequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        state = self._createState()
        StateTransition.transitionToState(state)
        state_transition_test_utils.verifyState(self, self._mExpectedStateData)

    ## Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()

        expected_vm_context_state_data = []
        mode_val = self.choice((0, 1)) if self.getGlobalState('AppRegisterWidth') == 32 else self.choice((0, 8, 9))
        state.addVmContextStateElement('satp', 'MODE', mode_val)
        expected_vm_context_state_data.append(('satp', 'MODE', mode_val))
        mpp_val = self.choice((0, 1, 3))
        state.addVmContextStateElement('mstatus', 'MPP', mpp_val)
        expected_vm_context_state_data.append(('mstatus', 'MPP', mpp_val))

        for reg_field_name in ('MPRV', 'SPP', 'MXR', 'SUM', 'TVM'):
            reg_field_val = RandomUtils.random32(0, 1)
            state.addVmContextStateElement('mstatus', reg_field_name, reg_field_val)
            expected_vm_context_state_data.append(('mstatus', reg_field_name, reg_field_val))

        self._mExpectedStateData[EStateElementType.VmContext] = expected_vm_context_state_data

        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

