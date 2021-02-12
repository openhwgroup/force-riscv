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
from classes.ApplicationOption import CommandLineOption, ParameterProcessor
from common.path_utils import PathUtils


#  Define additional specific command line parameters
class CompileCmdLineOptions(object):
    cGroupName = "Compile related options"
    cGroupDescription = "Useful Compile options to control Compile usage"

    cOptions = [
        CommandLineOption(
            aName="compile",
            aDefault="",
            aNumArgs=0,
            aAdditionalArgs={"action": "store_true"},
            aHelpText="- When present, overrides the default (False) and "
            "enables compiling",
        ),
        CommandLineOption(
            aName="compile.path",
            aDefault="$PROJ_ROOT/verif/top/sim",
            aNumArgs=1,
            aAdditionalArgs={},
            aHelpText="- When present, overrides the default path "
            "($PROJ_ROOT/verif/top/sim",
        ),
        CommandLineOption(
            aName="compile.options",
            aDefault="",
            aNumArgs=1,
            aAdditionalArgs={"type": str},
            aHelpText="- When present, adds the specified option string "
            "for compilation",
        ),
        CommandLineOption(
            aName="compile.mp",
            aDefault="",
            aNumArgs=0,
            aAdditionalArgs={"action": "store_true"},
            aHelpText="- When present, overrides the default (False) and "
            "triggers mp specific before and after processes",
        ),
    ]


#  Used to process application specific parameters
#
class CompileParametersProcessor(ParameterProcessor):
    def __init__(self, aCmdLineOptions):
        super().__init__(CompileCmdLineOptions.cOptions, aCmdLineOptions)

        default_path = "$PROJ_ROOT/verif/top/sim"
        if (
            self.mAppParameters.parameter("compile")
            or self.mAppParameters.parameter("compile.options")
            or self.mAppParameters.parameter("compile.path") != default_path
        ):
            compile_path = self.mAppParameters.parameter("compile.path")
            compile_path = PathUtils.expandVars(compile_path)
            compile_makefile = (
                PathUtils.include_trailing_path_delimiter(compile_path)
                + "Makefile"
            )

            if not PathUtils.check_file(compile_makefile):
                raise Exception(compile_makefile + " does not exist.")

            self.mAppParameters.setParameter("compile.path", compile_path)
            self.mAppParameters.setParameter(
                "compile.options",
                PathUtils.expandVars(
                    self.mAppParameters.parameter("compile.options")
                ),
            )


#  Process compile control data
#
def process_compile_control_data(aControlData, aAppParameters):
    if aAppParameters is None:
        return  # TODO Temporary, to avoid failing in forrest run, to remove.

    keys = [
        "compile",
        "compile.options",
    ]  # Ignore compile.path for now because it has a default value and will
    # always be found, forcing a compile every time master run runs
    for key in keys:
        if aAppParameters.parameter(key):
            aControlData["run"] = True
            aControlData["path"] = aAppParameters.parameter("compile.path")
            aControlData["options"] = aAppParameters.parameter(
                "compile.options"
            )
            aControlData["mp"] = aAppParameters.parameter("compile.mp")
            return

    # Check compile.path here to determine if it has been changed and, if so,
    # make sure we run the compile app on that non-default path
    default_path = "$PROJ_ROOT/verif/top/sim"
    if aAppParameters.parameter("compile.path") != default_path:
        aControlData["run"] = True
        aControlData["path"] = aAppParameters.parameter("compile.path")
        aControlData["options"] = aAppParameters.parameter("compile.options")
        aControlData["mp"] = aAppParameters.parameter("compile.mp")
