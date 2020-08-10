from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from DV.riscv.trees.instruction_tree import ALU_Int_All_instructions



class MyMainSequence(Sequence):

    def generate(self, **kargs):

        #option = self.getOption("loopCount")
        loopCount, valid = self.getOption("loopCount")
        #self.error(">>>>>  Option Retrieved:  ".format(option))

        if not valid:
            self.error(">>>>>  No 'loopCount' option was specified.  Value is {}.".format(loopCount))
        else:
            self.notice(">>>>>  Value specified for 'loopCount' option is:  {}".format(loopCount))

        for _ in range(loopCount):

            instr = self.pickWeighted(ALU_Int_All_instructions)
            self.genInstruction(instr)
            


MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
