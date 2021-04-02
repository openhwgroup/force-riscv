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
# BootPriority.py
#
# This file defines the BootPriority helper class.


# The boot priority class defines helper methods associated with boot priority
class BootPriority:
    # Returns the appropriate boot priority based on the name and type of
    # register provided along with if the register is write only
    @classmethod
    def getBootPriority(cls, aName=None, aType=None, aWriteOnly=0):
        return 1
