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

from DV.riscv.trees.instruction_tree import (
    RV_G_instructions,
    RV32_G_instructions,
)
from base.ChoicesModifier import ChoicesModifier
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


# This test evaluates invoking the privilege level switching function of the
# system call sequence using a wide variety of parameters.
class MainSequence(Sequence):
    def generate(self, **kargs):
        self._testSpecificPrivilegeLevelSwitches()
        self._testRandomPrivilegeLevelSwitches()

    # Test switching between predetermined privilege levels.
    def _testSpecificPrivilegeLevelSwitches(self):
        for priv_level in (3, 1, 0, 1, 3, 0, 3):
            self._genRandomInstructions()

            orig_state = self._getState()
            sys_call_params = {"PrivilegeLevel": priv_level}
            self.systemCall(sys_call_params)
            self._verifyState(orig_state, self._getState(), sys_call_params)

            self._genRandomInstructions()

    # Test switching between randomly selected privilege levels with randomly
    # selected parameters.
    def _testRandomPrivilegeLevelSwitches(self):
        for _ in range(10):
            self._genRandomInstructions()

            orig_state = self._getState()
            sys_call_params = self._generateSystemCallParameters()
            self.systemCall(sys_call_params)
            self._verifyState(orig_state, self._getState(), sys_call_params)

            self._genRandomInstructions()

    # Generate a short sequence of random instructions.
    def _genRandomInstructions(self):
        for _ in range(10):
            if self.getGlobalState("AppRegisterWidth") == 32:
                self.genInstruction(self.pickWeighted(RV32_G_instructions))
            else:
                self.genInstruction(self.pickWeighted(RV_G_instructions))

    # Get the current system state relevant for privilege level switching.
    def _getState(self):
        state = {}
        state["PrivilegeLevel"] = self.getPEstate("PrivilegeLevel")
        state["PC"] = self.getPEstate("PC")

        for reg_field_name in ("MIE", "SIE", "UIE", "SUM", "MXR", "MPRV"):
            (state[reg_field_name], valid) = self.readRegister(
                "mstatus", reg_field_name
            )
            if not valid:
                self.error(
                    "Register field mstatus.%s does not have a valid value"
                    % reg_field_name
                )

        return state

    # Randomly generate parameters for a privilege level switch.
    def _generateSystemCallParameters(self):
        params = {}
        function = self.choice(("SwitchPrivilegeLevel", None))
        if function is not None:
            params["Function"] = function

        priv_level = self.choice(("U", "S", "M", 0, 1, 3, "Random", None))
        if priv_level is not None:
            params["PrivilegeLevel"] = priv_level

        target_addr = self._generateTargetAddress(priv_level)
        if target_addr is not None:
            params["TargetAddr"] = target_addr

        # Don't skip validation if no target address is specified
        skip_addr_validation = self.choice((0, 1, None))
        if (skip_addr_validation is not None) and (
            ("TargetAddr" in params) or (skip_addr_validation != 1)
        ):
            params["SkipAddrValidation"] = skip_addr_validation

        if RandomUtils.random32(0, 4) == 4:
            choices_mod = ChoicesModifier(self.genThread)
            choices_mod.modifyPagingChoices(
                "Page Allocation Scheme",
                {"RandomFreeAlloc": 0, "FlatMapAlloc": 10},
            )
            params["AddrChoicesModID"] = choices_mod.registerSet()

        # TODO(Noah): Add testing for SUM, MXR and MPRV when support for
        #  changing those fields is established.
        # for param_name in ('InterruptMask', 'SUM', 'MXR', 'MPRV'):
        for param_name in ("InterruptMask",):
            param_val = self.choice((0, 1, "Same", "Flip", "Random", None))

            if param_val is not None:
                params[param_name] = param_val

        return params

    # Verify the system state is as expected after a privilege level switch.
    #
    #  @param aOrigState The system state prior to the privilege level switch.
    #  @param aCurrentState The system state after the privilege level switch.
    #  @param aSysCallParams Parameters used for the privilege level switch.
    def _verifyState(self, aOrigState, aCurrentState, aSysCallParams):
        expected_priv_level = None
        priv_level_param = aSysCallParams.get("PrivilegeLevel")
        if (priv_level_param == "U") or (priv_level_param == 0):
            expected_priv_level = 0
        elif (priv_level_param == "S") or (priv_level_param == 1):
            expetected_priv_level = 1
        elif (priv_level_param == "M") or (priv_level_param == 3):
            expected_priv_level = 3

        if (expected_priv_level is not None) and (
            aCurrentState["PrivilegeLevel"] != expected_priv_level
        ):
            self.error(
                "Current privilege level does not match the expected "
                "value. Expected=%d, Actual=%d"
                % (expected_priv_level, aCurrentState["PrivilegeLevel"])
            )

        expected_target_addr = aSysCallParams.get("TargetAddr")
        if (expected_target_addr is not None) and (
            aCurrentState["PC"] != expected_target_addr
        ):
            self.error(
                "Current PC does not match the expected value. "
                "Expected=0x%x, Actual=0x%x"
                % (expected_target_addr, aCurrentState["PC"])
            )

        priv_level_name = None
        if aCurrentState["PrivilegeLevel"] == 0:
            priv_level_name = "U"
        elif aCurrentState["PrivilegeLevel"] == 1:
            priv_level_name = "S"
        elif aCurrentState["PrivilegeLevel"] == 3:
            priv_level_name = "M"

        gen_mode = self.getPEstate("GenMode")
        no_iss = gen_mode & 0x1
        if no_iss != 1:
            interrupt_field_name = "%sIE" % priv_level_name
            expected_interrupt_mask = self._getExpectedRegisterFieldValue(
                interrupt_field_name,
                aSysCallParams.get("InterruptMask"),
                aOrigState,
            )

            # Ignore UIE; it is hardwired to 0 because the N extension is not
            # supported
            if (
                (expected_interrupt_mask is not None)
                and (aCurrentState["PrivilegeLevel"] != 0)
                and (
                    aCurrentState[interrupt_field_name]
                    != expected_interrupt_mask
                )
            ):
                self.error(
                    "Current mstatus.%s does not match the expected "
                    "value. Expected=0x%x, Actual=0x%x"
                    % (
                        interrupt_field_name,
                        expected_interrupt_mask,
                        aCurrentState[interrupt_field_name],
                    )
                )

            for reg_field_name in ("SUM", "MXR"):
                expected_field_val = self._getExpectedRegisterFieldValue(
                    reg_field_name,
                    aSysCallParams.get(reg_field_name),
                    aOrigState,
                )

                if (expected_field_val is not None) and (
                    aCurrentState[reg_field_name] != expected_field_val
                ):
                    self.error(
                        "Current mstatus.%s does not match the expected "
                        "value. Expected=0x%x, Actual=0x%x"
                        % (
                            reg_field_name,
                            expected_field_val,
                            aCurrentState[reg_field_name],
                        )
                    )

            expected_mprv_val = self._getExpectedRegisterFieldValue(
                "MPRV", aSysCallParams.get("MPRV"), aOrigState
            )
            if (
                (expected_mprv_val is not None)
                and (priv_level_name == "M")
                and (aCurrentState["MPRV"] != expected_mprv_val)
            ):
                self.error(
                    "Current mstatus.MPRV does not match the expected "
                    "value. Expected=0x%x, Actual=0x%x"
                    % (expected_mprv_val, aCurrentState["MPRV"])
                )

    # Randomly generate a target address for a privilege level switch.
    #
    #  @param aTargetPrivLevel The target privilege level.
    def _generateTargetAddress(self, aTargetPrivLevel):
        target_addr = self.choice((0, None))
        if target_addr is not None:
            current_priv_level = self.getPEstate("PrivilegeLevel")

            # Don't specify a target address if we don't know which privilege
            # level we're targeting; otherwise we can't be sure our target
            # address is valid
            if (aTargetPrivLevel is not None) and (
                aTargetPrivLevel != "Random"
            ):
                target_addr = self.genVA(
                    Size=4, Align=4, Type="I", PrivilegeLevel=aTargetPrivLevel
                )
            else:
                target_addr = None

        return target_addr

    # Determine the expected register field value based on the original
    # register field value and the specified parameter.
    #
    #  @param aRegFieldName Name of the register field.
    #  @param aRegFieldParam Parameter used to specify a value for the
    #       register field for the privilege level switch.
    #  @param aOrigState The register field value prior to the privilege level
    #       switch.
    def _getExpectedRegisterFieldValue(
        self, aRegFieldName, aRegFieldParam, aOrigState
    ):
        expected_field_val = None
        if aRegFieldParam == 0:
            expected_field_val = 0
        elif aRegFieldParam == 1:
            expected_field_val = 1
        elif aRegFieldParam == "Same":
            expected_field_val = aOrigState[aRegFieldName]
        elif aRegFieldParam == "Flip":
            expected_field_val = ~aOrigState[aRegFieldName] & 0x1

        return expected_field_val


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
