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
from DV.riscv.trees.instruction_tree import ALU_Int32_instructions
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


class MyMainSequence(Sequence):
    def generate(self, **kargs):

        # Use the Sequence.sample method which randomly selects N members of
        # the list where N is specified in the second argument.  The first
        # argument must be a list.  Sequence.sample does not support using a
        # dictionary as the input build a list that corresponds to the key
        # words in the dictionary
        instr_list = []
        for instr, weight in ALU_Int32_instructions.items():
            instr_list.append(instr)

        sublist = self.sample(instr_list, 5)

        for instr in sublist:
            self.notice(">>>>>  Instruction: {}".format(instr))

        # Use the Sequence.choice method with a dictionary input which returns
        # one dictionary item as a (key, value) tuple
        instr = self.choice(ALU_Int32_instructions)
        self.notice(">>>>>  Instruction from self.choice:  {}".format(instr[0]))
        self.genInstruction(instr[0])

        # Use Sequence.choice with a tuple as the input
        small_tuple = (sublist[0], sublist[1], sublist[2])
        instr = self.choice(small_tuple)
        self.notice(">>>>>  Instruction from self.choice using tuple: {}".format(instr))
        self.genInstruction(instr)


MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
