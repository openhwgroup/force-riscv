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
from Enums import EStateElementType, EStateTransitionOrderMode
from State import State

import state_transition_test_utils
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.Utils import LoadGPR64


#  This test verifies that a baisc StateTransition can be executed with
#  ByPriority order mode.
class MainSequence(Sequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        # Disable floating point, so the StateTransition can enable it
        sys_reg_name = "misa"
        (sys_reg_val, valid) = self.readRegister(sys_reg_name)
        state_transition_test_utils.assert_valid_register_value(
            self, sys_reg_name, valid
        )

        load_gpr64_seq = LoadGPR64(self.genThread)
        rand_gpr_index = self.getRandomGPR(exclude="0")
        load_gpr64_seq.load(rand_gpr_index, 0x0000028)
        self.genInstruction(
            "CSRRC#register#RISCV",
            {
                "rd": 0,
                "rs1": rand_gpr_index,
                "csr": self.getRegisterIndex(sys_reg_name),
            },
        )

        state = self._createState()
        StateTransition.transitionToState(
            state, EStateTransitionOrderMode.ByPriority
        )

        state_transition_test_utils.verify_state(self, self._mExpectedStateData)

    # Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()
        test_utils = state_transition_test_utils
        # The priorities are set in such a way as to enable floating point
        # before loading the floating point registers
        expected_sys_reg_state_data = []
        sys_reg_name = "misa"
        (sys_reg_val, valid) = self.readRegister(sys_reg_name)
        test_utils.assert_valid_register_value(self, sys_reg_name, valid)

        sys_reg_val |= 0x0000028
        state.addRegisterStateElement(
            sys_reg_name, (sys_reg_val,), aPriority=1
        )
        expected_sys_reg_state_data.append((sys_reg_name, sys_reg_val))
        self._mExpectedStateData[
            EStateElementType.SystemRegister
        ] = expected_sys_reg_state_data

        self._mExpectedStateData[
            EStateElementType.FloatingPointRegister
        ] = test_utils.add_random_floating_point_register_state_elements(
            self,
            state,
            RandomUtils.random32(0, 10),
            aPriorityMin=2,
            aPriorityMax=4,
        )

        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
