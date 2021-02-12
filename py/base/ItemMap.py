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


#  A thin wrapper around an item dict, so that it can be used as a dict key
#
class ItemMap(SortableObject):
    def __init__(self, aName, aItemDict):
        super().__init__()
        self.mItemDict = aItemDict
        self._mSortableName = aName

    def pick(self, aGenThread):
        return aGenThread.pickWeighted(self.mItemDict)

    def getPermutated(self, aGenThread, skip_weight_check=False):
        return aGenThread.getPermutated(self.mItemDict, skip_weight_check)

    def __add__(self, aOther):
        if self.isCompatible(aOther):
            newDict = {k: v for k, v in self.mItemDict.items()}
            newDict.update(aOther.mItemDict)
            return self.__class__(
                self._mSortableName + aOther.mSortableName, newDict
            )
        else:
            return NotImplemented

    def __sub__(self, aOther):
        if self.isCompatible(aOther):
            return self.substract(aOther)
        else:
            return NotImplemented

    def itemType(self):
        return NotImplemented

    def isCompatible(self, aOther):
        return isinstance(aOther, ItemMap)

    # Return a string description of the ItemMape
    def __str__(self):
        return "%sMap(%s)" % (self.itemType(), self._mSortableName)

    def toSimpleString(self):
        simple_string = (
            " %s map name: " % self.itemType() + self._mSortableName + "\n"
        )
        if len(self.mItemDict) == 0:
            raise Exception(
                "Empty %s map %s", (self.itemType(), self._mSortableName)
            )
        for k, v in self.mItemDict.items():
            if isinstance(k, str):
                simple_string += " (key:value) = (%s : %d)" % (k, v)
            elif isinstance(k, ItemMap):
                simple_string += k.toSimpleString()
            else:
                return NotImplemented
        return simple_string

    def substract(self, aOther):
        self_set = set()
        self.getItemIdSet(self_set)
        other_set = set()
        aOther.getItemIdSet(other_set)
        sub_set = self_set - other_set
        if len(sub_set) == 0:
            return None
        if sub_set == self_set:
            return self
        return self.clone(sub_set)

    # get all item IDs and push them to set
    def getItemIdSet(self, aIdSet):
        for k, v in self.mItemDict.items():
            if isinstance(k, str):
                aIdSet.add(k)
            elif isinstance(k, ItemMap):
                k.getItemIdSet(aIdSet)
            else:
                return NotImplemented

    def clone(self, aIdSet):
        sort_name = self._mSortableName
        item_dict = self.deepCopyDict(aIdSet)
        if len(item_dict) == 0:
            return None

        return self.__class__(sort_name, item_dict)

    def deepCopyDict(self, aIdSet):
        copy_dict = {}
        for k, v in self.mItemDict.items():
            if isinstance(k, ItemMap):
                cloned_map = k.clone(aIdSet)
                if cloned_map:
                    copy_dict[cloned_map] = v
            elif isinstance(k, str):
                if k in aIdSet:
                    copy_dict[k] = v
            else:
                return NotImplemented

        return copy_dict

    def size(self, skip_weight_check=False):
        size = 0
        for k, v in self.mItemDict.items():
            if isinstance(k, ItemMap):
                size += k.size(skip_weight_check)
            elif isinstance(k, str):
                if skip_weight_check or (v > 0):
                    size += 1
            else:
                return NotImplemented

        return size


#  A thin wrapper around an instruction dict, so it can be used as a dict key
#
class RegisterMap(ItemMap):
    def __init__(self, aName, aItemDict):
        super().__init__(aName, aItemDict)

    def itemType(self):
        return "Register"

    def isCompatible(self, aOther):
        return isinstance(aOther, RegisterMap)
