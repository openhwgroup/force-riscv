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
    def generate(self, **kargs):
        self.initializeRegister("mscratch", 0x123)
        readValue, validCheck = self.readRegister("mscratch")

        self.writeRegister("mscratch", 0x4567)
        readValue, validCheck = self.readRegister("mscratch")


# Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

# Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

# Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
