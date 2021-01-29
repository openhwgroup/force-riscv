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

from riscv.PrivilegeLevel import PrivilegeLevelRISCV
from base.exception_handlers.ReusableSequence import ReusableSequence
from riscv.exception_handlers.ExceptionHandlerContext import RegisterCallRole

# Base class for Force 'Fast' mode synchronous exception handlers
class FastExceptionHandlersBaseRISCV(ReusableSequence):

    def __init__(self, aGenThread, aFactory, aStack):
        super().__init__(aGenThread, aFactory, aStack)

        self.mErrCode = None
        self.mScratchRegisters = None
        self.mPrivLevel = None
        self.mAddrTable = None
        self.use_addr_table = False

    def generateHandlerS(self, **kwargs):
        self.genHandler(**kwargs)

    def generateHandlerM(self, **kwargs):
        self.genHandler(**kwargs)

    def generateHandler(self, **kwargs):
        self.genHandler(**kwargs)

    # main entry point - generates exception handler. The handler must be able to generate/handle
    # exception at any privilege mode
    def genHandler(self, **kwargs):
        self.debug('[FastExceptionHandlersBaseRISCV] genHandler')

        print('[FastExceptionHandlersBaseRISCV] genHandler')

        try:
            handler_context = kwargs['handler_context']
        except KeyError:
            self.error('INTERNAL ERROR: one or more arguments to fast mode exception handler genHandler method missing.')

        self.mErrCode = handler_context.mErrCode
        self.mScratchRegisters = handler_context.getScratchRegisterIndices(RegisterCallRole.ALL, 2)
        self.mPrivLevel = handler_context.mPrivLevel
        self.mAddrTable = handler_context.mAddrTable

        if self.mPrivLevel not in ('S', 'M'):
            self.error('INTERNAL ERROR: unknown privilege level: "%s%.' % self.mPrivLevel)

        # call method to generate the code to process the exception
        self.processException()

        # return from exception
        self.notice("BLAH" +self.mPrivLevel  )
        priv_level = PrivilegeLevelRISCV[self.mPrivLevel]

        self.mAssemblyHelper.genExceptionReturn(priv_level)


    def useAddressTable(self):
        self.use_addr_table = True
        info_set = {}
        info_set['Function'] = 'AddrTableEC'
        info_set['EC'] = self.err_code
        self.exceptionRequest('UpdateHandlerInfo', info_set)

    # generate the code to process the exception
    def processException(self):
        NotImplementedError
