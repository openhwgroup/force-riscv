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
import copy
from base.Sequence import Sequence
from base.exception_handlers.MemoryBankHandlerRegistry import (
    MemoryBankHandlerRegistryRepository,
)
from base.exception_handlers.ExceptionHandlerAssignmentParser import (
    ExceptionHandlerAssignmentParser,
)
from base.exception_handlers.ExceptionHandlerAssignment import (
    ExceptionHandlerAssignmentRequest,
)

# generate exception-sets for all threads...
#
#  1. Define choices to allow user to specify how many exception-sets to
#     instance, where maximum number is no larger than the number of threads
#     in play.
#
#  2. Create the pool of exception-set instances.
#
#  3. Each thread then 'maps'* to an exceptions-set instance.
#
#  *note: maps in the sense that the vector base address registers for the
#         thread are initialized to addresses within the exception-set memory.


class ExceptionHandlerManager(Sequence):

    cSharedHandlerSetGenerated = False  # indicate if the shared code has been generated.

    def __init__(self, gen_thread, factory):
        super().__init__(gen_thread)

        self.factory = factory
        self.memBankHandlerRegistryRepo = MemoryBankHandlerRegistryRepository()
        self.exceptions_stack = factory.createExceptionHandlerStack(gen_thread)

        self.thread_handler_set = factory.createThreadHandlerSet(
            gen_thread, self.memBankHandlerRegistryRepo, self.exceptions_stack
        )

        self.address_table = None
        self.spIndex = None

        # Force provides two default exception handler sets: 'Comprehensive'
        # and 'Fast'
        self.default_set_name = "Comprehensive"
        (default_set_name, valid) = self.getOption("handlers_set")
        if valid:
            self.default_set_name = default_set_name

    def createShallowCopy(self, gen_thread):
        self_copy = copy.copy(self)
        self_copy.genThread = gen_thread
        factory = self_copy.factory
        self_copy.exceptions_stack = factory.createExceptionHandlerStack(gen_thread)
        self_copy.address_table = None

        return self_copy

    def setup(self, **kargs):
        super().setup()

        self.genThread.modifyGenMode("NoEscape,SimOff,NoJump,NoSkip," "DelayInit")

    def cleanUp(self, **kargs):
        super().cleanUp()

        self.genThread.revertGenMode("NoEscape,SimOff,NoJump,NoSkip," "DelayInit")

    def debugPrint(self, msg):
        self.debug("DEBUG [ExceptionHandlerManager]: %s" % msg)

    def registerSynchronousExceptionHandler(self, aAssignmentRequest):
        self.thread_handler_set.assignSynchronousExceptionHandler(aAssignmentRequest)

    # register user-defined asynchronous exception handler
    def registerAsynchronousExceptionHandler(self, handler_module_name, handler_class_name):
        for mem_bank in self.getMemoryBanks():
            repo = self.memBankHandlerRegistryRepo
            registry = repo.getMemoryBankHandlerRegistry(mem_bank)
            registry.registerExceptHandlerWithClassName(
                handler_module_name,
                handler_class_name,
                self.factory,
                self.exceptions_stack,
            )

        self.thread_handler_set.assignAsynchronousExceptionHandler(handler_class_name)

    # register user-defined dispatch code generator
    def registerSynchronousExceptionDispatcher(self, dispatcher):
        if self.fastMode():
            self.thread_handler_set.user_sync_dispatcher = dispatcher
        else:
            self.error(
                "Error: Handler set '%s' does not support user-defined "
                "dispatch." % self.default_set_name
            )

    # specify the default exception handler set
    def setDefaultExceptionHandlerSet(self, default_set_name):
        self.default_set_name = default_set_name

    def fastMode(self):
        return self.default_set_name == "Fast"

    # Generate all exception sets. Map a set to the current thread
    def generate(self, **kwargs):
        # generate stack for each thread
        # NOTE: result is same GPR used as stack pointer, for all threads
        if not self.fastMode():
            self.spIndex = self.exceptions_stack.generate(
                sp_index=self.spIndex, load_stack_pointer=False
            )

        # generate address table if it has not been generated
        address_table_manager = kwargs.get("address_table_manager")
        if address_table_manager:
            self.address_table = address_table_manager.addressTable()

        if not ExceptionHandlerManager.cSharedHandlerSetGenerated:
            ExceptionHandlerManager.cSharedHandlerSetGenerated = True
            self.registerDefaultExceptionHandlers()

            self.configureHandlerMemory()

            self.thread_handler_set.generate(
                exceptions_stack=self.exceptions_stack,
                address_table=self.address_table,
                default_set_name=self.default_set_name,
            )

        self.initializeVectorBaseAddressRegisters()

        # after all exception handlers are set, inform back-end the scratch
        # register sets and vbar_sets
        if not self.fastMode():
            scratch_reg_info_set = self.thread_handler_set.getScratchRegisterSets()
            scratch_reg_info_set["Function"] = "ExceptionRegisters"
            self.exceptionRequest("UpdateHandlerInfo", scratch_reg_info_set)

        vbar_info_set = self.thread_handler_set.getVectorBaseAddressSets()
        vbar_info_set["Function"] = "UpdateVectorBaseAddress"
        self.exceptionRequest("UpdateHandlerInfo", vbar_info_set)

        self.exceptionRequest(
            "UpdateHandlerInfo",
            {"Function": "FastMode", "is_fast_mode": self.fastMode()},
        )

    def registerDefaultExceptionHandlers(self):
        assignment_file_path = self.getDefaultAssignmentFilePath(self.default_set_name)
        assignment_parser = ExceptionHandlerAssignmentParser()
        handler_assignments = assignment_parser.parseHandlerAssignments(assignment_file_path)
        for (
            (exception_class_name, subexception_class_name),
            (handler_module_name, handler_class_name),
        ) in handler_assignments.items():
            exception_class = self.getExceptionClass(exception_class_name)

            subexception_class = None
            if subexception_class_name is not None:
                subexception_class = self.getExceptionClass(subexception_class_name)

            for (
                priv_level,
                security_state,
            ) in self.getPrivilegeLevelSecurityStateCombinations():
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

    def configureHandlerMemory(self):
        raise NotImplementedError

    def getDefaultAssignmentFilePath(self, defaultSetName):
        raise NotImplementedError

    # initialize vector base address registers for current thread from
    # exception set addresses
    def initializeVectorBaseAddressRegisters(self):
        raise NotImplementedError

    def getExceptionClass(self, exceptionClassName):
        raise NotImplementedError

    def getPrivilegeSecurityStateCombinations(self):
        raise NotImplementedError

    def getMemoryBanks(self):
        raise NotImplementedError
