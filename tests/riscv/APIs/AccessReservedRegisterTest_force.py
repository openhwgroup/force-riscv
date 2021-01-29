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

import sys

# import instruction_tree module from the current directory
from DV.riscv.trees.instruction_tree import *

#=========================================================================================
# check #1: reserve/unreserve successive GPR registers; should NOT be able to access
#           a reserved register via getRandomGPR functions.
# check #2: reserve sets of registers with limited access rights; generate random
#           instructions, insuring that each generated instruction honors the reserved
#           registers.
#=========================================================================================

class MainSequence(Sequence):

    def generate(self, **kargs):
        self.reservedIndexedRegCheck()
        self.instrCheck()

    def reservedIndexedRegCheck(self):
        for i in range(1, 30):
            reserved_gpr = "x%d" % i

            # lets try read 1st...
            if not self.isRegisterReserved(reserved_gpr,"Read"):
                print("[DEBUG] next 'Read' reserved register is %s (ID: %d)\n" % (reserved_gpr,i))
                self.reserveRegister(reserved_gpr,"Read")
                self.accessReservedReg(i)
                self.unreserveRegister(reserved_gpr,"Read")

            # then write...
            if not self.isRegisterReserved(reserved_gpr,"Write"):
                print("[DEBUG] next 'Write' reserved register is %s (ID: %d)\n" % (reserved_gpr,i))
                self.reserveRegister(reserved_gpr,"Write")
                self.accessReservedReg(i)
                self.unreserveRegister(reserved_gpr,"Write")

            # then read/write...
            if not self.isRegisterReserved(reserved_gpr,"ReadWrite"):
                print("[DEBUG] next 'Read/Write' reserved register is %s (ID: %d)\n" % (reserved_gpr,i))
                self.reserveRegister(reserved_gpr,"ReadWrite")
                self.accessReservedReg(i)
                self.unreserveRegister(reserved_gpr,"ReadWrite")

    def instrCheck(self):
        read_only_regs   = self.reservedRandomRegCheck("Write")
        write_only_regs  = self.reservedRandomRegCheck("Read")
        cant_access_regs = self.reservedRandomRegCheck("ReadWrite")

        print("[DEBUG INSTR-CHECK] read-only regs:  ",read_only_regs)
        print("[DEBUG INSTR-CHECK] write-only regs: ",write_only_regs)
        print("[DEBUG INSTR-CHECK] no-access regs:  ",cant_access_regs)

        for i in range(20):
            random_instr = self.pickWeighted(ALU_Int32_instructions)

            instr = self.genInstruction(random_instr)
            instr_info = self.queryInstructionRecord(instr)

            dests = instr_info["Dests"]

            for rname, rvalue in dests.items():
                if rname in read_only_regs:
                    raise ValueError("[DEBUG] OOPS! random '%s' instruction used reserved (read-only) register (X%d) as dest operand???" % 
                                     (instr_info["Name"],rname) )
                if rname in cant_access_regs:
                    raise ValueError("[DEBUG] OOPS! random '%s' instruction used reserved (read,write not allowed) register (X%d) as dest operand???" % 
                                     (instr_info["Name"],rname) )

            srcs = instr_info["Srcs"]

            for rname, rvalue in srcs.items():
                if rname in write_only_regs:
                    raise ValueError("[DEBUG] OOPS! random '%s' instruction used reserved (write-only) register (X%d) as src operand???" % 
                                     (instr_info["Name"],rname) )
                if rname in cant_access_regs:
                    raise ValueError("[DEBUG] OOPS! random '%s' instruction used reserved (read,write not allowed) register (X%d) as src operand???" % 
                                     (instr_info["Name"],rname) )

        self.freeReservedRegs(read_only_regs,"Write")
        self.freeReservedRegs(write_only_regs,"Read")
        self.freeReservedRegs(cant_access_regs,"ReadWrite")

    def accessReservedReg(self,reserved_reg_id):
        rnd_set = []
        for j in range(100):
            rnd_set.append(self.getRandomGPR())
            (r1,r2,r3,r4,r5) = self.getRandomRegisters(5, "GPR", "%d" % reserved_reg_id)
            if r1 not in rnd_set: rnd_set.append(r1)
            if r2 not in rnd_set: rnd_set.append(r2)
            if r3 not in rnd_set: rnd_set.append(r3)
            if r4 not in rnd_set: rnd_set.append(r4)
            if r5 not in rnd_set: rnd_set.append(r5)
        print("\t[DEBUG] random gprs: ",rnd_set)
        if reserved_reg_id in rnd_set:
            print("[DEBUG] OOPS! RandomGPR returned reserved register (X%d) ???" % reserved_reg_id)
            sys.stdout.flush();
            raise ValueError("OOPS! RandomGPR returned reserved register (X%d) ???" % reserved_reg_id)

    def reservedRandomRegCheck(self,access_to_deny):
        (gpr1, gpr2, gpr3) = self.getRandomRegisters(3, "GPR", "31")
        self.reserveRegister("x%d" % gpr1,access_to_deny)
        self.accessReservedReg(gpr1)
        self.reserveRegister("x%d" % gpr2,access_to_deny)
        self.accessReservedReg(gpr2)
        self.reserveRegister("x%d" % gpr3,access_to_deny)
        self.accessReservedReg(gpr3)
        return [ gpr1, gpr2, gpr3 ]

    def freeReservedRegs(self,rlist,denied_access):
        for reg in rlist:
            self.unreserveRegister("x%d" % reg,denied_access)



## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

