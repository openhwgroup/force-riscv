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

## This test verifies that a quad-widening integer multiply-add vector instruction can be generated
# and executed. It verifies that the initial values are correctly communicated to the simulator and
# that the resulting values are successfully returned. The test assumes the use of 512-bit vector
# registers and 16-bit vector register elements.
class MainSequence(Sequence):

    def generate(self, **kargs):
        # Ensure vector element size is set to 16 bites
        choices_mod = ChoicesModifier(self.genThread)
        choice_weights = {'0x0': 0, '0x1': 10, '0x2': 0, '0x3': 0, '0x4': 0, '0x5': 0, '0x6': 0, '0x7': 0}
        choices_mod.modifyRegisterFieldValueChoices('vtype.VSEW', choice_weights)
        choices_mod.commitSet()

        elem64_vals = self._initializeVectorRegisters()

        for _ in range(RandomUtils.random32(25, 50)):
            instr_id = self.genInstruction('VQMACCU.VV##RISCV', {'vm': 1})
            instr_record = self.queryInstructionRecord(instr_id)
            self._verifyQuadWideningMultiplyAdd(instr_record, elem64_vals)

    ## Initialize all vector registers and return a list of lists of 64-bit element values.
    def _initializeVectorRegisters(self):
        elem64_vals = []
        for reg_index in range(0, 32):
            reg_name = 'v%d' % reg_index
            elem64_vals.append([])

            for elem64_index in range(8):
                field_name = '%s_%d' % (reg_name, elem64_index)
                field_val = RandomUtils.random64()
                self.initializeRegisterFields(reg_name, {field_name: field_val})
                elem64_vals[reg_index].append(field_val)

        return elem64_vals

    ## Verify that the destination registers of a quad-widening multiply-add instruction are updated
    # wtih the expected values.
    #
    #  @param aInstrRecord A dictionary describing the generated instruction.
    #  @param aElem64Vals A list of lists of 64-bit element values.
    def _verifyQuadWideningMultiplyAdd(self, aInstrRecord, aElem64Vals):
        vd_val = aInstrRecord['Dests']['vd']
        vs1_val = aInstrRecord['Srcs']['vs1']
        vs2_val = aInstrRecord['Srcs']['vs2']

        for elem16_index in range(32):
            elem64_index = elem16_index % 8
            dest_reg_index = vd_val + elem16_index // 8

            expected_field_val = aElem64Vals[dest_reg_index][elem64_index]
            expected_field_val += self._getElem16Val(vs1_val, elem16_index, aElem64Vals) * self._getElem16Val(vs2_val, elem16_index, aElem64Vals)
            expected_field_val &= 0xFFFFFFFFFFFFFFFF
            aElem64Vals[dest_reg_index][elem64_index] = expected_field_val

            dest_reg_name = 'v%d' % dest_reg_index
            field_name = '%s_%d' % (dest_reg_name, elem64_index)
            (field_val, valid) = self.readRegister(dest_reg_name, field=field_name)
            self._assertValidRegisterValue(dest_reg_name, valid)

            if field_val != expected_field_val:
                self.error('Register field %s has unexpected value; Expected=0x%x, Actual=0x%x' % (field_name, expected_field_val, field_val))

    ## Get the value of a 16-bit element for the specified register and element indices.
    #
    #  @param aRegIndex A vector register index.
    #  @param aElem16Index A 16-bit vector register element index.
    #  @param aElem64Vals A list of lists of 64-bit element values.
    def _getElem16Val(self, aRegIndex, aElem16Index, aElem64Vals):
        elem64_index = aElem16Index // 4
        shift = (aElem16Index % 4) * 16
        elem64_val = aElem64Vals[aRegIndex][elem64_index]
        elem16_val = (elem64_val >> shift) & 0xFFFF
        return elem16_val

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
