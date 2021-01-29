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
import operator

from base.exception_handlers.ExceptionHandlerRegistry import (
    ExceptionHandlerRegistry,
)


#  This class holds handler sets for each memory bank. It is intended for
#  there to be only one instance of this class.
class MemoryBankHandlerRegistryRepository(object):
    def __init__(self):
        self._mMemBankHandlerRegistries = {}

    # Add a memory bank handler registry for the specified memory bank.
    #
    #  @param aMemBank A memory bank.
    def addMemoryBankHandlerRegistry(self, aMemBankHandlerRegistry):
        self._mMemBankHandlerRegistries[
            aMemBankHandlerRegistry.mMemBank
        ] = aMemBankHandlerRegistry

    # Return the memory bank handler registry for the specified memory bank.
    #
    #  @param aMemBank A memory bank.
    def getMemoryBankHandlerRegistry(self, aMemBank):
        return self._mMemBankHandlerRegistries[aMemBank]

    # Return a list of all memory bank handler registries.
    def getMemoryBankHandlerRegistries(self):
        return self._mMemBankHandlerRegistries.values()


#  This class represents a collection of the available handlers in a
#  particular memory bank.
class MemoryBankHandlerRegistry(object):
    def __init__(self, aGenThread, aMemBank):
        self.mMemBank = aMemBank
        self.mStartAddr = None
        self.mHandlerSubroutineGenerator = None
        self._mHandlerBoundaries = []
        self._mHandlerRegistry = ExceptionHandlerRegistry(aGenThread)

    # Retrieve an exception handler by name.
    #
    #  @param aExceptionHandlerClassName The exception handler class name.
    def getExceptionHandler(self, aExceptionHandlerClassName):
        return self._mHandlerRegistry.getExceptionHandler(
            aExceptionHandlerClassName
        )

    # Search for the specified exception handler by name. If an instance
    # exists, return it; otherwise, create a new instance and store it for
    # future retrieval. It is assumed that no two exception handler classes
    # will have the same name, even if they have different module names.
    #
    #  @param aExceptionHandlerModuleName The Python module that contains the
    #       specified exception handler class.
    #  @param aExceptionHandlerClassName The exception handler class name.
    #  @param aFactory A factory that provides methods for creating various
    #       objects necessary for the generation of exception handlers.
    #  @param aStack The exception handler stack.
    def requestExceptionHandler(
        self,
        aExceptionHandlerModuleName,
        aExceptionHandlerClassName,
        aFactory,
        aStack,
    ):
        return self._mHandlerRegistry.requestExceptionHandler(
            aExceptionHandlerModuleName,
            aExceptionHandlerClassName,
            aFactory,
            aStack,
        )

    # Store an exception handler instance for future retrieval.
    #
    #  @param aExceptionHandlerClassName The exception handler class name.
    #  @param aExceptionHandler The exception handler instance.
    def registerExceptionHandlerWithInstance(
        self, aExceptionHandlerClassName, aExceptionHandler
    ):
        self._mHandlerRegistry.addExceptionHandler(
            aExceptionHandlerClassName, aExceptionHandler
        )

    # Create an instance of the specified exception handler class and store
    # it for future retrieval. It is assumed that no two exception handler
    # classes will have the same name, even if they have different module
    # names.
    #
    #  @param aExceptionHandlerModuleName The Python module that contains
    #       the specified exception handler class.
    #  @param aExceptionHandlerClassName The exception handler class name.
    #  @param aFactory A factory that provides methods for creating various
    #       objects necessary for the generation of exception handlers.
    #  @param aStack The exception handler stack.
    def registerExceptHandlerWithClassName(
        self,
        aExceptionHandlerModuleName,
        aExceptionHandlerClassName,
        aFactory,
        aStack,
    ):
        self._mHandlerRegistry.createExceptionHandler(
            aExceptionHandlerModuleName,
            aExceptionHandlerClassName,
            aFactory,
            aStack,
        )

    # Return the address boundaries of exception handlers sorted by their
    # starting addresses in ascending order.
    def getHandlerBoundaries(self):
        self._mHandlerBoundaries.sort(key=operator.itemgetter(1))
        return self._mHandlerBoundaries

    # Add the address boundaries of an exception handler.
    #
    #  @param aHandlerName Name of the exception handler.
    #  @param aStartAddr Starting address of the exception handler.
    #  @param aEndAddr Ending address of the exception handler.
    def addHandlerBoundary(self, aHandlerName, aStartAddr, aEndAddr):
        self._mHandlerBoundaries.append((aHandlerName, aStartAddr, aEndAddr))
