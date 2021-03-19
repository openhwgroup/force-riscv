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
from base.Sequence import Sequence
from base.SequenceLibrary import SequenceLibrary


class MainSequence(Sequence):
    def generate(self, **kargs):
        if self.getGlobalState("AppRegisterWidth") == 32:
            seq_library = MySequenceLibrary32(self.genThread)
        else:
            seq_library = MySequenceLibrary(self.genThread)

        # 4 iterations of selecting a sequence randomly from the list
        for _ in range(4):
            current_sequence = seq_library.chooseOne()
            current_sequence.run()

        # In a random order, select and run with each sequence in the list
        for current_sequence in seq_library.getPermutated():
            current_sequence.run()


class MySequenceLibrary(SequenceLibrary):
    def createSequenceList(self):
        self.seqList = [
            (
                "Bunch_of_ALU_Int",
                "DV.riscv.sequences.BasicSequences",
                "Your Description",
                20,
            ),
            (
                "Bunch_of_LDST",
                "DV.riscv.sequences.BasicSequences",
                "Your Description",
                20,
            ),
        ]


class MySequenceLibrary32(SequenceLibrary):
    def createSequenceList(self):
        self.seqList = [
            (
                "Bunch_of_ALU_Int32",
                "DV.riscv.sequences.BasicSequences",
                "Your Description",
                20,
            ),
            (
                "Bunch_of_LDST32",
                "DV.riscv.sequences.BasicSequences",
                "Your Description",
                20,
            ),
        ]


#  Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

#  Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

#  Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
