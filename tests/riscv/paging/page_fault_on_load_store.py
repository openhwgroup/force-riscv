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
from riscv.ModifierUtils import PageFaultModifier

class MainSequence(PageFaultSequence):

    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread,name)

        # just a couple of load/stores...
        
        self._mInstrList = (
            'LD##RISCV',
            'SD##RISCV',
            )

        self._mExceptionCodes = ( 13, 15 ) # for risc-v, load: 13, store: 15
        self._mExceptionSubCodes = { } # for now, riscv does not support exception sub-codes
        
        def createPageFaultModifier(self):
            return PageFaultModifier(self.genThread)

        def getInstructionList(self):
            return self.mInstrList

        def getExceptionCodes(self):
            return self._mExceptionCodes
        
        def getPageFaultResolutionType(self,aFaultLevels, aFastHandlers):
            resolution_type = PageFaultResolutionType.SKIP_INSTRUCTION
            if any([x in aFaultLevels for x in (0, 1, 2, 3)]) and (not aFastHandlers):
                resolution_type = PageFaultResolutionType.RE_EXECUTE_INSTRUCTION

            return resolution_type

        
MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

