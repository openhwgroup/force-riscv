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
from base.Sequence import Sequence
from base.ChoicesModifier import ChoicesModifier
from base.UtilityFunctions import mask_to_size
from riscv.AssemblyHelperRISCV import AssemblyHelperRISCV
from Constraint import ConstraintSet
import RandomUtils

## This class provides a common execution flow for testing vector instructions.
class VectorTestSequence(Sequence):

    def generate(self, **kargs):
        self._setUpTest()

        max_instr_count = self._getMaxInstructionCount()
        instr_count = RandomUtils.random32((max_instr_count // 2), max_instr_count)
        for _ in range(instr_count):
            instr = self.choice(self._getInstructionList())
            instr_params = self._getInstructionParameters()
            instr_id = self.genInstruction(instr, instr_params)

            self._verifyInstruction(instr, instr_params, instr_id)

    ## Fail if the valid flag is false.
    #
    #  @param aRegName The index of the register.
    #  @param aValid A flag indicating whether the specified register has a valid value.
    def assertValidRegisterValue(self, aRegName, aValid):
        if not aValid:
            self.error('Value for register %s is invalid' % aRegName)

    ## Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        pass

    ## Return the maximum number of test instructions to generate.
    def _getMaxInstructionCount(self):
        return 100

    ## Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        raise NotImplementedError

    ## Return parameters to be passed to Sequence.genInstruction().
    def _getInstructionParameters(self):
        return {}

    ## Verify the instruction generated and executed as expected without triggering an exception.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrParams The parameters passed to Sequence.genInstruction().
    #  @param aInstrId The record ID of the generated instruction.
    def _verifyInstruction(self, aInstr, aInstrParams, aInstrId):
        if aInstrId:
            instr_record = self.queryInstructionRecord(aInstrId)
            self._performAdditionalVerification(aInstr, instr_record)
        elif not self._isSkipAllowed(aInstr, aInstrParams):
            self.error('Instruction %s did not generate correctly' % aInstr)

        except_count = 0
        disallowed_except_codes = set((0x2, 0x4, 0x5, 0x6, 0x7, 0xD, 0xF)) - self._getAllowedExceptionCodes()
        for except_code in disallowed_except_codes:
            except_count += self.queryExceptionRecordsCount(except_code)

        if except_count != 0:
            self.error('Instruction %s did not execute correctly' % aInstr)

    ## Return true if it is permissible for the generation to skip this instruction.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrParams The parameters passed to Sequence.genInstruction().
    def _isSkipAllowed(self, aInstr, aInstrParams):
        return False

    ## Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        pass

    ## Get allowed exception codes.
    def _getAllowedExceptionCodes(self):
        return set()


## This class provides some common parameters for testing vector load/store instructions.
class VectorLoadStoreTestSequence(VectorTestSequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mUnalignedAllowed = False
        self._mTargetAddrConstr = None
        self._mExceptCount = 0

    ## Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        if RandomUtils.random32(0, 1) == 1:
            choices_mod = ChoicesModifier(self.genThread)
            choice_weights = {'Aligned': 80, 'Unaligned': 20}
            choices_mod.modifyOperandChoices('Data alignment', choice_weights)
            choices_mod.commitSet()
            self._mUnalignedAllowed = True

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
        # Instructions with constrained parameters may legitimately be skipped sometimes if no
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

        # TODO(Noah): Remove the call to _resetVstart() when the issue with vstart after an
        # exception handler skips a vector instruction is resolved.
        self._genResetVstart()

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

    ## Generate an instruction to reset vstart to 0 if it is not currently 0. This is necessary when
    # an exception handler handling a fault triggered by a vector instruction decides to skip the
    # instruction.
    def _genResetVstart(self):
        except_count = 0
        for except_code in self._getAllowedExceptionCodes():
            except_count += self.queryExceptionRecordsCount(except_code)

        if except_count > self._mExceptCount:
            assembly_helper = AssemblyHelperRISCV(self)
            assembly_helper.genWriteSystemRegister('vstart', 0)
            self._mExceptCount = except_count
