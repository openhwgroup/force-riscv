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
from classes.ApplicationOption import AppPathCmdLineOption, ParameterProcessor
from common.path_utils import PathUtils
from common.version_ctrl_utils import VersionCtrlUtils
from common.msg_utils import Msg


#  Define additional FORCE specific command line parameters
#
class ForceCmdLineOptions(object):

    cGroupName = "FORCE related options"
    cGroupDescription = "Useful FORCE options to control FORCE usage"

    cOptions = [
        AppPathCmdLineOption(
            "path",
            "../../bin/force",
            1,
            None,
            "- Path to FORCE binary",
            None,
            "FORCE_PATH",
        )
    ]


#  Used to process application specific parameters
#
class ForceParametersProcessor(ParameterProcessor):
    def __init__(self, aCmdLineOptions):
        super().__init__(ForceCmdLineOptions.cOptions, aCmdLineOptions)

        force_path = self.mAppParameters.parameter("path")
        force_bin_dir, _ = PathUtils.split_path(force_path)
        force_dir = PathUtils.real_path(
            PathUtils.include_trailing_path_delimiter(force_bin_dir) + "../"
        )

        if not PathUtils.check_exe(force_path):
            raise Exception(
                force_path
                + " does not exist or is not executable, confirm valid exe"
            )

        # determine svn revision information and store as a parameter
        version_data = VersionCtrlUtils.get_scm_revisions(force_dir)
        version_output = VersionCtrlUtils.get_version_output(version_data)

        Msg.info("Force Version Data:\n%s" % version_output)

        self.mAppParameters.setParameter("version", version_data)
        self.mAppParameters.setParameter("version_dir", force_dir)


#  Process force control data
#
def process_force_control_data(aControlData, aAppParameters):
    """
    :param object aControlData:
    :param object aAppParameters:
    :return:
    """
    if aAppParameters is None:
        return  # TODO Temporary, to avoid failing in forrest run, to remove.

    key = "path"
    if aAppParameters.parameter(key):
        aControlData[key] = aAppParameters.parameter(key)
