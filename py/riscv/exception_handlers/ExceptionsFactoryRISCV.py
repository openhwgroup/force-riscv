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
from base.exception_handlers.ExceptionsFactory import ExceptionsFactory
from riscv.AssemblyHelperRISCV import AssemblyHelperRISCV
from riscv.exception_handlers.ThreadHandlerSetRISCV import ThreadHandlerSetRISCV
from riscv.exception_handlers.PrivilegeLevelHandlerSetRISCV import PrivilegeLevelHandlerSetRISCV
from riscv.exception_handlers.ExceptionHandlerStack import ExceptionHandlerStackRISCV
from riscv.exception_handlers.SyncDispatcher import DefaultSyncDispatcher
from riscv.exception_handlers.FastSyncDispatcher import DefaultFastSyncDispatcher

## This class provides an interface for creating various objects necessary for the generation of
# exception handlers for the RISCV architecture.
class ExceptionsFactoryRISCV(ExceptionsFactory):

    ## Create a set of exception handlers for a given thread.
    #
    #  @param aGenThread A reference to the GenThread object.
    #  @param aMemBankHandlerRegistryRepo A reference to the memory bank handler registry
    #       repository.
    #  @param aStack The exception handler stack.
    def createThreadHandlerSet(self, aGenThread, aMemBankHandlerRegistryRepo, aStack):
        return ThreadHandlerSetRISCV(aGenThread, aMemBankHandlerRegistryRepo, self, aStack)

    ## Create a set of exception handlers for a given privilege level.
    #
    #  @param aGenThread A reference to the GenThread object.
    #  @param aPrivLevel The excecution privilege level.
    #  @param aMemBankHandlerRegistryRepo A reference to the memory bank handler registry
    #       repository.
    #  @param aStack The exception handler stack.
    def createPrivilegeLevelHandlerSet(self, aGenThread, aPrivLevel, aMemBankHandlerRegistryRepo, aStack):
        return PrivilegeLevelHandlerSetRISCV(aGenThread, aPrivLevel, aMemBankHandlerRegistryRepo, self, aStack)

    ## Create an exception handler stack.
    #
    #  @param aGenThread A reference to the GenThread object.
    def createExceptionHandlerStack(self, aGenThread):
        return ExceptionHandlerStackRISCV(aGenThread)

    ## Create a default synchronous exception dispatcher.
    #
    #  @param aGenThread A reference to the GenThread object.
    def createDefaultSynchronousExceptionDispatcher(self, aGenThread):
        return DefaultSyncDispatcher(aGenThread, self)

    ## Create a default fast synchronous exception dispatcher.
    #
    #  @param aGenThread A reference to the GenThread object.
    def createDefaultFastSynchronousExceptionDispatcher(self, aGenThread):
        return DefaultFastSyncDispatcher(aGenThread, self)

    ## Create an assembly helper object.
    #
    #  @param aSequence A reference to the sequence being generated.
    def createAssemblyHelper(self, aSequence):
        return AssemblyHelperRISCV(aSequence)
