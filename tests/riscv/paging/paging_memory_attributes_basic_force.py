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
import MemoryTraits
import RandomUtils

# This test verifies that memory attributes can be set on specific regions of memory.
class MainSequence(Sequence):
    def generate(self, **kargs):
        for _ in range(50):
            size_bits = RandomUtils.random32(1, 20)
            size = 2 ** size_bits
            align_bits = RandomUtils.random32(1, size_bits)
            align = 2 ** align_bits
            arch_mem_attr = self.choice(
                ("MainRegion", "IORegion", "CacheableShared", "Uncacheable")
            )
            impl_mem_attr = self.choice(("DMA Controller", "UART 0", "UART 1", "DDR Control"))

            start_addr = self.genPA(
                Size=size,
                Align=align,
                Type="D",
                MemAttrArch=arch_mem_attr,
                MemAttrImpl=impl_mem_attr,
            )
            end_addr = start_addr + size - 1

            if not MemoryTraits.hasMemoryAttribute(arch_mem_attr, start_addr, end_addr):
                self.error(
                    "Memory attribute %s not assigned to physical address range 0x%x-0x%x"
                    % (arch_mem_attr, start_addr, end_addr)
                )

            if not MemoryTraits.hasMemoryAttribute(impl_mem_attr, start_addr, end_addr):
                self.error(
                    "Memory attribute %s not assigned to physical address range 0x%x-0x%x"
                    % (impl_mem_attr, start_addr, end_addr)
                )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
