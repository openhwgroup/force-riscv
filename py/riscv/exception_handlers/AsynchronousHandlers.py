#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# classes, code related to pc/stack alignment aborts...
from base.exception_handlers.ReusableSequence import ReusableSequence

#-------------------------------------------------------------------------------------------------------
# AsynchronousHandlerRISCV - service asynchronous exceptions, ie, interrupts...
#-------------------------------------------------------------------------------------------------------

class AsynchronousHandlerRISCV(ReusableSequence):

    def __init__(self, aGenThread, aFactory, aStack):
        super().__init__(aGenThread, aFactory, aStack)

    def generateHandler(self, **kwargs):
        pass # TODO
