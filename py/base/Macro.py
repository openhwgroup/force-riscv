#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from base.SortableObject import SortableObject

#
# Provide a Macro to wrap a non-hashable object (dictionary for example)
# so it can be hashable and comparable (eq, lt, and gt) with other object
#
class Macro(SortableObject):
    def __init__(self, objName, objVal):
        super().__init__()
        self.objName = objName
        self.objVal = objVal
        self.sortableName = self.serialize()
    
    def obj(self):
        return self.objName
        
    def value(self):
        return self.objVal
        
    def serialize(self):
        tmp_str = str(self.objName)
        if isinstance(self.objVal, dict):
            for k,v in sorted(self.objVal.items()):
                if isinstance(k, str):
                    tmp_str += k
                elif isinstance(k, int):
                    tmp_str += str(k)
                else:
                    raise TypeError
                if isinstance(v, str):
                    tmp_str += v
                elif isinstance(v, int):
                    tmp_str += str(v)
        elif isinstance(self.objVal, str):
            tmp_str += self.objVal
        else:
            raise TypeError
        return tmp_str        
