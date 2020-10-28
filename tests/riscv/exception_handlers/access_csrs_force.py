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
from base.ChoicesModifier import ChoicesModifier
from base.TestException import *
from riscv.AssemblyHelperRISCV import AssemblyHelperRISCV
from riscv.Utils import LoadGPR64
from base.InstructionMap import InstructionMap
from DV.riscv.trees.instruction_tree import *
from DV.riscv.trees.csr_trees import *

import RandomUtils

## --------------------------------------------------------------------------------------------
## Test issues read or read/write accesses to random CSRs, based on current privilege level...
## --------------------------------------------------------------------------------------------

class MainSequence(Sequence):

    def generate(self, **kargs):
        # generate sequences of random instructions followed by random CSR access...
        
        for i in range(20):    
            self.genRandomInstrs(1, 10)
            (csr,is_writable) = self.genSelectCSR()
            if csr == None:
                pass
            else:
                self.genAccessCSR(csr,is_writable)


    ## --------------------------------------------------------------------------------------------
    ## generate some random instructions...
    ## --------------------------------------------------------------------------------------------

    def genRandomInstrs(self, min_cnt, max_cnt):
        # select random instructions from this group:
        instruction_group = RV32I_instructions if self.getGlobalState('AppRegisterWidth') else RV_A_instructions

        for _ in range( RandomUtils.random32(min_cnt, max_cnt) ):
            the_instruction = self.pickWeighted(instruction_group)
            self.genInstruction(the_instruction)


    ## --------------------------------------------------------------------------------------------
    ## select CSR to access based on current privilege level, + random read/write...
    ## --------------------------------------------------------------------------------------------

    def genSelectCSR(self):
        priv_level = 'User'
        if self.getPEstate('PrivilegeLevel') == 3:
            priv_level = 'Machine'
        elif self.getPEstate('PrivilegeLevel') == 2:
            priv_level = 'Hypervisor'
        elif self.getPEstate('PrivilegeLevel') == 1:
            priv_level = 'Supervisor'

        read_write = 'RO'
        if RandomUtils.random32(0,1) == 1:
            read_write = 'RW'

        return self.csrIndexAccessRights(priv_level,read_write)


    ## --------------------------------------------------------------------------------------------
    ## random selection from CSR tree, indexed by privilege level and access type...
    ## --------------------------------------------------------------------------------------------

    def csrIndexAccessRights(self,priv_level,read_write):
        csr_trees = {
            'User_RW_CSRs'        : ( User_RW_CSRs, True ),
            'User_RO_CSRs'        : ( User_RO_CSRs, False ),
            'Supervisor_RW_CSRs'  : ( Supervisor_RW_CSRs, True ),
            'Supervisor_RO_CSRs'  : ( Supervisor_RO_CSRs, False ),
            'Hypervisor_RW_CSRs'  : ( Hypervisor_RW_CSRs, True ),
            'Hypervisor_RO_CSRs'  : ( Hypervisor_RO_CSRs, False ),
            'All_Machine_RW_CSRs' : ( Machine_RW_CSRs, True ),
            'All_Machine_RO_CSRs' : ( Machine_RO_CSRs, False )
        }

        if priv_level == 'Machine':
            csr_group_index = 'All_%s_%s_CSRs' % (priv_level,read_write)
        else:
            csr_group_index = '%s_%s_CSRs' % (priv_level,read_write)

        (csr_group, is_writable) = csr_trees[csr_group_index]
        if len(csr_group) == 0:
            return (None, False)

        try:
            csr = self.pickWeighted(csr_group)
        except TestException:
            # watch out for 'zero biased' CSR tree... 
            return (None, False)
            
        self.notice("ACCESS CSRS tree: '%s' csr: '%s'" % (csr_group_index, csr) )
        return (csr, is_writable)


    ## --------------------------------------------------------------------------------------------
    ## generate instructions to read from CSR, and (optionally) write its value back.
    ## check read value, write value, and if unexpected exceptions were taken.
    ##
    ## Will ASSUME that CSR read value can be written back to that same CSR without side effects.
    ## --------------------------------------------------------------------------------------------
    
    def genAccessCSR(self, csr, writeback = False):
        csr_index = self.getRegisterIndex(csr)
        csr_reg_info = self.getRegisterInfo(csr, csr_index)

        self.notice("ACCESS CSRS CSR info: %s" % str(csr_reg_info) )
        
        # pick a  scratch reg...

        scratch_reg_index = self.getRandomGPRs(1, exclude='0')[0]

        scratch_reg_name = "x%d" % scratch_reg_index
        
        # read the CSR...
        
        self.notice("ACCESS CSRS: Read of CSR %s" % csr)

        self.genInstruction('CSRRS#register#RISCV', {'rd': scratch_reg_index, 'rs1': 0, 'csr': csr_index } )

        # did the CSR access cause an exception???
        
        exceptions_history = self.queryExceptions(EC="1,2")
        if len(exceptions_history) > 0:
            self.error("Unexpected exception on read access of CSR '%s'" % csr)

        # this CSR can be written. write back the value read...

        if writeback:
            # generate some number of random instrs between the CSR read, and a write to the same CSR...
            
            self.reserveRegister(scratch_reg_name) # but don't disturb the CSR read value
        
            self.genRandomInstrs(0, 5)
            
            self.unreserveRegister(scratch_reg_name)

            self.notice("ACCESS CSRS: Write of CSR %s" % csr)
            self.genInstruction('CSRRW#register#RISCV', {'rd': 0, 'rs1': scratch_reg_index, 'csr': csr_index } )

            # we expect no exception on the CSR write...

            exceptions_history = self.queryExceptions(EC="1,2")
            if len(exceptions_history) > 0:
                self.error("Unexpected exception on write access to CSR '%s'" % csr)
    

MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
