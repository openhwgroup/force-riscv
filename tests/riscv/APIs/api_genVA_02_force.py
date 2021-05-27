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
    """Exercise the range parameter of genVA."""

    def generate(self, **kargs):

        ldstr_byte_ops = ["LB##RISCV", "SB##RISCV"]
        ldstr_half_ops = ["LH##RISCV", "SH##RISCV"]
        ldstr_word_ops = ["LW##RISCV", "SW##RISCV"]
        ldstr_double_ops = ["LD##RISCV", "SD##RISCV"]

        theSize = 16
        theAlign = 16
        theFlatMap = 1
        theBank = 0
        theType = "D"

        theAddressRange = "0x1F00-0x1FFF"

        if self.getGlobalState("AppRegisterWidth") == 32:
            ldstr_double_ops = ldstr_word_ops

        # Validate all of the range can be returned.  For range 0x1F00-0x1FFF,
        # 16 requests for 16 bytes each are possible.
        for _ in range(16):

            self.notice(
                ">>>>>>>>>>>  Size= {}     Align= {}     Type= {}     "
                "Bank= {}     Flatmap= {}     Range= {}".format(
                    theSize, theAlign, "D", 0, 1, theAddressRange
                )
            )
            rand_VA = self.genVA(
                Size=theSize,
                Align=theAlign,
                Type="D",
                Bank=0,
                FlatMap=1,
                Range=theAddressRange,
            )

            self.notice(
                ">>>>>> Requested Address Range:  {:<30s}       Returned "
                "VA:  {:016X}".format(theAddressRange, rand_VA)
            )

            instr_id = self.genInstruction(self.choice(ldstr_double_ops), {"LSTarget": rand_VA})

        # Try different address ranges
        for _ in range(50):

            theAddressRange = ["0x60200-0x611FF"]

            if self.getGlobalState("AppRegisterWidth") == 32:
                theAddressRange += ["0xF2200000-0xFFEFFFFF"]
                theAddressRange += ["0xFFF00000-0xFFFFFFFF"]
                theAddressRange += ["0x1000-0x1FFF,0x88000-0x89800"]
            else:
                theAddressRange += ["0xF220000000-0xFFFFFFFFFF"]
                theAddressRange += ["0xFFF00000000-0xFFFFFFFFFFF"]
                theAddressRange += ["0x1000-0x1FFF,0x88000-0x89800,0xFFF00000000-0xFFFFFFFFFFF"]

            for addrRange in theAddressRange:

                self.notice(
                    ">>>>>>>>>>>  Size= {}     Align= {}     Type= {}     "
                    "Bank= {}     Flatmap= {}     Range= {}".format(
                        theSize, theAlign, "D", "Default", 1, addrRange
                    )
                )

                rand_VA = self.genVA(
                    Size=theSize,
                    Align=theAlign,
                    Type="D",
                    Bank="Default",
                    FlatMap=1,
                    Range=addrRange,
                )
                self.notice(
                    ">>>>>> Requested Address Range:  {:<30s}       Returned "
                    "VA:  {:016X}".format(addrRange, rand_VA)
                )

                instr_id = self.genInstruction(
                    self.choice(ldstr_double_ops), {"LSTarget": rand_VA}
                )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
