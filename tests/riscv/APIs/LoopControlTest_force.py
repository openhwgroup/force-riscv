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
from base.Sequence import Sequence
from DV.riscv.trees.instruction_tree import ALU_Int_All_instructions
from DV.riscv.trees.instruction_tree import ALU_Int32_All_instructions

from riscv.Utils import LoopControl

class MainSequence(Sequence):

  # Utility function to create a new loop that generates instrNum instructions from instrList
  def genInstructionLoop(self, loopGpr, loopCount, instrNum, instrList):
    loop_ctrl_seq = LoopControl(self.genThread)
    loop_ctrl_seq.start(LoopReg=loopGpr, LoopCount=loopCount)

    for i in range(instrNum):
      random_instr = self.pickWeighted(instrList)
      self.genInstruction(random_instr)

    loop_ctrl_seq.end()

  def generate(self, **kargs):
    # Choose registers to serve as loop counters for test sequence
    (loop_gpr, inner_loop_gpr) = self.getRandomRegisters(2, "GPR", "0")

    # Any generated instructions after start() and before end() will be looped through twice
    loop_ctrl_seq = LoopControl(self.genThread)
    loop_ctrl_seq.start(LoopReg=loop_gpr, LoopCount=2)

    # Use genInstructionLoop sequence to create varying nested inner loops
    instr_tree = ALU_Int32_All_instructions if self.getGlobalState('AppRegisterWidth') == 32 else ALU_Int_All_instructions
    self.genInstructionLoop(inner_loop_gpr, 3, 10, instr_tree)
    self.genInstructionLoop(inner_loop_gpr, 5, 20, instr_tree)

    loop_ctrl_seq.end()

    
## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

