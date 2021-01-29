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
from base.GenSemaphore import GenSemaphore
from riscv.Utils import LoadGPR64


class GenSemaphoreRISCV(GenSemaphore):
    def __init__(self, aGenThread, aName, aCounter, aSize):
        super().__init__(aGenThread, aName, aCounter, "", aSize=aSize)

        self._mCounterReg = None
        self._mStatusReg = None

        with self.threadLockingContext():
            self.mAddrReg = self.getRandomGPR(exclude="0", no_skip=True)
            self.reserveRegister("x%d" % self.mAddrReg)
            self._reloadSemaphore()

    def __enter__(self):
        self._acquireSemaphore()
        return self

    def __exit__(self, *aUnused):
        self._releaseSemaphore()
        return False

    def cleanUp(self):
        with self.threadLockingContext():
            self.unreserveRegister("x%d" % self.mAddrReg)

    def _acquireSemaphore(self):
        # TODO(Noah): Adjust for the possibility of different threads having
        #  different endianness if and when this becomes necessary.

        (self._mCounterReg, self._mStatusReg) = self.getRandomGPRs(
            2, exclude="0"
        )

        block_id = self.beginLinearBlock()

        self.genInstruction(
            self._getLoadReservedInstruction(),
            {
                "rd": self._mCounterReg,
                "rs1": self.mAddrReg,
                "LSTarget": self.mSemaVA,
                "NoRestriction": 1,
            },
        )  # Load the counter
        self.genInstruction(
            "BEQ##RISCV",
            {
                "rs1": self._mCounterReg,
                "rs2": 0,
                "simm12": (-2 & 0xFFF),
                "NoBnt": 1,
                "NoRestriction": 1,
            },
        )  # Retry if the counter is 0
        self.genInstruction(
            "ADDI##RISCV",
            {
                "rd": self._mCounterReg,
                "rs1": self._mCounterReg,
                "simm12": (-1 & 0xFFF),
            },
        )  # Decrement the counter
        self.genInstruction(
            self._getStoreConditionalInstruction(),
            {
                "rd": self._mStatusReg,
                "rs1": self.mAddrReg,
                "rs2": self._mCounterReg,
                "LSTarget": self.mSemaVA,
                "NoRestriction": 1,
            },
        )  # Store the counter
        self.genInstruction(
            "BNE##RISCV",
            {
                "rs1": self._mStatusReg,
                "rs2": 0,
                "simm12": (-8 & 0xFFF),
                "NoBnt": 1,
                "NoRestriction": 1,
            },
        )  # Retry the whole sequence if the store fails

        # Set the maximum numer of instructions to re-execute such that the
        # whole sequence could be executed from the start no matter where in
        # the sequence the thread resumes execution; i.e. if the thread resumes
        # on the second instruction, it could execute until it loops back to
        # the start and then execute the entire sequence.
        self.endLinearBlock(block_id, max_re_execution_instructions=9)

    def _releaseSemaphore(self):
        # TODO(Noah): Adjust for the possibility of different threads having
        #  different endianness if and when this becomes necessary.

        block_id = self.beginLinearBlock()

        self.genInstruction(
            self._getLoadReservedInstruction(),
            {
                "rd": self._mCounterReg,
                "rs1": self.mAddrReg,
                "LSTarget": self.mSemaVA,
                "NoRestriction": 1,
            },
        )  # Load the counter
        self.genInstruction(
            "ADDI##RISCV",
            {"rd": self._mCounterReg, "rs1": self._mCounterReg, "simm12": 1},
        )  # Increment the counter
        self.genInstruction(
            self._getStoreConditionalInstruction(),
            {
                "rd": self._mStatusReg,
                "rs1": self.mAddrReg,
                "rs2": self._mCounterReg,
                "LSTarget": self.mSemaVA,
                "NoRestriction": 1,
            },
        )  # Store the counter
        self.genInstruction(
            "BNE##RISCV",
            {
                "rs1": self._mStatusReg,
                "rs2": 0,
                "simm12": (-6 & 0xFFF),
                "NoBnt": 1,
                "NoRestriction": 1,
            },
        )  # Retry the whole sequence if the store fails

        # Set the maximum numer of instructions to re-execute such that the
        # whole sequence could be executed from the start no matter where in
        # the sequence the thread resumes execution; i.e. if the thread resumes
        # on the second instruction, it could execute until it loops back to
        # the start and then execute the entire sequence.
        self.endLinearBlock(block_id, max_re_execution_instructions=7)

        self._mCounterReg = None
        self._mStatusReg = None

    def _loadAddressRegister(self):
        load_gpr64_seq = LoadGPR64(self.genThread)
        load_gpr64_seq.load(self.mAddrReg, self.mSemaVA)

    def _getLoadReservedInstruction(self):
        raise NotImplementedError

    def _getStoreConditionalInstruction(self):
        raise NotImplementedError


class GenSemaphore32RISCV(GenSemaphoreRISCV):
    def __init__(self, aGenThread, aName, aCounter):
        super().__init__(aGenThread, aName, aCounter, 4)

    def _getLoadReservedInstruction(self):
        return "LR.W##RISCV"

    def _getStoreConditionalInstruction(self):
        return "SC.W##RISCV"


class GenSemaphore64RISCV(GenSemaphoreRISCV):
    def __init__(self, aGenThread, aName, aCounter):
        super().__init__(aGenThread, aName, aCounter, 8)

    def _getLoadReservedInstruction(self):
        return "LR.D##RISCV"

    def _getStoreConditionalInstruction(self):
        return "SC.D##RISCV"
