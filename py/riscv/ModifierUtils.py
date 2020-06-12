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
from base.ChoicesModifier import ChoicesModifier

class PageFaultModifier(ChoicesModifier):

    def update(self, **kwargs):
        if 'All' in kwargs:
            self.updatePageFaultsAll()
        else:
            level = 0
            if 'Level' in kwargs:
                level = kwargs['Level']
            privilege = 'S'
            if 'Privilege' in kwargs:
                privilege = kwargs['Privilege']
            weight = 100
            if 'Weight' in kwargs:
                weight = kwargs['Weight']
            self.updatePageFaultChoices(level, privilege, weight)

    def updatePageFaultChoices(self, level, priv, weight):
        choice_name = 'Translation fault#level {}#{}#stage 1'.format(level, priv)
        choice_dict = {'false':100-weight, 'true':weight}
        self.modifyPagingChoices(choice_name, choice_dict)

    def updatePageFaultsAll(self):
        for level in range(4):
            for priv in ['S', 'U']:
               self.updatePageFaultChoices(level, priv, 100)
