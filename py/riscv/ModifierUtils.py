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
import Log
import RandomUtils

class PageFaultModifier(ChoicesModifier):

    def __init__(self, aGenThread):
        super().__init__(aGenThread, 'PageFaultModifier')
        self._mFaultTypes = [
                'Invalid Descriptor',
                #'Va Address Error',
                #'Invalid DA',
                'Misaligned Superpage',
                #'Invalid XWR',
                ]

    def update(self, **kwargs):
        level = 0
        weight = 100
        privilege = 'S'

        if 'All' in kwargs:
            self.updateAllPageFaultChoices()
        else:
            if 'Level' in kwargs:
                level = kwargs['Level']
            else:
                level = RandomUtils.random32(0,3)

            if 'Privilege' in kwargs:
                privilege = kwargs['Privilege']

            if 'Weight' in kwargs:
                if kwargs['Weight'] > 100 or kwargs['Weight'] < 0:
                   Log.error('invalid weight specified, must be in range [0, 100] weight={}'.format(kwargs['Weight']))
                weight = kwargs['Weight']

            if 'Type' in kwargs:
                if kwargs['Type'] in self._mFaultTypes:
                    self.updatePageFaultChoices(level, privilege, weight, kwargs['Type'])
                else:
                    Log.error('invalid type specified, type={}'.format(kwargs['Type']))
            else:
                self.updateAllTypePageFaultChoices(level, privilege, weight)

    def updatePageFaultChoices(self, aLevel, aPriv, aWeight, aType):
        choice_name = '{}#level {}#{}#stage 1'.format(aType, aLevel, aPriv)
        choice_dict = {'false':100-aWeight, 'true':aWeight}
        self.modifyPagingChoices(choice_name, choice_dict)

    def updateAllTypePageFaultChoices(self, aLevel, aPriv, aWeight):
        for fault_type in self._mFaultTypes:
            self.updatePageFaultChoices(aLevel, aPriv, aWeight, fault_type)

    def updateAllPageFaultChoices(self):
        for level in range(4):
            for privilege in ['S']: #,'U']
                self.updateAllTypePageFaultChoices(level, privilege, 100)
