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
from VectorTestSequence import VectorTestSequence
from base.ChoicesModifier import ChoicesModifier
import RandomUtils

## This test verifies that a quad-widening integer multiply-add vector instruction can be generated
# and executed. It verifies that the initial values are correctly communicated to the simulator and
# that the resulting values are successfully returned. The test assumes the use of 512-bit vector
# registers and 16-bit vector register elements.
class MainSequence(VectorTestSequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = ('VQMACCU.VV##RISCV',)
        self._mElem64Vals = []

    ## Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        # Ensure vector element size is set to 16 bits and vector register group size is set to 1
        choices_mod = ChoicesModifier(self.genThread)
        vsew_choice_weights = {'0x0': 0, '0x1': 10, '0x2': 0, '0x3': 0, '0x4': 0, '0x5': 0, '0x6': 0, '0x7': 0}
        choices_mod.modifyRegisterFieldValueChoices('vtype.VSEW', vsew_choice_weights)
        vlmul_choice_weights = {'0x0': 10, '0x1': 0, '0x2': 0, '0x3': 0, '0x4': 0, '0x5': 0, '0x6': 0, '0x7': 0}
        choices_mod.modifyRegisterFieldValueChoices('vtype.VLMUL', vlmul_choice_weights)
        choices_mod.commitSet()

        self._initializeVectorRegisters()

    ## Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    ## Return parameters to be passed to Sequence.genInstruction().
    def _getInstructionParameters(self):
        return {'vm': 1}

    ## Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        self._verifyQuadWideningMultiplyAdd(aInstrRecord)

    ## Initialize all vector registers and populate a list of lists of 64-bit element values.
    def _initializeVectorRegisters(self):
        for reg_index in range(0, 32):
            reg_name = 'v%d' % reg_index
            self._mElem64Vals.append([])

            for elem64_index in range(8):
                field_name = '%s_%d' % (reg_name, elem64_index)
                field_val = RandomUtils.random64()
                self.initializeRegisterFields(reg_name, {field_name: field_val})
                self._mElem64Vals[reg_index].append(field_val)

    ## Verify that the destination registers of a quad-widening multiply-add instruction are updated
    # wtih the expected values.
    #
    #  @param aInstrRecord A dictionary describing the generated instruction.
    def _verifyQuadWideningMultiplyAdd(self, aInstrRecord):
        vd_val = aInstrRecord['Dests']['vd']
        vs1_val = aInstrRecord['Srcs']['vs1']
        vs2_val = aInstrRecord['Srcs']['vs2']

        for elem16_index in range(32):
            elem64_index = elem16_index % 8
            dest_reg_index = vd_val + elem16_index // 8

            expected_field_val = self._mElem64Vals[dest_reg_index][elem64_index]
            expected_field_val += self._getElem16Val(vs1_val, elem16_index) * self._getElem16Val(vs2_val, elem16_index)
            expected_field_val &= 0xFFFFFFFFFFFFFFFF
            self._mElem64Vals[dest_reg_index][elem64_index] = expected_field_val

            dest_reg_name = 'v%d' % dest_reg_index
            field_name = '%s_%d' % (dest_reg_name, elem64_index)
            (field_val, valid) = self.readRegister(dest_reg_name, field=field_name)
            self.assertValidRegisterValue(dest_reg_name, valid)

            if field_val != expected_field_val:
                self.error('Register field %s has unexpected value; Expected=0x%x, Actual=0x%x' % (field_name, expected_field_val, field_val))

    ## Get the value of a 16-bit element for the specified register and element indices.
    #
    #  @param aRegIndex A vector register index.
    #  @param aElem16Index A 16-bit vector register element index.
    def _getElem16Val(self, aRegIndex, aElem16Index):
        elem64_index = aElem16Index // 4
        shift = (aElem16Index % 4) * 16
        elem64_val = self._mElem64Vals[aRegIndex][elem64_index]
        elem16_val = (elem64_val >> shift) & 0xFFFF
        return elem16_val


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
