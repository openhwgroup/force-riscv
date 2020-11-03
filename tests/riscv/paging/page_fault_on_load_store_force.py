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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from PageFaultSequence import PageFaultSequence
from PageFaultSequence import PageFaultResolutionType
from base.ChoicesModifier import ChoicesModifier
from riscv.ModifierUtils import PageFaultModifier

class MainSequence(PageFaultSequence):

    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread,name)

        # just a couple of load/stores...
        
        self._mInstrList = (
            'LD##RISCV',
            'SD##RISCV',
            )
        if self.getGlobalState('AppRegisterWidth') == 32:
            self._mInstrList = (
                'LW##RISCV',
                'SW##RISCV',
            )
            
        self._mExceptionCodes = ( 13, 15 ) # for risc-v, load: 13, store: 15
        self._mExceptionSubCodes = { } # for now, riscv does not support exception sub-codes
        
    def createPageFaultModifier(self):
            return PageFaultModifier(self.genThread, self.getGlobalState('AppRegisterWidth'))

    def getInstructionList(self):
            return self._mInstrList

    def getExceptionCodes(self):
            return self._mExceptionCodes

def gen_thread_initialization(gen_thread):
    (delegate_opt, valid) = gen_thread.getOption("DelegateExceptions")
    if valid and delegate_opt == 1:
        # enable exception delegation for some portion of the generated tests...
        delegation_enables = ChoicesModifier(gen_thread)
        weightDict = { "0x0":0, "0x1":50 }
        delegation_enables.modifyRegisterFieldValueChoices( 'medeleg.Load page fault', weightDict )
        delegation_enables.modifyRegisterFieldValueChoices( 'medeleg.Store/AMO page fault', weightDict )
        delegation_enables.commitSet()
    
GenThreadInitialization = gen_thread_initialization

MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

