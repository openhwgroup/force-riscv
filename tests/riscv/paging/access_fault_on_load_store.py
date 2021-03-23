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
from PageFaultSequence import PageFaultResolutionType
from PageFaultSequence import PageFaultSequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.ModifierUtils import AccessFaultModifier


#  This test verifies recover from a Access Exception on a load or store.
class MainSequence(PageFaultSequence):
    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread, name)

        # just a couple of load/stores...

        self._mInstrList = ["LD##RISCV", "SD##RISCV"]
        if self.getGlobalState("AppRegisterWidth") == 32:
            self._mInstrList = ["LW##RISCV", "SW##RISCV"]

        # for risc-v, access exception - load: 5, store: 7
        self._mExceptionCodes = (
            5,
            7,
        )

        # for now, riscv does not support exception sub-codes
        self._mExceptionSubCodes = {}

    def createPageFaultModifier(self):
        return AccessFaultModifier(self.genThread)

    def getInstructionList(self):
        return self.mInstrList

    def getExceptionCodes(self):
        return self._mExceptionCodes

    def getPageFaultResolutionType(self, aFaultLevels, aFastHandlers):
        resolution_type = PageFaultResolutionType.SKIP_INSTRUCTION
        if any([x in aFaultLevels for x in (0, 1, 2, 3)]) and (
            not aFastHandlers
        ):
            resolution_type = PageFaultResolutionType.RE_EXECUTE_INSTRUCTION

        return resolution_type


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
