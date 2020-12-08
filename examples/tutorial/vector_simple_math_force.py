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
from VectorTestSequence import VectorTestSequence
from base.ChoicesModifier import ChoicesModifier
import RandomUtils

## This test verifies that a basic add vector instruction can be generated and executed. It verifies
# that the initial values are correctly communicated to the simulator and that the resulting values
# are successfully returned. The test assumes the use of 512-bit vector registers.
class MainSequence(VectorTestSequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = ('VADD.VV##RISCV',
                            'VADD.VX##RISCV',
                            'VADD.VI##RISCV',
                            'VSUB.VV##RISCV',
                            'VSUB.VX##RISCV',)
        self._mRegIndex1 = None
        self._mRegIndex2 = None

    ## Entry point into the test.
    def generate(self, **kargs):
        super().generate(**kargs)
        self._logChoices()

    ## Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        # Allowing fractional VLMULs and any valid VSEW values
        choices_mod = ChoicesModifier(self.genThread)
        vsew_choice_weights = {'0x0': 0, '0x1': 10, '0x2': 10, '0x3': 10, '0x4': 0, '0x5': 0, '0x6': 0, '0x7': 0}
        choices_mod.modifyRegisterFieldValueChoices('vtype.VSEW', vsew_choice_weights)
        vlmul_choice_weights = {'0x0': 10, '0x1': 0, '0x2': 0, '0x3': 0, '0x4': 0, '0x5': 10, '0x6': 10, '0x7': 10}
        choices_mod.modifyRegisterFieldValueChoices('vtype.VLMUL', vlmul_choice_weights)
        choices_mod.commitSet()

        (self._mRegIndex1, self._mRegIndex2) = self.getRandomRegisters(2, 'VECREG', exclude='0')
        self._initializeVectorRegister('v%d' % self._mRegIndex1)
        self._initializeVectorRegister('v%d' % self._mRegIndex2)

    ## Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    ## Return parameters to be passed to Sequence.genInstruction().
    def _getInstructionParameters(self):
        return {'vd': self._mRegIndex1, 'vs1': self._mRegIndex1, 'vs2': self._mRegIndex2, 'vm': 1}

    ## Initialize the specified vector register.
    def _initializeVectorRegister(self, aRegName):
        for sub_index in range(8):
            field_value = RandomUtils.random64(0, 0xFFFFFFFF)
            field_name = '%s_%d' % (aRegName, sub_index)
            self.initializeRegisterFields(aRegName, {field_name: field_value})

    ## Logging selected VLMUL and VSEW choices
    def _logChoices(self):
        (vlmul, _) = self.readRegister('vtype', field='VLMUL')
        (vsew, _) = self.readRegister('vtype', field='VSEW')
        if vlmul == 0x0:
            lmul = '1'
        elif vlmul == 0x5:
            lmul = '1/8'
        elif vlmul == 0x6:
            lmul = '1/4'
        elif vlmul == 0x7:
            lmul = '1/2'
        if vsew == 0x1:
            sew = '16'
        elif vsew == 0x2:
            sew = '32'
        elif vsew == 0x3:
            sew = '64'
        self.notice('LMUL: %s' % lmul)
        self.notice('SEW: %s' % sew)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
