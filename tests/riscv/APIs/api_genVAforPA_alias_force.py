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

class MainSequence(Sequence):

    def generate(self, **kargs):
        rand_PA = self.genPA(Size=8, Align=8, Type='D', Bank='Default', CanAlias=1)
        rand_VA_1 = self.genVAforPA(PA=rand_PA, Bank='Default', FlatMap=0, Type='D', Size=8)
        rand_VA_2 = self.genVAforPA(PA=rand_PA, Bank='Default', FlatMap=0, Type='D', Size=8, ForceNewAddr=1)

        self.genInstruction('SD##RISCV', {'LSTarget':rand_VA_1})
        self.genInstruction('LD##RISCV', {'LSTarget':rand_VA_2})

MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

