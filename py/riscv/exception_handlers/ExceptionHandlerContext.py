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
from enum import Enum

## This class provides access to common objects and data required by most exception handlers.
class ExceptionHandlerContext(object):

    def __init__(self, aErrCode, aScratchRegIndices, aPrivLevel, aStack, aAddrTable, aMemBankHandlerRegistry):
        self.mErrCode = aErrCode
        self.mScratchRegIndices = {RegisterCallRole.ALL: aScratchRegIndices}
        self.mPrivLevel = aPrivLevel
        self.mStack = aStack
        self.mAddrTable = aAddrTable
        self.mMemBankHandlerRegistry = aMemBankHandlerRegistry

    @property
    def mMemBank(self):
        return self.mMemBankHandlerRegistry.mMemBank

    ## Retrieve indices of available scratch registers for the specified purpose. This method should
    # be called only once per RegisterCallRole value in the context of a given exception handler, as
    # calling it multiple times with the same RegisterCallRole value yields overlapping lists of
    # registers. If one index is returned, it is returned as a single value; otherwise, a list is
    # returned.
    #
    #  @param aRegCallRole A hint as to the role of the registers.
    #  @param aCount The number of register indices to return; returns all available for the
    #       specified role when no count is specified
    def getScratchRegisterIndices(self, aRegCallRole, aCount=None):
        if aRegCallRole in self.mScratchRegIndices:
            call_role_reg_indices = self.mScratchRegIndices[aRegCallRole]
        else:
            raise TypeError("Registers for %s are not available." % aRegCallRole)

        if isinstance(call_role_reg_indices, tuple):
            if aCount:
                if aCount > len(call_role_reg_indices):
                    raise ValueError("More scratch registers requested than are available for %s." % aRegCallRole)

                if aCount == 1:
                    call_role_reg_indices = call_role_reg_indices[0]
                else:
                    call_role_reg_indices = call_role_reg_indices[0:aCount]
        else:
            if aCount and (aCount != 1):
                raise ValueError("More scratch registers requested than are available for %s." % aRegCallRole)

        return call_role_reg_indices


## This class extends the exception handler context by partitioning the scratch registers into
# various roles; this is likely only useful for the comprehensive exception handlers. The aim is to
# provide a sufficient number of caller-saved registers for most exception handlers, but avoid
# allocating more than necessary, as each additional register requires execution of additional
# instructions. The few handlers that require a large number of registers can use the callee-saved
# registers and do the work of preserving their values within the handler itself.
class ComprehensiveExceptionHandlerContext(ExceptionHandlerContext):

    def __init__(self, aErrCode, aScratchRegIndices, aPrivLevel, aStack, aAddressTable, aMemBank):
        super().__init__(aErrCode, aScratchRegIndices, aPrivLevel, aStack, aAddressTable, aMemBank)

        self.mScratchRegIndices[RegisterCallRole.PRIV_LEVEL_VALUE] = aScratchRegIndices[0]
        self.mScratchRegIndices[RegisterCallRole.CAUSE_VALUE] = aScratchRegIndices[1]
        self.mScratchRegIndices[RegisterCallRole.EC_VALUE] = aScratchRegIndices[2]
        self.mScratchRegIndices[RegisterCallRole.ARGUMENT] = aScratchRegIndices[3:5]
        self.mScratchRegIndices[RegisterCallRole.TEMPORARY] = aScratchRegIndices[5:8]
        self.mScratchRegIndices[RegisterCallRole.CALLER_SAVED] = aScratchRegIndices[:8]
        self.mScratchRegIndices[RegisterCallRole.CALLEE_SAVED] = aScratchRegIndices[8:]


## This class defines roles for the various scratch registers. The caller is responsible for the
# preservation of all scratch register values, except those designated CALLEE_SAVED.
class RegisterCallRole(Enum):
    ALL = 0
    PRIV_LEVEL_VALUE = 1
    CAUSE_VALUE = 2
    EC_VALUE = 3
    ARGUMENT = 4
    TEMPORARY = 5
    CALLER_SAVED = 6
    CALLEE_SAVED = 7
