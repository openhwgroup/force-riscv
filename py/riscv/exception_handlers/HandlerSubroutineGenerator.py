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
from base.exception_handlers.ReusableSequence import ReusableSequence
from riscv.exception_handlers.ExceptionHandlerContext import RegisterCallRole

## This class generates reusable subroutines that exception handlers can call.
class HandlerSubroutineGeneratorRISCV(ReusableSequence):

    #constant values - indexed by mode (sv32/39/48)
   
    #max levels in walk
    LEVELS = {
            32 : 2,
            39 : 4,
            48 : 4,
            }

    #pte size, in bytes
    PTE_SIZE = {
            32 : 4,
            39 : 8,
            48 : 8,
            }

    #shift pased on pte size, in bits
    PTESIZE_SHIFT = {
            32 : 2,
            39 : 3,
            48 : 3,
            }
   
    #constant values - same between SV32/39/48 walks
    PPN_MASK = 0x3ffffffffffc00 #masks lower 10 bits of descriptor to isolate ppn
    PPN_SHIFT = 10
    PTE_SHIFT = 12
    PTE_XWRV_MASK = 0xf
    PTE_PTR_VAL = 0x1
    LEVEL_MASK = 0x1ff
    LEVEL_BITS = {
            3 : (47, 39),
            2 : (38, 30),
            1 : (29, 21),
            0 : (20, 12),
            }

    #LEVEL_MASK_32 = 0x3ff
    #LEVEL_BITS_32 = { 1 : (31, 22),  0 : (21, 12) }
    def __init__(self, aGenThread, aFactory, aStack):
        super().__init__(aGenThread, aFactory, aStack)

        self._mExceptionsStack = aStack
        self._mPrivRegIndex = None
        self._mCauseRegIndex = None
        self._mPteAddrRegIndex = None
        self._mPteRegIndex = None
        self._mR1 = None
        self._mR2 = None
        self._mR3 = None
        self._mCalleeSavedRegIndices = None
        self._mWalkLevelRegIndex = None
        self._mAtpRegIndex = None
        self._mFaultAddrRegIndex = None

    ## Generate code to walk the page table and retrieve the address and value of a faulting
    # descriptor. The descriptor address is written to Argument Register 0; the descriptor value is
    # written to Argument Register 1.
    #
    #  @param kwargs Keyword arguments containing a handler context object.
    def generateTableWalk(self, **kwargs):
        try:
            handler_context = kwargs['handler_context']
        except KeyError:
            self.error('INTERNAL ERROR: one or more arguments to generateTableWalk() method missing.')

        self._assignScratchRegisterIndices(handler_context)

        self.debug('[HandlerSubroutineGenerator::generateTableWalk] entry point: 0x%x' % self.getPEstate('PC'))

        self.mAssemblyHelper.clearLabels('TableWalk')
        self.mAssemblyHelper.logDebugSymbol('generateTableWalk')

        # Before modifying registers, save their old values
        self._pushExceptionSpecificRegisters()

        self._genLoadAllContextRegisters(self._mR1)

        self.mAssemblyHelper.logDebugSymbol('Current Privilege Level: X%d' % self._mPrivRegIndex)
        self.mAssemblyHelper.logDebugSymbol('Fault Address: X%d' % self._mFaultAddrRegIndex)

        #self.mAssemblyHelper.addLabel('ATP MODE check') TODO add switching for diff modes
        self.callRoutine('TableWalkSV48')

        #save walk level value in temp register so we don't lose value on pop
        self.mAssemblyHelper.genMoveRegister(self._mR1, self._mWalkLevelRegIndex)
        self._popExceptionSpecificRegisters()

        #transfer fault level back to page fault handler via error code register
        ec_code_reg_index = handler_context.getScratchRegisterIndices(RegisterCallRole.EC_VALUE)
        self.mAssemblyHelper.genMoveRegister(ec_code_reg_index, self._mR1)

        self.mAssemblyHelper.genReturn()

    def generateTableWalkSV48(self, **kwargs):
        try:
            handler_context = kwargs['handler_context']
        except KeyError:
            self.error('INTERNAL ERROR: one or more arguments to generateTableWalk() method missing.')
        self._assignScratchRegisterIndices(handler_context)
        self._genTableWalk()


    def getPrerequisiteRoutineNames(self, aRoutineName):
        if aRoutineName == 'TableWalk':
            return ('TableWalkSV48',)
        else:
            return tuple()

    ## Initialize scratch register index member variables.
    #
    #  @param aHandlerContext The exception handler context from which register indices can be
    #       retrieved by role.
    def _assignScratchRegisterIndices(self, aHandlerContext):
        self._mPrivRegIndex = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.PRIV_LEVEL_VALUE)
        self._mCauseRegIndex = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.CAUSE_VALUE)
        (self._mPteAddrRegIndex, self._mPteRegIndex) = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.ARGUMENT, 2)
        (self._mR1, self._mR2, self._mR3) = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.TEMPORARY, 3)
        self._mCalleeSavedRegIndices = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.CALLEE_SAVED, 3)
        (self._mWalkLevelRegIndex, self._mAtpRegIndex, self._mFaultAddrRegIndex) = self._mCalleeSavedRegIndices

    ## Record the values of all callee-saved registers that are used.
    def _pushExceptionSpecificRegisters(self):
        for reg in self._mCalleeSavedRegIndices:
            self._mExceptionsStack.push(reg)

    ## Restore the values of all callee-saved registers that are used.
    def _popExceptionSpecificRegisters(self):
        for reg in reversed(self._mCalleeSavedRegIndices):
            self._mExceptionsStack.pop(reg)

    ## Extract the address size, page granule size, faulting address, page table base address and
    # fault level from the relevant registers.
    #
    #  @param aScratchRegIndex The index of a register than can be freely modified.
    def _genLoadAllContextRegisters(self, aScratchRegIndex):
        #load ATP register for mode/ppn
        self.mAssemblyHelper.logDebugSymbol('_genLoadAllContextRegisters')
        self.mAssemblyHelper.genReadSystemRegister(self._mAtpRegIndex, 'satp')

        #read faulting address based on privilege level
        self.mAssemblyHelper.genMoveImmediate(self._mR1, 3)
        self.mAssemblyHelper.genConditionalBranchToLabel(self._mR1, self._mPrivRegIndex, 6, 'NE', 'S PRIV')
        self.mAssemblyHelper.genReadSystemRegister(self._mFaultAddrRegIndex, 'mtval')
        self.mAssemblyHelper.genRelativeBranchToLabel(4, 'PPN MASK')

        self.mAssemblyHelper.addLabel('S PRIV')
        self.mAssemblyHelper.genReadSystemRegister(self._mFaultAddrRegIndex, 'stval')

        #mask and shift root PPN into address from atp register
        self.mAssemblyHelper.addLabel('PPN MASK')
        #self.mAssemblyHelper.genAndImmediate(self._mAtpRegIndex, 0xfffffffffff)
        self.mAssemblyHelper.genShiftLeftImmediate(self._mAtpRegIndex, self.PTE_SHIFT)

        #set up register to count levels walked
        self.mAssemblyHelper.genMoveImmediate(self._mWalkLevelRegIndex, self.LEVELS[48]-1)

    def _genTableWalk(self):
        self.mAssemblyHelper.logDebugSymbol('Gen Table Walk Start')
        self.debug('[_genTableWalk] PC before generating walk levels 0x%x' % self.getPEstate('PC'))
        start_level = self.LEVELS[48] - 1
        self.debug('[_genTableWalk] start level 0x%x' % start_level)

        for cur_level in range(start_level, -1, -1):
            self.debug('[_genTableWalk] Generating level %d' % cur_level)
            self._genLevelWalk(cur_level)

    #_mAtpRegIndex should hold the address of the next table either from ATP/PTE lookup
    #_R3 used to compute new pte addr
    #_R2 used to hold new pte val, check for PTE pointer
    #_R1 used to hold immediates for comparison, and to load new value into mAtpRegIndex

    def _genLevelWalk(self, aCurLevel):
        #self._mWalkLevelLabels['LEVEL %d' % aCurLevel] = self.getPEstate('PC')
        (top_bit, bottom_bit) = self.LEVEL_BITS[aCurLevel]
        
        #mask vpn from faulting address put in R3
        self.mAssemblyHelper.genShiftRightImmediate(self._mR3, bottom_bit, self._mFaultAddrRegIndex)
        self.mAssemblyHelper.genAndImmediate(self._mR3, self.LEVEL_MASK)
        self.mAssemblyHelper.genShiftLeftImmediate(self._mR3, self.PTESIZE_SHIFT[48])

        #mask add page offset to base
        self.mAssemblyHelper.genAddRegister(self._mR3, self._mAtpRegIndex)
        self.mAssemblyHelper.genMoveRegister(self._mPteAddrRegIndex, self._mR3)

        #load pte from memory
        self.mAssemblyHelper.genLoadMemory(self._mR2, self._mR3, 0)
        self.mAssemblyHelper.genMoveRegister(self._mPteRegIndex, self._mR2)

        #check pte pointer
        self.mAssemblyHelper.genMoveImmediate(self._mR1, self.PTE_PTR_VAL)
        self.mAssemblyHelper.genAndImmediate(self._mR2, self.PTE_XWRV_MASK)
        self.mAssemblyHelper.genConditionalBranchToLabel(self._mR2, self._mR1, 4, 'EQ', 'LAST LEVEL CHECK %d' % aCurLevel) 
        
        #if PTE is a leaf node, we can return.
        self.mAssemblyHelper.genReturn()

        #otherwise, setup next level walk addr from descriptor ppn
        self.mAssemblyHelper.addLabel('LAST LEVEL CHECK %d' % aCurLevel)
        self.mAssemblyHelper.genConditionalBranchToLabel(self._mWalkLevelRegIndex, 0, 4, 'NE', 'NEXT LEVEL WALK %d' % aCurLevel)
        #if this is a level 0 descriptor, return
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel('NEXT LEVEL WALK %d' % aCurLevel)
        self.mAssemblyHelper.genAddImmediate(self._mWalkLevelRegIndex, 0xfff)

        self.mAssemblyHelper.genMoveRegister(self._mR1, self._mPteRegIndex)
        #self.mAssemblyHelper.genAndImmediate(self._mR1, self.PPN_MASK)
        self.mAssemblyHelper.genShiftRightImmediate(self._mR1, self.PPN_SHIFT)
        self.mAssemblyHelper.genShiftLeftImmediate(self._mAtpRegIndex, self.PTE_SHIFT, self._mR1)
