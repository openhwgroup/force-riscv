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
import itertools
import os

from base.exception_handlers.ExceptionHandlerAssignment import (
    ExceptionHandlerAssignmentRequest,
)
from base.exception_handlers.ExceptionHandlerAssignmentParser import (
    ExceptionHandlerAssignmentParser,
)
from base.exception_handlers.ExceptionHandlerManager import (
    ExceptionHandlerManager,
)
from base.exception_handlers.MemoryBankHandlerRegistry import (
    MemoryBankHandlerRegistry,
)
from riscv.MemoryBank import MemoryBankRISCV
from riscv.PrivilegeLevel import PrivilegeLevelRISCV
from riscv.SecurityState import SecurityStateRISCV
from riscv.exception_handlers.ExceptionClass import ExceptionClassRISCV
from riscv.exception_handlers.HandlerSubroutineGenerator import (
    HandlerSubroutineGeneratorRISCV,
)


class ExceptionHandlerManagerRISCV(ExceptionHandlerManager):
    def __init__(self, gen_thread, factory):
        super().__init__(gen_thread, factory)

        self.mGenThread = gen_thread

        for mem_bank in MemoryBankRISCV:
            self.memBankHandlerRegistryRepo.addMemoryBankHandlerRegistry(
                MemoryBankHandlerRegistry(gen_thread, mem_bank)
            )

    # return True if a particular exception can be redirected at a given
    # privilege level, based on General choices...
    def useTrapHandler(
        self, exceptionClassName, subexceptionClassName, privilegeLevel
    ):
        if privilegeLevel == PrivilegeLevelRISCV.M:
            # trap redirection supported (for now) only from machine-mode...
            pass
        else:
            return False

        # translate exception class name into redirect-trap choices tree...
        (class_prefix, exception_name) = exceptionClassName.split(".")
        exc_choice_trees = {
            "INSTRUCTION_ADDRESS_MISALIGNED": "Redirect Trap - Instruction "
            "address misaligned",
            "INSTRUCTION_ACCESS_FAULT": "Redirect Trap - Instruction access "
            "fault",
            "ILLEGAL_INSTRUCTION": "Redirect Trap - Illegal instruction",
            "BREAKPOINT": "Redirect Trap - Breakpoint",
            "LOAD_ADDRESS_MISALIGNED": "Redirect Trap - Load address "
            "misaligned",
            "LOAD_ACCESS_FAULT": "Redirect Trap - Load access fault",
            "STORE_AMO_ADDRESS_MISALIGNED": "Redirect Trap - Store/AMO "
            "address misaligned",
            "STORE_AMO_ACCESS_FAULT": "Redirect Trap - Store/AMO access fault",
            "ENV_CALL_FROM_U_MODE": "Redirect Trap - Environment call from "
            "U-mode",
            "ENV_CALL_FROM_S_MODE": "Redirect Trap - Environment call from "
            "S-mode",
            "INSTRUCTION_PAGE_FAULT": "Redirect Trap - Instruction page fault",
            "LOAD_PAGE_FAULT": "Redirect Trap - Load page fault",
            "STORE_AMO_PAGE_FAULT": "Redirect Trap - Store/AMO page fault",
        }

        use_trap_handler = False

        try:
            # make the choice...
            choices = self.mGenThread.getChoicesTreeInfo(
                exc_choice_trees[exception_name], "GeneralChoices"
            )
            use_trap_handler = self.pickWeighted(choices) == "DoRedirect"
        except KeyError:
            # not all exceptions can be redirected...
            pass

        return use_trap_handler

    def registerDefaultExceptionHandlers(self):
        assignment_file_path = self.getDefaultAssignmentFilePath(
            self.default_set_name
        )
        assignment_parser = ExceptionHandlerAssignmentParser()
        handler_assignments = assignment_parser.parseHandlerAssignments(
            assignment_file_path
        )

        trap_handler_module_name = None
        trap_handler_class_name = None
        have_trap_handler = False

        for (
            (exception_class_name, subexception_class_name),
            (handler_module_name, handler_class_name),
        ) in handler_assignments.items():
            if "TRAP_REDIRECTION" in exception_class_name:
                trap_handler_module_name = handler_module_name
                trap_handler_class_name = handler_class_name
                have_trap_handler = True
                continue

            exception_class = self.getExceptionClass(exception_class_name)

            subexception_class = None
            if subexception_class_name is not None:
                subexception_class = self.getExceptionClass(
                    subexception_class_name
                )

            for (
                priv_level,
                security_state,
            ) in self.getPrivilegeLevelSecurityStateCombinations():
                if have_trap_handler and self.useTrapHandler(
                    exception_class_name, subexception_class_name, priv_level
                ):
                    handler_module_name = trap_handler_module_name
                    handler_class_name = trap_handler_class_name

                handler_assignment_request = ExceptionHandlerAssignmentRequest(
                    exception_class,
                    (priv_level,),
                    (security_state,),
                    handler_module_name,
                    handler_class_name,
                    aMemBank=None,
                    aSubexcClass=subexception_class,
                )

                self.thread_handler_set.assignSynchronousExceptionHandler(
                    handler_assignment_request
                )

        for (
            mem_bank_handler_registry
        ) in self.memBankHandlerRegistryRepo.getMemoryBankHandlerRegistries():
            mem_bank_handler_registry.mHandlerSubroutineGenerator = HandlerSubroutineGeneratorRISCV(
                self.genThread, self.factory, self.exceptions_stack
            )

            if self.fastMode():
                mem_bank_handler_registry.registerExceptHandlerWithClassName(
                    "riscv.exception_handlers.FastExceptionHandlers",
                    "FastEmptyHandlerRISCV",
                    self.factory,
                    self.exceptions_stack,
                )
                self.thread_handler_set.assignAsynchronousExceptionHandler(
                    "FastEmptyHandlerRISCV"
                )
            else:
                mem_bank_handler_registry.registerExceptHandlerWithClassName(
                    "riscv.exception_handlers.AsynchronousHandlers",
                    "AsynchronousHandlerRISCV",
                    self.factory,
                    self.exceptions_stack,
                )
                self.thread_handler_set.assignAsynchronousExceptionHandler(
                    "AsynchronousHandlerRISCV"
                )

    def configureHandlerMemory(self):
        (exc_memory, mem_size) = self.queryHandlerSetMemory("0")

        self.debugPrint(
            "[ExceptionHandlerManagerRISCV] EXC MEMORY START: 0x%x"
            % exc_memory
        )

        handler_registry = self.memBankHandlerRegistryRepo.getMemoryBankHandlerRegistry(
            MemoryBankRISCV.DEFAULT
        )
        handler_registry.mStartAddr = exc_memory

        exception_bounds_info_set = {"Function": "UpdateExceptionBounds"}
        exception_bounds_info_set["memory_bounds"] = "%s,%s" % (
            exc_memory,
            mem_size,
        )
        self.exceptionRequest("UpdateHandlerInfo", exception_bounds_info_set)

    def getDefaultAssignmentFilePath(self, defaultSetName):
        assignment_file_name = (
            "default_%s_exception_handlers.json" % defaultSetName.lower()
        )
        assignment_file_path = os.path.join(
            os.path.dirname(__file__), assignment_file_name
        )
        return assignment_file_path

    # initialize vector base address registers for current thread from
    # exception set addresses
    def initializeVectorBaseAddressRegisters(self):
        mtvec_val = self.thread_handler_set.getVectorBaseAddress(
            PrivilegeLevelRISCV.M, SecurityStateRISCV.DEFAULT
        )
        self.initializeRegister("mtvec", mtvec_val)

        stvec_val = self.thread_handler_set.getVectorBaseAddress(
            PrivilegeLevelRISCV.S, SecurityStateRISCV.DEFAULT
        )
        self.initializeRegister("stvec", stvec_val)

    def getExceptionClass(self, exceptionClassName):
        (
            exception_code_class_name,
            _,
            err_code_name,
        ) = exceptionClassName.partition(".")
        return ExceptionClassRISCV[err_code_name]

    def getMemoryBanks(self):
        return tuple(MemoryBankRISCV)

    def getPrivilegeLevelSecurityStateCombinations(self):
        combinations = itertools.product(
            PrivilegeLevelRISCV, SecurityStateRISCV
        )

        def filter_valid(aPrivLevelSecurityStateCombo):
            (priv_level, security_state) = aPrivLevelSecurityStateCombo
            valid = True
            if priv_level == PrivilegeLevelRISCV.U:
                valid = False

            return valid

        return filter(filter_valid, combinations)
