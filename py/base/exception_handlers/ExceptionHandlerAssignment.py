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


# This class encapsulates the information required to assign an exception
# handler class to handle a particular exception in a specified operating
# state.
class ExceptionHandlerAssignmentRequest(object):
    def __init__(
        self,
        aExcClass,
        aPrivLevels,
        aSecurityStates,
        aHandlerModuleName,
        aHandlerClassName,
        aMemBank=None,
        aSubexcClass=None,
    ):
        self.mExcClass = aExcClass
        self.mPrivLevels = aPrivLevels
        self.mSecurityStates = aSecurityStates
        self.mHandlerModuleName = aHandlerModuleName
        self.mHandlerClassName = aHandlerClassName
        self.mMemBank = aMemBank
        self.mSubexcClass = aSubexcClass


#  This class tracks assignment of an exception handler to handle a
#  particular exception. It provides for a hierarchical structure to allow for
# more granular assignments within a particular exception based on defined
# subexception classes.
class ExceptionHandlerAssignment(object):
    def __init__(self, aMemBank, aHandlerClassName=None):
        self.mMemBank = aMemBank
        self.mHandlerClassName = aHandlerClassName
        self._mSubassignments = {}

    # Add a more granular assignment to handle a particular subexception class.
    #
    # @param aSubexcClass The subexception class that the specified handler
    #       should handle. @param aHandlerClassName The name of the class
    #       containing the code to handle the exception.
    # @param aMemBank The memory bank in which the handler should be
    #       generated.
    def addSubassignment(self, aSubexcClass, aHandlerClassName, aMemBank):
        if aSubexcClass not in self._mSubassignments:
            self._mSubassignments[aSubexcClass] = ExceptionHandlerAssignment(
                aMemBank, aHandlerClassName
            )

    # Return handler assignments for subexception classes if any.
    def getSubassignments(self):
        return self._mSubassignments

    # Return whether there are handler assignments for subexception classes.
    def hasSubassignments(self):
        if self._mSubassignments:
            return True
        else:
            return False

    # Return the exception code class used for the subassignments. It is
    # assumed that only one exception code class is used, as the assignments
    # are ambiguous otherwise.
    def getSubexceptionCodeClass(self):
        # We only need to retrieve one key if there are any subassignments.
        # I don't know of any simple way to retrieve an arbitrary item from a
        # dictionary, so a loop is used with an unconditional break on the
        # first iteration.
        subexception_code_class = None
        for subexception_code in self._mSubassignments:
            subexception_code_class = subexception_code.__class__
            break

        return subexception_code_class
