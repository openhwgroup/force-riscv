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

import state_transition_test_utils as utils
from StateTransitionHandlerTest import StateTransitionHandlerTest
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


# This test verifies that a baisc StateTransition can be executed with
# ByStateElementType order mode.
class MainSequence(Sequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        StateTransition.registerStateTransitionHandler(
            StateTransitionHandlerTest(self.genThread),
            EStateTransitionType.Explicit,
            (EStateElementType.Memory, EStateElementType.VmContext),
        )

        state = self._createState()
        state_elem_type_order = (
            EStateElementType.SystemRegister,
            EStateElementType.Memory,
            EStateElementType.VectorRegister,
            EStateElementType.FloatingPointRegister,
            EStateElementType.VmContext,
            EStateElementType.GPR,
            EStateElementType.PrivilegeLevel,
            EStateElementType.PC,
            EStateElementType.PredicateRegister,
        )
        StateTransition.transitionToState(
            state,
            EStateTransitionOrderMode.ByStateElementType,
            state_elem_type_order,
        )

        utils.verify_state(self, self._mExpectedStateData)

    # Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()

        expected_sys_reg_state_data = []

        mcause_name = "mcause"
        exception_code_var_val = RandomUtils.random32(0, 9)
        state.addSystemRegisterStateElementByField(
            mcause_name, "EXCEPTION CODE_VAR", exception_code_var_val
        )
        self.randomInitializeRegister(mcause_name)
        (mcause_val, valid) = self.readRegister(mcause_name)
        utils.assert_valid_register_value(
            self, mcause_name, valid
        )
        mcause_val = utils.combine_register_value_with_field_value(
            self,
            mcause_name,
            mcause_val,
            "EXCEPTION CODE_VAR",
            exception_code_var_val,
        )
        expected_sys_reg_state_data.append((mcause_name, mcause_val))

        mtvec_name = "mtvec"
        mode_val = RandomUtils.random32(0, 1)
        state.addSystemRegisterStateElementByField("mtvec", "MODE", mode_val)
        self.randomInitializeRegister(mtvec_name)
        (mtvec_val, valid) = self.readRegister(mtvec_name)
        utils.assert_valid_register_value(
            self, mtvec_name, valid
        )
        mtvec_val = utils.combine_register_value_with_field_value(
            self, mtvec_name, mtvec_val, "MODE", mode_val
        )
        expected_sys_reg_state_data.append((mtvec_name, mtvec_val))

        self._mExpectedStateData[
            EStateElementType.SystemRegister
        ] = expected_sys_reg_state_data

        self._mExpectedStateData[
            EStateElementType.GPR
        ] = utils.add_random_gpr_state_elements(
            self, state, RandomUtils.random32(0, 5)
        )
        self._mExpectedStateData[
            EStateElementType.PC
        ] = utils.add_random_pc_state_element(self, state)

        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
