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
#  Choices Modifier class
#  Base ChoicesModifier class, pass on API calls to GenThread class


class ChoicesModifier(object):
    def __init__(self, gen_thread, name=None):
        self.genThread = gen_thread
        if name:
            self.name = name
        else:
            self.name = self.__class__.__name__

        # print("choices modifier name %s" % self.name)
        self.modifyChoices = list()
        self.commitChoices = list()
        self.commitId = 0
        self.registerId = 0

    def modifyOperandChoices(self, tree_name, mod_dict):
        setId = self.genThread.addChoicesModification(0, tree_name, mod_dict)
        if (setId, 0) not in self.modifyChoices:
            self.modifyChoices.append((setId, 0))

    def modifyRegisterFieldValueChoices(self, tree_name, mod_dict):
        setId = self.genThread.addChoicesModification(1, tree_name, mod_dict)
        if (setId, 1) not in self.modifyChoices:
            self.modifyChoices.append((setId, 1))

    def modifyPagingChoices(self, tree_name, mod_dict):
        setId = self.genThread.addChoicesModification(2, tree_name, mod_dict)
        if (setId, 2) not in self.modifyChoices:
            self.modifyChoices.append((setId, 2))

    def modifyGeneralChoices(self, tree_name, mod_dict):
        setId = self.genThread.addChoicesModification(3, tree_name, mod_dict)
        if (setId, 3) not in self.modifyChoices:
            self.modifyChoices.append((setId, 3))

    def modifyDependenceChoices(self, tree_name, mod_dict):
        setId = self.genThread.addChoicesModification(4, tree_name, mod_dict)
        if (setId, 4) not in self.modifyChoices:
            self.modifyChoices.append((setId, 4))

    # record the set type and ID commited
    def commitSet(self):
        self.commitId = len(self.commitChoices)
        for (set_id, my_type) in self.modifyChoices:
            self.genThread.commitModificationSet(my_type, set_id)
            self.commitChoices.append((set_id, my_type))
        self.modifyChoices.clear()
        return self.commitId

    def registerSet(self):
        if len(self.modifyChoices) == 0:
            self.genThread.error("no choices modification to register")
        self.registerId, reg_type = self.modifyChoices[0]
        for (setId, my_type) in self.modifyChoices:
            self.genThread.registerModificationSet(my_type, setId)
        self.modifyChoices.clear()
        return self.registerId

    # main entrance for sub class to call various modify function
    def apply(self, **kwargs):
        self.update(**kwargs)
        if len(self.modifyChoices):
            return self.commitSet()

    # register all modifications
    def register(self, **kwargs):
        self.update(**kwargs)
        if len(self.modifyChoices):
            return self.registerSet()

    def update(self, **kwargs):
        self.genThread.error("unimplemented method: update()")

    # revert all modification set applied
    def revert(self, commitId=None):
        if commitId is None:
            commitId = self.commitId
        while len(self.commitChoices) > commitId:
            (setId, my_type) = self.commitChoices.pop()
            self.genThread.revertModificationSet(my_type, setId)

    # get choices tree info, choices tree type: OperandChoices,
    # RegisterFieldValueChoices, PagingChoices, GeneralChoices,
    # DependenceChoices and so on
    def getChoicesTreeInfo(self, tree_name, tree_type):
        return self.genThread.getChoicesTreeInfo(tree_name, tree_type)
