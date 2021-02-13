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
from Enums import EStateElementType
from State import State

import state_transition_test_utils as utils
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


#  This test verifies that the default system register StateTransitionHandler
#  alters the system register State as expected.
class MainSequence(Sequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        state = self._createState()
        StateTransition.transitionToState(state)
        utils.verify_state(self, self._mExpectedStateData)

    # Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()

        expected_sys_reg_state_data = []

        mscratch_val = (
            RandomUtils.random32()
            if self.getGlobalState("AppRegisterWidth") == 32
            else RandomUtils.random64()
        )
        state.addRegisterStateElement("mscratch", (mscratch_val,))
        expected_sys_reg_state_data.append(("mscratch", mscratch_val))

        scause_name = "scause"
        exception_code_var_val = RandomUtils.random32(0, 9)
        state.addSystemRegisterStateElementByField(
            scause_name, "EXCEPTION CODE_VAR", exception_code_var_val
        )
        self.randomInitializeRegister(scause_name)
        (scause_val, valid) = self.readRegister(scause_name)
        utils.assert_valid_register_value(
            self, scause_name, valid
        )
        scause_val = utils.combine_register_value_with_field_value(
            self,
            scause_name,
            scause_val,
            "EXCEPTION CODE_VAR",
            exception_code_var_val,
        )
        expected_sys_reg_state_data.append((scause_name, scause_val))

        stvec_name = "stvec"
        mode_val = RandomUtils.random32(0, 1)
        state.addSystemRegisterStateElementByField(
            stvec_name, "MODE", mode_val
        )
        self.randomInitializeRegister(stvec_name)
        (stvec_val, valid) = self.readRegister(stvec_name)
        utils.assert_valid_register_value(
            self, stvec_name, valid
        )
        stvec_val = utils.combine_register_value_with_field_value(
            self, stvec_name, stvec_val, "MODE", mode_val
        )
        expected_sys_reg_state_data.append((stvec_name, stvec_val))

        self._mExpectedStateData[
            EStateElementType.SystemRegister
        ] = expected_sys_reg_state_data

        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
