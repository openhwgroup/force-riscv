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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from DV.riscv.counter.depdenceSequence import depSequence
from DV.riscv.trees.instruction_tree import LDST_All_instructions, LDST32_All_instructions
import sys


class MainSequence(depSequence): 

    def generate(self, **kargs): 

        self.choiceMod()

        self.notice('Warm up resource entropy')
        for i in range(100):
            #instr = instMap.pick(self)
            instr_tree = LDST32_All_instructions if self.getGlobalState('AppRegisterWidth') == 32 else LDST_All_instructions
            instr = self.pickWeighted(instr_tree)
            self.genMetaInstruction(instr)
            #self.genInstruction(instr)

        self.notice('Querying GPR entropy')
        entropy_dict0 = self.queryResourceEntropy("GPR")
        self.show_notice(entropy_dict0)

        self.notice('Querying FPR entropy')
        entropy_dict1 = self.queryResourceEntropy("FPR")
        self.show_notice(entropy_dict1)

    def show_notice(self, entropy_dict):
        source_entropy = entropy_dict["Source"]
        self.notice("Source entropy state:%s" % source_entropy["State"])
        self.notice("Source entropy value:%d" % source_entropy["Entropy"])
        self.notice("Source onThreshold:%d" % source_entropy["OnThreshold"])
        self.notice("Source offThreshold:%d" % source_entropy["OffThreshold"])
        
        dest_entropy = entropy_dict["Dest"]
        self.notice("Dest entropy state:%s" % dest_entropy["State"])
        self.notice("Dest entropy value:%d" % dest_entropy["Entropy"])
        self.notice("Dest onThreshold:%d" % dest_entropy["OnThreshold"])
        self.notice("Dest offThreshold:%d" % dest_entropy["OffThreshold"])


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

