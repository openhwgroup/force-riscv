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
from base.ChoicesModifier import ChoicesModifier
import RandomUtils

## This test verifies that a basic add vector instruction can be generated and executed. It verifies
# that the initial values are correctly communicated to the simulator and that the resulting values
# are successfully returned. The test assumes the use of 512-bit vector registers and 32-bit vector
# register elements.
class MainSequence(Sequence):

    def generate(self, **kargs):
        # Ensure vector element size is set to 32 bites
        choices_mod = ChoicesModifier(self.genThread)
        choice_weights = {'0x0': 0, '0x1': 0, '0x2': 10, '0x3': 0, '0x4': 0, '0x5': 0, '0x6': 0, '0x7': 0}
        choices_mod.modifyRegisterFieldValueChoices('vtype.VSEW', choice_weights)
        choices_mod.commitSet()

        (reg_index_1, reg_index_2) = self.getRandomRegisters(2, 'VECREG', exclude='0')
        reg_name_1 = 'v%d' % reg_index_1
        reg_name_2 = 'v%d' % reg_index_2
        elem_vals_1 = self._initializeVectorRegister(reg_name_1)
        elem_vals_2 = self._initializeVectorRegister(reg_name_2)

        for _ in range(RandomUtils.random32(25, 50)):
            self.genInstruction('VADD.VV##RISCV', {'vd': reg_index_1, 'vs1': reg_index_1, 'vs2': reg_index_2, 'vm': 1})

            for (elem_index, val) in enumerate(elem_vals_2):
                elem_vals_1[elem_index] += val

            for sub_index in range(8):
                field_name = '%s_%d' % (reg_name_1, sub_index)
                (field_val, valid) = self.readRegister(reg_name_1, field=field_name)
                self._assertValidRegisterValue(reg_name_1, valid)
                expected_field_val = self._getFieldValue(sub_index, elem_vals_1)

                if field_val != expected_field_val:
                    self.error('Register field %s has unexpected value; Expected=0x%x, Actual=0x%x' % (field_name, expected_field_val, field_val))

    ## Initialize the specified vector register and return a list of 32-bit element values.
    def _initializeVectorRegister(self, aRegName):
        elem_vals = []
        for elem_index in range(16):
            elem_val = RandomUtils.random32(0, 0xFFFF)
            elem_vals.append(elem_val)

        for sub_index in range(8):
            field_name = '%s_%d' % (aRegName, sub_index)
            field_val = self._getFieldValue(sub_index, elem_vals)
            self.initializeRegisterFields(aRegName, {field_name: field_val})

        return elem_vals

    ## Get the value of a 64-bit field for a vector register.
    #
    #  @param aSubIndex A 64-bit vector register field index.
    #  @param aElemVals A list of 32-bit element values.
    def _getFieldValue(self, aSubIndex, aElemVals):
        field_value = aElemVals[2 * aSubIndex]
        field_value |= aElemVals[2 * aSubIndex + 1] << 32
        return field_value

    ## Fail if the valid flag is false.
    #
    #  @param aRegName The index of the register.
    #  @param aValid A flag indicating whether the specified register has a valid value.
    def _assertValidRegisterValue(self, aRegName, aValid):
        if not aValid:
            self.error('Value for register %s is invalid' % aRegName)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
