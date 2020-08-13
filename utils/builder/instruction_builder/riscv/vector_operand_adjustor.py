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
from operand_adjustor import OperandAdjustor
from shared.instruction import Operand

class VectorOperandAdjustor(OperandAdjustor):
    def __init__(self, aInstruction):
        super().__init__(aInstruction)

        # dictionary for instructions that use rd instead of vd where vd/rd is defined
        self.rd_dictionary = {#'VSETVLI', #TODO: VSETVLI and VSETVL are not arithmetic instructions and will be taken care of after arithmetic instructions are done
                         #'VSETVL',
                         'VPOPC',
                         'VFIRST',
                         'VMV.X.S',
                         'VFMV.F.S'}

    def add_vtype_layout_operand(self):
        layout_opr = Operand()
        layout_opr.name = "vtype"
        layout_opr.type = "VectorLayout"
        layout_opr.oclass = "VtypeLayoutOperand"
        self.mInstr.insert_operand(0, layout_opr)

    def add_custom_layout_operand(self, aRegCount, aElemWidth):
        layout_opr = Operand()
        layout_opr.name = "B" #TODO (Chris): change name?
        layout_opr.type = "VectorLayout"
        layout_opr.oclass = "CustomLayoutOperand"
        layout_opr.regCount = aRegCount
        layout_opr.elemWidth = aElemWidth
        self.mInstr.insert_operand(0, layout_opr)

    def add_whole_register_layout_operand(self, aRegCount=1, aRegIndexAlignment=1):
        layout_opr = Operand()
        layout_opr.name = "B"
        layout_opr.type = "VectorLayout"
        layout_opr.oclass = "WholeRegisterLayoutOperand"
        layout_opr.regCount = aRegCount
        layout_opr.regIndexAlignment = aRegIndexAlignment
        self.mInstr.insert_operand(0, layout_opr)

    def set_reg_vec(self, aOperand):
        aOperand.type = "VECREG" #TODO: some might possibly be SIMDVR or FPR
        aOperand.choices = "Vector registers"
        self.add_asm_op(aOperand)

    def set_reg_vec_nonzero(self, aOperand):
        aOperand.type = "VECREG"
        aOperand.choices = "Nonzero vector registers"
        self.add_asm_op(aOperand)

    def set_vm(self):
        vm_opr = self.mInstr.find_operand('vm')
        vm_opr.type = 'Choices'
        vm_opr.choices = 'Vector mask'
        vm_opr.oclass = 'VectorMaskOperand'
        if self.mInstr.find_operand('vd', fail_not_found=False):
            vm_opr.differ = 'vd'

        self.add_asm_op(vm_opr)

    def set_vs1(self):
        vs1_opr = self.mInstr.find_operand('vs1')
        vs1_opr.access = 'Read'
        self.set_reg_vec(vs1_opr)

    def set_vs1_differ_vd(self):
        vs1_opr = self.mInstr.find_operand('vs1', fail_not_found=False)
        if vs1_opr:
            vs1_opr.differ = 'vd'

    def set_vs2(self):
        vs2_opr = self.mInstr.find_operand('vs2')
        vs2_opr.access = 'Read'
        self.set_reg_vec(vs2_opr)

    def set_vs2_differ_vd(self):
        vs2_opr = self.mInstr.find_operand('vs2')
        vs2_opr.differ = 'vd'

    def set_vs3(self):
        vs3_opr = self.mInstr.find_operand('vs3')
        vs3_opr.access = 'Read'
        self.set_reg_vec(vs3_opr)

    def set_vs3_ls_source(self):
        vs3_opr = self.mInstr.find_operand('vs3')
        vs3_opr.oclass = 'MultiVectorRegisterOperandRISCV'
        self.set_vs3()

    def set_vdrd_int(self):
        vdrd_opr = self.mInstr.find_operand('vd/rd')
        if self.mInstr.name in self.rd_dictionary:
            vdrd_opr.name = 'rd'
            self.set_reg_int(vdrd_opr)
        else:
            vdrd_opr.name = 'vd'
            self.set_reg_vec(vdrd_opr)
        vdrd_opr.access = 'Write'

    def set_vd(self):
        vd_opr = self.mInstr.find_operand('vd')
        vd_opr.access = 'Write'
        self.set_reg_vec(vd_opr)

    def set_vd_nonzero(self):
        vd_opr = self.mInstr.find_operand('vd$\\neq$0')
        vd_opr.name = 'vd'
        vd_opr.access = 'Write'
        self.set_reg_vec_nonzero(vd_opr)

    def set_vd_ls_dest(self):
        vd_opr = self.mInstr.find_operand('vd')
        vd_opr.oclass = 'MultiVectorRegisterOperandRISCV'
        self.set_vd()

    def set_vdrd_sp(self):
        vdrd_opr = self.mInstr.find_operand('vd/rd')
        if self.mInstr.name in self.rd_dictionary:
            vdrd_opr.name = 'rd'
            self.set_reg_sp(vdrd_opr)
        else:
            vdrd_opr.name = 'vd'
            self.set_reg_vec(vdrd_opr)
        vdrd_opr.access = 'Write'

    def set_wide_dest(self, aLayoutMultiple=2):
        dest_opr = self.mInstr.find_operand('vd', fail_not_found=False)
        if dest_opr is None:
            dest_opr = self.mInstr.find_operand('vd/rd')

        dest_opr.layoutMultiple = aLayoutMultiple

    def set_wide_source(self, aLayoutMultiple=2):
        vs2_opr = self.mInstr.find_operand('vs2')
        vs2_opr.layoutMultiple = aLayoutMultiple
