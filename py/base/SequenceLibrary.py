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
        if not self.seqList:
            # the sequence list is not created
            return None
        name, path = self._pickWeighted()
        if name is not None:
            myClass = self._getClass(name, path)
            if myClass is not None:
                if issubclass(myClass, Sequence):
                    key = name + path
                    if key not in self.seqDict:
                        mySeq = self._generateSeqInstance(myClass)
                        self.seqDict[key] = mySeq
                        return mySeq
                    return self.seqDict[key]
                elif issubclass(myClass, SequenceLibrary):
                    mySeqLib = self._generateSeqLibInstance(myClass)
                    return mySeqLib.chooseOne()
                return None
            return None
        return None

    # grab one from permutated sequence list, one at a time
    def getPermutated(self, skip_weight_check=False):
        shuffle_list = []
        for i, item in enumerate(self.seqList):
            if skip_weight_check or self.seqList[i][3] > 0:
                shuffle_list.append(i)
        new_list = UtilityFunctions.shuffle_list(shuffle_list)
        permutated_list = []
        for i, item in enumerate(new_list):
            name = self.seqList[item][0]
            path = self.seqList[item][1]
            myClass = self._getClass(name, path)
            if myClass is not None:
                if issubclass(myClass, Sequence):
                    key = name + path
                    if key not in self.seqDict:
                        mySeq = self._generateSeqInstance(myClass)
                        self.seqDict[key] = mySeq
                        permutated_list.append(mySeq)
                    else:
                        permutated_list.append(self.seqDict[key])
                elif issubclass(myClass, SequenceLibrary):
                    seq_lib = self._generateSeqLibInstance(myClass)
                    for seq in seq_lib.getPermutated(skip_weight_check):
                        yield seq
        for seq in permutated_list:
            yield seq

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

    # generaate the sequence instance from given sequence class reference
    def _generateSeqInstance(self, seq_ref):
        myInstance = seq_ref(self.genThread)
        return myInstance

    # generate sequence library from given sequence library class reference
    def _generateSeqLibInstance(self, seq_lib_ref):
        myInstance = seq_lib_ref(self.genThread)
        return myInstance
