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
# classes, code related to Eret preamble sequence.

from base.Sequence import Sequence

# -------------------------------------------------------------------------------------------------------
# EretPreambleSequence to provide base class for eret preamble sequence.
# -------------------------------------------------------------------------------------------------------


class EretPreambleSequence(Sequence):
    def __init__(self, gen_thread):
        super().__init__(gen_thread)

    def generate(self, **kargs):
        pass
