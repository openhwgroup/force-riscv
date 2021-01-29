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
from DV.riscv.trees.instruction_tree import ALU_Int32_instructions
from DV.riscv.trees.instruction_tree import ALU_Int64_instructions


class MyMainSequence(Sequence):

    def generate(self, **kargs):

        # Use the Sequence.choicePermutated to randomly pick an item from a list, 
        # tuple or dictionary.
        for k, v in self.choicePermutated(ALU_Int32_instructions):
            self.notice(">>>>>  Instr: {:15}  Weight: {:4}".format(k,v))



MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

