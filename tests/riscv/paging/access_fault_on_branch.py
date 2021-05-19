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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from PageFaultSequence import PageFaultSequence
from PageFaultSequence import PageFaultResolutionType
from riscv.ModifierUtils import AccessFaultModifier


# This test verifies recovery from a Access Exception on a branch operation.
class MainSequence(PageFaultSequence):
    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread, name)
        self._mInstrList = ("JAL##RISCV", "JALR##RISCV")
        self._mExceptionCodes = 1
        self._mExceptionSubCodes = {}  # no sub-codes for risc

    # Create an instance of the appropriate page fault modifier.
    def createPageFaultModifier(self):
        return AccessFaultModifier(self.genThread)

    # Create a list of instructions to choose from to trigger an
    # access exception..
    def getInstructionList(self):
        return self._mInstrList

    # Return exception sub-codes.
    def getExceptionSubCodes(self):
        return self._mExceptionSubCodes

    # Return the expected resolution type for the page fault.
    #
    #  @param aFaultLevels  - the translation level at which the page
    #                           fault was triggered.
    #  @param aFastHandlers - indicates whether or not the fast handlers
    #                           are enabled.
    def getPageFaultResolutionType(self, aFaultLevels, aFastHandlers):
        resolution_type = PageFaultResolutionType.SKIP_INSTRUCTION
        if any([x in aFaultLevels for x in (0, 1, 2, 3)]) and (not FastHandlers):
            resolution_type = PageFaultResolutionType.RE_EXECUTE_INSTRUCTION
        return resolution_type


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
