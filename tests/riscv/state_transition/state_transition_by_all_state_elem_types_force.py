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
from DV.riscv.instruction_list import instructions
import state_transition_test_utils
from Enums import EStateElementType, EStateTransitionOrderMode
from State import State
import RandomUtils
import StateTransition
import UtilityFunctions

## This test verifies that a StateTransition can be executed with ByStateElementType order mode for
# a State with StateElements of all types.
class MainSequence(Sequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        state = self._createState()

        state_elem_type_order = (EStateElementType.VectorRegister, EStateElementType.Memory, EStateElementType.SystemRegister, EStateElementType.FloatingPointRegister, EStateElementType.PredicateRegister, EStateElementType.GPR, EStateElementType.VmContext, EStateElementType.PrivilegeLevel, EStateElementType.PC)
        StateTransition.transitionToState(state, EStateTransitionOrderMode.ByStateElementType, state_elem_type_order)

        state_transition_test_utils.verifyState(self, self._mExpectedStateData)

    ## Create a random State to test an explicit StateTransition.
    def _createState(self):
        state = State()

        self._mExpectedStateData[EStateElementType.PC] = state_transition_test_utils.addRandomPcStateElement(self, state)
        self._mExpectedStateData[EStateElementType.FloatingPointRegister] = state_transition_test_utils.addRandomFloatingPointRegisterStateElements(self, state, RandomUtils.random32(0, 20))
        self._mExpectedStateData[EStateElementType.GPR] = state_transition_test_utils.addRandomGprStateElements(self, state, RandomUtils.random32(0, 20))
        self._mExpectedStateData[EStateElementType.Memory] = state_transition_test_utils.addRandomMemoryStateElements(self, state, RandomUtils.random32(0, 20))

        expected_sys_reg_state_data = []
        mscratch_val = RandomUtils.random64()
        state.addRegisterStateElement('mscratch', (mscratch_val,))
        expected_sys_reg_state_data.append(('mscratch', mscratch_val))
        mepc_val = UtilityFunctions.getAlignedValue(RandomUtils.random64(), 4)
        state.addRegisterStateElement('mepc', (mepc_val,))
        expected_sys_reg_state_data.append(('mepc', mepc_val))
        self._mExpectedStateData[EStateElementType.SystemRegister] = expected_sys_reg_state_data

        expected_vm_context_state_data = []
        mode_val = self.choice((0, 8, 9))
        state.addVmContextStateElement('satp', 'MODE', mode_val)
        expected_vm_context_state_data.append(('satp', 'MODE', mode_val))
        mpp_val = self.choice((0, 1, 3))
        state.addVmContextStateElement('mstatus', 'MPP', mpp_val)
        expected_vm_context_state_data.append(('mstatus', 'MPP', mpp_val))
        sum_val = RandomUtils.random32(0, 1)
        state.addVmContextStateElement('mstatus', 'SUM', sum_val)
        expected_vm_context_state_data.append(('mstatus', 'SUM', sum_val))
        self._mExpectedStateData[EStateElementType.VmContext] = expected_vm_context_state_data

        # TODO(Noah): Add vector register StateElements when the default
        # VectorRegisterStateTransitionHandler is implemented.

        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

