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
        self._mValidFaultTypes = [
                'Invalid Descriptor',
                'Misaligned Superpage',
                #'Last Level Pointer',
                'Invalid DA',
                'Invalid XWR',
                #'Invalid U',
                #'Va Address Error',
                ]

        self._mValidFaultLevels = {
                'Invalid Descriptor':[3,2,1,0],
                'Misaligned Superpage':[3,2,1],
                'Last Level Pointer':[0],
                'Invalid DA':[0,1,2,3],
                'Invalid XWR':[0,1,2,3],
                'Invalid U':[0,1,2,3],
                }

        self._mValidPrivilegeLevels = {
                'Invalid Descriptor':['S'],
                'Misaligned Superpage':['S'],
                'Last Level Pointer':['S'],
                'Invalid DA':['S'],
                'Invalid XWR':['S'],
                'Invalid U':['S'],
                }


    def update(self, **kwargs):
        if 'All' in kwargs:
            self.updateAllFaultChoices()
        else:
            if 'Type' in kwargs:
                if kwargs['Type'] in self._mValidFaultTypes:
                    self.updateFaultTypeChoices(kwargs['Type'], **kwargs)
                else:
                    Log.error('invalid type specified, type={}'.format(kwargs['Type']))
            else:
                Log.error('specify All or fault name as kwarg to update choices.')

    def updatePageFaultChoice(self, aType, aLevel, aPriv, aWeight):
        choice_name = '{}#level {}#{}#stage 1'.format(aType, aLevel, aPriv)
        choice_dict = {'false':100-aWeight, 'true':aWeight}
        self.modifyPagingChoices(choice_name, choice_dict)

    def updateFaultTypeChoices(self, aType, **kwargs):
        table_levels = None
        priv_levels = None
        weight = 0

        if 'Level' in kwargs:
            table_levels = kwargs['Level']
        else:
            table_levels = self._mValidFaultLevels[aType]

        if 'Privilege' in kwargs:
            priv_levels = kwargs['Privilege']
        else:
            priv_levels = self._mValidPrivilegeLevels[aType]

        if 'Weight' in kwargs:
            if kwargs['Weight'] not in range(101):
                Log.error('invalid weight specified, please use integer between 0-100. weight={}'.format(kwargs['Weight']))
            
            weight = kwargs['Weight']
        else:
            weight = 100

        for table_level in table_levels:
            if table_level not in self._mValidFaultLevels[aType]:
                Log.error('invalid table level={} for fault type={}'.format(table_level, aType))
            for priv_level in priv_levels:
                if priv_level not in self._mValidPrivilegeLevels[aType]:
                    Log.error('invalid priv level={} for fault type={}'.format(priv_level, aType))

                self.updatePageFaultChoice(aType, table_level, priv_level, weight)

        if 'All' in kwargs:
            #ensure we can get both superpages/full walks for last level ptr + misaligned superpage
            self.updateSuperpageSizeChoices(50)
        else:
            if aType == 'Misaligned Superpage':
                self.updateSuperpageSizeChoices(100) #needs superpage descriptor
            elif aType == 'Last Level Pointer':
                self.updateSuperpageSizeChoices(0) #needs level 0 (4K) table descriptor 

    def updateAllFaultChoices(self, **kwargs):
        for fault_type in self._mValidFaultTypes:
            self.updateFaultTypeChoices(fault_type, **kwargs)

    def updateSuperpageSizeChoices(self, aWeight):
        choice_name = 'Page size#4K granule#S#stage 1'
        choice_dict = {'4K':101-aWeight, '2M':aWeight, '1G':aWeight, '512G':aWeight}
        self.modifyPagingChoices(choice_name, choice_dict)
