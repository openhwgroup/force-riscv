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
from enum import Enum

class ExceptionClassRISCV(Enum):
    INSTRUCTION_ADDRESS_MISALIGNED = 0x0
    INSTRUCTION_ACCESS_FAULT = 0x1
    ILLEGAL_INSTRUCTION = 0x2
    BREAKPOINT = 0x3
    LOAD_ADDRESS_MISALIGNED = 0x4
    LOAD_ACCESS_FAULT = 0x5
    STORE_AMO_ADDRESS_MISALIGNED = 0x6
    STORE_AMO_ACCESS_FAULT = 0x7
    ENV_CALL_FROM_U_MODE = 0x8
    ENV_CALL_FROM_S_MODE = 0x9
    UNUSED_10 = 0xA
    ENV_CALL_FROM_M_MODE = 0xB
    INSTRUCTION_PAGE_FAULT = 0xC
    LOAD_PAGE_FAULT = 0xD
    UNUSED_14 = 0xE
    STORE_AMO_PAGE_FAULT = 0xF

    TRAP_REDIRECTION = 0x99

    def __lt__(self, other):
        if self.__class__ is other.__class__:
            return self.value < other.value

        return NotImplemented
