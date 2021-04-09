#!/usr/bin/env python3
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

# insert parent directory to access shared builder files
import sys

# sys.path.insert(0, "..")


class BaseTestBuilder:
    def __init__(
        self,
        archName,
        groupByNum,
        groupSize,
        additionalImport,
        postMainSeq,
        debugOutputPath,
    ):
        # input and output paths and filenames
        self.mInputPath = "input/"
        self.mOutputPath = "output/"

        self.mXmlPath = "xml/"
        self.mTxtPath = "txt/"
        self.mXmlFiles = []
        self.mTxtFiles = []

        self.mArchName = archName

        # additional hook strings into template to allow for custom
        # imports/post sequence specifiers (such as gen thread init)
        self.mAdditionalImport = additionalImport
        self.mPostMainSeq = postMainSeq

        # data structures for reading input data, key=filename w/o suffix
        # val=data (either instr file obj/list of strings)
        self.mInstrFiles = {}

        # data structures to manage subgrouping inside of instruction files,
        # contains input from txt files
        self.mUnsupportedInstructions = []
        self.mSubgroupInstructions = {}

        # instruction grouping switches
        self.mGroupByNum = groupByNum
        self.mGroupSize = groupSize

        # if debugOutputPath is defined, messages will be printed to outfile
        # instead of stdout
        if debugOutputPath != "":
            self.fDebugOutput = open(debugOutputPath, "w")
        else:
            self.fDebugOutput = None

    def debug(self, s):
        print("DEBUG: {}".format(s), file=self.fDebugOutput)

    def get_xml_input_path(self):
        return self.mInputPath + self.mXmlPath

    def get_txt_input_path(self):
        return self.mInputPath + self.mTxtPath

    def make_output_test_dir(self, dir_name):
        import os

        try:
            os.makedirs(self.mOutputPath + dir_name)
        except FileExistsError:
            pass
        except OSError:
            self.debug(
                "Error creating directory {}{}, exiting".format(
                    self.mOutputPath, dir_name
                )
            )
            sys.exit(1)

        return self.mOutputPath + dir_name + "/"

    def delete_output_test_dir(self, dir_name):
        import os

        try:
            os.rmdir(dir_name)
        except OSError:
            self.debug("Error removing directory {}, exiting".format(dir_name))
            sys.exit(1)

    def read_inputs(self):
        from shared.instruction_file import InstructionFile
        from shared.instruction_file_parser import InstructionFileParser

        for xml_file in self.mXmlFiles:
            instr_file = InstructionFile()
            file_parser = InstructionFileParser(instr_file)
            file_parser.parse(self.get_xml_input_path() + xml_file)
            self.mInstrFiles[xml_file.rstrip(".xml")] = instr_file

        for txt_file in self.mTxtFiles:
            with open(self.get_txt_input_path() + txt_file, "r") as txt_handle:
                instr_subgroup_name = txt_file.rstrip(".tx")
                txt_lines = txt_handle.read().splitlines()
                if instr_subgroup_name == "unsupported":
                    self.mUnsupportedInstructions = txt_lines
                else:
                    self.mSubgroupInstructions[instr_subgroup_name] = txt_lines

    def process_instr_group(
        self,
        instrFile,
        instrFileName,
        testOutputDir,
        subgroupPrefix,
        skipInstructions,
        validInstructions,
    ):
        instr_grp = self.gen_grouped_instr_file(
            instrFile,
            testOutputDir,
            subgroupPrefix,
            skipInstructions,
            validInstructions,
        )
        if instr_grp.has_any_instructions():
            with open(
                testOutputDir + "instruction_grouping.txt", "w"
            ) as group_handle:
                instr_grp.write(group_handle)

            with open(
                testOutputDir + instrFileName + ".txt", "w"
            ) as test_handle:
                instr_grp.print_tests(test_handle)

            return True
        else:
            self.delete_output_test_dir(testOutputDir)

        return False

    def gen_grouped_instr_file(
        self,
        instrFile,
        testOutputDir,
        subgroupPrefix,
        skipInstructions,
        validInstructions,
    ):
        instr_grp = None
        if self.mGroupByNum:
            from shared.instruction_file_grouping import (
                NumGroupedInstructionFile,
            )

            instr_grp = NumGroupedInstructionFile(
                instrFile,
                self.mArchName,
                testOutputDir,
                self.mGroupSize,
                self.mAdditionalImport,
                self.mPostMainSeq,
                skipInstructions,
                validInstructions,
                subgroupPrefix,
            )
        else:
            from shared.instruction_file_grouping import (
                FormatGroupedInstructionFile,
            )

            instr_grp = FormatGroupedInstructionFile(
                instrFile,
                self.mArchName,
                testOutputDir,
                self.mAdditionalImport,
                self.mPostMainSeq,
                skipInstructions,
                validInstructions,
                subgroupPrefix,
            )
        return instr_grp

    def write_tests(self):
        for instr_file_name, instr_file in self.mInstrFiles.items():
            valid_subgroups = []
            test_output_dir = self.make_output_test_dir(instr_file_name)
            import copy

            default_skip_instrs = copy.deepcopy(self.mUnsupportedInstructions)
            for (
                subgroup_name,
                subgroup_instrs,
            ) in self.mSubgroupInstructions.items():
                default_skip_instrs += subgroup_instrs
                subgroup_output_dir = self.make_output_test_dir(
                    instr_file_name + "/" + subgroup_name
                )
                valid_subgroup = self.process_instr_group(
                    instr_file,
                    instr_file_name + "_" + subgroup_name,
                    subgroup_output_dir,
                    subgroup_name + "_",
                    self.mUnsupportedInstructions,
                    subgroup_instrs,
                )
                if valid_subgroup:
                    valid_subgroups.append(subgroup_name)
            self.process_instr_group(
                instr_file,
                instr_file_name,
                test_output_dir,
                "",
                default_skip_instrs,
                None,
            )
            self.write_ctrl_files(valid_subgroups, test_output_dir)

    def write_ctrl_files(self, subgroupNames, outputDir):
        from shared.ctrl_file_builder import CtrlFileBuilder

        ctrl_file_builder = CtrlFileBuilder(
            self.mArchName.lower(), subgroupNames
        )
        ctrl_file_builder.gen_ctrl_files(outputDir)

    def run(self):
        self.read_inputs()
        self.write_tests()
