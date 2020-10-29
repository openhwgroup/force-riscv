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
from riscv.AssemblyHelperRISCV import AssemblyHelperRISCV

## This test verifies that instructions to read read-only system registers can be generated.
class MainSequence(Sequence):

    def generate(self, **kargs):
        read_only_reg_names = ('marchid', 'mhartid', 'mimpid', 'mvendorid', 'vl', 'vlenb', 'vtype')
        assembly_helper = AssemblyHelperRISCV(self)
        for _ in range(10):
            gpr_index = self.getRandomGPR(exclude='0')
            read_only_reg_name = self.choice(read_only_reg_names)
            assembly_helper.genReadSystemRegister(gpr_index, read_only_reg_name)

            (sys_reg_val, valid) = self.readRegister(read_only_reg_name)
            self._assertValidRegisterValue(read_only_reg_name, valid)
            gpr_name = 'x%d' % gpr_index
            (gpr_val, valid) = self.readRegister(gpr_name)
            self._assertValidRegisterValue(gpr_name, valid)

            gen_mode = self.getPEstate("GenMode")
            no_iss = gen_mode & 0x1
            if (no_iss != 1) and (gpr_val != sys_reg_val):
                self.error('Register %s was not read correctly. Expected=0x%x, Actual=0x%x' % (read_only_reg_name, sys_reg_val, gpr_val))

    ## Fail if the valid flag is false.
    #
    #  @param aRegName The name of the register.
    #  @param aValid A flag indicating whether the specified register has a valid value.
    def _assertValidRegisterValue(self, aRegName, aValid):
        if not aValid:
            self.error('Value for register %s is invalid' % aRegName)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
