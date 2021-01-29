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
from PageFaultSequence import PageFaultSequence
from base.ChoicesModifier import ChoicesModifier
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.ModifierUtils import PageFaultModifier


#  This test verifies recovery from a page fault on a branch operation.
class MainSequence(PageFaultSequence):
    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread, name)
        self._mInstrList = ("JAL##RISCV", "JALR##RISCV")
        self._mExceptionCodes = [12]
        self._mExceptionSubCodes = {}  # no sub-codes for risc

    # Create an instance of the appropriate page fault modifier.
    def createPageFaultModifier(self):
        return PageFaultModifier(
            self.genThread, self.getGlobalState("AppRegisterWidth")
        )

    # Create a list of instructions to choose from to trigger a page fault.
    def getInstructionList(self):
        return self._mInstrList

    # Return exception codes.
    def getExceptionCodes(self):
        return self._mExceptionCodes


def gen_thread_initialization(gen_thread):
    (delegate_opt, valid) = gen_thread.getOption("DelegateExceptions")
    if valid and delegate_opt == 1:
        # enable exception delegation for some portion of the generated tests.
        delegation_enables = ChoicesModifier(gen_thread)
        weightDict = {"0x0": 0, "0x1": 50}
        delegation_enables.modifyRegisterFieldValueChoices(
            "medeleg.Instruction page fault", weightDict
        )
        delegation_enables.commitSet()


GenThreadInitialization = gen_thread_initialization
MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
