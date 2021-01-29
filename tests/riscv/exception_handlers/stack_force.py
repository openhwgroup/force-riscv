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
import RandomUtils

import exception_handlers_test_utils
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.Utils import LoadGPR64
from riscv.exception_handlers.ExceptionHandlerStack import (
    ExceptionHandlerStackRISCV,
)


#  This test verifies the exception handler stack stores and retrieves values
#  as expected.
class MainSequence(Sequence):
    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread, name)

        self._mReferenceStack = []
        self._mReferenceStackFrames = []

    def generate(self, **kargs):
        stack = ExceptionHandlerStackRISCV(self.genThread)
        stack_mem_size = 0x800
        stack_mem_addr = self.genVA(Size=stack_mem_size, Align=8, Type="D")
        for addr_offset in range(0, stack_mem_size, 8):
            self.initializeMemory(
                addr=(stack_mem_addr + addr_offset),
                bank=0,
                size=8,
                data=RandomUtils.random64(),
                is_instr=False,
                is_virtual=True,
            )

        stack.generate(
            stack_mem=stack_mem_addr,
            stack_top=(stack_mem_addr + stack_mem_size),
        )

        self._verifyRandomStackOperations(stack)
        self._verifyRandomStackFrameOperations(stack)

    # Verify that random push, pop, peek and modify operations yield the
    # expected results.
    #
    #  @param aStack An initialized stack object.
    def _verifyRandomStackOperations(self, aStack):
        for _ in range(RandomUtils.random32(50, 100)):
            gpr_index = self.getRandomGPR(
                exclude=("0,%d" % aStack.pointerIndex())
            )
            gpr_val = self._loadRandomizedGprValue(gpr_index)

            operation = "push"
            if not self._isStackEmpty(aStack):
                operation = self.choice(("push", "pop", "peek", "modify"))

            if operation == "push":
                self._mReferenceStack.append(gpr_val)
                aStack.push(gpr_index)
            elif operation == "pop":
                aStack.pop(gpr_index)
                exception_handlers_test_utils.assertGprHasValue(
                    self, gpr_index, self._mReferenceStack.pop()
                )

            # Stack items are indexed using the aOffset parameter from 0 at
            # the top to size - 1 at the bottom
            elif operation == "peek":
                max_stack_index = len(self._mReferenceStack) - 1
                stack_index = RandomUtils.random32(0, max_stack_index)
                aStack.peek(gpr_index, aOffset=stack_index)
                exception_handlers_test_utils.assertGprHasValue(
                    self,
                    gpr_index,
                    self._mReferenceStack[max_stack_index - stack_index],
                )
            elif operation == "modify":
                max_stack_index = len(self._mReferenceStack) - 1
                stack_index = RandomUtils.random32(0, max_stack_index)
                self._mReferenceStack[stack_index] = gpr_val
                aStack.modify(
                    gpr_index, aOffset=(max_stack_index - stack_index)
                )
            else:
                self.error("Unexpected stack operation %s" % operation)

    # Verify that random stack frame operations yield the expected results.
    #
    #  @param aStack An initialized stack object.
    def _verifyRandomStackFrameOperations(self, aStack):
        for _ in range(RandomUtils.random32(10, 20)):
            operation = ""

            if not self._mReferenceStackFrames:
                operation = "newStackFrame"
            else:
                operation = self.choice(
                    ("newStackFrame", "freeStackFrame", "arg", "modifyArg")
                )

            if operation == "newStackFrame":
                # The link register x1 is automatically captured in every
                # stack frame, so we exclude it to avoid capturing it twice in
                # the same frame.
                gpr_indices = self.getRandomGPRs(
                    RandomUtils.random32(1, 5),
                    exclude=("0,1,%d" % aStack.pointerIndex()),
                )

                reference_stack_frame = []
                for gpr_index in gpr_indices:
                    gpr_val = self._loadRandomizedGprValue(gpr_index)
                    reference_stack_frame.append([gpr_index, gpr_val])

                self._mReferenceStackFrames.append(reference_stack_frame)
                aStack.newStackFrame(gpr_indices)
                self._assertStackFrameSize(aStack, len(reference_stack_frame))
            elif operation == "freeStackFrame":
                reference_stack_frame = self._mReferenceStackFrames.pop()
                self._assertStackFrameSize(aStack, len(reference_stack_frame))
                aStack.freeStackFrame()

                for (gpr_index, expected_gpr_val) in reference_stack_frame:
                    exception_handlers_test_utils.assertGprHasValue(
                        self, gpr_index, expected_gpr_val
                    )
            elif operation == "arg":
                reference_stack_frame = self._mReferenceStackFrames[-1]
                arg_index = RandomUtils.random32(
                    0, (len(reference_stack_frame) - 1)
                )
                temp_gpr_index = self.getRandomGPR(
                    exclude=("0,%d" % aStack.pointerIndex())
                )
                aStack.arg(temp_gpr_index, arg_index)
                exception_handlers_test_utils.assertGprHasValue(
                    self, temp_gpr_index, reference_stack_frame[arg_index][1]
                )
            elif operation == "modifyArg":
                reference_stack_frame = self._mReferenceStackFrames[-1]
                arg_index = RandomUtils.random32(
                    0, (len(reference_stack_frame) - 1)
                )
                temp_gpr_index = self.getRandomGPR(
                    exclude=("0,%d" % aStack.pointerIndex())
                )
                updated_arg_val = self._loadRandomizedGprValue(temp_gpr_index)
                reference_stack_frame[arg_index][1] = updated_arg_val
                aStack.modifyArg(temp_gpr_index, arg_index)
            else:
                self.error("Unexpected stack frame operation %s" % operation)

    # Randomly choose between initializing the specified register with a
    # random value or loading a random value into the register.
    #
    #  @param aGprIndex The index of the register to set.
    def _loadRandomizedGprValue(self, aGprIndex):
        gpr_val = 0
        if RandomUtils.random32(0, 1) == 0:
            gpr_name = "x%d" % aGprIndex
            self.randomInitializeRegister(gpr_name)
            (gpr_val, valid) = self.readRegister(gpr_name)
            exception_handlers_test_utils.assertValidGprValue(
                self, aGprIndex, valid
            )
        else:
            load_gpr64_seq = LoadGPR64(self.genThread)
            gpr_val = (
                RandomUtils.random32()
                if self.getGlobalState("AppRegisterWidth") == 32
                else RandomUtils.random64()
            )
            load_gpr64_seq.load(aGprIndex, gpr_val)

        return gpr_val

    # Return whether the stack is empty.
    #
    #  @param aStack An initialized stack object.
    def _isStackEmpty(self, aStack):
        stack_pointer_index = aStack.pointerIndex()
        (stack_pointer_val, valid) = self.readRegister(
            "x%d" % stack_pointer_index
        )
        exception_handlers_test_utils.assertValidGprValue(
            self, stack_pointer_index, valid
        )
        stack_empty = stack_pointer_val == aStack.mStackTop

        reference_stack_empty = False
        if not self._mReferenceStack:
            reference_stack_empty = True

        if stack_empty != reference_stack_empty:
            if stack_empty:
                self.error("Stack is empty when it shouldn't be")
            else:
                self.error("Stack is not empty when it should be")

        return stack_empty

    # Fail if the top-most stack frame does not have the specified size.
    #
    #  @param aStack An initialized stack object.
    #  @param aExpectedFrameSize The expected size of the top-most stack frame.
    def _assertStackFrameSize(self, aStack, aExpectedFrameSize):
        stack_frame_size = (aStack.frameInstructionCount() // 2) - 1
        if stack_frame_size != aExpectedFrameSize:
            self.error(
                "Stack frame size %d is not equal to the expected size %d"
                % (stack_frame_size, aExpectedFrameSize)
            )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
