
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


# Utility function used to combine multiple dictionaries into one single dictionary
def Merge(*args):
    result = {}
    for dict1 in args:
        result.update(dict1)
    return result

# CSRs, grouped by Mode and access rights...

User_RW_CSRs = {
    'ustatus':0,
    'fflags':10,
    'frm':10,
    'fcsr':10,
    'uie':0,
    'utvec':0,
    'vstart':0,
    'vxsat':0,
    'vxrm':0,
    'uscratch':0,
    'uepc':0,
    'ucause':0,
    'utval':0,
    'uip':0,
}

User_RO_CSRs = {
    'cycle':0,
    'time':10,
    'instret':10,
    'hpmcounter3':0,
    'hpmcounter4':0,
    'hpmcounter5':0,
    'hpmcounter6':0,
    'hpmcounter7':0,
    'hpmcounter8':0,
    'hpmcounter9':0,
    'hpmcounter10':0,
    'hpmcounter11':0,
    'hpmcounter12':0,
    'hpmcounter13':0,
    'hpmcounter14':0,
    'hpmcounter15':0,
    'hpmcounter16':0,
    'hpmcounter17':0,
    'hpmcounter18':0,
    'hpmcounter19':0,
    'hpmcounter20':0,
    'hpmcounter21':0,
    'hpmcounter22':0,
    'hpmcounter23':0,
    'hpmcounter24':0,
    'hpmcounter25':0,
    'hpmcounter26':0,
    'hpmcounter27':0,
    'hpmcounter28':0,
    'hpmcounter29':0,
    'hpmcounter30':0,
    'hpmcounter31':0,
    'vl':0,
    'vtype':0,
    'vlenb':0,
}

Supervisor_RW_CSRs = {
    'sstatus':10,
    'sedeleg':0,
    'sideleg':0,
    'sie':0,
    'stvec':10,
    'scounteren':0,
    'sscratch':10,
    'sepc':10,
    'scause':10,
    'stval':10,
    'sip':0,
    'satp':10,
}

Supervisor_RO_CSRs = {
}

Hypervisor_RW_CSRs = {
    'vsstatus':10,
    'vsie':0,
    'vstvec':10,
    'vsscratch':10,
    'vsepc':10,
    'vscause':10,
    'vstval':10,
    'vsip':0,
    'vsatp':10,
    'hstatus':10,
    'hedeleg':10,
    'hideleg':10,
    'hie':10,
    'hcounteren':0,
    'hgeie':10,
    'htval':10,
    'hip':10,
    'htinst':10,
    'hgatp':10,
}

Hypervisor_RO_CSRs = {
    'hgeip':10,
}

Machine_RW_CSRs = {
    'mstatus':10,
    'mstatus_hyp':0,
    'misa':10,
    'medeleg':10,
    'mideleg':10,
    'mie':0,
    'mtvec':10,
    'mcounteren':0,
    'mcountinhibit':0,
    'mhpmevent3':0,
    'mhpmevent4':0,
    'mhpmevent5':0,
    'mhpmevent6':0,
    'mhpmevent7':0,
    'mhpmevent8':0,
    'mhpmevent9':0,
    'mhpmevent10':0,
    'mhpmevent11':0,
    'mhpmevent12':0,
    'mhpmevent13':0,
    'mhpmevent14':0,
    'mhpmevent15':0,
    'mhpmevent16':0,
    'mhpmevent17':0,
    'mhpmevent18':0,
    'mhpmevent19':0,
    'mhpmevent20':0,
    'mhpmevent21':0,
    'mhpmevent22':0,
    'mhpmevent23':0,
    'mhpmevent24':0,
    'mhpmevent25':0,
    'mhpmevent26':0,
    'mhpmevent27':0,
    'mhpmevent28':0,
    'mhpmevent29':0,
    'mhpmevent30':0,
    'mhpmevent31':0,
    'mscratch':10,
    'mepc':10,
    'mcause':10,
    'mtval':10,
    'mip':0,
    'mtinst':0,
    'mtval2':0,
    'pmpcfg0':0,
    'pmpcfg2':0,
    'pmpaddr0':0,
    'pmpaddr1':0,
    'pmpaddr2':0,
    'pmpaddr3':0,
    'pmpaddr4':0,
    'pmpaddr5':0,
    'pmpaddr6':0,
    'pmpaddr7':0,
    'pmpaddr8':0,
    'pmpaddr9':0,
    'pmpaddr10':0,
    'pmpaddr11':0,
    'pmpaddr12':0,
    'pmpaddr13':0,
    'pmpaddr14':0,
    'pmpaddr15':0,
    'mcycle':0,
    'minstret':0,
    'mhpmcounter3':0,
    'mhpmcounter4':0,
    'mhpmcounter5':0,
    'mhpmcounter6':0,
    'mhpmcounter7':0,
    'mhpmcounter8':0,
    'mhpmcounter9':0,
    'mhpmcounter10':0,
    'mhpmcounter11':0,
    'mhpmcounter12':0,
    'mhpmcounter13':0,
    'mhpmcounter14':0,
    'mhpmcounter15':0,
    'mhpmcounter16':0,
    'mhpmcounter17':0,
    'mhpmcounter18':0,
    'mhpmcounter19':0,
    'mhpmcounter20':0,
    'mhpmcounter21':0,
    'mhpmcounter22':0,
    'mhpmcounter23':0,
    'mhpmcounter24':0,
    'mhpmcounter25':0,
    'mhpmcounter26':0,
    'mhpmcounter27':0,
    'mhpmcounter28':0,
    'mhpmcounter29':0,
    'mhpmcounter30':0,
    'mhpmcounter31':0,
}

Machine_RO_CSRs = {
    'mvendorid':0,
    'marchid':0,
    'mimpid':0,
    'mhartid':0,
}

All_Machine_RO_CSRs = Merge(Machine_RO_CSRs, Supervisor_RO_CSRs)
All_Machine_RW_CSRs = Merge(Machine_RW_CSRs, Supervisor_RW_CSRs)
