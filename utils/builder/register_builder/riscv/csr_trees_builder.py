#!/usr/bin/env python3
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

import sys
import os
import getopt
import argparse
import re
import defusedxml.defusedxml.ElementTree as ET
import copy

license_string = """
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

"""

tree_utility_string = """
# Utility function used to combine multiple dictionaries into one dictionary
def Merge(*args):
    result = {}
    for dict1 in args:
        result.update(dict1)
    return result

"""

merge_trees_string = """
All_Machine_RO_CSRs = Merge(Machine_RO_CSRs, Supervisor_RO_CSRs)
All_Machine_RW_CSRs = Merge(Machine_RW_CSRs, Supervisor_RW_CSRs)
"""


# csr_trees_builder.py
# This file reads existing system registers (XML)file to create a CSR
# trees file.

class CSRTreeBuilder:
    def __init__(self, aSysRegsStarterFile=None, aCSRTreeFile=None):
        if aSysRegsStarterFile is None:
            raise ValueError("Internal error (CSRTreeBuilder): "
                             "No starter file specified.")
        if aCSRTreeFile is None:
            raise ValueError("Internal error (CSRTreeBuilder): "
                             "No starter file specified.")

        self.mSysRegsStarterFile = aSysRegsStarterFile
        self.mCSRTreeFile = aCSRTreeFile

        self.mUserRW_CSRs = []
        self.mUserRO_CSRs = []

        self.mSupervisorRW_CSRs = []
        self.mSupervisorRO_CSRs = []

        self.mHypervisorRW_CSRs = []
        self.mHypervisorRO_CSRs = []

        self.mMachineRW_CSRs = []
        self.mMachineRO_CSRs = []

        self.setupCSRIndices()
        self.parseSystemRegistersFile()
        self.dumpCSRTrees()

    # map all valid CSR indexes to privilege-mode/access-rights...
    def setupCSRIndices(self):
        self.mCSRTypesByIndex = {}

        for i in range(0, 0xfff):
            # standard user CSRs...
            if i >= 0 and i < 0xff:
                self.mCSRTypesByIndex[i] = ('User', 'RW')
            elif i >= 0x400 and i < 0x4ff:
                self.mCSRTypesByIndex[i] = ('User', 'RW')
            elif i >= 0xc00 and i < 0xc7f:
                self.mCSRTypesByIndex[i] = ('User', 'R')
            elif i >= 0xc80 and i < 0xcbf:
                self.mCSRTypesByIndex[i] = ('User', 'R')
            # standard supervisor CSRs...
            if i >= 0x100 and i < 0x1ff:
                self.mCSRTypesByIndex[i] = ('Supervisor', 'RW')
            elif i >= 0x500 and i < 0x5bf:
                self.mCSRTypesByIndex[i] = ('Supervisor', 'RW')
            elif i >= 0x900 and i < 0x9bf:
                self.mCSRTypesByIndex[i] = ('Supervisor', 'RW')
            elif i >= 0xd00 and i < 0xdbf:
                self.mCSRTypesByIndex[i] = ('Supervisor', 'R')
            # standard hypervisor CSRs...
            if i >= 0x200 and i < 0x2ff:
                self.mCSRTypesByIndex[i] = ('Hypervisor', 'RW')
            elif i >= 0x600 and i < 0x6bf:
                self.mCSRTypesByIndex[i] = ('Hypervisor', 'RW')
            elif i >= 0xa00 and i < 0xabf:
                self.mCSRTypesByIndex[i] = ('Hypervisor', 'RW')
            elif i >= 0xe00 and i < 0xebf:
                self.mCSRTypesByIndex[i] = ('Hypervisor', 'R')
            # standard machine CSRs...
            if i >= 0x300 and i < 0x3ff:
                self.mCSRTypesByIndex[i] = ('Machine', 'RW')
            elif i >= 0x700 and i < 0x7af:
                self.mCSRTypesByIndex[i] = ('Machine', 'RW')
            elif i >= 0xb00 and i < 0xbbf:
                self.mCSRTypesByIndex[i] = ('Machine', 'RW')
            elif i >= 0xf00 and i < 0xfbf:
                self.mCSRTypesByIndex[i] = ('Machine', 'R')
            # machine-mode Debug CSRs...
            elif i >= 0x7b0 and i < 0x7bf:
                self.mCSRTypesByIndex[i] = ('Debug', 'RW')
            # one known custom (machine mode) CSR...
            elif i == 0x7c0:
                self.mCSRTypesByIndex[i] = ('Custom', 'RO')

    # parse system registers starter file to glean all CSR definitions.
    # Group CSRs by privilege level and access 'rights'...
    def parseSystemRegistersFile(self):
        aTree = ET.parse(self.mSysRegsStarterFile)
        registers = aTree.findall('.//register')
        for register in registers:
            reg_name = register.get('name')
            reg_index = int(register.get('index'), 0)
            try:
                (mode, access) = self.mCSRTypesByIndex[reg_index]
                # print(reg_name, reg_index, mode, access)
                if mode == 'User':
                    if access == 'RW':
                        self.mUserRW_CSRs.append(reg_name)
                    else:
                        self.mUserRO_CSRs.append(reg_name)
                if mode == 'Supervisor':
                    if access == 'RW':
                        self.mSupervisorRW_CSRs.append(reg_name)
                    else:
                        self.mSupervisorRO_CSRs.append(reg_name)
                if mode == 'Hypervisor':
                    if access == 'RW':
                        self.mHypervisorRW_CSRs.append(reg_name)
                    else:
                        self.mHypervisorRO_CSRs.append(reg_name)
                if mode == 'Machine':
                    if access == 'RW':
                        self.mMachineRW_CSRs.append(reg_name)
                    else:
                        self.mMachineRO_CSRs.append(reg_name)

            except KeyError:
                print("??? %s 0x%x (%d)" % (reg_name, reg_index, reg_index))
                sys.exit(1)

    # write CSR trees file...
    def dumpCSRTrees(self):
        with open(self.mCSRTreeFile, 'w') as f:
            f.write(license_string)
            f.write(tree_utility_string)
            f.write('# CSRs, grouped by Mode and access rights...\n')
            self.dumpOneCSRTree(f, 'User_RW_CSRs', self.mUserRW_CSRs)
            self.dumpOneCSRTree(f, 'User_RO_CSRs', self.mUserRO_CSRs)
            self.dumpOneCSRTree(f, 'Supervisor_RW_CSRs',
                                self.mSupervisorRW_CSRs)
            self.dumpOneCSRTree(f, 'Supervisor_RO_CSRs',
                                self.mSupervisorRO_CSRs)
            self.dumpOneCSRTree(f, 'Hypervisor_RW_CSRs',
                                self.mHypervisorRW_CSRs)
            self.dumpOneCSRTree(f, 'Hypervisor_RO_CSRs',
                                self.mHypervisorRO_CSRs)
            self.dumpOneCSRTree(f, 'Machine_RW_CSRs', self.mMachineRW_CSRs)
            self.dumpOneCSRTree(f, 'Machine_RO_CSRs', self.mMachineRO_CSRs)
            f.write(merge_trees_string)

    # identify counter, event, pmp, interrupt related CSRs...
    def deselectCSR(self, csr):
        # counter/event registers are unpredictable...
        # riscv pmp feature is not yet supported by force...
        # hypervisor not yet supported...
        # N extension exceptions are not yet supported...
        # interrupt related registers (at least the enable/pending regs) are
        # not yet supported...
        unsupported_csrs = [
            'count', 'event', 'pmp', 'cycle', 'time', 'instret',
            'utvec', 'utval', 'uip', 'uepc', 'ucause', 'uscratch', 'ustatus',
            'uie',
            'vtype', 'vxsat', 'vstart', 'vl', 'vxrm',
            'sedeleg', 'sideleg',
            'mtval2', 'mtinst', 'minstret',
            'mip', 'mie', 'sip', 'sie',
            'mstatus_hyp'
        ]
        # skip implementation specific CSRs...
        implementation_specific_csrs = [
            'mvendorid', 'marchid', 'mimpid', 'mhartid'
        ]
        # a CSR should be 'deselected' if it appears in one of the two lists
        # above, either by exact match or (in the case of an indexed CSR
        # (example: pmpaddr0) as a sub-string
        for s in unsupported_csrs + implementation_specific_csrs:
            if csr.find(s) == -1:
                pass
            else:
                return True

        return False

    # return default bias for CSR...
    def csrBias(self, aCSR):
        if self.deselectCSR(aCSR):
            # zero bias certain CSRs...
            return 0
        return 10

    # write a single CSR set...
    def dumpOneCSRTree(self, aFile, aTitle, aCSRs):
        aFile.write('\n%s = {\n' % aTitle)
        for rn in aCSRs:
            aFile.write("    '%s':%d,\n" % (rn, self.csrBias(rn)))
        aFile.write('}\n')


usage_str = """
Use this script to produce a CSR 'trees' file, from the system registers
starter file, as follows:

   ./csr_trees_builder.py -i <starter file> -o <trees file>

   or:

   ./csr_trees_builder.py --system_registers_starter_file=<starter file> \
                          --csr_trees_file=<trees file>

   and, if no arguments are specified:

     

"""

if __name__ == '__main__':
    mSysRegsStarterFile = 'input/system_registers_starter_rv64.xml'
    mCSRTreeFile = 'output/csr_trees.py'
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hi:o:',
                                   ['help', 'system_registers_starter_file=',
                                    'csr_trees_file='])
    except getopt.GetoptError as error:
        print(error)
        print(usage_str)
        sys.exit(1)

    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()
        if o in ['-i', '--system_registers_starter_file']:
            mSysRegsStarterFile = a
        elif o in ['-o', '--csr_trees_file']:
            mCSRTreeFile = a

    print("\tstarter file: '%s'" % mSysRegsStarterFile)
    print("\toutput file: '%s'" % mCSRTreeFile)

    try:
        CSRTreeBuilder(mSysRegsStarterFile, mCSRTreeFile)
    except ValueError as error:
        print(error)
        print(usage_str)
        sys.exit(1)
