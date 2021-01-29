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
import copy

#-------------------------------------------------------------------------------------------------------
# ThreadHandlerSet
#
# this class is responsible for generating specific exception handlers for all
# exception-level/exception-vector-offsets/error-codes.
#
# ASSUME: this is called for each thread (or set of threads, say a pool of handlers)
#
# process:
#
#   foreach exception level:
#      pick sync-exceptions exc handler table address
#
#      pick exception-handlers code block address
#
#      exc_handlers = ExceptionHandlers(exc-level, handlers-code-block-addr, 64k, SP_index)
#
#      foreach sync-excep error-code:
#          NOTE: not generating unique sync exc handler for each vector offset (but we could)
#          exc handler table[error-code] = exc_handlers.generate(0, err_code)
#
#      NOTE: only one sync-exc dispatcher, for all vector offsets
#
#      generate sync exc dispatch code(exc-level, exc handler table address)
#
#      pick, set vector base address register value
#
#      foreach exc vector offset:
#         if sync-offset:
#           branch to dispatcher code; exception handlers already generated
#           generate-branch-to(vector base address + offset, exc dispatch code)
#         else:
#           generate async exc handler here for this exc vector offset
#           async_handler_address = exc_handlers.generate(offset)
#           branch directly to the handler
#           generate-branch-to(vector base address + offset, async_handler_address)
#-------------------------------------------------------------------------------------------------------

class ThreadHandlerSet(Sequence):

    def __init__(self, gen_thread, memBankHandlerRegistryRepo, factory, exceptionsStack):
        super().__init__(gen_thread)

        self.memBankHandlerRegistryRepo = memBankHandlerRegistryRepo
        self.factory = factory
        self.exceptions_stack = exceptionsStack
        self.vector_offset_tables = {}
        self.handler_memory = {}
        self.scratch_registers = None  # all generated handlers in set will use the same set of scratch registers
        self.default_set_name = None  # may be 'Fast', 'Comprehensive', etc.
        self.user_sync_dispatcher = None
        self.memBankHandlerRegistries = None
        self.address_table = None

        self.priv_level_handler_sets = {}
        for priv_level in self.getPrivilegeLevels():
            self.priv_level_handler_sets[priv_level] = self.factory.createPrivilegeLevelHandlerSet(gen_thread, priv_level, memBankHandlerRegistryRepo, exceptionsStack)

    def generate(self, **kwargs):
        self.address_table = kwargs['address_table']  # handlers can use address table to get recovery address

        self.memBankHandlerRegistries = self.memBankHandlerRegistryRepo.getMemoryBankHandlerRegistries()
        for mem_bank_handler_registry in self.memBankHandlerRegistries:
            self.debugPrint('MEMORY POOL ADDR: (%s) 0x%x' % (mem_bank_handler_registry.mMemBank, mem_bank_handler_registry.mStartAddr))

            self.handler_memory[mem_bank_handler_registry.mMemBank] = mem_bank_handler_registry.mStartAddr

        self.default_set_name = kwargs['default_set_name']  # default handler set impacts scratch registers, handler generation

        # generate exception handlers, vector offset branches, etc. for all
        # exception levels/memory-bank combinations
        self._genExcepHandlerCombos()

        # Notify the backend about the generated handlers and their addresses
        info_set = {}
        address_pair_format = '%s:%s:%s'
        for mem_bank_handler_registry in self.memBankHandlerRegistries:
            handler_boundaries = ''

            for (handler_name, handler_start_addr, handler_end_addr) in mem_bank_handler_registry.getHandlerBoundaries():
                handler_boundaries += address_pair_format % (handler_name, handler_start_addr, handler_end_addr)
                handler_boundaries += ';'

            # Trim the last separator
            handler_boundaries = handler_boundaries.rstrip(';')

            info_set[('%s_bounds' % mem_bank_handler_registry.mMemBank.name)] = handler_boundaries

        info_set['Function'] = 'RecordExceptionSpecificAddressBounds'
        self.exceptionRequest('UpdateHandlerInfo', info_set)

    # register any custom exception handlers BEFORE generate is called
    def assignSynchronousExceptionHandler(self, aAssignmentRequest):
        for priv_level in aAssignmentRequest.mPrivLevels:
            self.debugPrint('[ThreadHandlerSet:assignSynchronousExceptionHandler] priv_level: %s, exception_class: %s, handler_class_name: %s\n' % (priv_level, aAssignmentRequest.mExcClass, aAssignmentRequest.mHandlerClassName))

            self.priv_level_handler_sets[priv_level].assignSynchronousExceptionHandler(aAssignmentRequest)

    def assignAsynchronousExceptionHandler(self, aHandlerClassName):
        for priv_level in self.getPrivilegeLevels():
            self.priv_level_handler_sets[priv_level].assignAsynchronousExceptionHandler(aHandlerClassName)

    ## return set of scratch (gpr) registers for a handler set.
    ## NOTE: call this method after handlers are generated
    def getScratchRegisterSets(self):
        raise NotImplementedError

    def getVectorBaseAddressSets(self):
        raise NotImplementedError

    def getVectorBaseAddress(self, privLevel, securityState):
        return self.vector_offset_tables[(privLevel, securityState)]

    def savePrivilegeLevel(self):
        raise NotImplementedError

    # set privilege level. may affect translation
    def setPrivilegeLevel(self, newSecurityState):
        raise NotImplementedError

    def restorePrivilegeLevel(self):
        raise NotImplementedError

    def getPrivilegeLevels(self):
        raise NotImplementedError

    def getSupportedSecurityStates(self, aPrivLevel):
        raise NotImplementedError

    def getMemoryBanks(self):
        raise NotImplementedError

    def getVectorTableSize(self):
        raise NotImplementedError

    def getVectorOffsetIncrement(self):
        raise NotImplementedError

    def isSynchronousVectorEntry(self, aVectorOffset):
        raise NotImplementedError

    def getVectorEntryErrorCode(self):
        raise NotImplementedError

    # use this method to lay down a relative branch
    def genRelativeBranchAtAddr(self, br_address, br_target_address):
        raise NotImplementedError

    def genRelativeBranch(self, br_target_address):
        raise NotImplementedError

    def fastMode(self):
        return self.default_set_name == 'Fast'

    def debugPrint(self, msg):
        self.debug('DEBUG [ThreadHandlerSet]: %s' % msg)

    # Generate a minimal number of handler sets: one set for each memory bank. Then map the handler
    # sets to each privilege level/security state combination. This is done to minimize the amount
    # of instructions and memory the exception handler sets will take.
    def _genExcepHandlerCombos(self):
        self.savePrivilegeLevel()

        # Reverse the order of privilege levels to start with the highest, so that we can do the
        # bulk of the generation with full permissions.
        for priv_level in reversed(self.getPrivilegeLevels()):
            for security_state in self.getSupportedSecurityStates(priv_level):
                self.setPrivilegeLevel(security_state)
                self._genPrivilegeLevelSecurityStateHandlerSet(priv_level, security_state)

        if self.fastMode():
            self._reserveScratchRegisters()

        self.restorePrivilegeLevel()

    def _genPrivilegeLevelSecurityStateHandlerSet(self, privLevel, securityState):
        default_mem_bank = securityState.getDefaultMemoryBank()
        priv_level_security_state = (privLevel, securityState)

        # exception vectors and handlers all in same block of memory, to allow PC-relative branches
        # to be used at each exception vector.
        vector_base_address = self.getNextVectorBaseAddress(self.handler_memory[default_mem_bank])
        self.vector_offset_tables[priv_level_security_state] = vector_base_address
        self.handler_memory[default_mem_bank] = vector_base_address + self.getVectorTableSize()

        self.debug('HANDLER MEM(%s): 0x%x' % (priv_level_security_state, self.handler_memory[default_mem_bank]))

        priv_level_handler_set = self.priv_level_handler_sets[privLevel]
        priv_level_handler_set.generate(address_table=self.address_table,
                              handler_memory=copy.deepcopy(self.handler_memory),
                              scratch_regs=self.scratch_registers,
                              default_set_name=self.default_set_name)

        priv_level_handler_set.setupScratchRegisters()
        self.scratch_registers = priv_level_handler_set.scratchRegisters()

        priv_level_handler_set.generateHandlerSubroutines(securityState)

        if self.fastMode():
            if self.user_sync_dispatcher is not None:
                sync_dispatch_addr = priv_level_handler_set.generateUserSyncDispatch(securityState, self.user_sync_dispatcher)
            else:
                sync_dispatcher = self.factory.createDefaultFastSynchronousExceptionDispatcher(self.genThread)
                sync_dispatch_addr = priv_level_handler_set.generateSynchronousHandlers(securityState, sync_dispatcher)
        else:
            sync_dispatcher = self.factory.createDefaultSynchronousExceptionDispatcher(self.genThread)
            sync_dispatch_addr = priv_level_handler_set.generateSynchronousHandlers(securityState, sync_dispatcher)

        for mem_bank in self.getMemoryBanks():
            self.handler_memory[mem_bank] = priv_level_handler_set.getNextCodeAddress(mem_bank)

        # at each exception vector offset, generate branch to either the synchronous exception
        # dispatcher, or to an asynchronous exception handler
        vector_base_addr = self.vector_offset_tables[priv_level_security_state]
        for vec_offset in range(0, self.getVectorTableSize(), self.getVectorOffsetIncrement()):  # for each exception vector offset
            branch_addr = vector_base_addr + vec_offset

            if self.isSynchronousVectorEntry(vec_offset):
                self.notice('EXCEPTION HANDLER: sync vector base 0x%x, offset 0x%x, set %s/%s' % (vector_base_addr, vec_offset, privLevel, securityState))
                self.debugPrint('%s VECTOR SYNC OFFSET 0x%x, BR ADDR: 0x%x, DISPATCH ADDR: 0x%x' %
                                (priv_level_security_state, vec_offset, branch_addr, sync_dispatch_addr))

                self.genRelativeBranchAtAddr(branch_addr, sync_dispatch_addr)
                self._recordSpecificHandlerBoundary(default_mem_bank, self.getVectorEntryErrorCode(), branch_addr, branch_addr)
            else:
                priv_level_handler_set.generateAsynchronousHandler(securityState)

                self.notice('EXCEPTION HANDLER: async vector base 0x%x, offset 0x%x, set %s/%s' % (vector_base_addr, vec_offset, privLevel, securityState))
                self.debugPrint('%s VECTOR ASYNC OFFSET 0x%x, BR ADDR: 0x%x' %
                                (priv_level_security_state, vec_offset, branch_addr))

                save_pc = self.getPEstate('PC')
                self.setPEstate('PC', branch_addr)
                priv_level_handler_set.genJumpToAsynchronousHandler(securityState)
                self.setPEstate('PC', save_pc)

                self._recordSpecificHandlerBoundary(default_mem_bank, self.getVectorEntryErrorCode(), branch_addr, branch_addr)

        # TODO(Noah): Try just collecting the handler boundaries for the memory bank corresponding
        # to memBank after getting this class working correctly. As this method is only
        # generating handlers for one memory bank, it seems like it should only be necessary to
        # collect the handler boundaries for one memory bank.
        # collect the handler boundaries from the PrivilegeLevelHandlerSets.
        for mem_bank_handler_registry in self.memBankHandlerRegistries:
            for (handler_name, handler_start_addr, handler_end_addr) in priv_level_handler_set.getHandlerBoundaries(mem_bank_handler_registry.mMemBank):
               self._recordSpecificHandlerBoundary(mem_bank_handler_registry.mMemBank, handler_name, handler_start_addr, handler_end_addr)

    # reserve registers (if required) only after all handlers, all modes, have been generated
    def _reserveScratchRegisters(self):
        for scratch_reg in self.scratch_registers:
            self.reserveRegisterByIndex(64, scratch_reg, 'GPR', 'ReadWrite')

    def _recordSpecificHandlerBoundary(self, memBank, handler_name, start_addr, end_addr):
        mem_bank_handler_registry = self.memBankHandlerRegistryRepo.getMemoryBankHandlerRegistry(memBank)
        mem_bank_handler_registry.addHandlerBoundary(handler_name, start_addr, end_addr)
