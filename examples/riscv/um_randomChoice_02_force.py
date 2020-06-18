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

