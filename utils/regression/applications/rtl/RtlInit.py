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
from classes.ApplicationOption import (
    CommandLineOption,
    AppCmdLineOption,
    AppPathCmdLineOption,
    ApplicationOption,
    ParameterProcessor,
)
from common.msg_utils import Msg
from common.path_utils import PathUtils
from common.version_ctrl_utils import VersionCtrlUtils


# Define additional RTL specific command line parameters
#
class RtlCmdLineOptions(object):

    cGroupName = "RTL related options"
    cGroupDescription = "Useful RTL options to control RTL usage"

    #          "number of value arguments"
    #    "option name"               | "additional arguments"
    #          |    "default value"  |    |                     "help text"
    #          |           |         |    |                        |
    cOptions = [
        CommandLineOption(
            "rtl",
            "",
            0,
            {"action": "store_true"},
            "- When present, adds rtl simulation to the work flow",
        ),
        CommandLineOption(
            "rtl.report.skip",
            False,
            0,
            {"action": "store_true"},
            "- When present, prevents a report from being sent to the triage database",
        ),
        CommandLineOption(
            "rtl.report.name",
            "master_run",
            1,
            {},
            "- When present, overwrites the default report name with the users choice",
        ),
        CommandLineOption(
            "rtl.report.xml",
            False,
            0,
            {"action": "store_true"},
            "- When present, dumps the report to an xml file rather than uploading it",
        ),
        AppPathCmdLineOption(
            "root",
            "",
            1,
            None,
            "- Specify RTL root.  When present, overrides the default RTL "
            'path specified in "PROJ_ROOT" environmental variable.',
            None,
            "PROJ_ROOT",
        ),
    ]


# Used to process application specific parameters
#
class RtlParametersProcessor(ParameterProcessor):
    """Used to process application specific parameters"""

    def __init__(self, aCmdLineOptions):
        """Iintialize the object

        :param CmdLineOptions aCmdLineOptions:
        """
        super().__init__(RtlCmdLineOptions.cOptions, aCmdLineOptions)

        rtl_root = self.mAppParameters.parameter("root")
        if not PathUtils.check_dir(rtl_root):
            raise Exception(rtl_root + " is not a directory.")

        meta_conv_path = PathUtils.append_path(
            aCmdLineOptions.mProgramPath, "metaargs_to_plusargs.py"
        )
        self.mAppParameters.setParameter("meta_converter", meta_conv_path)

        # determine svn revision information and store as a parameter
        version_data = VersionCtrlUtils.get_scm_revisions(rtl_root)
        version_output = VersionCtrlUtils.get_version_output(version_data)

        Msg.info("RTL Version Data:\n%s" % version_output)

        self.mAppParameters.setParameter("version", version_data)
        self.mAppParameters.setParameter("version_dir", rtl_root)


# Defined application options not configured on the master_run command line.
#
class RtlAppOptions(object):
    """Defined application options not configured on the master_run
    command line."""

    #     "option name"                       "env variable associated if any"
    #          |           "default value"                  |
    cOptions = [
        ApplicationOption(
            "regr", "logs", "RTL_REGR"
        ),  # This corresponds to OUTPUT_DIR in RTL makefile
        ApplicationOption(
            "bld_cfg", "target_config", "RTL_CFG"
        ),  # This corresponds to BUILD_CFG in RTL makefile
        ApplicationOption(
            "exe", "uvm_simv_opt", "RTL_EXE"
        ),  # Allow specifying a different simulation executable
        ApplicationOption(
            "report_script_path", "/utils/regression/report.py", ""
        ),  # Specify the relative location of the reporting script
        ApplicationOption(
            "rtl_metrics_path", "~/force_regr_cycle_count", ""
        ),  # file to save cycle count and instruction count
        ApplicationOption(
            "img_modify", "imgmodify.py", ""
        ),  # script that generates dat files from the ELF files for use
        # with force_init=on
        ApplicationOption(
            "tst_handler", "tstHandler.py", ""
        ),  # script that creates hex files from the dat files for use with
        # force_init=on
    ]


# Process rtl control data
#
def process_rtl_control_data(aControlData, aAppParameters):
    """Process rtl control data

    :param object aControlData:
    :param object aAppParameters:
    """
    if aAppParameters is None:
        return

    if not (aAppParameters.parameter("rtl") or len(aControlData) > 0):
        return

    for copy_key in ["root", "meta_converter"]:
        aControlData[copy_key] = aAppParameters.parameter(copy_key)

    rtl_root = aControlData["root"]
    # the location of the default wave_fsdb signal output selection file to
    # copy for use with rerun scripts
    aControlData["fsdb_do"] = rtl_root + "verif/top/tests/wave_fsdb.do"
    # specify the name of the debug RTL executable
    aControlData["debug_exe"] = "uvm_simv_dbg"

    for app_opt in RtlAppOptions.cOptions:
        app_opt.resolveItemParameter(aControlData)
