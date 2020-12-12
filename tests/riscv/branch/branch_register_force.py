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

## This test verifies that unconditional jumps go to the expected address based on the register
# operand value.
class MainSequence(Sequence):

    def generate(self, **kargs):
        if self.getGlobalState('AppRegisterWidth') == 32:
            random_instructions = ['ADD##RISCV', 'SRLI#RV32I#RISCV', 'ADDI##RISCV', 'SLLI#RV32I#RISCV', 'LUI##RISCV']
        else:
            random_instructions = ['ADDW##RISCV', 'SRLI#RV64I#RISCV', 'ADDI##RISCV', 'SLLI#RV64I#RISCV', 'LUI##RISCV']
        branch_instr = 'JALR##RISCV'
        for _ in range(10):
            for _ in range(self.random32(0, 5)):
                self.genInstruction(self.choice(random_instructions))

            instr_id = self.genInstruction(branch_instr)
            instr_record = self.queryInstructionRecord(instr_id)


            # Skip instructions where the base register and destination register are the same, as
            # the base register will be overwritten with the return address, preventing calculation
            # of the expected target address
            base_reg_index = instr_record['Addressing']['Base'][0]
            dest_reg_index = instr_record['Dests']['rd']
            if base_reg_index != dest_reg_index:
                target_addr = instr_record['BRTarget']
                expected_target_addr = self._getExpectedTargetAddress(base_reg_index, instr_record['Addressing']['Offset'][0])

                if target_addr != expected_target_addr:
                    self.error('Unexpected target address: Expected = 0x%x, Actual = 0x%x' % (expected_target_addr, target_addr))

                if self.getPEstate('PC') != expected_target_addr:
                    self.error('Unexpected PC value: Expected = 0x%x, Actual = 0x%x' % (expected_target_addr, self.getPEstate('PC')))

    ## Compute the expected target address for a JALR instruction given the original PC value and
    # the offset operand value.
    #
    #  @param aBaseRegIndex The base register index for the JALR instruction.
    #  @param aOffsetOprValue The offset operand value for the JALR instruction.
    def _getExpectedTargetAddress(self, aBaseRegIndex, aOffsetOprValue):

        expected_target_addr = 0

        reg_name = 'x%d' % aBaseRegIndex
        (reg_val, valid) = self.readRegister(reg_name)
        if not valid:
            self.error('Unable to read register %s' % reg_name)

        self.debug('_getExpectedTargetAddress base reg X{:d}={:#x}, aOffsetOprValue:{:#x}'.format(aBaseRegIndex, reg_val, aOffsetOprValue))

        #process negative/positive 12b simm value | JALR sets bit 0 of target to '0' after computing base+offset
        if ((aOffsetOprValue & 0x800) != 0):
            offset = ((aOffsetOprValue ^ 0xFFF) + 1)
            expected_target_addr = ((reg_val - offset) & ~1)
        else:
            expected_target_addr = ((reg_val + aOffsetOprValue) & ~1)

        return expected_target_addr

MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

