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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from PageFaultSequence import PageFaultSequence
from base.ChoicesModifier import ChoicesModifier
from riscv.ModifierUtils import PageFaultModifier, displayPageInfo
from EnumsRISCV import EPagingMode
import VirtualMemory


class MainSequence(PageFaultSequence):
    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread, name)

        # just a couple of load/stores...

        self._mInstrList = (
            "LW##RISCV",
            "SW##RISCV",
        )

        self._mExceptionCodes = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 15)

        self._requested_fault_type = None
        self._page_fault_level = None
        self._iter_count = 20  # default to generating 20 instructions

        self._exception_counts = dict.fromkeys(self._mExceptionCodes, 0)

    def generate(self, **kwargs):

        # Process the options specified in the fctrl file
        # Will set:  _requested_fault_type
        #            _page_fault_level
        #            _iter_count     - number of instructions to generate
        self._processFctrlOptions()

        # Generate some no-ops before making changes to the paging choices
        self._genDummyInstructions()

        # Modify the paging choices based on the options parsed from the fctrl file
        page_fault_mod = PageFaultModifier(self.genThread, self.getGlobalState("AppRegisterWidth"))

        if not self._requested_fault_type in page_fault_mod.getValidFaultTypes():
            self.error("'PageFaultType' of {} is an unsupported type.").format(
                self._requested_fault_type
            )
        else:
            if self._page_fault_level == None:
                page_fault_mod.apply(**{"Type": self._requested_fault_type})
            else:
                page_fault_mod.apply(
                    **{"Type": self._requested_fault_type, "Level": [self._page_fault_level]}
                )

        # Generate the faulting instructions
        for _ in range(self._iter_count):
            instr_id = self.genInstruction(self.choice(self._mInstrList))
            instr_obj = self.queryInstructionRecord(instr_id)

            # Write exception count info into the gen.log
            self._displayExceptionInfo(instr_obj)

            # Write the page translation info into the gen.log
            ls_target_addr = instr_obj["LSTarget"]
            page_obj = self.getPageInfo(ls_target_addr, "VA", 0)
            displayPageInfo(self, page_obj)

        # Restore the paging choices to values before the page_fault_mod.apply
        page_fault_mod.revert()

        # Some no-ops after the paging choices were restored
        self._genDummyInstructions()

    def _genDummyInstructions(self):
        for _ in range(5):
            self.genInstruction("ORI##RISCV", {"rd": 0, "rs1": 0, "simm12": 0})

    def _processFctrlOptions(self):

        ## Process options from the fctrl file:
        ## - PagingDisabled   = [0, 1]
        ## - IterCount        = integer
        ## - PageFaultType    = ["Invalid_DA", "Invalid_U", "Invalid_X", "Invalid_WR", "Invalid_V"]
        ## - PageFaultLevel   = [0, 1] for sv32; [0, 1, 2] for sv39; [0, 1, 2, 3] for sv48.

        # Check to make sure paging is not disabled in the options
        (paging_disabled, valid) = self.getOption("PagingDisabled")
        if valid and (paging_disabled == 1):
            self.error("'PagingDisabled' option set, can't generate page faults.")

        # Check to see if a specific page fault type is requested in the options.
        (self._requested_fault_type, valid) = self.getOption("PageFaultType")
        if not valid:
            self.error("Option 'PageFaultType' was not specified.")
        # Temporary - I'm not sure how to code the option to allow a space in the option string.
        if self._requested_fault_type == "Invalid_DA":
            self._requested_fault_type = "Invalid DA"
        if self._requested_fault_type == "Invalid_U":
            self._requested_fault_type = "Invalid U"
        if self._requested_fault_type == "Invalid_X":
            self._requested_fault_type = "Invalid X"
        if self._requested_fault_type == "Invalid_WR":
            self._requested_fault_type = "Invalid WR"
        if self._requested_fault_type == "Invalid_V":
            self._requested_fault_type = "Invalid V"

        # Get the value of the requested page table level from the options in the fctrl file
        # Valid levels are 0, 1, 2, 3 for rv64; 0, 1 for rv32.  The isValidPageFaultLevel
        # considers whether the configuration is rv32 or rv64.
        # If the page_fault_level option is not specified, the paging choices for all levels
        # of that page fault type are modified.
        # The default weight of 100 is used unless the 'Weight' is included in the dictionary
        # passed to apply.
        (self._page_fault_level, valid) = self.getOption("PageFaultLevel")
        if not valid:
            self._page_fault_level = None
        elif (self._page_fault_level == 3) and (VirtualMemory.getPagingMode() == EPagingMode.Sv39):
            self._page_fault_level = 2

        # Get the iteration count
        (self._iter_count, valid) = self.getOption("IterCount")

    def _displayExceptionInfo(self, instr_obj):
        # Report the exception generated
        for ec in self._mExceptionCodes:
            backend_exception_count = self.queryExceptionRecordsCount(ec)
            new_exception = backend_exception_count > self._exception_counts[ec]
            self._exception_counts[ec] = backend_exception_count
            self.notice(
                ">>>>>>>  Exception:  {}    Query value:  {}".format(
                    ec, self.queryExceptionRecordsCount(ec)
                )
            )
            self.notice(
                ">>>>>>>  Instr:  {}    EType:  {}    New Exception? {}   count {}".format(
                    instr_obj["Name"], ec, new_exception, backend_exception_count
                )
            )


def gen_thread_initialization(gen_thread):
    (delegate_opt, valid) = gen_thread.getOption("DelegateExceptions")
    if valid and delegate_opt == 1:
        # enable exception delegation for some portion of the generated tests...
        delegation_enables = ChoicesModifier(gen_thread)
        weightDict = {"0x0": 0, "0x1": 50}
        delegation_enables.modifyRegisterFieldValueChoices("medeleg.Load page fault", weightDict)
        delegation_enables.modifyRegisterFieldValueChoices(
            "medeleg.Store/AMO page fault", weightDict
        )
        delegation_enables.commitSet()


GenThreadInitialization = gen_thread_initialization

MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
