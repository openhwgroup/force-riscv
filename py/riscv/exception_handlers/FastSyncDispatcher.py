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
from riscv.exception_handlers.SyncDispatcherBase import SyncDispatcherBaseRISCV
from riscv.exception_handlers.ExceptionHandlerContext import RegisterCallRole
from riscv.exception_handlers.ExceptionClass import ExceptionClassRISCV

#----------------------------------------------------------------------------
# Fast Exception Handler dispatcher, to dispatch the sync exception handlers
#----------------------------------------------------------------------------

class DefaultFastSyncDispatcher(SyncDispatcherBaseRISCV):
    def __init__(self, gen_thread, factory):
        super().__init__(gen_thread, factory)

    ## Generate instructions to dispatch to exception handlers based on the specified exception code
    # class. These instructions may be generated repeatedly: once for the top-level dispatch and
    # potentially multiple times at finer levels of dispatch granularity to handle various
    # subexception codes.
    #
    #  @param aHandlerContext A source for usable registers and other information useful in
    #       generating exception handler instructions.
    #  @param aExceptionCodeClass The class defining the possible exception codes for this level of
    #       dispatch granularity.
    def processDispatch(self, aHandlerContext, aExceptionCodeClass):
        scratch_reg_index = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.ALL, 1)

        err_code_reg_index = self._genLoadErrorCode(aHandlerContext)

        self.genInstruction('AUIPC##RISCV', {'rd': scratch_reg_index, 'simm20': 0})
        self.mAssemblyHelper.genAddImmediate(scratch_reg_index, 20)

        # Use error code as word offset into the table
        self.mAssemblyHelper.genShiftLeftImmediate(err_code_reg_index, 2)
        self.mAssemblyHelper.genAddRegister(scratch_reg_index, err_code_reg_index)
        self.genInstruction('JALR##RISCV', {'rd': 0, 'rs1': scratch_reg_index, 'NoRestriction': 1})

    def _genLoadErrorCode(self, aHandlerContext):
        (_, err_code_reg_index) = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.ALL, 2)

        self.mAssemblyHelper.genReadSystemRegister(err_code_reg_index, ('%scause' % self.privilegeLevel))

        # Drop the interrupt bit to isolate the error code
        self.mAssemblyHelper.genShiftLeftImmediate(err_code_reg_index, 1)
        self.mAssemblyHelper.genShiftRightImmediate(err_code_reg_index, 1)

        return err_code_reg_index
