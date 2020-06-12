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
# classes, code related to Branch not taken. 

from base.Sequence import Sequence

#-------------------------------------------------------------------------------------------------------
# BntSequence to provide base class for bnt squence 
#-------------------------------------------------------------------------------------------------------

class BntSequence(Sequence):
    def __init__(self, gen_thread, instr_num = 10):
        super().__init__(gen_thread)
        self.instr_num = instr_num
        self.bntCallback = None
        
    def setup(self, **kargs):
        super().setup(**kargs)
        self.genThread.modifyGenMode("Filler")

    def generate(self, **kargs):
        if self.bntCallback:
            self.bntCallback()

    def getInstrNum(self):
        return self.instr_num

    def setInstrNum(self, instr_num):
        self.instr_num = instr_num

    def setBntCallback(self, callback):
        self.bntCallback = callback
    
    def cleanUp(self, **kargs):
        super().cleanUp(**kargs)
        self.genThread.revertGenMode("Filler")
