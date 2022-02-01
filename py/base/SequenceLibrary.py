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
#  Sequence Library base class
from base.GenThread import GenThread
from base.Sequence import Sequence
import base.UtilityFunctions as UtilityFunctions
import RandomUtils

import importlib


class SequenceLibrary(object):
    def __init__(self, gen_thread, name=None):
        self.genThread = gen_thread
        self.name = name
        self.seqList = []
        self.seqDict = {}
        self.createSequenceList()

    def createSequenceList(self):
        pass

    # randomly choose a sequence instance
    def chooseOne(self):
        # randomly choose one from the list
        if self.seqList:
            name, path = self._pickWeighted()

            if name is not None:
                return self._chooseSequenceFromName(name, path)

        return None

    # grab one from permutated sequence list, one at a time
    def getPermutated(self, skip_weight_check=False):
        filtered_list = self.seqList
        if not skip_weight_check:
            filtered_list = filter(lambda item: item[3] > 0, filtered_list)

        shuffled_list = UtilityFunctions.shuffle_list(filtered_list)
        for item in shuffled_list:
            name = item[0]
            path = item[1]
            yield from self._getPermutatedFromName(name, path, skip_weight_check)

    #
    # Note: The following methods are called within Sequence Library
    #

    # pick name and path from the seuqence list with weighted random number
    # each item in sequence list has (name, path, description and weight)
    def _pickWeighted(self):
        length = len(self.seqList)
        total_weight = 0
        for i in range(length):
            total_weight += self.seqList[i][3]
        if total_weight <= 0:
            return None, None

        picked_value = RandomUtils.random64(0, total_weight - 1)
        # sort weight in the list
        sorted_list = sorted(self.seqList, key=lambda tup: tup[3])
        for i in range(length):
            if picked_value < sorted_list[i][3]:
                return sorted_list[i][0], sorted_list[i][1]
            picked_value -= sorted_list[i][3]
        return None, None

    def _chooseSequenceFromName(self, name, path):
        myClass = self._getClass(name, path)
        if myClass is not None:
            return self._chooseSequenceFromClass(name, path, myClass)

        return None

    def _getPermutatedFromName(self, name, path, skip_weight_check):
        myClass = self._getClass(name, path)
        if myClass is not None:
            yield from self._getPermutatedFromClass(name, path, myClass, skip_weight_check)

    # import a reference class based on given class name and module path
    def _getClass(self, name, path):
        try:
            module = importlib.import_module(path)
            myClass = getattr(module, name)
            return myClass
        except ImportError:
            # given module path is incorrect
            return None
        except AttributeError:
            # given class name is incorrect
            return None

    def _chooseSequenceFromClass(self, name, path, my_class):
        if issubclass(my_class, Sequence):
            key = name + path
            mySeq = self.seqDict.setdefault(key, self._generateSeqInstance(my_class))
            return mySeq
        elif issubclass(my_class, SequenceLibrary):
            mySeqLib = self._generateSeqLibInstance(my_class)
            return mySeqLib.chooseOne()

        return None

    def _getPermutatedFromClass(self, name, path, my_class, skip_weight_check):
        if issubclass(my_class, Sequence):
            key = name + path
            mySeq = self.seqDict.setdefault(key, self._generateSeqInstance(my_class))
            yield mySeq
        elif issubclass(my_class, SequenceLibrary):
            mySeqLib = self._generateSeqLibInstance(my_class)
            yield from mySeqLib.getPermutated(skip_weight_check)

    # generaate the sequence instance from given sequence class reference
    def _generateSeqInstance(self, seq_ref):
        myInstance = seq_ref(self.genThread)
        return myInstance

    # generate sequence library from given sequence library class reference
    def _generateSeqLibInstance(self, seq_lib_ref):
        myInstance = seq_lib_ref(self.genThread)
        return myInstance
