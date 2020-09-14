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
from base.UtilityFunctions import mask_to_size
from Constraint import ConstraintSet
import RandomUtils

## This test verifies that unit-stride load and store instructions can be generated and executed
# successfully.
class MainSequence(VectorTestSequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            'VLE16.V##RISCV',
            'VLE32.V##RISCV',
            'VLE64.V##RISCV',
            'VLE8.V##RISCV',
            'VSE16.V##RISCV',
            'VSE32.V##RISCV',
            'VSE64.V##RISCV',
            'VSE8.V##RISCV',
        )

        self._mUnalignedAllowed = False
        self._mTargetAddrConstr = None

    ## Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        if RandomUtils.random32(0, 1) == 1:
            choices_mod = ChoicesModifier(self.genThread)
            choice_weights = {'Aligned': 80, 'Unaligned': 20}
            choices_mod.modifyOperandChoices('Data alignment', choice_weights)
            choices_mod.commitSet()
            self._mUnalignedAllowed = True

    ## Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    ## Return parameters to be passed to Sequence.genInstruction().
    def _getInstructionParameters(self):
        self._mTargetAddrConstr = None
        instr_params = {}
        if RandomUtils.random32(0, 1) == 1:
            if RandomUtils.random32(0, 1) == 1:
                instr_params['NoPreamble'] = 1

            target_choice = RandomUtils.random32(0, 2)
            if target_choice == 1:
                target_addr = self.genVA(Size=512, Align=8, Type='D')
                self._mTargetAddrConstr = ConstraintSet(target_addr)
                instr_params['LSTarget'] = str(self._mTargetAddrConstr)
            elif target_choice == 2:
                va_range_size = RandomUtils.random32()
                min_target_addr = self.genVA(Size=512, Align=8, Type='D')
                max_target_addr = mask_to_size((min_target_addr + RandomUtils.random32()), 64)
                self._mTargetAddrConstr = ConstraintSet(min_target_addr, max_target_addr)
                instr_params['LSTarget'] = str(self._mTargetAddrConstr)

        return instr_params

    ## Return true if it is permissible for the generation to skip this instruction.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrParams The parameters passed to Sequence.genInstruction().
    def _isSkipAllowed(self, aInstr, aInstrParams):
        # Instructions with constraint parameters may legitimately be skipped sometimes if no
        # solution can be determined, but instructions without constrained parameters should always
        # generate
        if aInstrParams:
            return True

        return False

    ## Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        if (self._mTargetAddrConstr is not None) and (not self._mTargetAddrConstr.containsValue(aInstrRecord['LSTarget'])):
            self.error('Target address 0x%x was outside of the specified constraint %s' % (aInstrRecord['LSTarget'], self._mTargetAddrConstr))

    ## Get allowed exception codes.
    def _getAllowedExceptionCodes(self):
        allowed_except_codes = set()
        if self._mUnalignedAllowed:
            allowed_except_codes.add(0x4)
            allowed_except_codes.add(0x6)

        # TODO(Noah): Remove the line below permitting store page fault exceptions when the page
        # descriptor generation is improved. Currently, we are generating read-only pages for load
        # instructions, which is causing subsequent store instructions to the same page to fault.
        allowed_except_codes.add(0xF)

        return allowed_except_codes


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
