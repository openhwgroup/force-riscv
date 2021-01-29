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
from classes.ApplicationOption import AppCmdLineOption, AppPathCmdLineOption, ParameterProcessor
from common.path_utils import PathUtils
from common.version_ctrl_utils import VersionCtrlUtils
from os import path
from common.msg_utils import Msg

## Define additional FPIX specific command line parameters
#
class FpixCmdLineOptions(object):

    cGroupName = "FPIX related options"
    cGroupDescription = "Useful FPIX options to control FPIX usage"

    #                               "number of value arguments"
    #                         "option name"               | "additional arguments"
    #                               |    "default value"  |    |   "help text"
    #                               |           |         |    |       |
    cOptions = [ AppPathCmdLineOption('iss_so_path',  "../handcar/handcar_cosim.so", 1, None, \
                                      "- Specify Fpix ISS .so path.  When present, overrides the default simulator path\nIf not present then the \"ISS_SO_PATH\" environmental variable is used.\nIf the environmental variable does not exist then the default is used.\nIf a relative path is specified that path is calculated in the same manner as the default above",  \
                                      None, "ISS_SO_PATH"),
                 AppPathCmdLineOption('fpix_path',  "../../fpix/bin/fpix_riscv", 1, None, \
                                      "- Specify Fpix executable path.  When present, overrides the default Fpix executable path",  \
                                      None, None),
                 AppPathCmdLineOption('cfg',  "../../fpix/config/riscv_rv64.config", 1, None, \
                                      "- Specify Fpix additional config path.",  \
                                      None, None),
                AppCmdLineOption(    'skip',     '',                 0, {"action":"store_true"}, "- When present, the Fpix simulator driver does not run even though it may be an included module"),
                 ]

## Used to process application specific parameters
#
class FpixParametersProcessor(ParameterProcessor):

    def __init__(self, aCmdLineOptions):
        super().__init__(FpixCmdLineOptions.cOptions, aCmdLineOptions)

        #TODO add checks for ISS so file as well as for Fpix executable
        fpix_path = self.mAppParameters.parameter('fpix_path')
        if not PathUtils.check_exe(fpix_path):
            raise Exception(fpix_path + " does not exist or is not executable, condirm valid exe")


## Process iss control data
#
def processFpixControlData(aControlData, aAppParameters):
    if aAppParameters is None: return # TODO Temporary, to avoid failing in forrest run, to remove.
    key = 'fpix_path'
    if aAppParameters.parameter(key):
        aControlData[key] = aAppParameters.parameter(key)

    key = 'cfg'
    if aAppParameters.parameter(key):
        aControlData[key] = aAppParameters.parameter(key)

    key = 'iss_so_path'
    if aAppParameters.parameter(key):
        aControlData[key] = aAppParameters.parameter(key)

    key = 'skip'
    if aAppParameters.parameter(key):
        aControlData[key] = aAppParameters.parameter(key)

