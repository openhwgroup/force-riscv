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
from Constraint import ConstraintSet
from EnumsRISCV import EPagingMode
import MemoryTraits
import RandomUtils
import UtilityFunctions
import VirtualMemory

# This test verifies that memory attributes are preserved when generating virtual addresses for
# physical addresses with associated memory attributes.
class MainSequence(Sequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mMemAttrRanges = {}

    def generate(self, **kargs):
        for _ in range(50):
            pa_size_bits = RandomUtils.random32(1, 20)
            pa_size = 2 ** pa_size_bits
            pa_align_bits = RandomUtils.random32(1, pa_size_bits)
            pa_align = 2 ** pa_align_bits

            amo_mem_attr = self.choice(("AMONone", "AMOSwap", "AMOLogical", "AMOArithmetic"))
            coherency_mem_attr = self.choice(
                ("CoherentL1", "CoherentL2", "CoherentL3", "Incoherent")
            )
            idempotency_mem_attr = self.choice(("ReadIdempotent", "ReadNonIdempotent"))
            arch_mem_attributes = "%s,%s,%s" % (
                amo_mem_attr,
                coherency_mem_attr,
                idempotency_mem_attr,
            )

            impl_mem_attributes = self.choice(("Debug", "CLINT", "PLIC", "GPIO"))

            # Constrain the PA to be in the valid VA range, so flat mapping doesn't fail
            pa_range = "0x0-0x7fffffffffff"
            if VirtualMemory.getPagingMode() == EPagingMode.Sv39:
                pa_range = "0x0-0x3fffffffff"

            start_addr = self.genPA(
                Size=pa_size,
                Align=pa_align,
                Type="D",
                MemAttrArch=arch_mem_attributes,
                MemAttrImpl=impl_mem_attributes,
                Range=pa_range,
            )
            end_addr = start_addr + pa_size - 1
            self._recordMemoryAttributes(arch_mem_attributes, start_addr, end_addr)
            self._recordMemoryAttributes(impl_mem_attributes, start_addr, end_addr)

            pa_for_va = RandomUtils.random64(start_addr, end_addr)
            va_size = RandomUtils.random32(1, (end_addr - pa_for_va + 1))
            va_align_bits = RandomUtils.random32(0, (va_size.bit_length() - 1))
            va_align = 2 ** va_align_bits
            pa_for_va = UtilityFunctions.getAlignedValue(pa_for_va, va_align)
            va_size = UtilityFunctions.getAlignedValue(va_size, va_align)

            self.genVAforPA(
                PA=pa_for_va,
                Size=va_size,
                Type="D",
            )

        self._verifyMemoryAttributes()

    def _recordMemoryAttributes(self, mem_attributes, start_addr, end_addr):
        for arch_mem_attr in mem_attributes.split(","):
            addr_ranges = self._mMemAttrRanges.setdefault(arch_mem_attr, ConstraintSet())
            addr_ranges.addRange(start_addr, end_addr)

    # Verify memory attributes are assigned to the specified ranges, but not beyond.
    def _verifyMemoryAttributes(self):
        for (mem_attr, addr_ranges) in self._mMemAttrRanges.items():
            for addr_range in addr_ranges.getConstraints():
                if not MemoryTraits.hasMemoryAttribute(
                    mem_attr, addr_range.lowerBound(), addr_range.upperBound()
                ):
                    self.error(
                        "Memory attribute %s not assigned to physical address range 0x%x-0x%x"
                        % (mem_attr, addr_range.lowerBound(), addr_range.upperBound())
                    )

                invalid_end_addr = addr_range.upperBound() + 1
                if MemoryTraits.hasMemoryAttribute(
                    mem_attr, addr_range.lowerBound(), invalid_end_addr
                ):
                    self.error(
                        "Memory attribute %s erroneously assigned to physical address 0x%x"
                        % (mem_attr, invalid_end_addr)
                    )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
