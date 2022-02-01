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


class MainSequence(Sequence):
    """Exercise different combinations of values for the parameters for
    the genPA instruction. Focus in this test is to try values of the Size,
    Align and CanAlias parameters. Type is always 'D'; Bank is always
    "Default".
    """

    def generate(self, **kargs):

        ldstr_byte_ops = ["LB##RISCV", "SB##RISCV"]
        ldstr_half_ops = ["LH##RISCV", "SH##RISCV"]
        ldstr_word_ops = ["LW##RISCV", "SW##RISCV"]
        ldstr_double_ops = ["LD##RISCV", "SD##RISCV"]

        theType = "D"
        theBank = "Default"
        theCanAlias = 0
        loopCount = 1

        # Iterate through Size and Align values.  Force requires Align to be
        # a power of 2. This 1st block tests smaller values of size -
        # 1 byte to 32 bytes.
        for theSize in [2 ** x for x in range(0, 5)]:

            for theAlign in [2 ** x for x in range(0, 8)]:

                if theAlign < theSize:
                    continue

                for _ in range(loopCount):

                    rand_PA = self.genPA(
                        Size=theSize,
                        Align=theAlign,
                        Type=theType,
                        Bank=theBank,
                        CanAlias=theCanAlias,
                    )
                    rand_VA = self.genVAforPA(
                        PA=rand_PA,
                        Bank=theBank,
                        FlatMap=0,
                        Type=theType,
                        Size=theSize,
                    )
                    self.notice(
                        ">>>>>> Requested Alignment:  {:6d}     Requested "
                        "Size:  {:6d}     PA target= {:16X}     VA target= "
                        "{:16X}".format(theAlign, theSize, rand_PA, rand_VA)
                    )

                    instr_id = self.genInstruction(
                        self.choice(ldstr_byte_ops), {"LSTarget": rand_VA}
                    )

        # Iterate through Size and Align values.  Force requires Align to be
        # a power of 2.
        # This 2nd block tests larger values of size - 32K to 8M.
        for theSize in [2 ** x for x in range(15, 18)]:

            for theAlign in [2 ** x for x in range(15, 18)]:

                if theAlign < theSize:
                    continue

                for _ in range(loopCount):

                    rand_PA = self.genPA(
                        Size=theSize,
                        Align=theAlign,
                        Type=theType,
                        Bank=theBank,
                        CanAlias=theCanAlias,
                    )
                    rand_VA = self.genVAforPA(
                        PA=rand_PA,
                        Bank=theBank,
                        FlatMap=0,
                        CanAlias=0,
                        ForceNewAddress=1,
                        Type=theType,
                        Size=theSize,
                    )
                    self.notice(
                        ">>>>>> Requested Alignment:  {:6d}     Requested "
                        "Size:  {:6d}     PA target= {:16X}     VA target= "
                        "{:16X}".format(theAlign, theSize, rand_PA, rand_VA)
                    )

                    instr_id = self.genInstruction(
                        self.choice(ldstr_byte_ops), {"LSTarget": rand_VA}
                    )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
