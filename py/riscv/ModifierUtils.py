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
from base.ChoicesModifier import ChoicesModifier
import Log
import RandomUtils


class PageFaultModifier(ChoicesModifier):
    def __init__(self, aGenThread, aAppRegWidth):
        super().__init__(aGenThread, "PageFaultModifier")

        self._mValidFaultTypes = [
            "Invalid DA",
            "Invalid U",
            "Invalid X",
            "Invalid WR",
            "Invalid V",
            # 'Va Address Error',
            # 'Misaligned Superpage',
            # 'Last Level Pointer',
        ]

        self._mValidFaultLevels = {
            "Invalid DA": [0, 1, 2, 3],
            "Invalid U": [0, 1, 2, 3],
            "Invalid X": [0, 1, 2, 3],
            "Invalid WR": [0, 1, 2, 3],
            "Invalid V": [3, 2, 1, 0],
            # 'Va Address Error':[3,2,1,0]
            # 'Misaligned Superpage':[3,2,1],
            # 'Last Level Pointer':[0],
        }

        if aAppRegWidth == 32:
            # Sv32...
            self._mValidFaultLevels = {
                "Invalid DA": [0, 1],
                "Invalid U": [0, 1],
                "Invalid X": [0, 1],
                "Invalid WR": [0, 1],
                "Invalid V": [1, 0],
                # 'Va Address Error':[3,2,1,0]
                # 'Misaligned Superpage':[3,2,1],
                # 'Last Level Pointer':[0],
            }

        self._mValidPrivilegeLevels = {
            "Invalid DA": ["S"],
            "Invalid U": ["S"],
            "Invalid X": ["S"],
            "Invalid WR": ["S"],
            "Invalid V": ["S"],
            # 'Va Address Error':['S']
            # 'Misaligned Superpage':['S'],
            # 'Last Level Pointer':['S'],
        }

    def update(self, **kwargs):
        if "All" in kwargs:
            self.updateAllFaultChoices()
        else:
            if "Type" in kwargs:
                if kwargs["Type"] in self._mValidFaultTypes:
                    self.updateFaultTypeChoices(kwargs["Type"], **kwargs)
                else:
                    Log.error(
                        "invalid type specified, type={}".format(
                            kwargs["Type"]
                        )
                    )
            else:
                Log.error(
                    "specify All or fault name as kwarg to update choices."
                )

    def modifyExceptionRegulation(self):
        choice_dict = {"Prevent": 100, "Allow": 1, "Trigger": 0}
        self.modifyPagingChoices("InstructionPageFault#S#stage 1", choice_dict)
        self.modifyPagingChoices("LoadPageFault#S#stage 1", choice_dict)
        self.modifyPagingChoices("StoreAmoPageFault#S#stage 1", choice_dict)

    def updatePageFaultChoice(self, aType, aLevel, aPriv, aWeight):
        choice_name = "{}#level {}#{}#stage 1".format(aType, aLevel, aPriv)
        choice_dict = {"false": 100 - aWeight, "true": aWeight}
        self.modifyPagingChoices(choice_name, choice_dict)

    def updateFaultTypeChoices(self, aType, **kwargs):
        table_levels = None
        priv_levels = None
        weight = 0

        if "Level" in kwargs:
            table_levels = kwargs["Level"]
        else:
            table_levels = self._mValidFaultLevels[aType]

        if "Privilege" in kwargs:
            priv_levels = kwargs["Privilege"]
        else:
            priv_levels = self._mValidPrivilegeLevels[aType]

        if "Weight" in kwargs:
            if kwargs["Weight"] not in range(101):
                Log.error(
                    "invalid weight specified, please use integer between "
                    "0-100. weight={}".format(kwargs["Weight"])
                )

            weight = kwargs["Weight"]
        else:
            weight = 100

        for table_level in table_levels:
            if table_level not in self._mValidFaultLevels[aType]:
                Log.error(
                    "invalid table level={} for fault type={}".format(
                        table_level, aType
                    )
                )
            for priv_level in priv_levels:
                if priv_level not in self._mValidPrivilegeLevels[aType]:
                    Log.error(
                        "invalid priv level={} for fault type={}".format(
                            priv_level, aType
                        )
                    )

                self.updatePageFaultChoice(
                    aType, table_level, priv_level, weight
                )

        self.modifyExceptionRegulation()

        if "All" in kwargs:
            # ensure we can get both superpages/full walks for last level
            # ptr + misaligned superpage
            self.updateSuperpageSizeChoices(50)
        else:
            if aType == "Misaligned Superpage":
                self.updateSuperpageSizeChoices(
                    100
                )  # needs superpage descriptor
            elif aType == "Last Level Pointer":
                self.updateSuperpageSizeChoices(
                    0
                )  # needs level 0 (4K) table descriptor

    def updateAllFaultChoices(self, **kwargs):
        for fault_type in self._mValidFaultTypes:
            self.updateFaultTypeChoices(fault_type, **kwargs)

    def updateSuperpageSizeChoices(self, aWeight):
        choice_name = "Page size#4K granule#S#stage 1"
        choice_dict = {
            "4K": 101 - aWeight,
            "2M": aWeight,
            "1G": aWeight,
            "512G": 0,
        }
        self.modifyPagingChoices(choice_name, choice_dict)


class TrapsRedirectModifier(ChoicesModifier):
    def __init__(self, aGenThread):
        super().__init__(aGenThread, "TrapsRedirectModifier")

        self.mSupportedExceptions = {
            "Instruction address misaligned": 1,
            "Instruction access fault": 1,
            "Illegal instruction": 1,
            "Breakpoint": 1,
            "Load address misaligned": 1,
            "Load access fault": 1,
            "Store/AMO address misaligned": 1,
            "Store/AMO access fault": 1,
            "Environment call from U-mode": 1,
            "Environment call from S-mode": 1,
            "Instruction page fault": 1,
            "Load page fault": 1,
            "Store/AMO page fault": 1,
        }

        self.mHaveMods = False

    def update(self, **kwargs):
        try:
            if "Weight" in kwargs:
                self.updateChoices(
                    kwargs["ExceptionCode"],
                    kwargs["TrapChoice"],
                    kwargs["Weight"],
                )
            else:
                self.updateChoices(
                    kwargs["ExceptionCode"], kwargs["TrapChoice"]
                )
        except KeyError:
            Log.error(
                "TrapDelegationRedirectionModifier: 'ExceptionCode' or "
                "'TrapChoice' arguments missing."
            )

    def updateChoices(self, aExceptionCode, aTrapChoice, aWeight):
        try:
            rcode = self.mSupportedExceptions[aExceptionCode]
        except KeyError:
            Log.error(
                "TrapDelegationRedirectionModifier: ExceptionCode '%s' is "
                "not supported.",
                aExceptionCode,
            )

        if aTrapChoice == "Delegate":
            self.delegateException(aExceptionCode, aWeight)
        elif aTrapChoice == "Redirect":
            self.redirectException(aExceptionCode, aWeight)
        else:
            Log.error(
                "TrapDelegationRedirectionModifier: TrapChoice '%s' not "
                "recognized" % aTrapChoice
            )

    def delegateException(self, aExceptionCode, aWeight=100):
        my_choice = "medeleg.{}".format(aExceptionCode)
        Log.notice("delegation choice:{}".format(my_choice))
        weightDict = {"0x0": 100 - aWeight, "0x1": aWeight}
        self.modifyRegisterFieldValueChoices(my_choice, weightDict)
        self.mHaveMods = True

    def redirectException(self, aExceptionCode, aWeight=100):
        my_choice = "Redirect Trap - {}".format(aExceptionCode)
        Log.notice("redirect choice:{}".format(my_choice))
        weightDict = {"DoNotRedirect": 100 - aWeight, "DoRedirect": aWeight}
        self.modifyGeneralChoices(my_choice, weightDict)
        self.mHaveMods = True

    def commit(self):
        if self.mHaveMods:
            self.commitSet()
