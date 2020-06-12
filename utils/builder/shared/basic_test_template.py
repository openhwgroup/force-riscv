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
basic_template_str = """from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.ModifierUtils import PageMemoryAttributeModifier
from base.Sequence import Sequence

class MainSequence(Sequence):

    def generate(self, **kargs):
        for instr in [%s]:
            self.genInstruction(instr, {"NoSkip":1})

def gen_thread_initialization(gen_thread):
    gen_thread.applyChoiceModifier(PageMemoryAttributeModifier)

## Points to the generator thread initialization function defined in this file, optional
GenThreadInitialization = gen_thread_initialization

## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
"""

basic_non_standard_template_str = """from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.ModifierUtils import PageMemoryAttributeModifier
from base.Sequence import Sequence

class MainSequence(Sequence):

    def generate(self, **kargs):
        for instr in [%s]:
            if (self.isRegisterReserved("X17", "Write") or self.isRegisterReserved("X16", "Read")):
              self.genInstruction(instr, {"NoSkip":0})
            else:
              self.genInstruction(instr, {"NoSkip":1})

def gen_thread_initialization(gen_thread):
    gen_thread.applyChoiceModifier(PageMemoryAttributeModifier)

## Points to the generator thread initialization function defined in this file, optional
GenThreadInitialization = gen_thread_initialization

## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
"""
