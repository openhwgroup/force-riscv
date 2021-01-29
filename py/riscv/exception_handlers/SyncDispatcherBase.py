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

#----------------------------------------------------------------------------
# dispatch the sync exception handlers base class  -- used to generate the dispatch code
#----------------------------------------------------------------------------
class SyncDispatcherBaseRISCV(Sequence):
    def __init__(self, gen_thread, factory):
        super().__init__(gen_thread)

        self.mAssemblyHelper = factory.createAssemblyHelper(self)
        self.exceptionsStack = None
        self.privilegeLevel = None
        self.memBank = None
        self.handlerAddress = None
        self.handlerEndAddress = None

    #------------------------------------------------------------------------
    # keep track of the start handler addresses
    #------------------------------------------------------------------------
    def recordStartAddress(self):
        start_addr = self.getPEstate("PC")
        self.handlerAddress = start_addr
        self.debug("[%s] dispatch (%s/%s) starting address: 0x%x" %
                   (self.__class__.__name__, self.privilegeLevel, self.memBank, self.handlerAddress) )
        return start_addr

    #------------------------------------------------------------------------
    # keep track of the end handler addresses
    #------------------------------------------------------------------------
    def recordEndAddress(self):
        end_addr = self.getPEstate("PC")
        self.handlerEndAddress = end_addr
        self.debug("[%s] dispatch (%s/%s) end address: 0x%x" % 
                   (self.__class__.__name__, self.privilegeLevel, self.memBank, self.handlerEndAddress) )

        return end_addr

    #------------------------------------------------------------------------
    # return extents (address range(s) this code spans)
    #------------------------------------------------------------------------
    def extents(self, mem_bank):
        return [ (self.handlerAddress, self.handlerEndAddress) ]

    def generatePreDispatch(self, aHandlerContext):
        self.exceptionsStack = aHandlerContext.mStack
        self.privilegeLevel = aHandlerContext.mPrivLevel
        self.memBank = aHandlerContext.mMemBank

        start_addr = self.recordStartAddress()

        self.processPreDispatch(aHandlerContext)

    def generateDispatch(self, aHandlerContext, aExceptionCodeClass):
        # call method that is to generate the actual code to dispatch sync exception
        self.processDispatch(aHandlerContext, aExceptionCodeClass)

    def generatePostDispatch(self, aHandlerContext):
        self.processPostDispatch(aHandlerContext)

        self.recordEndAddress()

    ## Generate the necessary instructions to execute prior to dispatching to exception handlers.
    # These instructions should be generated once at the top-level dispatch.
    #
    #  @param aHandlerContext A source for usable registers and other information useful in
    #       generating exception handler instructions.
    def processPreDispatch(self, aHandlerContext):
        pass

    ## Generate instructions to dispatch to exception handlers based on the specified exception code
    # class. These instructions may be generated repeatedly: once for the top-level dispatch and
    # potentially multiple times at finer levels of dispatch granularity to handle various
    # subexception codes.
    #
    #  @param aHandlerContext A source for usable registers and other information useful in
    #       generating exception handler instructions.
    #  @param aExceptionCodeClass The class defining the possible exception codes for this level of
    #       dispatch granularity.
    def processDispatch(self, aHandlerContext, aExceptionCodeClass):
        raise NotImplementedError

    ## Generate the necessary instructions to execute after returning from an exception handler.
    # These instructions should be generated once at the top-level dispatch.
    #
    #  @param aHandlerContext A source for usable registers and other information useful in
    #       generating exception handler instructions.
    def processPostDispatch(self, aHandlerContext):
        pass
