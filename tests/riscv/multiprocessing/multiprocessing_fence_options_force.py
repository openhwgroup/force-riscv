from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV

from base.Sequence import Sequence


class MainSequence(Sequence):
    def generate(self, **kargs):
        self.genInstruction("FENCE##RISCV", {"NoSkip": 1})
        self.genInstruction(
            "FENCE##RISCV", {"fm": 0x8, "pred": 0xF, "succ": 0xD, "NoSkip": 1}
        )
        self.genInstruction("FENCE.I##RISCV", {"NoSkip": 1})
        self.genInstruction(
            "FENCE.I##RISCV", {"rd": 0x1, "rs": 0x2, "NoSkip": 1}
        )


## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
