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
base_template_str = """from {1}.Env{2} import Env{2}
from {1}.GenThread{2} import GenThread{2}
{3}
from base.Sequence import Sequence

class MainSequence(Sequence):

    def generate(self, **kargs):
        for instr in [{0}]:
            self.genInstruction(instr, {{"NoSkip":1}})

{4}

## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThread{2}

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = Env{2}
"""

RISCVAdditionalImport = "from riscv.ModifierUtils import PageMemoryAttributeModifier"

RISCVGenThreadInit = """def gen_thread_initialization(gen_thread):
    gen_thread.applyChoiceModifier(PageMemoryAttributeModifier)

## Points to the generator thread initialization function defined in this file, optional
GenThreadInitialization = gen_thread_initialization"""
