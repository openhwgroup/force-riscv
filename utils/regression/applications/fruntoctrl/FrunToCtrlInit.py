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
from classes.ApplicationOption import CommandLineOption, ParameterProcessor
from common.path_utils import PathUtils

## Define additional FRUN_TO_CTRL specific command line parameters
#
class FrunToCtrlCmdLineOptions(object):

    cGroupName = "Frun to ctrl related options"
    cGroupDescription = "Useful FRUN_TO_CTRL options to control FRUN_TO_CTRL usage"

    #                               "number of value arguments"
    #                         "option name"               | "additional arguments"
    #                               |    "default value"  |    |   "help text"
    #                               |              |      |    |       |
    cOptions = [ CommandLineOption('frun-to-ctrl', "",    0, {"action":"store_true"}, "- When present, overrides the default (False), and enables conversion from the frun file to an additional control file\non tests that have an frun file"),
    ]

        
## Used to process application specific parameters
#
class FrunToCtrlParametersProcessor(ParameterProcessor):

    def __init__(self, aCmdLineOptions):
        super().__init__(FrunToCtrlCmdLineOptions.cOptions, aCmdLineOptions)

        if self.mAppParameters.parameter('frun-to-ctrl'):
            frun_to_ctrl_path = PathUtils.include_trailing_path_delimiter( aCmdLineOptions.mProgramPath ) + "../frun_to_ctrl/frun_to_ctrl.py"
            
            if not PathUtils.check_exe( frun_to_ctrl_path ):
                raise Exception( frun_to_ctrl_path + " does not exist or is not executable, confirm valid exe" )
            frun_to_ctrl_path =  PathUtils.real_path(frun_to_ctrl_path)
            self.mAppParameters.setParameter('path', frun_to_ctrl_path)


## Process fruntoctrl control data
#
def processFrunToCtrlControlData(aControlData, aAppParameters):
    if aAppParameters is None: return # TODO Temporary, to avoid failing in forrest run, to remove.

    key = 'frun-to-ctrl'
    if aAppParameters.parameter(key):
        aControlData['run'] = aAppParameters.parameter(key)
        aControlData['path'] = aAppParameters.parameter('path')
