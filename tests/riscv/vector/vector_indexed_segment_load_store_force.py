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
from VectorTestSequence import VectorLoadStoreTestSequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
import re


#  This test verifies that strided load and store instructions can be
#  generated and executed successfully.
class MainSequence(VectorLoadStoreTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        instr_base_list = (
            "VLOXSEG%dEI%d.V##RISCV",
            "VLUXSEG%dEI%d.V##RISCV",
            "VSOXSEG%dEI%d.V##RISCV",
            "VSUXSEG%dEI%d.V##RISCV",
        )

        self._mInstrList = []
        for instr_base in instr_base_list:
            for field_count in range(2, 9):
                for eew in (8, 16, 32, 64):
                    self._mInstrList.append(instr_base % (field_count, eew))

    # Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    # Calculate the largest register count for the given instruction's operands.
    #
    #  @param aInstr The name of the instruction.
    def _calculateMaxRegisterCount(self, aInstr):
        match = re.fullmatch(r"V(L|S)(O|U)XSEG(\d)EI(\d+)\.V\#\#RISCV", aInstr)
        field_count = int(match.group(3))
        dest_reg_count = self.getRegisterCount(field_count)

        index_reg_count = self._calculateEmul(aInstr)

        return max(dest_reg_count, index_reg_count)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
