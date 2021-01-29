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
from riscv.PrivilegeLevel import PrivilegeLevelRISCV
from riscv.exception_handlers.SyncDispatcherBase import SyncDispatcherBaseRISCV
from riscv.exception_handlers.ExceptionHandlerContext import RegisterCallRole

#----------------------------------------------------------------------------
# Comprehensive Exception Handler dispatcher, to dispatch the sync exception handlers
#----------------------------------------------------------------------------

class DefaultSyncDispatcher(SyncDispatcherBaseRISCV):
    def __init__(self, gen_thread, factory):
        super().__init__(gen_thread, factory)

    ## Generate the necessary instructions to execute prior to dispatching to exception handlers.
    # These instructions should be generated once at the top-level dispatch.
    #
    #  @param aHandlerContext A source for usable registers and other information useful in
    #       generating exception handler instructions.
    def processPreDispatch(self, aHandlerContext):
        caller_saved_reg_indices = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.CALLER_SAVED)
        self.exceptionsStack.newStackFrame(caller_saved_reg_indices)

        scratch_reg_index = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.TEMPORARY, 1)
        priv_level_reg_index = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.PRIV_LEVEL_VALUE)
        cause_reg_index = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.CAUSE_VALUE)
        err_code_reg_index = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.EC_VALUE)

        priv_level = PrivilegeLevelRISCV[self.privilegeLevel]
        self.mAssemblyHelper.genMoveImmediate(priv_level_reg_index, priv_level.value)
        self.mAssemblyHelper.genReadSystemRegister(cause_reg_index, ('%scause' % priv_level.name.lower()))

        # Drop the interrupt bit to isolate the error code
        self.mAssemblyHelper.genShiftLeftImmediate(err_code_reg_index, 1, aSrcRegIndex=cause_reg_index)
        self.mAssemblyHelper.genShiftRightImmediate(err_code_reg_index, 1)

        # Jump to the exception dispatch; return here afterward to unwind the stack and return from
        # the exception
        self.mAssemblyHelper.genRelativeBranchWithLink((self.exceptionsStack.frameInstructionCount() + 2) * 2)

        self.exceptionsStack.freeStackFrame()
        self.mAssemblyHelper.genExceptionReturn(priv_level)

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
        (base_reg_index, offset_reg_index) = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.TEMPORARY, 2)
        err_code_reg_index = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.EC_VALUE)

        self.genInstruction('AUIPC##RISCV', {'rd': base_reg_index, 'simm20': 0})
        self.mAssemblyHelper.genAddImmediate(base_reg_index, 20)

        # Use error code as word offset into the table
        self.mAssemblyHelper.genShiftLeftImmediate(offset_reg_index, 2, aSrcRegIndex=err_code_reg_index)
        self.mAssemblyHelper.genAddRegister(base_reg_index, offset_reg_index)
        self.genInstruction('JALR##RISCV', {'rd': 0, 'rs1': base_reg_index, 'simm12': 0, 'NoRestriction': 1})
