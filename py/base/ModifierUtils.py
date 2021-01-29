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
from base.ChoicesModifier import ChoicesModifier

class AddressReuseEnabledChoicesModifier(ChoicesModifier):

    def apply(self, **kwargs):
        reuse_mod_dict = {'No reuse': 0, 'Reuse': 100}
        self.modifyOperandChoices('Read after read address reuse', reuse_mod_dict)
        self.modifyOperandChoices('Read after write address reuse', reuse_mod_dict)
        self.modifyOperandChoices('Write after read address reuse', reuse_mod_dict)
        self.modifyOperandChoices('Write after write address reuse', reuse_mod_dict)
        self.commitSet()


class AlignedDataOnlyChoicesModifier(ChoicesModifier):

    def apply(self, **kwargs):
        data_align_mod_dict = {'Aligned': 100, 'Unaligned': 0, 'Whole data aligned': 0}
        self.modifyOperandChoices('Data alignment', data_align_mod_dict)
        self.commitSet()


class UnalignedDataOnlyChoicesModifier(ChoicesModifier):

    def apply(self, **kwargs):
        data_align_mod_dict = {'Aligned': 0, 'Unaligned': 100, 'Whole data aligned': 0}
        self.modifyOperandChoices('Data alignment', data_align_mod_dict)
        self.commitSet()


class DataProcessingAddressResultChoicesModifier(ChoicesModifier):

    def apply(self, **kwargs):
        data_proc_result_mod_dict = {'Load store address': 100, 'Random': 0}
        self.modifyOperandChoices('Data processing result', data_proc_result_mod_dict)
        self.commitSet()


## NearBranchOnlyChoicesModifier alters the branch choices to always select near branches.
class NearBranchOnlyChoicesModifier(ChoicesModifier):

    def apply(self, **kwargs):
        near_branch_mod_dict = {'Near branch': 100, 'Long branch': 0}
        self.modifyOperandChoices('Branch type choice', near_branch_mod_dict)
        self.commitSet()


class NoSpXzrChoicesModifier(ChoicesModifier):

    def apply(self, **kwargs):
        gprs_or_sp_mod_dict = {'SP': 0}
        self.modifyOperandChoices('64-bit GPRs or SP', gprs_or_sp_mod_dict)
        gprs_mod_dict = {'XZR': 0}
        self.modifyOperandChoices('64-bit GPRs', gprs_mod_dict)
        self.commitSet()
