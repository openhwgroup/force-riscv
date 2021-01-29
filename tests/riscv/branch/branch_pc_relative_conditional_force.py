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
from base.Sequence import Sequence

## This test verifies that conditional branches go to the expected address based on the offset
# operand value.
class MainSequence(Sequence):

    def generate(self, **kargs):
        if self.getGlobalState('AppRegisterWidth') == 32:
            random_instructions = ['ADD##RISCV', 'SRLI#RV32I#RISCV', 'ADDI##RISCV', 'SLLI#RV32I#RISCV', 'LUI##RISCV']
        else:
            random_instructions = ['ADDW##RISCV', 'SRLI#RV64I#RISCV', 'ADDI##RISCV', 'SLLI#RV64I#RISCV', 'LUI##RISCV']
        branch_instructions = ['BEQ##RISCV', 'BNE##RISCV', 'BLT##RISCV', 'BGE##RISCV', 'BLTU##RISCV', 'BGEU##RISCV']
        for _ in range(10):
            for _ in range(self.random32(0, 5)):
                self.genInstruction(self.choice(random_instructions))

            instr_id = self.genInstruction(self.choice(branch_instructions))
            instr_record = self.queryInstructionRecord(instr_id)
            instr_pc = instr_record['VA']
            target_addr = instr_record['BRTarget']
            expected_target_addr = self._getExpectedTargetAddress(instr_pc, instr_record['Addressing']['Offset'][0])
            if target_addr != expected_target_addr:
                self.error('Unexpected target address: Expected = 0x%x, Actual = 0x%x' % (expected_target_addr, target_addr))

            if self.getPEstate('PC') not in (expected_target_addr, (instr_pc + 4)):
                self.error('Unexpected PC value: Expected = 0x%x, Actual = 0x%x' % (expected_target_addr, self.getPEstate('PC')))

    ## Compute the expected target address for a conditional branch instruction given the original
    # PC value and the offset operand value.
    #
    #  @param aOrigPc The PC value of the conditional branch instruction.
    #  @param aOffsetOprValue The offset operand value for the conditional branch instruction.
    def _getExpectedTargetAddress(self, aOrigPc, aOffsetOprValue):

        expected_target_addr = 0
        self.debug('_getExpectedTargetAddress aOrigPc:{:#x}, aOffsetOprValue:{:#x}'.format(aOrigPc, aOffsetOprValue))

        #process negative/positive 12b simm value | offset is multiple of 2 bytes, so lsh 1 gets the true byte offset
        if ((aOffsetOprValue & 0x800) != 0):
            offset = (((aOffsetOprValue ^ 0xFFF) + 1) << 1)
            expected_target_addr = aOrigPc - offset
        else:
            offset = aOffsetOprValue << 1
            expected_target_addr = aOrigPc + offset

        return expected_target_addr


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

