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
import enum
import os

from base.Sequence import Sequence
from base.ModifierUtils import AlignedDataOnlyChoicesModifier
from riscv.PrivilegeLevel import PrivilegeLevelRISCV
from riscv.ModifierUtils import PageFaultModifier

## This class provides a base sequence for testing various types of page faults.
class PageFaultSequence(Sequence):

    def __init(self, gen_thread, name=None):
        super().__init__(gen_thread, name)

        # avoid alignment exceptions...
        gen_thread.addSetupModifier(AlignedDataOnlyChoicesModifier)

    def generate(self, **kwargs):
        (paging_opt, valid) = self.getOption("PagingDisabled")
        if valid and (paging_opt == 1):
            self.error("'PagingDisabled' option set, can't generate page faults.")

        self.generatePreFaultInstructions()

        self.notice("Applying all valid level and exception level page fault choices")
        page_fault_mod = self.createPageFaultModifier()
        page_fault_mod.apply(**{"All":1})
        #page_fault_mod.apply(**{"Type":"Invalid U"})

        instruction_list = self.getInstructionList()
        instr_count = 0
        while (not self._hasPageFaultOccurred()) and instr_count < 100:
            pc_val = self.getPEstate("PC")
            instr_id = self.genInstruction(self.choice(instruction_list))
            instr_count += 1

        #self._verifyPageFaultResolution(instr_id, pc_val)

        page_fault_mod.revert()

        for _ in range(5):
            self.genInstruction('ORI##RISCV', {'rd': 0, 'rs1': 0, 'simm12': 0})
            
    ## Generate any necessary setup instructions before triggering the page fault.
    def generatePreFaultInstructions(self):
        # Generate a few instructions before the page fault attempt, to hopefully 'clear' the boot code
        for _ in range(5):
            self.genInstruction('ORI##RISCV', {'rd': 0, 'rs1': 0, 'simm12': 0})

    ## Create an instance of the appropriate page fault modifier.
    def createPageFaultModifier(self):
        self.notice("[PageFaultSequence::createPageFaultModifier]")
        raise NotImplementedError()

    ## Get a list of instructions to choose from to trigger a page fault.
    def getInstructionList(self):
        raise NotImplementedError()

    ## Return the expected resolution type for the page fault.
    #
    # @param aFaultLevels  - the translation level at which the page fault has been triggered.
    # @param aFastHandlers - flag indicating whether or not fast handlers are enabled.
    def getPageFaultResolutionType(self, aFaultLevels, aFastHandlers):
        resolution_type = PageFaultResolutionType.RE_EXECUTE_INSTRUCTION
        if aFastHandlers:
            resolution_type = PageFaultResolutionType.SKIP_INSTRUCTION
        return resolution_type

    ## Return the exception codes for the expected page fault type.
    def getExceptionCodes(self):
        raise NotImplementedError()

    ## Return whether the appropriate page fault has been triggered.
    def _hasPageFaultOccurred(self):
        #fault_levels = self._getFaultLevels()
        #return (len(fault_levels) > 0)
        return self._expectedExceptionRecorded()
    
    ## Verify that a page fault has been triggered and appropriately handled.
    #
    # @param aFaultingInstrId - the ID of the instruction that should have triggered a page fault.
    # @param aPcVal - the value of the PC prior to generating the instruction that should have
    #                 triggered a page fault.
    def _verifyPageFaultResolution(self, aFaultingInstrId, aPcVal):
        if not self._hasPageFaultOccurred():
            self.error("A fault of the expected type was not triggered.")

        (handlers_set, valid) = self.getOption("handlers_set")
        fast_handlers = (handlers_set == "Fast")
        resolution_type = self.getPageFaultResolutionType(self._getFaultLevels(), fast_handlers)

        xepc_val = self._readXepcRegister()
        instr_record = self.queryInstructionRecord(aFaultingInsterId)
        instr_va = instr_record["VA"]

        # check the PC value in addition to the instruction VA in case a preamble
        # instruction triggered the fault
        if resolution_type == PageFaultResolutionType.SKIP_INSTRUCTION:
            if xepc_val in [instr_va, aPcVal]:
                self.error("Fault handler did not skip the faulting instruction.")
        elif resolution_type == PageFaultResolutionType.RE_EXECUTE_INSTRUCTION:
            if xepc_val in [instr_va, aPcVal]:
                self.error("Instruction at 0x%x did not re-execute after the fault handler returned" % instr_va)
        elif resolution_type == PageFaultResolutionType.NONE:
            pass
        else:
            self.error("No verification provided for %s" % resolution_type)

    ## Return true if a recorded exception code was one of the ones expected for this test
    def _expectedExceptionRecorded(self):
        for ec in self.getExceptionCodes():
            if self.queryExceptionRecordsCount(ec) > 0:
                return True
        return False
    
    ## Return the translation level(s) at which the appropriate page fault has been triggered, if any.
    def _getFaultLevels(self):
        # riscv does not have exception sub-code, still we may, at some point, record a sub-code
        # of sorts in the handler itself(?)...
        raise NotImplementedError()
        #except_list = self.queryExceptions()
        #for except_recx in except_list:
        #    except_code = except_rec[0]
        #    if except_code in self.getExceptionCodes():
        #        except_sub_code = except_rec[-1]
        #        matching_except_sub_codes = self.getExceptionSubCodes()
        #        if except_sub_code in matching_except_sub_codes:
        #            fault_levels.add(matching_except_sub_codes[except_sub_code])
        #        fault_levels.add(except_code)
        #return fault_levels

    ## Read the appropriate xepc register value and return it if it is valid.
    def _readXepcRegister(self):
        self.privilegeLevel = handler_context.mPrivLevel
        priv_level = PrivilegeLevelRISCV[self.privilegeLevel]
        xepc_reg_name = "%sepc" % priv_level.name.lower()
        (xepc_val, valid) = self.readRegister(xepc_reg_name)
        if valid:
            self.error("%s value is not valid." % xepc_reg_name)
        return xepc_val

## This class defines the different modes for verifying the resolution of a page fault.
class PageFaultResolutionType(enum.Enum):
    NONE = 0
    SKIP_INSTRUCTION = 1
    RE_EXECUTE_INSTRUCTION = 2
