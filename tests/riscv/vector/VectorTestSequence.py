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
        if not self._isSkipAllowed(aInstr, aInstrParams):
            instr_record = self.queryInstructionRecord(aInstrId)

            if instr_record is None:
                self.error('Instruction %s did not generate correctly' % aInstr)

            self._performAdditionalVerification(aInstr, instr_record)

        except_count = 0
        for except_code in (0x2, 0x4, 0x5, 0x6, 0x7, 0xD, 0xF):
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
