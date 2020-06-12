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
from shared.ctrl_file_template_strings import *

class CtrlFileBuilder():

    def __init__(self, archName, ctrlItemPrefixes):
        self.sArchName = archName
        self.mCtrlItemPrefixes = ctrlItemPrefixes
        self.sDefCtrlFile = ""
        self.sNoissCtrlFile = ""
        self.sPerfCtrlFile = ""

    def gen_ctrl_files(self, outputPath):
        self.sDefCtrlFile = self.gen_def_ctrl_file()
        self.sNoissCtrlFile = self.gen_noiss_ctrl_file()
        self.sPerfCtrlFile = self.gen_perf_ctrl_file()
        with open(outputPath + "_def_fctrl.py", "w") as def_handle:
            def_handle.write(self.sDefCtrlFile)

        with open(outputPath + "_noiss_fctrl.py", "w") as noiss_handle:
            noiss_handle.write(self.sNoissCtrlFile)

        with open(outputPath + "_perf_fctrl.py", "w") as perf_handle:
            perf_handle.write(self.sPerfCtrlFile)

    def gen_def_ctrl_file(self):
        ctrl_file_string = ""
        ctrl_file_string += ctrl_items_template
        ctrl_item = CtrlItem(False, False, False, self.sArchName, "", "default", [], [], [])
        ctrl_file_string += ctrl_item.gen_ctrl_item_string()
        ctrl_file_string += ctrl_item_separator
        for ctrl_item_prefix in self.mCtrlItemPrefixes:
            if ctrl_item_prefix == "atomic":
                atomic_ctrl_item = CtrlItem(True, False, False, self.sArchName, "atomic/", "atomic", ["options"], [], [])
                ctrl_file_string += atomic_ctrl_item.gen_ctrl_item_string()
                ctrl_file_string += ctrl_item_separator
            elif ctrl_item_prefix == "genonly":
                pass
            else:
                print("ERROR: def fctrl not processing prefix: {}".format(ctrl_item_prefix))
        ctrl_file_string += "]"
        return ctrl_file_string

    def gen_noiss_ctrl_file(self):
        ctrl_file_string = ""
        ctrl_file_string += ctrl_items_template
        ctrl_item = CtrlItem(True, False, False, self.sArchName, "", "default", ["noiss"], [], [])
        ctrl_file_string += ctrl_item.gen_ctrl_item_string()
        ctrl_file_string += ctrl_item_separator
        for ctrl_item_prefix in self.mCtrlItemPrefixes:
            if ctrl_item_prefix == "atomic":
                atomic_ctrl_item = CtrlItem(True, False, False, self.sArchName, "atomic/", "atomic", ["noiss", "options"], [], [])
                ctrl_file_string += atomic_ctrl_item.gen_ctrl_item_string()
                ctrl_file_string += ctrl_item_separator
            elif ctrl_item_prefix == "genonly":
                genonly_ctrl_item = CtrlItem(True, True, False, self.sArchName, "genonly/", "genonly", ["noiss"], ["nosim"], [])
                ctrl_file_string += genonly_ctrl_item.gen_ctrl_item_string()
                ctrl_file_string += ctrl_item_separator
            else:
                print("ERROR: noiss fctrl not processing prefix: {}".format(ctrl_item_prefix))
        ctrl_file_string += "]"
        return ctrl_file_string

    def gen_perf_ctrl_file(self):
        ctrl_file_string = ""
        ctrl_file_string += ctrl_items_template
        ctrl_item = CtrlItem(False, True, True, self.sArchName, "", "default", [], ["group"], ["group"])
        ctrl_file_string += ctrl_item.gen_ctrl_item_string()
        ctrl_file_string += ctrl_item_separator
        for ctrl_item_prefix in self.mCtrlItemPrefixes:
            if ctrl_item_prefix == "atomic":
                atomic_ctrl_item = CtrlItem(True, True, True, self.sArchName, "atomic/", "atomic", ["options"], ["group"], ["group"])
                ctrl_file_string += atomic_ctrl_item.gen_ctrl_item_string()
                ctrl_file_string += ctrl_item_separator
            elif ctrl_item_prefix == "genonly":
                genonly_ctrl_item = CtrlItem(True, True, True, self.sArchName, "genonly/", "genonly", ["noiss"], ["nosim", "group"], ["group"])
                ctrl_file_string += genonly_ctrl_item.gen_ctrl_item_string()
                ctrl_file_string += ctrl_item_separator
            else:
                print("ERROR: perf fctrl not processing prefix: {}".format(ctrl_item_prefix))
        ctrl_file_string += "]"
        return ctrl_file_string


class CtrlItem():

    def __init__(self, hasGenerator, hasOptions, hasPerformance, archName, fnamePrefix, groupName, generatorArgs, optionsArgs, performanceArgs):
        self.bHasGenerator = hasGenerator
        self.bHasOptions = hasOptions
        self.bHasPerformance = hasPerformance

        self.sArchName = archName
        self.sFnamePrefix = fnamePrefix
        self.sGroupName = groupName

        self.mGeneratorArgs = generatorArgs
        self.mOptionsArgs = optionsArgs
        self.mPerformanceArgs = performanceArgs

        self.sCtrlItemString = ""

    def gen_ctrl_item_string(self):
        self.sCtrlItemString = "{ "
        self.sCtrlItemString += fname_template.format(self.sFnamePrefix)

        if self.bHasGenerator:
            self.sCtrlItemString += self.gen_generator_string()

        if self.bHasOptions:
            self.sCtrlItemString += self.gen_options_string()

        if self.bHasPerformance:
            self.sCtrlItemString += self.gen_performance_string()

        self.sCtrlItemString += " },"
        return self.sCtrlItemString

    def gen_generator_string(self):
        generator_arg_string = ""

        for arg in self.mGeneratorArgs:
            if arg == "noiss":
                generator_arg_string += noiss
            elif arg == "options":
                generator_arg_string += options.format(arch_genopts)
            else:
                print("ERROR: invalid generator arg type: {}".format(arg))

        return generator_template.format(generator_arg_string)

    def gen_options_string(self):
        options_arg_string = ""
        for arg in self.mOptionsArgs:
            if arg == "nosim":
                options_arg_string += nosim
            elif arg == "group":
                options_arg_string += group.format(self.sGroupName)
            else:
                print("ERROR: invalid options arg type: {}".format(arg))

        return options_template.format(options_arg_string)

    def gen_performance_string(self):
        performance_arg_string = ""
        for arg in self.mPerformanceArgs:
            if arg == "group":
                performance_arg_string += group.format(self.sGroupName)
            else:
                print("ERROR: invalid performance arg type: {}".format(arg))

        return performance_template.format(performance_arg_string)
