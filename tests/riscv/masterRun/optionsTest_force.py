from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from DV.riscv.trees.instruction_tree import ALU_Int_All_instructions
from DV.riscv.trees.instruction_tree import ALU_Int32_All_instructions

class MyMainSequence(Sequence):

    def generate(self, **kargs):
        loopCount, valid = self.getOption("loopCount")

        if not valid:
            self.error(">>>>>  No 'loopCount' option was specified.  Value is {}.".format(loopCount))
        else:
            self.notice(">>>>>  Value specified for 'loopCount' option is:  {}".format(loopCount))

        instrs = ALU_Int32_All_instructions if self.getGlobalState("AppRegisterWidth") == 32 else ALU_Int_All_instructions

        for _ in range(loopCount):
            instr = self.pickWeighted(instrs)
            self.genInstruction(instr)
            

MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
