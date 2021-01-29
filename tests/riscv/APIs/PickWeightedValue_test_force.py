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
#This template tests the API pickWeightedValue()
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence

class MainSequence(Sequence):
    def generate(self, **kargs):
        reg_dict = {"0-14, 16-29":50, "15, 30-31":0}
        for i in range(10):
            reg_index = self.pickWeightedValue(reg_dict)
            if reg_index == 15 or reg_index == 30 or reg_index == 31 : 
                self.error("Failed to pick weighted value : %d" % reg_index)

            self.notice("Pick weighted value from register: %d" % reg_index)

        mem_dict = {"0x0-0x7fffffff":0, "0x80000000-0xffffffff":100}
        for i in range(10):
            mem_addr = self.pickWeightedValue(mem_dict)
            if mem_addr < 0x80000000 : 
                self.error("Failed to pick weighted value: 0x%x" % mem_addr)
            
            self.notice("Pick weighted value from memory : 0x%x" % mem_addr)
            
        
## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

