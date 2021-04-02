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
from base.SortableObject import SortableObject


# Provide a FunctionWrapper to wrap a non-sortable object
# so it can be sortable
class FunctionWrapper(SortableObject):
    def __init__(self, function):
        super().__init__()
        self.function = function
        self.sortableName = str(function)

    def returnFunction(self):
        return self.function

    def display(self):
        print("function name=%s" % self.sortableName)
