from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from DV.riscv.trees.instruction_tree import Zicsr_instructions


class MyMainSequence(Sequence):

    def generate(self, **kargs):
        
        for _ in range(20):
            
            #instr = self.pickWeighted(Zicsr_instructions)
            #reg_value = self.random32(32,63)
            #reg_index = self.getRandomGPR()
            #reg_id = "x{}".format(reg_index)
            #self.writeRegister(reg_id, reg_value)
            #self.notice(">>>>>  Instruction:  {}".format(instr))
            #instr_record_id = self.genInstruction("CSRRW#register#RISCV", {"rd":0, "rs1":reg_index, "csr":0x340})
            instr_record_id = self.genInstruction("CSRRW#register#RISCV", {"rd":0, "csr":0x140})
            instr_record_id = self.genInstruction("CSRRC#register#RISCV", {"rs1":0, "csr":0x140})




MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
