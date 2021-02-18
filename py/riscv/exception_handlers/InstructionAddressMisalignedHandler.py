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


class InstructionAddressMisalignedHandlerRISCV(ReusableSequence):
    def __init__(self, aGenThread, aFactory, aStack):
        super().__init__(aGenThread, aFactory, aStack)
        self.privilegeLevel = None

    def generateHandler(self, **kwargs):
        try:
            handler_context = kwargs["handler_context"]
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to "
                "InstructionAddressMisalignedHandlerRISCV generate method "
                "missing."
            )

        self.debug(
            "[InstructionAddressMisalignedHandlerRISCV] generate handler "
            "address: 0x%x" % self.getPEstate("PC")
        )

        self.privilegeLevel = handler_context.mPrivLevel
        priv_level = PrivilegeLevelRISCV[self.privilegeLevel]

        stval_reg_index = handler_context.getScratchRegisterIndices(
            RegisterCallRole.TEMPORARY, 1
        )

        # retreive misaligned instruction address...

        self.mAssemblyHelper.genReadSystemRegister(
            stval_reg_index, ("%stval" % priv_level.name.lower())
        )

        # mask off low order 2 bits to yield word-aligned address...

        self.mAssemblyHelper.genOrImmediate(stval_reg_index, 0x3)
        self.mAssemblyHelper.genXorImmediate(stval_reg_index, 0x3)

        # write the aligned branch target as the exception return address...

        self.mAssemblyHelper.genWriteSystemRegister(
            ("%sepc" % priv_level.name.lower()), stval_reg_index
        )

        # done.
        self.mAssemblyHelper.genReturn()
