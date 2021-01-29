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
from abc import ABC
from abc import abstractmethod
from base.exception_handlers.SecurityStateHandlerSet import SecurityStateHandlerSet

## This class provides an interface for creating various objects necessary for the generation of
# exception handlers.
class ExceptionsFactory(ABC):

    ## Create a set of exception handlers for a given thread.
    #
    #  @param aGenThread A reference to the GenThread object.
    #  @param aMemBankHandlerRegistryRepo A reference to the memory bank handler registry
    #       repository.
    #  @param aStack The exception handler stack.
    @abstractmethod
    def createThreadHandlerSet(self, aGenThread, aMemBankHandlerRegistryRepo, aStack):
        raise NotImplementedError

    ## Create a set of exception handlers for a given privilege level.
    #
    #  @param aGenThread A reference to the GenThread object.
    #  @param aPrivLevel The excecution privilege level.
    #  @param aMemBankHandlerRegistryRepo A reference to the memory bank handler registry
    #       repository.
    #  @param aStack The exception handler stack.
    @abstractmethod
    def createPrivilegeLevelHandlerSet(self, aGenThread, aPrivLevel, aMemBankHandlerRegistryRepo, aStack):
        raise NotImplementedError

    ## Create a set of exception handlers for a given security state.
    #
    #  @param aSecurityState The excecution security state.
    #  @param aMemBankHandlerRegistryRepo A reference to the memory bank handler registry
    #       repository.
    #  @param aStack The exception handler stack.
    def createSecurityStateHandlerSet(self, aSecurityState, aMemBankHandlerRegistryRepo, aStack):
        return SecurityStateHandlerSet(aSecurityState, aMemBankHandlerRegistryRepo, self, aStack)

    ## Create an exception handler stack.
    #
    #  @param aGenThread A reference to the GenThread object.
    @abstractmethod
    def createExceptionHandlerStack(self, aGenThread):
        raise NotImplementedError

    ## Create a default synchronous exception dispatcher.
    #
    #  @param aGenThread A reference to the GenThread object.
    @abstractmethod
    def createDefaultSynchronousExceptionDispatcher(self, aGenThread):
        raise NotImplementedError

    ## Create a default fast synchronous exception dispatcher.
    #
    #  @param aGenThread A reference to the GenThread object.
    @abstractmethod
    def createDefaultFastSynchronousExceptionDispatcher(self, aGenThread):
        raise NotImplementedError

    ## Create an assembly helper object.
    #
    #  @param aSequence A reference to the sequence being generated.
    @abstractmethod
    def createAssemblyHelper(self, aSequence):
        raise NotImplementedError
