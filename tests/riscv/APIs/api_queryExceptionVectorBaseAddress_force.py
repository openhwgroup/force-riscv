from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from base.ChoicesModifier import ChoicesModifier
from base.SequenceLibrary import SequenceLibrary

class MainSequence(Sequence):

    def generate(self, **kargs):

        mvector_base_address = self.queryExceptionVectorBaseAddress("DefaultMachineModeVector")
        svector_base_address = self.queryExceptionVectorBaseAddress("DefaultSupervisorModeVector")
        self.notice(">>>>>>>>>>  M Vector Base Address:  {:12x}".format(mvector_base_address))
        self.notice(">>>>>>>>>>  S Vector Base Address:  {:12x}".format(svector_base_address))





## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
