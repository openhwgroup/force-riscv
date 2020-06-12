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

    def generate(self, **kwargs):
        self.table_index = kwargs.get('table_index', None)
        if self.table_index is None:
            self.table_index = self.getRandomGPR(exclude="0,1,2")
        self.reserveRegisterByIndex(64,self.table_index,"GPR","ReadWrite")
        return self.table_index

    def tableIndex(self):
        return self.table_index

    def getAddress(self, reg_index):
        # TODO
        pass


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
