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
from base.Sequence import Sequence
from base.Utils import LoopControlBase


#  This class generates sequences to load 64-bit immediate values into general
#  purpose registers.
class LoadGPR64(Sequence):
    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mRegisterIndex = None

    # Generate instructions to load a 64-bit value into a general purpose
    # register. Reserve the register temporarily if it has not already been
    # reserved.
    #
    #  @param aRegIndex The index of the register to load.
    #  @param aValue A 64-bit value.
    def load(self, aRegIndex, aValue):
        self._mRegisterIndex = aRegIndex

        reserved_reg = False
        reg_name = "x%d" % aRegIndex
        if not self.isRegisterReserved(reg_name):
            self.reserveRegister(reg_name)
            reserved_reg = True

        self._loadValue(aValue)

        if reserved_reg:
            self.unreserveRegister(reg_name)

    # Generate instructions to load a 64-bit value into a general purpose
    # register. This method first computes the parameter values required to
    # load the lower bits, if necessary, then generates instructions to load
    # the top 32 bits. After that, it uses the parameter values initially
    # computed to generate the instructions to load the remaining bits. The
    # maximum number of instructions this method generates should be 8. The
    # method attempts use larger shift amounts and avoids adding 0 to reduce
    # the number of instructions required to load values with few set bits.
    #
    #  @param aValue A 64-bit value.
    def _loadValue(self, aValue):
        if self.getGlobalState("AppRegisterWidth") == 32:
            reg_val = aValue & 0xFFFFFFFF
            self._load32BitValue(aValue)
            self.writeRegister("x%d" % self._mRegisterIndex, reg_val, "", True)
            return

        if (aValue & 0xFFFFFFFFFFFFF800) == 0xFFFFFFFFFFFFF800:
            self._loadValueTop53BitsSet(aValue)
            self.writeRegister("x%d" % self._mRegisterIndex, aValue, "", True)
            return

        # Initialize list of shift amounts and ADDI immediate parameters
        imm_params = []

        value = aValue
        while value.bit_length() > 32:
            bottom_12_bits = value & 0xFFF
            top_52_bits = ((value + 0x800) >> 12) & 0xFFFFFFFFFFFFF

            # Determine the lowest set bit position in the top 52 bits
            lsb = (top_52_bits & -top_52_bits).bit_length() - 1

            shift_amount = lsb + 12
            imm_params.append((shift_amount, bottom_12_bits))

            # Set value equal to the remaining significant bits
            value = top_52_bits >> lsb

        self._load32BitValue(value)

        for (shift_amount, imm_value) in reversed(imm_params):
            self.genInstruction(
                "SLLI#RV64I#RISCV",
                {
                    "rd": self._mRegisterIndex,
                    "rs1": self._mRegisterIndex,
                    "shamt": shift_amount,
                },
            )

            if imm_value != 0:
                self.genInstruction(
                    "ADDI##RISCV",
                    {
                        "rd": self._mRegisterIndex,
                        "rs1": self._mRegisterIndex,
                        "simm12": imm_value,
                    },
                )

        # Due to limitations in the RISC-V architecture, _load32BitValue() is
        # forced to use instructions that sign-extend the result to 64 bits.
        # This yeilds the wrong value when the argument passed to
        # _load32BitValue() has Bit 31 as the most significant bit. We remove
        # the sign extension by shifting.
        if (value.bit_length() == 32) and (aValue.bit_length() != 64):
            shift_amount = 64 - aValue.bit_length()
            self.genInstruction(
                "SLLI#RV64I#RISCV",
                {
                    "rd": self._mRegisterIndex,
                    "rs1": self._mRegisterIndex,
                    "shamt": shift_amount,
                },
            )
            self.genInstruction(
                "SRLI#RV64I#RISCV",
                {
                    "rd": self._mRegisterIndex,
                    "rs1": self._mRegisterIndex,
                    "shamt": shift_amount,
                },
            )

        self.writeRegister("x%d" % self._mRegisterIndex, aValue, "", True)

    # Generate instructions to load a value with the top 53 bits set into a
    # general purpose register. This method takes advantage of the sign
    # extension for LUI and ADDI to perform the operation in two instructions.
    #
    #  @param aValue A value with the top 53 bits set.
    def _loadValueTop53BitsSet(self, aValue):
        self.genInstruction(
            "LUI##RISCV", {"rd": self._mRegisterIndex, "simm20": 0}
        )
        bottom_12_bits = aValue & 0xFFF
        self.genInstruction(
            "ADDIW##RISCV",
            {
                "rd": self._mRegisterIndex,
                "rs1": self._mRegisterIndex,
                "simm12": bottom_12_bits,
            },
        )

    # Generate instructions to load a 32-bit value into a general purpose
    # register. If Bit 31 is set in loadValue, the value loaded into the
    # register will be sign-extended to 64 bits.
    #
    #  @param aValue A 32-bit value.
    def _load32BitValue(self, aValue):
        top_20_bits = ((aValue + 0x800) >> 12) & 0xFFFFF

        # Always execute the LUI, even when top 20 bits are 0, because it will
        # clear the remaining register bits, as desired.
        self.genInstruction(
            "LUI##RISCV", {"rd": self._mRegisterIndex, "simm20": top_20_bits}
        )

        bottom_12_bits = aValue & 0xFFF
        if bottom_12_bits != 0:
            if self.getGlobalState("AppRegisterWidth") == 32:
                self.genInstruction(
                    "ADDI##RISCV",
                    {
                        "rd": self._mRegisterIndex,
                        "rs1": self._mRegisterIndex,
                        "simm12": bottom_12_bits,
                    },
                )
            else:
                self.genInstruction(
                    "ADDIW##RISCV",
                    {
                        "rd": self._mRegisterIndex,
                        "rs1": self._mRegisterIndex,
                        "simm12": bottom_12_bits,
                    },
                )


#  A simple register-based loop control class for RISCV.
class LoopControl(LoopControlBase):

    # Generate instructions to load the loop register with the loop count value
    def loadLoopRegister(self):
        load_gpr = LoadGPR64(self.genThread)
        load_gpr.load(self.mLoopRegIndex, self.mLoopCount)

    # Generate instructions to return execution to the beginning of the loop.
    def generateLoopBackInstructions(self):
        self.genInstruction(
            "ADDI##RISCV",
            {
                "rd": self.mLoopRegIndex,
                "rs1": self.mLoopRegIndex,
                "simm12": 0xFFF,
            },
        )

        # Try using a conditional PC-relative branch
        branch_pc = self.getPEstate("PC")
        (branch_offset, is_valid, num_hw) = self.getBranchOffset(
            branch_pc, self.mLoopBackAddress, 12, 1
        )

        self.notice(
            "Loop Register = x%d, Loop Back Address = 0x%x, PC = 0x%x, "
            "Branch Offset = 0x%x, Offset Valid = %d"
            % (
                self.mLoopRegIndex,
                self.mLoopBackAddress,
                branch_pc,
                branch_offset,
                is_valid,
            )
        )

        if is_valid:
            self.mPostLoopAddress = branch_pc + 4
            self.genInstruction(
                "BNE##RISCV",
                {
                    "rs1": self.mLoopRegIndex,
                    "rs2": 0,
                    "simm12": branch_offset,
                    "NoBnt": 1,
                    "NoRestriction": 1,
                },
            )
        else:
            # Try using an unconditional PC-relative branch
            (branch_offset, is_valid, num_hw) = self.getBranchOffset(
                (branch_pc + 4), self.mLoopBackAddress, 20, 1
            )

            if is_valid:
                self.mPostLoopAddress = branch_pc + 4 * 2
                self.genInstruction(
                    "BEQ##RISCV",
                    {
                        "rs1": self.mLoopRegIndex,
                        "rs2": 0,
                        "simm12": 4,
                        "NoBnt": 1,
                        "NoRestriction": 1,
                    },
                )
                self.genInstruction(
                    "JAL##RISCV",
                    {"rd": 0, "simm20": branch_offset, "NoRestriction": 1},
                )
            else:
                # Use a register branch
                load_gpr = LoadGPR64(self.genThread)
                branch_gpr_index = self.getBranchRegisterIndex()
                load_gpr.load(branch_gpr_index, self.mLoopBackAddress)

                self.mPostLoopAddress = self.getPEstate("PC") + 4 * 2
                self.genInstruction(
                    "BEQ##RISCV",
                    {
                        "rs1": self.mLoopRegIndex,
                        "rs2": 0,
                        "simm12": 4,
                        "NoBnt": 1,
                        "NoRestriction": 1,
                    },
                )
                self.genInstruction(
                    "JALR##RISCV",
                    {
                        "rd": 0,
                        "rs1": branch_gpr_index,
                        "simm12": 0,
                        "NoRestriction": 1,
                    },
                )

    # Get the name of a general purpose register based on its index.
    #
    #  @param aGprIndex The index of the general purpose register.
    def getGprName(self, aGprIndex):
        return "x%d" % aGprIndex

    # Return a random general purpose register index for use in generating loop
    # control instructions.
    def getRandomGprForLoopControl(self):
        return self.getRandomGPR(exclude="0,1,2")
