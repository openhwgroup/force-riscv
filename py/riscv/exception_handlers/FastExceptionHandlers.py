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

from riscv.exception_handlers.FastExceptionHandlersBase import (
    FastExceptionHandlersBaseRISCV,
)


class FastEmptyHandlerRISCV(FastExceptionHandlersBaseRISCV):
    def processException(self):
        self.debug(
            "[FastEmptyHandlerRISCV] process exception %s, privilege level: %s"
            % (self.mErrCode, self.mPrivLevel)
        )

        print(
            "[FastEmptyHandlerRISCV] process exception %s, privilege level: %s"
            % (self.mErrCode, self.mPrivLevel)
        )


class FastSkipInstructionHandlerRISCV(FastExceptionHandlersBaseRISCV):
    def processException(self):
        self.debug(
            "[FastSkipInstructionHandlerRISCV] process exception %s, "
            "privilege level: %s" % (self.mErrCode, self.mPrivLevel)
        )

        print(
            "[FastSkipInstructionHandlerRISCV] process exception %s, "
            "privilege level: %s" % (self.mErrCode, self.mPrivLevel)
        )

        scratch_reg_index = self.mScratchRegisters[0]

        epc_name = (
            "mepc"
            if self.mPrivLevel == "M"
            else "sepc"
            if self.mPrivLevel == "S"
            else "unknown_pc_error"
        )

        self.mAssemblyHelper.genReadSystemRegister(scratch_reg_index, epc_name)
        self.mAssemblyHelper.genAddImmediate(scratch_reg_index, 4)
        self.mAssemblyHelper.genWriteSystemRegister(
            epc_name, scratch_reg_index
        )


class FastRecoveryAddressHandlerRISCV(FastExceptionHandlersBaseRISCV):
    def processException(self):
        self.debug(
            "[FastRecoveryAddressHandlerRISCV] process exception %s, "
            "privilege level: %s" % (self.mErrCode, self.mPrivLevel)
        )

        scratch_reg_index = self.mScratchRegisters[0]
        recovery_reg_index = self.mScratchRegisters[1]

        # get the next address from the address table
        self.mAddrTable.getAddress(recovery_reg_index, scratch_reg_index)

        # update the appropriate epc with the recovery address
        epc_name = (
            "mepc"
            if self.mPrivLevel == "M"
            else "sepc"
            if self.mPrivLevel == "S"
            else "unknown_pc_error"
        )

        self.mAssemblyHelper.genWriteSystemRegister(
            epc_name, recovery_reg_index
        )
