from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from DV.riscv.trees.instruction_tree import Zicsr_instructions


class MyMainSequence(Sequence):

    def generate(self, **kargs):
        
        for _ in range(1):
            
            instr = self.pickWeighted(Zicsr_instructions)
            #instr_record_id = self.genInstruction("CSRRW#register#RISCV", {"rd":0, "csr":0x140})
            instr_record_id = self.genInstruction("CSRRC#register#RISCV", {"rs1":0, "csr":0x140})
            self.notice(">>>>>  Instruction:  {}".format(instr))




MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
