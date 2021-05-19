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
import math
import re

import RandomUtils
from Config import Config
from Constraint import ConstraintSet
from Enums import ELimitType

from base.ChoicesModifier import ChoicesModifier
from base.Sequence import Sequence
from base.TestUtils import assert_equal
from base.UtilityFunctions import mask_to_size


#  This class provides a common execution flow for testing vector instructions.
class VectorTestSequence(Sequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExceptCounts = {
            0x2: 0,
            0x4: 0,
            0x5: 0,
            0x6: 0,
            0x7: 0,
            0xD: 0,
            0xF: 0,
        }

    def generate(self, **kargs):
        self._setUpTest()

        max_instr_count = self._getMaxInstructionCount()
        instr_count = RandomUtils.random32((max_instr_count // 2), max_instr_count)
        for _ in range(instr_count):
            instr = self.choice(self._getInstructionList())
            instr_params = self._getInstructionParameters()
            instr_id = self.genInstruction(instr, instr_params)

            self._verifyInstruction(instr, instr_params, instr_id)

    # Calculate the value of LMUL given VLMUL.
    #
    #  @param aVlmul The value of the vtype.VLMUL field.
    def calculateLmul(self, aVlmul):
        lmul = 2 ** (aVlmul & 0x3)
        if (aVlmul & 0x4) == 0x4:
            lmul /= 16

        return lmul

    # Calculate the value of SEW given VSEW.
    #
    #  @param aVsew The value of the vtype.VSEW field.
    def calculateSew(self, aVsew):
        sew = 8 * (2 ** aVsew)
        return sew

    # Fail if the valid flag is false.
    #
    #  @param aRegName The name of the register.
    #  @param aValid A flag indicating whether the specified register has a
    #       valid value.
    def assertValidRegisterValue(self, aRegName, aValid):
        if not aValid:
            self.error("Value for register %s is invalid" % aRegName)

    # Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        pass

    # Return the maximum number of test instructions to generate.
    def _getMaxInstructionCount(self):
        return 100

    # Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        raise NotImplementedError

    # Return parameters to be passed to Sequence.genInstruction().
    def _getInstructionParameters(self):
        return {}

    # Verify the instruction generated and executed as expected without
    # triggering an exception.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrParams The parameters passed to Sequence.genInstruction().
    #  @param aInstrId The record ID of the generated instruction.
    def _verifyInstruction(self, aInstr, aInstrParams, aInstrId):
        if aInstrId:
            instr_record = self.queryInstructionRecord(aInstrId)
            self._performAdditionalVerification(aInstr, instr_record)
        elif not self._isSkipAllowed(aInstr, aInstrParams):
            self.error("Instruction %s did not generate correctly" % aInstr)

        for except_code in self._mExceptCounts:
            self._verifyExceptionCount(aInstr, except_code)

    # Return true if it is permissible for the generation to skip this
    # instruction.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrParams The parameters passed to Sequence.genInstruction().
    def _isSkipAllowed(self, aInstr, aInstrParams):
        return False

    # Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        pass

    # Verify the instruction didn't unexpectedly trigger the specified
    # exception.
    #
    #  @param aInstr The name of the instruction.
    #  @param aExceptCode The exception code.
    def _verifyExceptionCount(self, aInstr, aExceptCode):
        new_except_count = self.queryExceptionRecordsCount(aExceptCode)
        if new_except_count > self._mExceptCounts[aExceptCode]:
            if aExceptCode in self._getAllowedExceptionCodes(aInstr):
                self._mExceptCounts[aExceptCode] = new_except_count
            else:
                self.error("Instruction %s did not execute correctly" % aInstr)

    # Get allowed exception codes.
    #
    #  @param aInstr The name of the instruction.
    def _getAllowedExceptionCodes(self, aInstr):
        return set()


#  This class provides some common parameters for testing vector load/store
#  instructions.
class VectorLoadStoreTestSequence(VectorTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mUnalignedAllowed = False
        self._mTargetAddrConstr = None

    # Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        if RandomUtils.random32(0, 1) == 1:
            choices_mod = ChoicesModifier(self.genThread)
            choice_weights = {"Aligned": 80, "Unaligned": 20}
            choices_mod.modifyOperandChoices("Data alignment", choice_weights)
            choices_mod.commitSet()
            self._mUnalignedAllowed = True

    # Return parameters to be passed to Sequence.genInstruction().
    def _getInstructionParameters(self):
        self._mTargetAddrConstr = None
        instr_params = {}
        if RandomUtils.random32(0, 1) == 1:
            instr_params = self._generateInstructionParameters()

        return instr_params

    # Return true if it is permissible for the generation to skip this
    # instruction.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrParams The parameters passed to Sequence.genInstruction().
    def _isSkipAllowed(self, aInstr, aInstrParams):
        if aInstrParams or (self._calculateEmul(aInstr) > 8):
            return True

        return False

    # Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        if (
            (self._mTargetAddrConstr is not None)
            and (0x2 not in self._getAllowedExceptionCodes(aInstr))
            and (not self._mTargetAddrConstr.containsValue(aInstrRecord["LSTarget"]))
        ):
            self.error(
                "Target address 0x%x was outside of the specified "
                "constraint %s" % (aInstrRecord["LSTarget"], self._mTargetAddrConstr)
            )

    # Generate parameters for a load or store instruction.
    def _generateInstructionParameters(self):
        instr_params = {}
        if RandomUtils.random32(0, 1) == 1:
            instr_params["NoPreamble"] = 1

        target_choice = RandomUtils.random32(0, 2)
        if target_choice == 1:
            target_addr = self.genVA(Size=512, Align=8, Type="D")
            self._mTargetAddrConstr = ConstraintSet(target_addr)
            instr_params["LSTarget"] = str(self._mTargetAddrConstr)
        elif target_choice == 2:
            va_range_size = RandomUtils.random32()
            min_target_addr = self.genVA(Size=512, Align=8, Type="D")
            max_target_addr = mask_to_size((min_target_addr + RandomUtils.random32()), 64)
            self._mTargetAddrConstr = ConstraintSet(min_target_addr, max_target_addr)
            instr_params["LSTarget"] = str(self._mTargetAddrConstr)

        return instr_params

    # Get allowed exception codes.
    #
    #  @param aInstr The name of the instruction.
    def _getAllowedExceptionCodes(self, aInstr):
        allowed_except_codes = set()
        if self._mUnalignedAllowed:
            allowed_except_codes.add(0x4)
            allowed_except_codes.add(0x6)

        if self._calculateEmul(aInstr) > 8:
            allowed_except_codes.add(0x2)

        allowed_except_codes.add(0xF)

        return allowed_except_codes

    # Calculate EMUL for the given instruction.
    #
    #  @param aInstr The name of the instruction.
    def _calculateEmul(self, aInstr):
        eew = self._getEew(aInstr)

        (vlmul_val, valid) = self.readRegister("vtype", field="VLMUL")
        self.assertValidRegisterValue("vtype", valid)
        lmul = self.calculateLmul(vlmul_val)

        (vsew_val, valid) = self.readRegister("vtype", field="VSEW")
        self.assertValidRegisterValue("vtype", valid)
        sew = self.calculateLmul(vsew_val)

        return round((eew / sew) * lmul)

    # Determine EEW for the given instruction.
    #
    #  @param aInstr The name of the instruction.
    def _getEew(self, aInstr):
        match = re.fullmatch(r"V[A-Z]+(\d+)\.V\#\#RISCV", aInstr)
        return int(match.group(1))
