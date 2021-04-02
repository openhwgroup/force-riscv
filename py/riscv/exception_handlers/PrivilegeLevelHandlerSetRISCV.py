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
from base.exception_handlers.PrivilegeLevelHandlerSet import (
    PrivilegeLevelHandlerSet,
)
from riscv.SecurityState import SecurityStateRISCV
from riscv.exception_handlers.ExceptionClass import ExceptionClassRISCV
from riscv.exception_handlers.ExceptionHandlerContext import (
    ExceptionHandlerContext,
    ComprehensiveExceptionHandlerContext,
)


class PrivilegeLevelHandlerSetRISCV(PrivilegeLevelHandlerSet):
    def __init__(
        self,
        gen_thread,
        privLevel,
        memBankHandlerRegistryRepo,
        factory,
        exceptionsStack,
    ):
        super().__init__(
            gen_thread,
            privLevel,
            memBankHandlerRegistryRepo,
            factory,
            exceptionsStack,
        )

        self.handlersBoundaries = []

    # pick scratch registers to use in exception handlers
    def setupScratchRegisters(self):
        if len(self.scratchRegs) > 0:
            return

        # Exclude the zero register, implied register operands, the stack
        # pointer and the address table pointer
        excluded_regs = "0,1,2,%d" % self.address_table.tableIndex()
        if not self.fastMode():
            excluded_regs = "%d,%s" % (
                self.exceptions_stack.pointerIndex(),
                excluded_regs,
            )

        self.scratchRegs = self.getRandomGPRs(
            self._scratchRegisterCount(), exclude=excluded_regs
        )
        if not self.scratchRegs:
            raise RuntimeError(
                "Unable to allocate scratch registers required by exception "
                "handlers."
            )

    def generateHandlerSubroutines(self, aSecurityState):
        if not self.fastMode():
            default_mem_bank = aSecurityState.getDefaultMemoryBank()

            save_pc = self.getPEstate("PC")
            start_addr = self.nextCodeAddresses[default_mem_bank]
            self.setPEstate("PC", start_addr)
            repo = self.memBankHandlerRegistryRepo
            registry = repo.getMemoryBankHandlerRegistry(default_mem_bank)
            handler_context = self.createExceptionHandlerContext(
                0, default_mem_bank
            )

            generator = registry.mHandlerSubroutineGenerator
            generator.generateRoutine(
                "TableWalk", handler_context=handler_context
            )

            end_addr = self.getPEstate("PC")
            self.setPEstate("PC", save_pc)
            self.nextCodeAddresses[default_mem_bank] = end_addr

    def getHandlerBoundaries(self, mem_bank):
        return self.handlersBoundaries

    def getSecurityStates(self):
        return tuple(SecurityStateRISCV)

    def getExceptionCodeClass(self):
        return ExceptionClassRISCV

    def recordSpecificHandlerBoundary(
        self, mem_bank, handler_name, start_addr, end_addr
    ):
        self.handlersBoundaries.append((handler_name, start_addr, end_addr))

    def getAsynchronousHandlerErrorCode(self):
        return 63

    def getDispatchErrorCode(self):
        return 64

    def createExceptionHandlerContext(self, err_code, mem_bank):
        repo = self.memBankHandlerRegistryRepo
        registry = repo.getMemoryBankHandlerRegistry(mem_bank)
        if self.fastMode():
            handler_context = ExceptionHandlerContext(
                err_code,
                self.scratchRegs,
                self.privLevel.name,
                self.exceptions_stack,
                self.address_table,
                registry,
            )
        else:
            handler_context = ComprehensiveExceptionHandlerContext(
                err_code,
                self.scratchRegs,
                self.privLevel.name,
                self.exceptions_stack,
                self.address_table,
                registry,
            )

        return handler_context

    def getInstructionLength(self):
        return 4
