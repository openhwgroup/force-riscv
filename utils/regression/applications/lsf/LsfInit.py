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
from classes.ApplicationOption import AppCmdLineOption, ParameterProcessor


# Define additional LSF specific command line parameters
#
class LsfCmdLineOptions(object):
    cGroupName = "LSF related options"
    cGroupDescription = "Useful LSF options to control LSF usage"

    #         "number of value arguments"
    #   "option name"               | "additional arguments"
    #         |    "default value"  |    |   "help text"
    #         |           |         |    |       |
    cOptions = [
        AppCmdLineOption(
            "sla",
            "trg-dvClass",
            1,
            None,
            "- Specify LSF serice class for scheduling polcy",
        ),
        AppCmdLineOption(
            "group", "trg", 1, None, "- Specify LSF service group"
        ),
        AppCmdLineOption(
            "queue", "normal", 1, None, "- Specify LSF queue to use"
        ),
    ]


# Used to process application specific parameters
#
class LsfParametersProcessor(ParameterProcessor):
    def __init__(self, aCmdLineOptions):
        super().__init__(LsfCmdLineOptions.cOptions, aCmdLineOptions)
