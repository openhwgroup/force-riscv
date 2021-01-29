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
from base.Sequence import Sequence
from base.Bitstream import Bitstream

class MainSequence(Sequence):

    def generate(self, **kargs):
        
        my_stream = Bitstream()
        my_stream.append(" X ").append("0000 xx01").prepend("1101")
        assert (my_stream.stream() == "1101xxxx0000xx01")
        assert (my_stream.value() == 53249)
        assert (my_stream.mask() == 61683)
        assert (my_stream.valueMask() == "0xd001/0xf0f3")
        assert (my_stream["1-3, 15"] == "0xx1")
        assert (my_stream["15, 3-1"] == "1xx0")
        assert (my_stream["15-0"] == "1101xxxx0000xx01")
        assert (my_stream["0-15"] == "10xx0000xxxx1011")
        assert (my_stream["13-12, 10-8, 6-5, 1-0"] == "01xxx0001")
        assert (my_stream["0-1, 3-5, 7-9, 12-15"] == "10x000xx1011")
        assert (my_stream["0-2, 5"] == "10x0")
        assert (my_stream["15-12, 8, 3-1"] == "1101xxx0")
 
        for instr in ["LUI##RISCV", "LUI##RISCV"]:
            instr_rec = self.genInstruction(instr)

        # get page information test
        # Note: should not have any record returned in this case       
        self.notice("GetPageInfo validation...")
        bank = instr_rec[0:instr_rec.find('#')]
        addr = instr_rec[instr_rec.find('#')+1:]
        results = self.getPageInfo(addr, "VA", bank)        
        for keys, record in sorted(results.items()):
            self.notice("Processing %s record..." % keys)
            for key, item in record.items():
                if key == "DescriptorDetails":
                    self.notice("Decriptor details...")
                    for k, v in item.items():
                        self.notice("Detail Item: %s = %s" % (k, v))                
                elif isinstance (item, str):
                    self.notice("Record: %s = %s" %(key, item))
                else:
                    self.notice("Record: %s = 0x%x" %(key, item)) 

## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

