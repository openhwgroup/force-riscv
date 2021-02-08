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
from base.exception_handlers.ExceptionHandlerAssignment import (
    ExceptionHandlerAssignment,
)


# This class represents the set of exception handlers assigned to execute in
# a given security state within a certain privilegel level for a certain
# thread.
class SecurityStateHandlerSet(object):
    def __init__(
        self,
        aSecurityState,
        aMemBankHandlerRegistryRepo,
        aFactory,
        aExceptionsStack,
    ):
        self._mSecurityState = aSecurityState
        self._mMemBankHandlerRegistryRepo = aMemBankHandlerRegistryRepo
        self._mFactory = aFactory
        self._mExceptionsStack = aExceptionsStack
        self._mHandlerAssignments = {}

    # Assign the specified handler within the specified memory bank to handle
    # the specified exception class or subexception class. If a handler is
    # already assigned to the exception class or subexception class, this
    # method does nothing. This allows user-specified exception handlers to be
    # specified first and default handlers to be inserted later only where
    # there are unhandled exception classes.
    #
    #  @param aAssignmentRequest An object containing information specifying
    #       an excpetion handler class and when it should be executed.
    def assignSynchronousExceptionHandler(self, aAssignmentRequest):
        mem_bank = aAssignmentRequest.mMemBank
        if mem_bank is None:
            mem_bank = self._mSecurityState.getDefaultMemoryBank()

        mem_bank_handler_registry = self._mMemBankHandlerRegistryRepo.getMemoryBankHandlerRegistry(
            mem_bank
        )
        mem_bank_handler_registry.registerExceptHandlerWithClassName(
            aAssignmentRequest.mHandlerModuleName,
            aAssignmentRequest.mHandlerClassName,
            self._mFactory,
            self._mExceptionsStack,
        )

        if aAssignmentRequest.mSubexcClass is not None:
            handler_assignment = self._mHandlerAssignments.get(
                aAssignmentRequest.mExcClass,
                ExceptionHandlerAssignment(mem_bank),
            )

            # Only permit adding subassignments if the top-level exception
            # class does not have a handler assigned to handle it.
            if handler_assignment.mHandlerClassName is None:
                handler_assignment.addSubassignment(
                    aAssignmentRequest.mSubexcClass,
                    aAssignmentRequest.mHandlerClassName,
                    mem_bank,
                )

            self._mHandlerAssignments[
                aAssignmentRequest.mExcClass
            ] = handler_assignment
        else:
            if aAssignmentRequest.mExcClass not in self._mHandlerAssignments:
                self._mHandlerAssignments[
                    aAssignmentRequest.mExcClass
                ] = ExceptionHandlerAssignment(
                    mem_bank, aAssignmentRequest.mHandlerClassName
                )

    # Return all exception handler assignments.
    def getSynchronousExceptionHandlerAssignments(self):
        return self._mHandlerAssignments

    # Return the exception handler assignment for the specified exception class
    #
    #  @param aExceptionClass The type of exception.
    def getSynchronousExceptionHandlerAssignment(self, aExceptionClass):
        return self._mHandlerAssignments[aExceptionClass]
