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
import itertools
import os
from base.exception_handlers.ExceptionHandlerManager import ExceptionHandlerManager
from base.exception_handlers.MemoryBankHandlerRegistry import MemoryBankHandlerRegistryRepository, MemoryBankHandlerRegistry
from riscv.MemoryBank import MemoryBankRISCV
from riscv.PrivilegeLevel import PrivilegeLevelRISCV
from riscv.SecurityState import SecurityStateRISCV
from riscv.exception_handlers.ExceptionClass import ExceptionClassRISCV
from riscv.exception_handlers.HandlerSubroutineGenerator import HandlerSubroutineGeneratorRISCV

class ExceptionHandlerManagerRISCV(ExceptionHandlerManager):

    def __init__(self, gen_thread, factory):
        super().__init__(gen_thread, factory)

        for mem_bank in MemoryBankRISCV:
            self.memBankHandlerRegistryRepo.addMemoryBankHandlerRegistry(MemoryBankHandlerRegistry(gen_thread, mem_bank))

    def registerDefaultExceptionHandlers(self):
        super().registerDefaultExceptionHandlers()

        for mem_bank_handler_registry in self.memBankHandlerRegistryRepo.getMemoryBankHandlerRegistries():
            mem_bank_handler_registry.mHandlerSubroutineGenerator = HandlerSubroutineGeneratorRISCV(self.genThread, self.factory, self.exceptions_stack)

            if self.fastMode():
                mem_bank_handler_registry.registerExceptionHandlerWithClassName('riscv.exception_handlers.FastExceptionHandlers', 'FastEmptyHandlerRISCV', self.factory, self.exceptions_stack)
                self.thread_handler_set.assignAsynchronousExceptionHandler('FastEmptyHandlerRISCV')
            else:
                mem_bank_handler_registry.registerExceptionHandlerWithClassName('riscv.exception_handlers.AsynchronousHandlers', 'AsynchronousHandlerRISCV', self.factory, self.exceptions_stack)
                self.thread_handler_set.assignAsynchronousExceptionHandler('AsynchronousHandlerRISCV')

    def configureHandlerMemory(self):
        (exc_memory, mem_size) = self.queryHandlerSetMemory("0")

        self.debugPrint("[ExceptionHandlerManagerRISCV] EXC MEMORY START: 0x%x" % exc_memory)

        handler_registry = self.memBankHandlerRegistryRepo.getMemoryBankHandlerRegistry(MemoryBankRISCV.DEFAULT)
        handler_registry.mStartAddr = exc_memory

        exception_bounds_info_set = {"Function": "UpdateExceptionBounds"}
        exception_bounds_info_set["memory_bounds"] = "%s,%s" % (exc_memory, mem_size)
        self.exceptionRequest("UpdateHandlerInfo", exception_bounds_info_set)

    def getDefaultAssignmentFilePath(self, defaultSetName):
        assignment_file_name = 'default_%s_exception_handlers.json' % defaultSetName.lower()
        assignment_file_path = os.path.join(os.path.dirname(__file__), assignment_file_name)
        return assignment_file_path

    # initialize vector base address registers for current thread from exception set addresses
    def initializeVectorBaseAddressRegisters(self):
        mtvec_val = self.thread_handler_set.getVectorBaseAddress(PrivilegeLevelRISCV.M, SecurityStateRISCV.DEFAULT)
        self.initializeRegister("mtvec", mtvec_val)

        stvec_val = self.thread_handler_set.getVectorBaseAddress(PrivilegeLevelRISCV.S, SecurityStateRISCV.DEFAULT)
        self.initializeRegister("stvec", stvec_val)

    def getExceptionClass(self, exceptionClassName):
        (exception_code_class_name, _, err_code_name) = exceptionClassName.partition('.')
        return ExceptionClassRISCV[err_code_name]

    def getMemoryBanks(self):
        return tuple(MemoryBankRISCV)

    def getPrivilegeLevelSecurityStateCombinations(self):
        combinations = itertools.product(PrivilegeLevelRISCV, SecurityStateRISCV)

        def filterValid(aPrivLevelSecurityStateCombo):
            (priv_level, security_state) = aPrivLevelSecurityStateCombo
            valid = True
            if priv_level == PrivilegeLevelRISCV.U:
                valid = False

            return valid

        return filter(filterValid, combinations)
