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
            return self.__class__(self._mSortableName + aOther.mSortableName, newDict)
        else:
            return NotImplemented

    def __sub__(self, aOther):
        if self.isCompatible(aOther):
            return self.substract(aOther)
        else:
            return NotImplemented

    def itemType(self):
        raise NotImplementedError

    def isCompatible(self, aOther):
        return isinstance(aOther, ItemMap)

    # Return a string description of the ItemMape
    def __str__(self):
        return "%sMap(%s)" % (self.itemType(), self._mSortableName)

    def toSimpleString(self):
        simple_string = " %s map name: " % self.itemType() + self._mSortableName + "\n"
        if len(self.mItemDict) == 0:
            raise Exception("Empty %s map %s", (self.itemType(), self._mSortableName))

        for k, v in self.mItemDict.items():
            simple_string += self._getItemString(k, v)

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
                raise TypeError

    def clone(self, aIdSet):
        sort_name = self._mSortableName
        item_dict = self.deepCopyDict(aIdSet)
        if len(item_dict) == 0:
            return None

        return self.__class__(sort_name, item_dict)

    def deepCopyDict(self, aIdSet):
        dict_copy = {}
        for k, v in self.mItemDict.items():
            (k_copy, v_copy) = self._deepCopyDictItem(k, v, aIdSet)

            if (k_copy is not None) and (v_copy is not None):
                dict_copy[k_copy] = v_copy

        return dict_copy

    def size(self, skip_weight_check=False):
        get_item_size = self._getItemSizeIfWeighted
        if skip_weight_check:
            get_item_size = self._getItemSize

        size = functools.reduce(
            lambda total, item: total + get_item_size(key, val),
            self.mItemDict.items(),
        )

    def _getItemString(self, key, val):
        simple_string = ""
        if isinstance(key, str):
            simple_string = " (key:value) = (%s : %d)" % (key, val)
        elif isinstance(key, ItemMap):
            simple_string = key.toSimpleString()
        else:
            raise TypeError

        return simple_string

    def _deepCopyDictItem(self, key, val, id_set):
        (key_copy, val_copy) = (None, None)
        if isinstance(key, ItemMap):
            (key_copy, val_copy) = self._deepCopyDictItemMap(key, val, id_set)
        elif isinstance(key, str):
            (key_copy, val_copy) = self._deepCopyDictString(key, val, id_set)
        else:
            raise TypeError

        return (key_copy, val_copy)

    def _getItemSize(self, key, val):
        size = 0
        if isinstance(key, ItemMap):
            size = key.size(True)
        elif isinstance(key, str):
            size = 1
        else:
            raise TypeError

        return size

    def _getItemSizeIfWeighted(self, key, val):
        size = 0
        if isinstance(key, ItemMap):
            size = key.size(False)
        elif isinstance(key, str):
            if val > 0:
                size = 1
        else:
            raise TypeError

        return size

    def _deepCopyDictItemMap(self, item_map, val, id_set):
        cloned_map = key.clone(id_set)
        if cloned_map:
            return (cloned_map, val)

        return (None, None)

    def _deepCopyDictString(self, string, val, id_set):
        if string in id_set:
            return (string, val)

        return (None, None)


#  A thin wrapper around an instruction dict, so it can be used as a dict key
#
class RegisterMap(ItemMap):
    def __init__(self, aName, aItemDict):
        super().__init__(aName, aItemDict)

    def itemType(self):
        return "Register"

    def isCompatible(self, aOther):
        return isinstance(aOther, RegisterMap)
