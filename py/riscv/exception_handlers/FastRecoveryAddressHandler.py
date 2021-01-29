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
from base.exception_handlers.ReusableSequence import ReusableSequence
from riscv.exception_handlers.ExceptionHandlerContext import RegisterCallRole
from riscv.PrivilegeLevel import PrivilegeLevelRISCV

class FastRecoveryAddressHandler(ReusableSequence):

    def generateHandler(self, **kwargs):
        try:
            handler_context = kwargs['handler_context']
        except KeyError:
            self.error('INTERNAL ERROR: one or more arguments to FastRecoveryAddresshandler generate method missing.')

        self.debug('[FastRecoveryAddressHandler] generate handler address: 0x%x' % self.getPEstate('PC'))

        priv_level_reg_index = handler_context.getScratchRegisterIndices(RegisterCallRole.PRIV_LEVEL_VALUE)
        (scratch_reg_index, recovery_reg_index) = handler_context.getScratchRegisterIndices(RegisterCallRole.TEMPORARY, 2)

        handler_context.mAddrTable.getAddress(recovery_reg_index, scratch_reg_index)

        self.mAssemblyHelper.genProvidedExceptionReturnAddress(scratch_reg_index, recovery_reg_index, priv_level_reg_index)
        priv_level = PrivilegeLevelRISCV[handler_context.mPrivLevel]
        self.mAssemblyHelper.genExceptionReturn(priv_level)


