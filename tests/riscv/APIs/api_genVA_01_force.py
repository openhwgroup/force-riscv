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
import itertools


class MainSequence(Sequence):
    """Exercise different combinations of values for the parameters for
    the genVA instruction. Focus in this test is to try values of the
    Size, Align, and Flatmap parameters. Type is always 'D'; Bank is
    always "Default".
    """

    def generate(self, **kargs):
        # Test smaller values of size - 1 byte to 32 bytes.
        self._test_gen_va(range(0, 6), range(0, 16), 1)

        # Test larger values of size - 32K to 8M.
        self._test_gen_va(range(15, 24), range(15, 25), 0)

    def _test_gen_va(self, size_exp_range, align_exp_range, flat_map):
        # Iterate through Size and Align values.  Force requires Align to
        # be a power of 2.
        for (theSize, theAlign) in itertools.product(
            [2 ** x for x in size_exp_range], [2 ** x for x in align_exp_range]
        ):
            if theAlign < theSize:
                continue

            self._gen_load_store_instructions(theSize, theAlign, flat_map)

    def _gen_load_store_instructions(self, addr_size, addr_align, flat_map):

        for _ in range(2):
            rand_VA = self.genVA(
                Size=addr_size,
                Align=addr_align,
                Type="D",
                Bank="Default",
                FlatMap=flat_map,
            )
            self.notice(
                ">>>>>> Requested Alignment:  {:6d}     Requested "
                "Size:  {:6d}     gen target VA={:12X}".format(addr_align, addr_size, rand_VA)
            )

            for instr_list in self._get_instruction_lists(addr_align):
                self.genInstruction(self.choice(instr_list), {"LSTarget": rand_VA})

    # Return the lists of instructions applicable for the specified alignment.
    def _get_instruction_lists(self, addr_align):
        ldstr_byte_ops = ("LB##RISCV", "SB##RISCV")
        ldstr_half_ops = ("LH##RISCV", "SH##RISCV")
        ldstr_word_ops = ("LW##RISCV", "SW##RISCV")
        ldstr_double_ops = ("LD##RISCV", "SD##RISCV")

        instr_lists = [ldstr_byte_ops]

        if addr_align % 2 == 0:
            instr_lists.append(ldstr_half_ops)
        if addr_align % 4 == 0:
            instr_lists.append(ldstr_word_ops)
        if (addr_align % 8 == 0) and (self.getGlobalState("AppRegisterWidth") != 32):
            instr_lists.append(ldstr_double_ops)

        return instr_lists


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
