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
from base.Sequence import Sequence


# manage exception handlers
# this class is responsible for generating specific exception handlers for all
# exception-vector-offsets/error-codes, for a specified privilege level.
#
# The generate method sets up the exception handler registry.
#
# The generateHandler method is used to generate the instructions to handle
# a particular exception.


class PrivilegeLevelHandlerSet(Sequence):
    def __init__(
        self,
        gen_thread,
        privLevel,
        memBankHandlerRegistryRepo,
        factory,
        exceptionsStack,
    ):
        super().__init__(gen_thread)

        self.privLevel = privLevel
        self.memBankHandlerRegistryRepo = memBankHandlerRegistryRepo
        self.factory = factory
        self.exceptions_stack = exceptionsStack

        self.security_state_handler_sets = {}
        for security_state in self.getSecurityStates():
            self.security_state_handler_sets[
                security_state
            ] = self.factory.createSecurityStateHandlerSet(
                security_state, memBankHandlerRegistryRepo, exceptionsStack
            )

        self.scratchRegs = []
        self.address_table = None
        self.default_set_name = None
        self.master_async_handler_name = None
        self.nextCodeAddresses = None

    # the generate method sets up table and code addresses, and registers
    # all default exception handlers.
    # NOTE: register custom handlers after generate, and before any calls
    #       to generateHandler method
    def generate(self, **kwargs):
        self.address_table = kwargs[
            "address_table"
        ]  # handlers can use address table to get recovery address

        if ("scratch_regs" in kwargs) and kwargs["scratch_regs"]:
            self.scratchRegs = kwargs["scratch_regs"]

        self.default_set_name = kwargs[
            "default_set_name"
        ]  # impacts scratch registers, handler generation
        self.nextCodeAddresses = kwargs[
            "handler_memory"
        ]  # code allocated sequentially, from here

        for (mem_bank, addr) in self.nextCodeAddresses.items():
            self._debugPrint("NEXT %s CODE ADDR: 0x%x" % (mem_bank, addr))

    # pick scratch registers to use in exception handlers
    def setupScratchRegisters(self):
        raise NotImplementedError

    def scratchRegisters(self):
        return self.scratchRegs

    # assignSynchronousExceptionHandler - assign synchronous exception
    #       handler code-generator based on error code
    def assignSynchronousExceptionHandler(self, aAssignmentRequest):
        for security_state in aAssignmentRequest.mSecurityStates:
            self._debugPrint(
                "[PrivilegeLevelHandlerSet:assignSynchronousExceptionHandler "
                "at %s] %s --> 0x%x"
                % (
                    self.privLevel,
                    aAssignmentRequest.mExcClass,
                    aAssignmentRequest.mExcClass.value,
                )
            )

            self.security_state_handler_sets[
                security_state
            ].assignSynchronousExceptionHandler(aAssignmentRequest)

    # assignAsynchronousExceptionHandler - assign asynchronous exception
    #       handler code-generator
    def assignAsynchronousExceptionHandler(self, aHandlerClassName):
        if self.master_async_handler_name is None:
            self.master_async_handler_name = aHandlerClassName

    def generateHandlerSubroutines(self, aSecurityState):
        raise NotImplementedError

    def generateSynchronousHandlers(self, aSecurityState, aSyncDispatcher):
        default_mem_bank = aSecurityState.getDefaultMemoryBank()

        save_pc = self.getPEstate("PC")
        start_addr = self.nextCodeAddresses[default_mem_bank]
        self.setPEstate("PC", start_addr)

        err_code = self.getDispatchErrorCode()
        handler_context = self.createExceptionHandlerContext(
            err_code, default_mem_bank
        )
        aSyncDispatcher.generatePreDispatch(handler_context)

        security_state_handler_set = self.security_state_handler_sets[
            aSecurityState
        ]
        ss_handlers = security_state_handler_set
        handler_assignments = (
            ss_handlers.getSynchronousExceptionHandlerAssignments()
        )
        self._generateSynchronousDispatchLevel(
            self.getExceptionCodeClass(),
            handler_assignments,
            aSecurityState,
            aSyncDispatcher,
        )

        aSyncDispatcher.generatePostDispatch(handler_context)

        self.nextCodeAddresses[default_mem_bank] = self.getPEstate("PC")
        self.setPEstate("PC", save_pc)

        return start_addr

    # method used to generate asynchronous handlers
    def generateAsynchronousHandler(self, security_state):
        self.debug(
            "[GENHANDLER] [%s] err-code: asynchronous\n" % self.privLevel
        )

        default_mem_bank = security_state.getDefaultMemoryBank()

        # generate asynchronous exception handler, indexed by exc vector offset
        repo = self.memBankHandlerRegistryRepo
        mem_bank_handler_registry = repo.getMemoryBankHandlerRegistry(
            default_mem_bank
        )
        handler = mem_bank_handler_registry.getExceptionHandler(
            self.master_async_handler_name
        )

        # generate the handler; record its address
        save_pc = self.getPEstate("PC")
        start_addr = self.nextCodeAddresses[default_mem_bank]
        self.setPEstate("PC", start_addr)

        err_code = self.getAsynchronousHandlerErrorCode()
        handler.generateRoutine(
            self._getHandlerRoutineName(handler),
            handler_context=self.createExceptionHandlerContext(
                err_code, default_mem_bank
            ),
        )

        end_addr = self.getPEstate("PC")
        self.setPEstate("PC", save_pc)
        self.nextCodeAddresses[default_mem_bank] = end_addr

        self.recordSpecificHandlerBoundary(
            default_mem_bank, err_code, start_addr, end_addr
        )

    # method used to generate synchronous exception dispatcher
    def generateUserSyncDispatch(self, security_state, sync_dispatcher):
        default_mem_bank = security_state.getDefaultMemoryBank()

        save_pc = self.getPEstate("PC")
        start_addr = self.nextCodeAddresses[default_mem_bank]
        self.setPEstate("PC", start_addr)

        err_code = self.getDispatchErrorCode()
        sync_dispatch_addr = sync_dispatcher.generate(
            handler_context=self.createExceptionHandlerContext(
                err_code, default_mem_bank
            )
        )

        end_addr = self.getPEstate("PC")
        self.setPEstate("PC", save_pc)
        self.nextCodeAddresses[default_mem_bank] = end_addr

        self.recordSpecificHandlerBoundary(
            default_mem_bank, err_code, start_addr, end_addr
        )
        return start_addr

    def getNextCodeAddress(self, aMemBank):
        return self.nextCodeAddresses[aMemBank]

    def genJumpToAsynchronousHandler(self, aSecurityState):
        repo = self.memBankHandlerRegistryRepo
        registry = repo.getMemoryBankHandlerRegistry(
            aSecurityState.getDefaultMemoryBank()
        )
        handler = registry.getExceptionHandler(self.master_async_handler_name)
        handler.jumpToRoutine(self._getHandlerRoutineName(handler))

    def getSynchronousExceptionHandler(self, security_state, exception_class):
        security_state_handler_set = self.security_state_handler_sets[
            security_state
        ]

        ss_handlers = security_state_handler_set
        assignment = ss_handlers.getSynchronousExceptionHandlerAssignment(
            exception_class
        )
        repo = self.memBankHandlerRegistryRepo
        mem_bank_handler_registry = repo.getMemoryBankHandlerRegistry(
            assignment.mMemBank
        )
        handler = mem_bank_handler_registry.getExceptionHandler(
            assignment.mHandlerClassName
        )
        return handler

    def getHandlerBoundaries(self, mem_bank):
        raise NotImplementedError

    def getSecurityStates(self):
        raise NotImplementedError

    def getExceptionCodeClass(self):
        raise NotImplementedError

    def recordSpecificHandlerBoundary(
        self, mem_bank, handler_name, start_addr, end_addr
    ):
        raise NotImplementedError

    def getAsynchronousHandlerErrorCode(self):
        raise NotImplementedError

    def getDispatchErrorCode(self):
        raise NotImplementedError

    def createExceptionHandlerContext(self, err_code):
        raise NotImplementedError

    def getInstructionLength(self):
        raise NotImplementedError

    def fastMode(self):
        return self.default_set_name == "Fast"

    def _generateSynchronousDispatchLevel(
        self,
        aExceptionCodeClass,
        aHandlerAssignments,
        aSecurityState,
        aSyncDispatcher,
    ):
        err_code = self.getDispatchErrorCode()
        default_mem_bank = aSecurityState.getDefaultMemoryBank()
        aSyncDispatcher.generateDispatch(
            self.createExceptionHandlerContext(err_code, default_mem_bank),
            aExceptionCodeClass,
        )

        # Leave space to generate a jump table after generating the handlers
        jump_table_pc = self.getPEstate("PC")
        handler_pc = (
            jump_table_pc
            + len(aHandlerAssignments) * self.getInstructionLength()
        )
        self.setPEstate("PC", handler_pc)

        dispatch_addresses = {}
        sorted_handler_assignments = sorted(aHandlerAssignments.items())
        for (
            exception_class,
            handler_assignment,
        ) in sorted_handler_assignments:
            if handler_assignment.hasSubassignments():
                dispatch_addresses[exception_class] = self.getPEstate("PC")
                self._generateSynchronousDispatchLevel(
                    handler_assignment.getSubexceptionCodeClass(),
                    handler_assignment.getSubassignments(),
                    aSecurityState,
                    aSyncDispatcher,
                )
            else:
                self._generateSynchronousHandler(
                    exception_class,
                    handler_assignment.mHandlerClassName,
                    handler_assignment.mMemBank,
                    aSecurityState,
                )

        end_handler_pc = self.getPEstate("PC")
        self.setPEstate("PC", jump_table_pc)

        assembly_helper = self.factory.createAssemblyHelper(self)
        for (
            exception_class,
            handler_assignment,
        ) in sorted_handler_assignments:
            if handler_assignment.hasSubassignments():
                assembly_helper.genRelativeBranchToAddress(
                    dispatch_addresses[exception_class]
                )
            else:
                self._genJumpToSynchronousHandler(
                    handler_assignment.mHandlerClassName,
                    handler_assignment.mMemBank,
                )

        self.setPEstate("PC", end_handler_pc)

    # generateHandler method is used to generate the instructions to handle
    # a particular exception handler
    def _generateSynchronousHandler(
        self, aExceptionClass, aHandlerClassName, aMemBank, aSecurityState
    ):
        self.debug(
            "[GENHANDLER] [%s] err-code: 0x%x\n"
            % (self.privLevel, aExceptionClass.value)
        )

        # generate synchronous exception handler, indexed by error code
        repo = self.memBankHandlerRegistryRepo
        registry = repo.getMemoryBankHandlerRegistry(aMemBank)
        handler = registry.getExceptionHandler(aHandlerClassName)

        # call custom handler code generator. that code better add in return
        # (or eret if appropriate) if there is not already a generated handler
        # instance that can service this exception, generate a new instance
        err_code = aExceptionClass.value
        handler_routine_name = self._getHandlerRoutineName(handler)
        if not handler.hasGeneratedRoutine(handler_routine_name):
            start_addr = self.getPEstate("PC")

            # The dispatch code and jump tables will be generated in the
            # default memory bank. We generate handlers in this memory bank in
            # a continuous block without needing to adjust the PC value. For
            # handlers that need to be generated in a different memory bank, we
            # need to modify the PC value and restore it afterward.
            save_pc = None
            if aMemBank != aSecurityState.getDefaultMemoryBank():
                save_pc = self.getPEstate("PC")
                start_addr = self.nextCodeAddresses[aMemBank]
                self.setPEstate("PC", start_addr)

            handler.generateRoutine(
                handler_routine_name,
                handler_context=self.createExceptionHandlerContext(
                    err_code, aMemBank
                ),
            )

            end_addr = self.getPEstate("PC")
            if save_pc is not None:
                self.setPEstate("PC", save_pc)
                self.nextCodeAddresses[aMemBank] = end_addr

            # Update the dictionary with the new addresses
            self.recordSpecificHandlerBoundary(
                aMemBank, err_code, start_addr, end_addr
            )

            self.debug(
                "[GENHANDLER] [%s] NEW CUSTOM SYNC HANDLER AT ADDR: "
                "0x%x; err-code: 0x%x" % (self.privLevel, start_addr, err_code)
            )
        else:
            self.debug(
                "[GENHANDLER] [%s] REUSING CUSTOM SYNC HANDLER: err-code: 0x%x"
                % (self.privLevel, err_code)
            )

        if hasattr(handler, "use_addr_table") and (handler.use_addr_table):
            info_set = {}
            info_set["Function"] = "AddrTableEC"
            info_set["EC"] = err_code
            self.exceptionRequest("UpdateHandlerInfo", info_set)

    def _genJumpToSynchronousHandler(self, aHandlerClassName, aMemBank):
        repo = self.memBankHandlerRegistryRepo
        registry = repo.getMemoryBankHandlerRegistry(aMemBank)
        handler = registry.getExceptionHandler(aHandlerClassName)
        handler.jumpToRoutine(self._getHandlerRoutineName(handler))

    def _scratchRegisterCount(self):
        if self.fastMode():
            return 2
        else:
            return 18

    def _getHandlerRoutineName(self, handler):
        routine_name = "Handler%s" % self.privLevel.name
        if not handler.hasRoutine(routine_name):
            routine_name = "Handler"

        return routine_name

    def _debugPrint(self, msg):
        self.debug("DEBUG [ExceptionLevelHandlers]: %s" % msg)
