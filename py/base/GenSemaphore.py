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
#  GenSemaphore class
# base class for Semaphore

from base.Sequence import Sequence


class GenSemaphore(Sequence):
    def __init__(
        self, aGenThread, aName, aCounter, aMemAttrImpl, aBank=0, aSize=8
    ):
        super().__init__(aGenThread)
        self.mAddrReg = None  # register for address
        self.mSemaVA = None  # semaphore virtual address
        self.mVaAttr = aMemAttrImpl  # semaphore va attribute
        self.mName = aName  # semaphore name
        self.mCounter = aCounter  # semaphore initial value
        self.mBank = aBank  # which bank to allocate semaphore
        self.mSize = aSize  # semaphore size
        self.mSharePA = None  # physical address allocated
        self.mReverseEndian = None  # Whether or not Reverse data endian

        self.setup()

    def _acquireSemaphore(self):
        raise NotImplementedError

    def _releaseSemaphore(self):
        raise NotImplementedError

    def _reloadSemaphore(self):
        (
            self.mSharedPA,
            self.mReverseEndian,
            valid,
        ) = self.genThread.genSemaphore(
            self.mName, self.mCounter, self.mBank, self.mSize
        )  # Shared PA has been initialized with the counter
        if not valid:
            self.error(
                "Thread %d failed to generate semaphore as the PA 0x%x is "
                "out of address size" % (self._threadId(), self.mSharePA)
            )

        self.mSemaVA = self.genVAforPA(
            Size=self.mSize,
            Align=self.mSize,
            Type="D",
            PA=self.mSharedPA,
            Bank=self.mBank,
            MemAttrImpl=self.mVaAttr,
            CanAlias=1,
        )
        if (self.mSemaVA & 0x00FFFFFFFFFF0000) == 0:
            self.error("ERROR VA=%x is invalid" % self.mSemaVA)

        # TODO(Noah): Implement a better mechanism for checking whether paging
        #  is enabled than checking whether the page info object is empty when
        #  such a mechanism can be devised. There may be error conditions when
        #  paging is enabled that also yield an empty page info object.
        shared_va_page_info = self.getPageInfo(self.mSemaVA, "VA", self.mBank)
        if shared_va_page_info and (
            shared_va_page_info["Page"]["MemoryAttr"] != self.mVaAttr
        ):
            self.error(
                "ERROR VA=%x is set to %s instead of %s"
                % (
                    self.mSemaVA,
                    shared_va_page_info["Page"]["MemoryAttr"],
                    self.mVaAttr,
                )
            )

        self.notice(
            "Thread %d map va 0x%x to [%d] pa 0x%x"
            % (self._threadId(), self.mSemaVA, self.mBank, self.mSharedPA)
        )

        self._loadAddressRegister()

    def _threadId(self):
        return self.genThread.genThreadID

    def _handleLowPower(self):
        gen_mode = self.getPEstate("GenMode")
        while gen_mode & (1 << 9):  # low power mode
            restart_pc = self.getPEstate("PC")
            gen_mode &= ~(1 << 9)
            self.setPEstate("GenMode", gen_mode)
            self.genSequence("ReExecution", {"Address": restart_pc})
            gen_mode = self.getPEstate("GenMode")

    def _loadAddressRegister(self):
        raise NotImplementedError
