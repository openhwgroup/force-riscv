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
import itertools

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

    def getValidFaultTypes(self):
        return self._mValidFaultTypes

    def getValidFaultLevels(self):
        return self._mValidFaultLevels

    def update(self, **kwargs):
        if "All" in kwargs:
            self.updateAllFaultChoices()
        elif "Type" in kwargs:
            self._updateWithType(kwargs["Type"], **kwargs)
        else:
            Log.error("specify All or fault name as kwarg to update choices.")

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
        table_levels = kwargs.get("Level", self._mValidFaultLevels[aType])
        self._validateTableLevels(aType, table_levels)
        priv_levels = kwargs.get("Privilege", self._mValidPrivilegeLevels[aType])
        self._validatePrivilegeLevels(aType, priv_levels)
        weight = kwargs.get("Weight", 100)
        self._validateWeight(aType, weight)

        for (table_level, priv_level) in itertools.product(table_levels, priv_levels):
            self.updatePageFaultChoice(aType, table_level, priv_level, weight)

        self.modifyExceptionRegulation()

        if "All" in kwargs:
            # ensure we can get both superpages/full walks for last level
            # ptr + misaligned superpage
            self.updateSuperpageSizeChoices(50)
        elif aType == "Misaligned Superpage":
            self.updateSuperpageSizeChoices(100)  # needs superpage descriptor
        elif aType == "Last Level Pointer":
            self.updateSuperpageSizeChoices(0)  # needs level 0 (4K) table descriptor

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

    def _updateWithType(self, aType, **kwargs):
        if aType in self._mValidFaultTypes:
            self.updateFaultTypeChoices(aType, **kwargs)
        else:
            Log.error("invalid type specified, type={}".format(aType))

    def _validateTableLevels(self, aType, aTableLevels):
        for table_level in aTableLevels:
            if table_level not in self._mValidFaultLevels[aType]:
                Log.error("invalid table level={} for fault type={}".format(table_level, aType))

    def _validatePrivilegeLevels(self, aType, aPrivLevels):
        for priv_level in aPrivLevels:
            if priv_level not in self._mValidPrivilegeLevels[aType]:
                Log.error("invalid priv level={} for fault type={}".format(priv_level, aType))

    def _validateWeight(self, aType, aWeight):
        if not (0 <= aWeight <= 100):
            Log.error(
                "invalid weight specified, please use integer between "
                "0-100. weight={}".format(aWeight)
            )


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
                self.updateChoices(kwargs["ExceptionCode"], kwargs["TrapChoice"])
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
                "TrapDelegationRedirectionModifier: ExceptionCode '%s' is " "not supported.",
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


# displayPageInfo
# formats some of the information contained in the page object returned by the self.getPageInfo() API.
# The infomation becomes [notice] records in the gen.log file.
# Required arguments:
# - MainSequence object from test template
# - Page object returned from the self.getPageInfo()


def displayPageInfo(seq, page_obj):

    seq.notice(">>>>>>>  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
    seq.notice(">>>>>>>  Page Object:  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")

    for key, value in page_obj.items():
        if key == "Page":
            if value["PageSize"] == 2 ** 12:
                page_size_string = "4KB"
            elif value["PageSize"] == 2 ** 21:
                page_size_string = "2MB"
            elif value["PageSize"] == 2 ** 30:
                page_size_string = "1GB"
            elif value["PageSize"] == 2 ** 39:
                page_size_string = "512GB"
            else:
                page_size_string = "unknown"
            seq.notice(">>>>>>>      Page Size:  {}".format(page_size_string))
            seq.notice(
                ">>>>>>>      Virtual Address Range:    {:#018x} - {:#018x}".format(
                    value["Lower"], value["Upper"]
                )
            )
            seq.notice(
                ">>>>>>>      Physical Address Range:   {:#018x} - {:#018x}".format(
                    value["PhysicalLower"], value["PhysicalUpper"]
                )
            )
            seq.notice(">>>>>>>      Descriptor:  {:#018x}".format(value["Descriptor"]))
            seq.notice(
                ">>>>>>>      Descriptor Address:  {}".format(
                    value["DescriptorDetails"]["Address"]
                )
            )
            seq.notice(
                ">>>>>>>      Descriptor Details:    DA         G          U          X          WR         V"
            )
            seq.notice(
                (">>>>>>>      Descriptor Details:    " + "{:<10} " * 6 + " ").format(
                    value["DescriptorDetails"]["DA"],
                    value["DescriptorDetails"]["G"],
                    value["DescriptorDetails"]["U"],
                    value["DescriptorDetails"]["X"],
                    value["DescriptorDetails"]["WR"],
                    value["DescriptorDetails"]["V"],
                )
            )

        if key.startswith("Table"):
            for field, info in value.items():
                if field == "DescriptorAddr" or field == "Descriptor":
                    seq.notice(">>>>>>>      {:<20}:    {:#018x}".format(field, info))
                if field == "Level":
                    seq.notice(">>>>>>>    {}  {}".format(field, info))
                if field == "DescriptorDetails":
                    seq.notice(
                        ">>>>>>>      PPN of Next PTE     :    {}".format(
                            value["DescriptorDetails"]["Address"]
                        )
                    )

    seq.notice(">>>>>>>  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
