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
import Log

from base.Sequence import Sequence
from riscv.Utils import LoadGPR64


# This class maintains a stack for the saving and restoring of register
# values by exception dispatchers and exception handlers. The stack grows
# downward in memory such that the top of the stack will always have the
# lowest address of any entry. The stack assumes the conventional RISCV link
# register x1 is used. This register is implicitly saved when a stack frame is
# created and reloaded when a stack frame is freed.


class ExceptionHandlerStackRISCV(Sequence):
    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mStackMemAddr = None
        self.mStackTop = None
        self._mSpIndex = None
        self._mSpName = None
        self._mStackFrames = []
        self._mAppRegSize = 64

    def generate(self, **kwargs):
        self._mStackMemAddr = kwargs.get("stack_mem")
        if self._mStackMemAddr is None:
            self._mStackMemAddr = self.virtualMemoryRequest(
                "PhysicalRegion",
                {
                    "RegionType": "ExceptionStack",
                    "Size": 0x400,
                    "Align": 8,
                    "Type": "D",
                },
            )

        self.mStackTop = kwargs.get("stack_top")
        if self.mStackTop is None:
            self.mStackTop = self._mStackMemAddr + 0x400

        self._mSpIndex = kwargs.get("sp_index")
        if self._mSpIndex is None:
            self._mSpIndex = self.getRandomGPR(exclude="0,1,2")

        self._mSpName = "x%d" % self._mSpIndex

        self._initializeStackPointer(kwargs.get("load_stack_pointer", True))

        self._mAppRegSize = self.getGlobalState("AppRegisterWidth")

        return self._mSpIndex

    # Create a new stack frame with the values of the specified registers.
    #
    #  @param aRegIndices A list of registers to copy into the stack.
    def newStackFrame(self, aSaveRegIndices):
        # Save link register first
        self.push(1)

        self._mStackFrames.append(aSaveRegIndices)
        for gpr in reversed(aSaveRegIndices):
            self.push(gpr)

    # Remove the top stack frame and restore captured registers with their
    # original values.
    def freeStackFrame(self):
        for gpr in self._mStackFrames.pop():
            self.pop(gpr)

        # Restore link register last
        self.pop(1)

    # Return the number of instructions required to free the top stack frame.
    def frameInstructionCount(self):
        instr_count = 0
        if self._mStackFrames:
            instr_count = (len(self._mStackFrames[-1]) + 1) * 2

        return instr_count

    # Read an argument value from the top stack frame.
    #
    #  @param aRegIndex The index of the register to read the argument value
    #       into.
    #  @param aArgIndex The index of the argument to read. The value should
    #       be from 0 to 1 less than the stack frame size. For example, if the
    #       stack frame was created with three registers r1, r2 and r3, then
    #       arg(0) retrieves the value of r1, arg(1) the value of r2 and arg(2)
    #       the value of r3.
    def arg(self, aRegIndex, aArgIndex):
        return self.peek(aRegIndex, aArgIndex)

    # Modify an argument value in the top stack frame.
    #
    #  @param aRegIndex The index of the register containing the new argument
    #       value.
    #  @param aArgIndex The index of the argument to modify. The value should
    #       be from 0 to 1 less than the stack frame size. For example, if the
    #       stack frame was created with three registers r1, r2 and r3, then
    #       arg(0) modifies the value of r1, arg(1) the value of r2 and arg(2)
    #       the value of r3.
    def modifyArg(self, aRegIndex, aArgIndex):
        return self.modify(aRegIndex, aArgIndex)

    # Push a register value onto the top of the stack.
    #
    #  @param aRegIndex The index of the register containing the value to push.
    def push(self, aRegIndex):
        # Decrement the stack pointer before the push, so the stack pointer
        # always points to the last value pushed
        if self._mAppRegSize == 32:
            self.genInstruction(
                "ADDI##RISCV",
                {"rd": self._mSpIndex, "rs1": self._mSpIndex, "simm12": 0xFFC},
            )
            self.genInstruction(
                "SW##RISCV",
                {
                    "rs1": self._mSpIndex,
                    "rs2": aRegIndex,
                    "simm12": 0,
                    "NoRestriction": 1,
                },
            )
        else:
            self.genInstruction(
                "ADDI##RISCV",
                {"rd": self._mSpIndex, "rs1": self._mSpIndex, "simm12": 0xFF8},
            )
            self.genInstruction(
                "SD##RISCV",
                {
                    "rs1": self._mSpIndex,
                    "rs2": aRegIndex,
                    "simm12": 0,
                    "NoRestriction": 1,
                },
            )

    # Pop a value from the top of the stack.
    #
    #  @param aRegIndex The index of the register in which to load the popped
    #  value.
    def pop(self, aRegIndex):
        if self._mAppRegSize == 32:
            self.genInstruction(
                "LW##RISCV",
                {
                    "rd": aRegIndex,
                    "rs1": self._mSpIndex,
                    "simm12": 0,
                    "NoRestriction": 1,
                },
            )
        else:
            self.genInstruction(
                "LD##RISCV",
                {
                    "rd": aRegIndex,
                    "rs1": self._mSpIndex,
                    "simm12": 0,
                    "NoRestriction": 1,
                },
            )

        # Increment the stack pointer after the pop, so the stack pointer
        # always points to the last value pushed
        if self._mAppRegSize == 32:
            self.genInstruction(
                "ADDI##RISCV",
                {"rd": self._mSpIndex, "rs1": self._mSpIndex, "simm12": 0x4},
            )
        else:
            self.genInstruction(
                "ADDI##RISCV",
                {"rd": self._mSpIndex, "rs1": self._mSpIndex, "simm12": 0x8},
            )

    # Read a value from the stack without modifying the stack.
    #
    #  @param aRegIndex The index of the register in which to load the read
    #       value.
    #  @param aOffset The number of entries from the top of the stack to read.
    #       The top of the stack has an aOffset value of 0.
    def peek(self, aRegIndex, aOffset=0):
        if self._mAppRegSize == 32:
            offset = aOffset * 4
            self.genInstruction(
                "LW##RISCV",
                {
                    "rd": aRegIndex,
                    "rs1": self._mSpIndex,
                    "simm12": offset,
                    "NoRestriction": 1,
                },
            )
        else:
            offset = aOffset * 8
            self.genInstruction(
                "LD##RISCV",
                {
                    "rd": aRegIndex,
                    "rs1": self._mSpIndex,
                    "simm12": offset,
                    "NoRestriction": 1,
                },
            )

    # Modify a value in the stack without modifying the structure of the stack.
    #
    #  @param aRegIndex The index of the register containing the value to
    #       insert into the stack.
    #  @param aOffset The number of entries from the top of the stack to
    #       modify. The top of the stack has an aOffset value of 0.
    def modify(self, aRegIndex, aOffset=0):
        if self._mAppRegSize == 32:
            offset = aOffset * 4
            self.genInstruction(
                "SW##RISCV",
                {
                    "rs1": self._mSpIndex,
                    "rs2": aRegIndex,
                    "simm12": offset,
                    "NoRestriction": 1,
                },
            )
        else:
            offset = aOffset * 8
            self.genInstruction(
                "SD##RISCV",
                {
                    "rs1": self._mSpIndex,
                    "rs2": aRegIndex,
                    "simm12": offset,
                    "NoRestriction": 1,
                },
            )

    # Return the index of the register used as the stack pointer.
    def pointerIndex(self):
        return self._mSpIndex

    # Initialize the stack pointer register to point to the top of the stack
    # and reserve the register.
    #
    #  @param aLoadStackPointer Flag indicating whether to generate
    #       instructions to load the stack top value into the stack pointer.
    #       This is necessary when the stack pointer register has already been
    #       initialized.
    def _initializeStackPointer(self, aLoadStackPointer):
        self.initializeRegister(self._mSpName, self.mStackTop)

        if aLoadStackPointer:
            load_gpr64_seq = LoadGPR64(self.genThread)
            load_gpr64_seq.load(self._mSpIndex, self.mStackTop)

        # Reserve the stack register to prevent its corruption by
        # randomly-generated instructions
        self.reserveRegister(self._mSpName, "ReadWrite")

        Log.debug(
            "DEBUG [ExceptionHandlerStackRISCV::_initializeStackPointer] "
            "STACK REG: %s (index: %d), value: 0x%x\n"
            % (self._mSpName, self._mSpIndex, self.mStackTop)
        )
