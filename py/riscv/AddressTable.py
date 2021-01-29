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
from base.Sequence import Sequence
import inspect
import sys
import os

## AddressTableRISCV
# Provide available address for exceptions handlers.
class AddressTableRISCV(Sequence):

    def __init__(self, gen_thread):
        super().__init__(gen_thread)
        self.table_index = None
        self._mAppRegSize = 64

    def generate(self, **kwargs):
        self.table_index = kwargs.get('table_index', None)
        if self.table_index is None:
            self.table_index = self.getRandomGPR(exclude="0,1,2")
        self.reserveRegisterByIndex(64,self.table_index,"GPR","ReadWrite")
        self._mAppRegSize = self.getGlobalState('AppRegisterWidth')
        
        return self.table_index

    def tableIndex(self):
        return self.table_index

    def getAddress(self, reg_index, scratch_index):
        #Load the value pointed to by the reg table_index into reg reg_index and then increment the value in the reg table_index by 8
        self.genLoadRegFromDataBlock(reg_index, self.table_index, True)
        
        #get scratch GPR and load it with the number 1 
        self.genInstruction('ADDI##RISCV', {'rd': scratch_index, 'rs1': 0, 'simm12': 1})

        #Compare the value that had been loaded into reg reg_index to one. One means the table entry after this one is actually the address of another table.
        pc = self.getPEstate("PC")
        self.genInstruction('BNE##RISCV', {'rs1': scratch_index, 'rs2': reg_index, 'simm12': 8, 'NoBnt': 1, 'NoRestriction': 1})
        self.setPEstate("PC", pc + 4)

        #Load the value pointed to by the reg table_index into reg table_index.
        self.genLoadRegFromDataBlock(self.table_index, self.table_index)

        #Load the value pointed to by the reg table_index into reg reg_index and then increment the value in the reg table_index by 8
        self.genLoadRegFromDataBlock(reg_index, self.table_index, True)

    def genLoadRegFromDataBlock(self, aDestReg, aAddrReg, aIncrementAddr = False):
        if self._mAppRegSize == 32:
            self.genInstruction('LW##RISCV', {'rd': aDestReg, 'rs1': aAddrReg, 'simm12': 0, 'NoRestriction': 1})
            if aIncrementAddr:
                self.genInstruction('ADDI##RISCV', {'rd': aAddrReg, 'rs1': aAddrReg, 'simm12': 4})
        else:
            self.genInstruction('LD##RISCV', {'rd': aDestReg, 'rs1': aAddrReg, 'simm12': 0, 'NoRestriction': 1})
            if aIncrementAddr:
                self.genInstruction('ADDI##RISCV', {'rd': aAddrReg, 'rs1': aAddrReg, 'simm12': 8})

## AddressTableManagerRISCV class
class AddressTableManagerRISCV(Sequence):

    def __init__(self, gen_thread):
        super().__init__(gen_thread)
        self.table_index = None        # address table index
        self.address_table = None      # address table

    def generate(self):
        self.address_table = AddressTableRISCV(self.genThread)
        self.table_index = self.address_table.generate(table_index=self.table_index)
        fast_mode = self.genThread.fastMode 
        addr_table_info_set = {}
        addr_table_info_set["table_index"] = self.table_index;
        addr_table_info_set["fast_mode"] = fast_mode 
        self.genSequence("InitializeAddrTables", addr_table_info_set)

    # only copy table index.
    def createShallowCopy(self, gen_thread):
        address_table_manager_copy = AddressTableManagerRISCV(gen_thread)
        address_table_manager_copy.table_index = self.table_index
        return address_table_manager_copy

    def addressTable(self):
        return self.address_table
