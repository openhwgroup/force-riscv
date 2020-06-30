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
from base.ChoicesModifier import *
from riscv.ModifierUtils import *

class MainSequence(Sequence):

    #************************************************************************************************
    # Test demonstrates how to specify initial value for bits within misa extensions.
    #
    #   1. Use ChoicesModifier in gen_thread_initialization to disable Compressed ISA (clear misa.C bit)
    #   2. Read initial value for misa, as setup by Force, confirm that C bit is clear.
    #   3. Execute CSR read instruction to get simulator misa value.
    #   4. Initial misa value should match simulator misa value.
    #************************************************************************************************

    def generate(self, **kargs):
        misaInitialValue, validCheck = self.readRegister("misa")

        if not validCheck:
            self.error("Can't read initial value of misa???")

        self.displayMisa(misaInitialValue,'MISA initial value')


        # expect the misa.C bit to be turned off...

        if (misaInitialValue & 0x4) != 0:
            self.error("misa.C is is on???")
            
            
        (gpr1, gpr2) = self.getRandomRegisters( 2, "GPR", "0" )
        self.genInstruction('CSRRS#register#RISCV', {'rd': gpr1, 'rs1': 0, 'csr': self.getRegisterIndex('misa') } )

        simulatorMisaValue, validCheck = self.readRegister("x%d" % gpr1)

        if not validCheck:
            self.error("Can't read current value of misa???")

        self.displayMisa(simulatorMisaValue,'MISA runtime value')

        # the default initial value for misa should match the simulator reported value...
        
        if misaInitialValue != simulatorMisaValue:
            self.error("Error: Initial value for misa (0x%x) does NOT match simulator reported value (0x%x)" % (misaInitialValue,simulatorMisaValue) )


    # print misa value to generator log...

    def displayMisa(self, misaValue, srcText):
        bits = [ 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z' ]
        
        MXL = (misaValue>>62) & 3
        
        set_bits = []
        for i in range(26):
            next_bit = (misaValue>>i) & 1
            if next_bit == 1:
                set_bits.append(bits[i])

        self.notice("%s: 0x%x (MXL: %d, extensions present: %s" % (srcText,misaValue,MXL,str(set_bits)))


# can we specify a misa field value?...

def gen_thread_initialization(gen_thread):
    turn_off_misa_bits = ChoicesModifier(gen_thread)
    weightDict = { "0x0":100, "0x1":0 }
    turn_off_misa_bits.modifyRegisterFieldValueChoices("misa.C",weightDict)
    turn_off_misa_bits.commitSet()
    
GenThreadInitialization = gen_thread_initialization


## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

