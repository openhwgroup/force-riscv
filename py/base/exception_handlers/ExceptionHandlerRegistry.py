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
import Log


#  This class creates and retains instances of various exception handler
#  classes. This allows the same handler instance to be retrieved repeatedly,
# so that we can reuse its generated instructions.
class ExceptionHandlerRegistry(object):
    def __init__(self, aGenThread):
        self._mGenThread = aGenThread
        self._mExceptionHandlers = {}

    # Retrieve an exception handler by name.
    #
    #  @param aExceptionHandlerClassName The exception handler class name.
    def getExceptionHandler(self, aExceptionHandlerClassName):
        return self._mExceptionHandlers[aExceptionHandlerClassName]

    # Search for the specified exception handler by name. If an instance
    # exists, return it; otherwise, create a new instance and store it for
    # future retrieval. It is assumed that no two exception handler classes
    # will have the same name, even if they have different module names.
    #
    #  @param aExceptionHandlerModuleName The Python module that contains
    #       the specified exception handler class.
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
        if aExceptionHandlerClassName in self._mExceptionHandlers:
            exception_handler = self._mExceptionHandlers[
                aExceptionHandlerClassName
            ]
        else:
            exception_handler = self.createExceptionHandler(
                aExceptionHandlerModuleName,
                aExceptionHandlerClassName,
                aFactory,
                aStack,
            )

        return exception_handler

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
    def createExceptionHandler(
        self,
        aExceptionHandlerModuleName,
        aExceptionHandlerClassName,
        aFactory,
        aStack,
    ):
        try:

            exec_locals = {
                "self": self,
                "aFactory": aFactory,
                "aStack": aStack,
            }
            exec(
                "from %s import %s; exception_handler = %s(self._mGenThread, "
                "aFactory, aStack)"
                % (
                    aExceptionHandlerModuleName,
                    aExceptionHandlerClassName,
                    aExceptionHandlerClassName,
                ),
                globals(),
                exec_locals,
            )
            exception_handler = exec_locals["exception_handler"]

        except TypeError as eTypeError:

            import traceback

            tb_str = "".join(
                traceback.format_exception(
                    None, eTypeError, eTypeError.__traceback__
                )
            )
            Log.error(
                "[createExceptionHandler] failed to create handler from "
                "module: %s, class name: %s. %s"
                % (
                    aExceptionHandlerModuleName,
                    aExceptionHandlerClassName,
                    tb_str,
                )
            )

        self._mExceptionHandlers[
            aExceptionHandlerClassName
        ] = exception_handler
        return exception_handler

    # Store an exception handler instance for future retrieval.
    #
    #  @param aExceptionHandlerClassName The exception handler class name.
    #  @param aExceptionHandler The exception handler instance.
    def addExceptionHandler(
        self, aExceptionHandlerClassName, aExceptionHandler
    ):
        self._mExceptionHandlers[
            aExceptionHandlerClassName
        ] = aExceptionHandler
