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

from VectorTestSequence import VectorTestSequence
from base.ChoicesModifier import ChoicesModifier
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


#  This test verifies that a basic add vector instruction can be generated and
#  executed. It verifies that the initial values are correctly communicated to
# the simulator and that the resulting values are successfully returned. The
# test assumes the use of 512-bit vector registers and 32-bit vector register
# elements.
class MainSequence(VectorTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = ("VADD.VV##RISCV",)
        self._mRegIndex1 = None
        self._mRegIndex2 = None
        self._mElemVals1 = None
        self._mElemVals2 = None

    # Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        # Ensure vector element size is set to 32 bits and vector register
        # group size is set to 1
        choices_mod = ChoicesModifier(self.genThread)
        vsew_choice_weights = {
            "0x0": 0,
            "0x1": 0,
            "0x2": 10,
            "0x3": 0,
            "0x4": 0,
            "0x5": 0,
            "0x6": 0,
            "0x7": 0,
        }
        choices_mod.modifyRegisterFieldValueChoices(
            "vtype.VSEW", vsew_choice_weights
        )
        vlmul_choice_weights = {
            "0x0": 10,
            "0x1": 0,
            "0x2": 0,
            "0x3": 0,
            "0x4": 0,
            "0x5": 0,
            "0x6": 0,
            "0x7": 0,
        }
        choices_mod.modifyRegisterFieldValueChoices(
            "vtype.VLMUL", vlmul_choice_weights
        )
        choices_mod.commitSet()

        (self._mRegIndex1, self._mRegIndex2) = self.getRandomRegisters(
            2, "VECREG", exclude="0"
        )
        self._mElemVals1 = self._initializeVectorRegister(
            "v%d" % self._mRegIndex1
        )
        self._mElemVals2 = self._initializeVectorRegister(
            "v%d" % self._mRegIndex2
        )

    # Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    # Return parameters to be passed to Sequence.genInstruction().
    def _getInstructionParameters(self):
        return {
            "vd": self._mRegIndex1,
            "vs1": self._mRegIndex1,
            "vs2": self._mRegIndex2,
            "vm": 1,
        }

    # Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        for (elem_index, val) in enumerate(self._mElemVals2):
            self._mElemVals1[elem_index] += val

        reg_name_1 = "v%d" % self._mRegIndex1
        for sub_index in range(8):
            field_name = "%s_%d" % (reg_name_1, sub_index)
            (field_val, valid) = self.readRegister(
                reg_name_1, field=field_name
            )
            self.assertValidRegisterValue(reg_name_1, valid)
            expected_field_val = self._getFieldValue(
                sub_index, self._mElemVals1
            )

            if field_val != expected_field_val:
                self.error(
                    "Register field %s has unexpected value; "
                    "Expected=0x%x, Actual=0x%x"
                    % (field_name, expected_field_val, field_val)
                )

    # Initialize the specified vector register and return a list of 32-bit
    # element values.
    def _initializeVectorRegister(self, aRegName):
        elem_vals = []
        for elem_index in range(16):
            elem_val = RandomUtils.random32(0, 0xFFFF)
            elem_vals.append(elem_val)

        for sub_index in range(8):
            field_name = "%s_%d" % (aRegName, sub_index)
            field_val = self._getFieldValue(sub_index, elem_vals)
            self.initializeRegisterFields(aRegName, {field_name: field_val})

        return elem_vals

    # Get the value of a 64-bit field for a vector register.
    #
    #  @param aSubIndex A 64-bit vector register field index.
    #  @param aElemVals A list of 32-bit element values.
    def _getFieldValue(self, aSubIndex, aElemVals):
        field_value = aElemVals[2 * aSubIndex]
        field_value |= aElemVals[2 * aSubIndex + 1] << 32
        return field_value


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
