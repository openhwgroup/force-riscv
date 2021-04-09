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
from abc import ABC, abstractmethod
from shared.instruction_file import InstructionFile


class BaseInstructionGroup(ABC):
    def __init__(self, groupName, archName, additionalImport, postMainSeq):
        self.mInstructions = []

        from shared.test_template_strings import base_template_str

        self.sTemplate = base_template_str
        self.sAdditionalImport = additionalImport
        self.sPostMainSeq = postMainSeq
        self.sGroupName = groupName
        self.sArchName = archName

    @abstractmethod
    def add_instruction(self, instr):
        pass

    def write(self, fileHandle):
        fileHandle.write("{} {}\n".format(self.sArchName, self.sGroupName))
        for instr in self.mInstructions:
            fileHandle.write(instr.get_full_ID() + "\n")

    def get_template_string(self):
        return self.sTemplate

    def get_additional_import_string(self):
        return self.sAdditionalImport

    def get_post_mainseq_string(self):
        return self.sPostMainSeq

    def print_test(self, testOutputPath):
        single_instr_str = ""
        if len(self.mInstructions) == 1:
            single_instr_str = "-{}".format(
                self.mInstructions[0].name.replace("/", "-").replace(".", "-")
            )

        file_name = "T{0:d}-{1}{2}_force.py".format(
            len(self.mInstructions), self.sGroupName, single_instr_str
        )
        instr_list = []
        for instr in self.mInstructions:
            instr_list.append('"' + instr.get_full_ID() + '"')

        instr_names = ",\n                      ".join(instr_list)
        with open(testOutputPath + file_name, "w") as test_handle:
            test_str = self.get_template_string().format(
                instr_names,
                self.sArchName.lower(),
                self.sArchName.upper(),
                self.get_additional_import_string(),
                self.get_post_mainseq_string(),
            )
            test_handle.write(test_str)

        return file_name


class BaseGroupedInstructionFile(ABC):
    def __init__(
        self,
        instrFile,
        archName,
        testOutputPath,
        additionalImport,
        postMainSeq,
        skipInstructions,
        validInstructions,
        subgroupPrefix,
    ):
        self.mInstructionDict = {}
        self.mSkipInstructions = skipInstructions
        self.mValidInstructions = validInstructions
        self.mInstrFile = instrFile
        self.sArchName = archName
        self.sTestOutputPath = testOutputPath
        self.sAdditionalImport = additionalImport
        self.sPostMainSeq = postMainSeq
        self.sSubgroupPrefix = subgroupPrefix

    def create_instruction_grouping(self):
        for instr in self.mInstrFile.instruction_iterator():
            instr_id = instr.get_full_ID()
            if self.is_instruction_valid(
                instr_id
            ) and not self.is_instruction_skipped(instr_id):
                self.add_instruction(instr)

    def is_instruction_valid(self, instrId):
        return not self.mValidInstructions or (
            instrId in self.mValidInstructions
        )

    def is_instruction_skipped(self, instrId):
        return self.mSkipInstructions and (instrId in self.mSkipInstructions)

    @abstractmethod
    def add_instruction(self, instr):
        pass

    def write(self, fileHandle):
        for key, grp in sorted(self.mInstructionDict.items()):
            grp.write(fileHandle)

    def print_tests(self, testsFileHandle):
        for key, grp in sorted(self.mInstructionDict.items()):
            test_file_name = grp.print_test(self.sTestOutputPath)
            testsFileHandle.write(test_file_name + "\n")

    def has_any_instructions(self):
        for grp in self.mInstructionDict.values():
            if grp.mInstructions:
                return True
        return False


class FormatInstructionGroup(BaseInstructionGroup):
    def __init__(self, groupName, archName, additionalImport, postMainSeq):
        super().__init__(groupName, archName, additionalImport, postMainSeq)

    def add_instruction(self, instr):
        if instr.get_format() != self.sGroupName:
            print(
                "ERROR: instruction format doesn't match group instr_name={} "
                "instr_format={} group_name={}".format(
                    instr.name, instr.get_format(), self.sGroupName
                )
            )
        self.instructions.append(instr)


class FormatGroupedInstructionFile(BaseGroupedInstructionFile):
    def __init__(
        self,
        instrFile,
        archName,
        testOutputPath,
        additionalImport,
        postMainSeq,
        skipInstructions,
        validInstructions,
        subgroupPrefix,
    ):
        super().__init__(
            instrFile,
            archName,
            testOutputPath,
            additionalImport,
            postMainSeq,
            skipInstructions,
            validInstructions,
            subgroupPrefix,
        )
        self.create_instruction_grouping()

    def add_instruction(self, instr):
        instr_format = instr.get_format()
        if instr_format in self.mInstructionDict:
            instr_group = self.mInstructionDict[instr_format]
        else:
            instr_group = FormatInstructionGroup(
                self.sSubgroupPrefix + instr_format,
                self.sArchName,
                self.sAdditionalImport,
                self.sPostMainSeq,
            )
            self.mInstructionDict[instr_format] = group

        group.add_instruction(instr)


class NumInstructionGroup(BaseInstructionGroup):
    def __init__(
        self, groupName, archName, groupSize, additionalImport, postMainSeq
    ):
        super().__init__(groupName, archName, additionalImport, postMainSeq)
        self.dGroupSize = groupSize

    def add_instruction(self, instr):
        if (len(self.mInstructions) + 1) <= self.dGroupSize:
            self.mInstructions.append(instr)
        else:
            print(
                "ERROR: attempting to add instruction to full group "
                "instr_name={} group_size={} group_name={} curr_len={}".format(
                    instr.name,
                    self.dGroupSize,
                    self.sGroupName,
                    len(self.mInstructions),
                )
            )


class NumGroupedInstructionFile(BaseGroupedInstructionFile):
    def __init__(
        self,
        instrFile,
        archName,
        testOutputPath,
        groupSize,
        additionalImport,
        postMainSeq,
        skipInstructions,
        validInstructions,
        subgroupPrefix,
    ):
        super().__init__(
            instrFile,
            archName,
            testOutputPath,
            additionalImport,
            postMainSeq,
            skipInstructions,
            validInstructions,
            subgroupPrefix,
        )
        self.dGroupSize = groupSize
        self.dGroupIndex = 0
        self.mInstructionDict[self.dGroupIndex] = NumInstructionGroup(
            "{}Group{:d}".format(self.sSubgroupPrefix, self.dGroupIndex),
            self.sArchName,
            self.dGroupSize,
            self.sAdditionalImport,
            self.sPostMainSeq,
        )
        self.create_instruction_grouping()

    def add_instruction(self, instr):
        if (
            len(self.mInstructionDict[self.dGroupIndex].mInstructions)
            >= self.dGroupSize
        ):
            self.dGroupIndex += 1
            self.mInstructionDict[self.dGroupIndex] = NumInstructionGroup(
                "{}Group{:d}".format(self.sSubgroupPrefix, self.dGroupIndex),
                self.sArchName,
                self.dGroupSize,
                self.sAdditionalImport,
                self.sPostMainSeq,
            )
        self.mInstructionDict[self.dGroupIndex].add_instruction(instr)
