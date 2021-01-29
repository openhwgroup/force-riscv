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
from abc import ABC
from abc import abstractmethod

## This class facilitates generating common instructions using common defaults. It provides a
# simpler, higher-level alternative to using the Sequence.genInstruction() command with less
# flexibility.
class AssemblyHelper(ABC):

    def __init__(self, aSequence):
        self.mSequence = aSequence
        self._mBranchRecords = {}
        self._mLabelRoutineName = None

    ## Generate instructions to advance the system register containing the exception return address
    # to the next instruction address. This will generally result in skipping the instruction that
    # triggered the exception.
    #
    #  @param aScratchRegIndex The index of a register that can be freely modified.
    #  @param aPrivLevelRegIndex The index of a register containing a value representing the current
    #       privilege level.
    @abstractmethod
    def genIncrementExceptionReturnAddress(self, aScratchRegIndex, aPrivLevelRegIndex):
        raise NotImplementedError

    ## Generate a relative branch instruction to the specified address.
    #
    #  @param aTargetAddr The target address of the branch.
    @abstractmethod
    def genRelativeBranchToAddress(self, aTargetAddr):
        raise NotImplementedError

    ## Generate a relative branch instruction targeting a labeled address. Record the branch for
    # later verification that the expected address was targeted via the addLabel() method.
    #
    #  @param aBrOffset The branch offset.
    #  @param aLabel The label the branch is targeting.
    def genRelativeBranchToLabel(self, aBrOffset, aLabel):
        self.recordBranchToLabel(aLabel, aBrOffset)
        self.genRelativeBranch(aBrOffset)

    ## Generate a relative branch instruction.
    #
    #  @param aBrOffset The branch offset.
    @abstractmethod
    def genRelativeBranch(self, aBrOffset):
        raise NotImplementedError

    ## Generate a relative branch with link instruction to the specified address.
    #
    #  @param aTargetAddr The target address of the branch.
    @abstractmethod
    def genRelativeBranchWithLinkToAddress(self, aTargetAddr):
        raise NotImplementedError

    ## Generate a relative branch with link instruction.
    #
    #  @param aBrOffset The branch offset.
    @abstractmethod
    def genRelativeBranchWithLink(self, aBrOffset):
        raise NotImplementedError

    ## Generate an instruction to return to the address contained in the default link register.
    @abstractmethod
    def genReturn(self):
        raise NotImplementedError

    ## Generate an instruction to load the specified register with a small immediate value.
    #
    #  @param aRegIndex The index of the register to load.
    #  @param aImmVal The immediate value to load.
    @abstractmethod
    def genMoveImmediate(self, aRegIndex, aImmVal):
        raise NotImplementedError

    ## Generate an instruction to shift the specified register to the left.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aShiftAmount The number of bits to shift.
    #  @param aSrcRegIndex The index of the register containing the value to shift. May be omitted
    #       if the destination register is also the source register.
    @abstractmethod
    def genShiftLeftImmediate(self, aDestRegIndex, aShiftAmount, aSrcRegIndex=None):
        raise NotImplementedError

    ## Generate an instruction to shift the specified register to the right.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aShiftAmount The number of bits to shift.
    #  @param aSrcRegIndex The index of the register containing the value to shift. May be omitted
    #       if the destination register is also the source register.
    @abstractmethod
    def genShiftRightImmediate(self, aDestRegIndex, aShiftAmount, aSrcRegIndex=None):
        raise NotImplementedError

    ## Generate an instruction to AND a specified register with an immediate value.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aImmVal The immediate value to AND.
    #  @param aSrcRegIndex The index of the register containing the value to AND. May be omitted if
    #       the destination register is also the source register.
    @abstractmethod
    def genAndImmediate(self, aDestRegIndex, aImmVal, aSrcRegIndex=None):
        raise NotImplementedError

    ## Generate an instruction to OR a specified register with an immediate value.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aImmVal The immediate value to OR.
    #  @param aSrcRegIndex The index of the register containing the value to OR. May be omitted if
    #       the destination register is also the source register.
    @abstractmethod
    def genOrImmediate(self, aDestRegIndex, aImmVal, aSrcRegIndex=None):
        raise NotImplementedError

    ## Generate an instruction to XOR a specified register with an immediate value.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aImmVal The immediate value to XOR.
    #  @param aSrcRegIndex The index of the register containing the value to XOR. May be omitted if
    #       the destination register is also the source register.
    @abstractmethod
    def genXorImmediate(self, aDestRegIndex, aImmVal, aSrcRegIndex=None):
        raise NotImplementedError

    ## Generate an instruction to add an immediate value to a specified register.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aImmVal The immediate value to add to the register value.
    #  @param aSrcRegIndex The index of the register containing the value to be added to. May be
    #       omitted if the destination register is also the source register.
    @abstractmethod
    def genAddImmediate(self, aDestRegIndex, aImmVal, aSrcRegIndex=None):
        raise NotImplementedError

    ## Generate an instruction to load the specified register with the value in a different
    # register.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex The index of the register containing the value to copied.
    @abstractmethod
    def genMoveRegister(self, aDestRegIndex, aSrcRegIndex):
        raise NotImplementedError

    ## Generate an instruction to AND two registers.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex1 The index of the register containing one of the values to AND.
    #  @param aSrcRegIndex2 The index of the register containing one of the values to AND. May be
    #       omitted if the destination register is also one of the source registers.
    @abstractmethod
    def genAndRegister(self, aDestRegIndex, aSrcRegIndex1, aSrcRegIndex2=None):
        raise NotImplementedError

    ## Generate an instruction to OR two registers.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex1 The index of the register containing one of the values to OR.
    #  @param aSrcRegIndex2 The index of the register containing one of the values to OR. May be
    #       omitted if the destination register is also one of the source registers.
    @abstractmethod
    def genOrRegister(self, aDestRegIndex, aSrcRegIndex1, aSrcRegIndex2=None):
        raise NotImplementedError

    ## Generate an instruction to take the one's complement of a register.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex The index of the register containing the value to NOT. May be omitted if
    #       the destination register is also the source register.
    @abstractmethod
    def genNotRegister(self, aDestRegIndex, aSrcRegIndex=None):
        raise NotImplementedError

    ## Generate an instruction to add two registers.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSrcRegIndex1 The index of the register containing one of the values to add.
    #  @param aSrcRegIndex2 The index of the register containing one of the values to add. May be
    #       omitted if the destination register is also one of the source registers.
    @abstractmethod
    def genAddRegister(self, aDestRegIndex, aSrcRegIndex1, aSrcRegIndex2=None):
        raise NotImplementedError

    ## Generate an instruction to subtract one register from another.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSubtrahendRegIndex The index of the register containing the value to subtract from
    #       the minuend.
    #  @param aMinuendRegIndex The index of the register containing the value to be subtracted from.
    #       May be omitted if the destination register is also used as the minuend.
    @abstractmethod
    def genSubRegister(self, aDestRegIndex, aSubtrahendRegIndex, aMinuendRegIndex=None):
        raise NotImplementedError

    ## Generate an instruction to load a register with the value in the specified system register.
    #
    #  @param aDestRegIndex The index of the register to write the result to.
    #  @param aSysRegName The name of the system register to read.
    @abstractmethod
    def genReadSystemRegister(self, aDestRegIndex, aSysRegName):
        raise NotImplementedError

    ## Generate an instruction to write a system register with the value contained in another
    # register.
    #
    #  @param aSysRegName The name of the system register to write to.
    #  @param aSrcRegIndex The index of the register containing the value to write to the system
    #        register.
    @abstractmethod
    def genWriteSystemRegister(self, aSysRegName, aSrcRegIndex):
        raise NotImplementedError

    ## Associate the specified label with the current PC value. Verify that any previously-recorded
    # branches to this label were correctly targeting the current PC value. Fail if any of the
    # branch targets to do not match. Also, fail if no branches have targeted this label.
    #
    #  @param aLabel The label string.
    def addLabel(self, aLabel):
        if aLabel not in self._mBranchRecords:
            self.mSequence.error('INTERNAL ERROR (%s) BRANCH TARGET (%s) NOT RECORDED' % (self._mLabelRoutineName, aLabel))

        cur_pc = self.mSequence.getPEstate('PC')
        for (branch_pc, branch_offset) in self._mBranchRecords[aLabel]:
            target_addr = branch_pc + (branch_offset << self.getBranchShift())

            if target_addr != cur_pc:
                label_offset = (cur_pc - branch_pc) >> self.getBranchShift()
                self.mSequence.notice('INTERNAL ERROR (%s) BRANCH TARGET (%s) MISMATCH:' % (self._mLabelRoutineName, aLabel))
                self.mSequence.notice('INTERNAL ERROR (%s) BRANCH ADDRESS: 0x%x, BRANCH OFFSET (# OF INSTRS): %d, LABEL OFFSET: %d' % (self._mLabelRoutineName, branch_pc, branch_offset, label_offset))
                self.mSequence.error('INTERNAL ERROR (%s) LABEL ADDRESS: 0x%x, LABEL OFFSET (# OF INSTRS): %d' % (self._mLabelRoutineName, cur_pc, label_offset))

        self.logDebugSymbol(aLabel)

    ## Clear all recorded labels and branches to labels.
    #
    # aRoutineName A tag to associate with the labels to be recorded.
    def clearLabels(self, aRoutineName):
        self._mBranchRecords = {}
        self._mLabelRoutineName = aRoutineName

    ## Output a debug symbol to the log. Debug symbols can be used to aid understanding of the
    # instruction flow. 
    #
    #  @param aSymbol The name of the symbol.
    def logDebugSymbol(self, aSymbol):
        self.mSequence.debug('__SYMBOL__ %016x "%s" %s' % (self.mSequence.getPEstate('PC'), aSymbol, self.__class__.__name__ ))
        ## EXCEPTION_DEBUG self.notice('__SYMBOL__ %016x "%s" %s' % (self.mSequence.getPEstate('PC'), aSymbol, self.__class__.__name__ ))

    ## Get the branch offset required to hit the specified target address.
    #
    #  @param aTargetAddr The target address of the branch.
    #  @param aImmFieldWidth The number of bits available to hold the branch offset immediate value.
    def getBranchOffset(self, aTargetAddr, aImmFieldWidth):
        cur_pc = self.mSequence.getPEstate('PC')
        (br_offset, valid, num_hw) = self.mSequence.getBranchOffset(br_addr=cur_pc, target_addr=aTargetAddr, offset_size=aImmFieldWidth, shift=self.getBranchShift())
        if not valid:
            self.error('Unable to generate branch instruction from 0x%x to 0x%x' % (cur_pc, aTargetAddr))

        return br_offset

    ## Return the shift amount for PC-relative branch instructions.
    @abstractmethod
    def getBranchShift(self):
        raise NotImplementedError

    ## Record the address, label and offset of a branch instruction. When the label is later passed
    # to the addLabel() method, it will be verified that the branch correctly targets the labeled
    # address.
    #
    #  @param aLabel The label the branch is targeting.
    #  @param aBrOffset The branch offset.
    def recordBranchToLabel(self, aLabel, aBrOffset):
        label_branch_records = self._mBranchRecords.get(aLabel, list())
        label_branch_records.append((self.mSequence.getPEstate('PC'), aBrOffset))
        self._mBranchRecords[aLabel] = label_branch_records
