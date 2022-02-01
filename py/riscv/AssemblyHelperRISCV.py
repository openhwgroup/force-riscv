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
from base.AssemblyHelper import AssemblyHelper
from riscv.PrivilegeLevel import PrivilegeLevelRISCV


#  This class facilitates generating common RISC-V instructions using common
#  defaults. It provides a simpler, higher-level alternative to using the
# Sequence.genInstruction() command with less flexibility.
class AssemblyHelperRISCV(AssemblyHelper):

    # Generate instructions to advance the system register containing the
    # exception return address to the next instruction address. This will
    # generally result in skipping the instruction that triggered the
    # exception.
    #
    #  @param aScratchRegIndex The index of a register that can be freely
    #       modified.
    #  @param aPrivLevelRegIndex The index of a register containing a value
    #       representing the current privilege level.
    def genIncrementExceptionReturnAddress(self, aScratchRegIndex, aPrivLevelRegIndex):
        for priv_level in self.genPrivilegeLevelInstructions(
            aPrivLevels=tuple(PrivilegeLevelRISCV)[1:],
            aInstrCountPerLevel=3,
            aScratchRegIndex=aScratchRegIndex,
            aPrivLevelRegIndex=aPrivLevelRegIndex,
        ):
            self.genReadSystemRegister(aScratchRegIndex, ("%sepc" % priv_level.name.lower()))
            self.genAddImmediate(aScratchRegIndex, 4)
            self.genWriteSystemRegister(("%sepc" % priv_level.name.lower()), aScratchRegIndex)

    # Generate instructions to set the system register containing the
    # exception return address to the provided recovery address. This will
    # generally result in skipping the instruction that triggered the
    # exception and resuming execution at the recovery address.
    #
    #  @param aScratchRegIndex The index of a register that can be freely
    #       modified.
    #  @param aRecoveryRegIndex The index of a register that contains the
    #       desired recovery address.
    #  @param aPrivLevelRegIndex The index of a register containing a value
    #       representing the current privilege level.
    def genProvidedExceptionReturnAddress(
        self, aScratchRegIndex, aRecoveryRegIndex, aPrivLevelRegIndex
    ):
        for priv_level in self.genPrivilegeLevelInstructions(
            aPrivLevels=tuple(PrivilegeLevelRISCV)[1:],
            aInstrCountPerLevel=1,
            aScratchRegIndex=aScratchRegIndex,
            aPrivLevelRegIndex=aPrivLevelRegIndex,
        ):
            self.genWriteSystemRegister(("%sepc" % priv_level.name.lower()), aRecoveryRegIndex)

    # Generate a relative branch instruction to the specified address.
    #
    #  @param aTargetAddr The target address of the branch.
    def genRelativeBranchToAddress(self, aTargetAddr):
        br_offset = self.getBranchOffset(aTargetAddr, 20)
        self.genRelativeBranch(br_offset)

    # Generate a relative branch instruction.
    #
    #  @param aBrOffset The branch offset.
    def genRelativeBranch(self, aBrOffset):
        self.mSequence.genInstruction(
            "JAL##RISCV", {"rd": 0, "simm20": aBrOffset, "NoRestriction": 1}
        )

    # Generate a relative branch instruction targeting a labeled address.
    # Record the branch for later verification that the expected address was
    # targeted via the addLabel() method.
    #
    #  @param aBrOffset The branch offset.
    #  @param aLabel The label the branch is targeting.
    def genRelativeBranchToLabel(self, aBrOffset, aLabel):
        self.recordBranchToLabel(aLabel, aBrOffset)
        self.genRelativeBranch(aBrOffset)

    # Generate a relative branch with link instruction to the specified
    # address.
    #
    #  @param aTargetAddr The target address of the branch.
    def genRelativeBranchWithLinkToAddress(self, aTargetAddr):
        br_offset = self.getBranchOffset(aTargetAddr, 20)
        self.genRelativeBranchWithLink(br_offset)

    # Generate a relative branch with link instruction.
    #
    #  @param aBrOffset The branch offset.
    def genRelativeBranchWithLink(self, aBrOffset):
        # We use the conventional link register x1
        self.mSequence.genInstruction(
            "JAL##RISCV", {"rd": 1, "simm20": aBrOffset, "NoRestriction": 1}
        )

    # Generate an instruction to return to the address contained in the
    # default link register.
    def genReturn(self):
        # We use the conventional link register x1
        self.mSequence.genInstruction(
            "JALR##RISCV", {"rd": 0, "rs1": 1, "simm12": 0, "NoRestriction": 1}
        )

    # Generate an instruction to load the specified register with a small
    # immediate value.
    #
    #  @param aRegIndex The index of the register to load.
    #  @param aImmVal The immediate value to load.
    def genMoveImmediate(self, aRegIndex, aImmVal):
        self.genAddImmediate(aRegIndex, aImmVal, aSrcRegIndex=0)

    # Generate an instruction to shift the specified register to the left.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aShiftAmount The number of bits to shift.
    #  @param aSrcRegIndex The index of the register containing the value to
    #       shift. May be omitted if the destination register is also the
    #       source register.
    def genShiftLeftImmediate(self, aDestRegIndex, aShiftAmount, aSrcRegIndex=None):
        src_reg_index = aSrcRegIndex if aSrcRegIndex is not None else aDestRegIndex
        instr = "SLLI#%s#RISCV" % (self.getInstrForm())
        self.mSequence.genInstruction(
            instr,
            {"rd": aDestRegIndex, "rs1": src_reg_index, "shamt": aShiftAmount},
        )

    # Generate an instruction to shift the specified register to the right.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aShiftAmount The number of bits to shift.
    #  @param aSrcRegIndex The index of the register containing the value to
    #       shift. May be omitted if the destination register is also the
    #       source register.
    def genShiftRightImmediate(self, aDestRegIndex, aShiftAmount, aSrcRegIndex=None):
        src_reg_index = aSrcRegIndex if aSrcRegIndex is not None else aDestRegIndex
        instr = "SRLI#%s#RISCV" % (self.getInstrForm())
        self.mSequence.genInstruction(
            instr,
            {"rd": aDestRegIndex, "rs1": src_reg_index, "shamt": aShiftAmount},
        )

    def getInstrForm(self):

        return "RV32I" if (self.mSequence.getGlobalState("AppRegisterWidth") == 32) else "RV64I"

    # Generate an instruction to AND a specified register with an immediate
    # value.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aImmVal The immediate value to AND.
    #  @param aSrcRegIndex The index of the register containing the value to
    #       AND. May be omitted if the destination register is also the source
    #       register.
    def genAndImmediate(self, aDestRegIndex, aImmVal, aSrcRegIndex=None):
        src_reg_index = aSrcRegIndex if aSrcRegIndex is not None else aDestRegIndex
        self.mSequence.genInstruction(
            "ANDI##RISCV",
            {"rd": aDestRegIndex, "rs1": src_reg_index, "simm12": aImmVal},
        )

    # Generate an instruction to OR a specified register with an immediate
    # value.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aImmVal The immediate value to OR.
    #  @param aSrcRegIndex The index of the register containing the value to
    #       OR. May be omitted if the destination register is also the source
    #       register.
    def genOrImmediate(self, aDestRegIndex, aImmVal, aSrcRegIndex=None):
        src_reg_index = aSrcRegIndex if aSrcRegIndex is not None else aDestRegIndex
        self.mSequence.genInstruction(
            "ORI##RISCV",
            {"rd": aDestRegIndex, "rs1": src_reg_index, "simm12": aImmVal},
        )

    # Generate an instruction to XOR a specified register with an immediate
    # value.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aImmVal The immediate value to XOR.
    #  @param aSrcRegIndex The index of the register containing the value to
    #       XOR. May be omitted if the destination register is also the source
    #       register.
    def genXorImmediate(self, aDestRegIndex, aImmVal, aSrcRegIndex=None):
        src_reg_index = aSrcRegIndex if aSrcRegIndex is not None else aDestRegIndex
        self.mSequence.genInstruction(
            "XORI##RISCV",
            {"rd": aDestRegIndex, "rs1": src_reg_index, "simm12": aImmVal},
        )

    # Generate an instruction to add an immediate value to a specified
    # register.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aImmVal The immediate value to add to the register value.
    #  @param aSrcRegIndex The index of the register containing the value to
    #       be added to. May be omitted if the destination register is also
    #       the source register.
    def genAddImmediate(self, aDestRegIndex, aImmVal, aSrcRegIndex=None):
        src_reg_index = aSrcRegIndex if aSrcRegIndex is not None else aDestRegIndex
        self.mSequence.genInstruction(
            "ADDI##RISCV",
            {"rd": aDestRegIndex, "rs1": src_reg_index, "simm12": aImmVal},
        )

    # Generate an instruction to load the specified register with the value
    # in a different register.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex The index of the register containing the value to
    #       copied.
    def genMoveRegister(self, aDestRegIndex, aSrcRegIndex):
        self.genAddRegister(aDestRegIndex, aSrcRegIndex, aSrcRegIndex2=0)

    # Generate an instruction to AND two registers.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex1 The index of the register containing one of the
    #       values to AND.
    #  @param aSrcRegIndex2 The index of the register containing one of the
    #       values to AND. May be omitted if the destination register is also
    #       one of the source registers.
    def genAndRegister(self, aDestRegIndex, aSrcRegIndex1, aSrcRegIndex2=None):
        src_reg_index_2 = aSrcRegIndex2 if aSrcRegIndex2 is not None else aDestRegIndex
        self.mSequence.genInstruction(
            "AND##RISCV",
            {
                "rd": aDestRegIndex,
                "rs1": aSrcRegIndex1,
                "rs2": src_reg_index_2,
            },
        )

    # Generate an instruction to OR two registers.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex1 The index of the register containing one of the
    #       values to OR.
    #  @param aSrcRegIndex2 The index of the register containing one of the
    #       values to OR. May be omitted if the destination register is also
    #       one of the source registers.
    def genOrRegister(self, aDestRegIndex, aSrcRegIndex1, aSrcRegIndex2=None):
        src_reg_index_2 = aSrcRegIndex2 if aSrcRegIndex2 is not None else aDestRegIndex
        self.mSequence.genInstruction(
            "OR##RISCV",
            {
                "rd": aDestRegIndex,
                "rs1": aSrcRegIndex1,
                "rs2": src_reg_index_2,
            },
        )

    # Generate an instruction to take the one's complement of a register.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex The index of the register containing the value to
    #       NOT. May be omitted if the destination register is also the source
    #       register.
    def genNotRegister(self, aDestRegIndex, aSrcRegIndex=None):
        self.genXorImmediate(aDestRegIndex, 0xFFF, aSrcRegIndex)

    # Generate an instruction to add two registers.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex1 The index of the register containing one of the
    #       values to add.
    #  @param aSrcRegIndex2 The index of the register containing one of the
    #       values to add. May be omitted if the destination register is also
    #       one of the source registers.
    def genAddRegister(self, aDestRegIndex, aSrcRegIndex1, aSrcRegIndex2=None):
        src_reg_index_2 = aSrcRegIndex2 if aSrcRegIndex2 is not None else aDestRegIndex
        self.mSequence.genInstruction(
            "ADD##RISCV",
            {
                "rd": aDestRegIndex,
                "rs1": aSrcRegIndex1,
                "rs2": src_reg_index_2,
            },
        )

    # Generate an instruction to subtract one register from another.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSubtrahendRegIndex The index of the register containing the
    #       value to subtract from the minuend.
    #  @param aMinuendRegIndex The index of the register containing the value
    #       to be subtracted from. May be omitted if the destination register
    #       is also used as the minuend.
    def genSubRegister(self, aDestRegIndex, aSubtrahendRegIndex, aMinuendRegIndex=None):
        minuend_reg_index = aMinuendRegIndex if aMinuendRegIndex is not None else aDestRegIndex
        self.mSequence.genInstruction(
            "SUB##RISCV",
            {
                "rd": aDestRegIndex,
                "rs1": minuend_reg_index,
                "rs2": aSubtrahendRegIndex,
            },
        )

    # Generate an instruction to load a register with the value in the
    # specified system register.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSysRegName The name of the system register to read.
    def genReadSystemRegister(self, aDestRegIndex, aSysRegName):
        self.mSequence.genInstruction(
            "CSRRS#register#RISCV",
            {
                "rd": aDestRegIndex,
                "rs1": 0,
                "csr": self.mSequence.getRegisterIndex(aSysRegName),
            },
        )

    # Generate an instruction to write a system register with the value
    # contained in another register.
    #
    #  @param aSysRegName The name of the system register to write to.
    #  @param aSrcRegIndex The index of the register containing the value to
    #  write to the system register.
    def genWriteSystemRegister(self, aSysRegName, aSrcRegIndex):
        self.mSequence.genInstruction(
            "CSRRW#register#RISCV",
            {
                "rd": 0,
                "rs1": aSrcRegIndex,
                "csr": self.mSequence.getRegisterIndex(aSysRegName),
            },
        )

    # Generate a conditional branch instruction to the specified address.
    #
    #  @param aLhRegIndex The index of the register to use for the left-hand
    #       side of the comparison.
    #  @param aRhRegIndex The index of the register to use for the right-hand
    #       side of the comparison.
    #  @param aTargetAddr The target address of the branch.
    #  @param aCondition A two-letter string encoding the branch condition.
    def genConditionalBranchToAddress(self, aLhRegIndex, aRhRegIndex, aTargetAddr, aCondition):
        br_offset = self.getBranchOffset(aTargetAddr, 12)
        self.genConditionalBranch(aLhRegIndex, aRhRegIndex, br_offset, aCondition)

    # Generate a conditional branch instruction targeting a labeled address.
    # Record the branch for later verification that the expected address was
    # targeted via the addLabel() method.
    #
    #  @param aLhRegIndex The index of the register to use for the left-hand
    #       side of the comparison.
    #  @param aRhRegIndex The index of the register to use for the right-hand
    #       side of the comparison.
    #  @param aBrOffset The branch offset.
    #  @param aCondition A two-letter string encoding the branch condition.
    #  @param aLabel The label the branch is targeting.
    def genConditionalBranchToLabel(self, aLhRegIndex, aRhRegIndex, aBrOffset, aCondition, aLabel):
        self.recordBranchToLabel(aLabel, aBrOffset)
        self.genConditionalBranch(aLhRegIndex, aRhRegIndex, aBrOffset, aCondition)

    # Generate a conditional branch instruction.
    #
    #  @param aLhRegIndex The index of the register to use for the left-hand
    #       side of the comparison.
    #  @param aRhRegIndex The index of the register to use for the right-hand
    #       side of the comparison.
    #  @param aBrOffset The branch offset.
    #  @param aCondition A two-letter string encoding the branch condition.
    def genConditionalBranch(self, aLhRegIndex, aRhRegIndex, aBrOffset, aCondition):
        CONDITIONS = {"EQ": 0, "NE": 1, "LT": 2, "LTU": 3, "GE": 4, "GEU": 5}

        instr = "B%s##RISCV" % aCondition
        self.mSequence.genInstruction(
            instr,
            {
                "rs1": aLhRegIndex,
                "rs2": aRhRegIndex,
                "simm12": aBrOffset,
                "NoRestriction": 1,
            },
        )

    # Generate an instruction to return from an exception.
    #
    #  @param aScratchRegIndex The index of a register that can be freely
    #       modified.
    #  @param aPrivLevel The privilege level in which the exception return
    #       will be executed.
    def genExceptionReturn(self, aPrivLevel):
        self.mSequence.genInstruction("%sRET##RISCV" % aPrivLevel.name, {"NoRestriction": 1})

    # Generate branch instructions to determine the current privilege level.
    # This method yields at the appropriate locations in the branch instruction
    # structure to allow the caller to generate specific instructions for the
    # privilege level. It is assumed that each privilege level executes the
    # same number of instructions, as specified by aInstrCountPerLevel.
    #
    #  @param aPrivLevels A list of privilege levels for which to generate
    #       instructions.
    #  @param aInstrCountPerLevel The number of custom instructions to be
    #       generated for each privilege level.
    #  @param aScratchRegIndex The index of a register that can be freely
    #       modified.
    #  @param aPrivLevelRegIndex The index of a register containing a value
    #       representing the current privilege level.
    def genPrivilegeLevelInstructions(
        self,
        aPrivLevels,
        aInstrCountPerLevel,
        aScratchRegIndex,
        aPrivLevelRegIndex,
    ):
        # Loop through all privilege levels except the last one; the last
        # privilege level doesn't need the surroudning branch instructions.
        for (i, priv_level) in enumerate(aPrivLevels[:-1]):
            self.genMoveImmediate(aScratchRegIndex, priv_level.value)
            cond_branch_offset = (2 + aInstrCountPerLevel) * 2
            self.genConditionalBranch(
                aPrivLevelRegIndex, aScratchRegIndex, cond_branch_offset, "NE"
            )

            yield priv_level

            rel_branch_offset = ((3 + aInstrCountPerLevel) * (len(aPrivLevels) - i - 1) - 2) * 2
            self.genRelativeBranch(rel_branch_offset)

        # Generate code for the last privilege level.
        yield aPrivLevels[-1]

    # Return the shift amount for PC-relative branch instructions.
    def getBranchShift(self):
        return 1
