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
import argparse
from classes.ApplicationsInfo import ApplicationParameters
from common.path_utils import PathUtils


#  Base ApplicationOption class
#
class ApplicationOption(object):
    def __init__(self, aName, aDefault, aEnvVar=None):
        self._mName = aName
        self._mDefault = aDefault
        self._mEnvVar = aEnvVar

    def name(self):
        return self._mName

    # Get default value
    #
    def _getDefaultValue(self):
        # check environment variable if have that
        if self._mEnvVar:
            import os

            try:
                return os.environ[self._mEnvVar]
            except KeyError as arg_ex:
                pass

        return self._mDefault

    # Resolve an individual item's parameter
    #
    def resolveItemParameter(self, aItemDict):
        if self._mName not in aItemDict:
            aItemDict[self._mName] = self._getDefaultValue()


#  CommandLineOption class
#
class CommandLineOption(ApplicationOption):
    def __init__(
        self,
        aName,
        aDefault,
        aNumArgs,
        aAdditionalArgs,
        aHelpText,
        aShortOpt=None,
        aEnvVar=None,
    ):
        super().__init__(aName, aDefault, aEnvVar)
        self._mNumArgs = aNumArgs
        self._mAdditionalArgs = aAdditionalArgs
        self._mHelpText = aHelpText
        self._mShortOpt = aShortOpt

    # Return the parameters in an expanded list that might look like:
    # ["--lsf.sla", "--lsf.sla", 1, {"default":"trg-dvClass", "metavar":""},
    # "- Specify LSF serice class for scheduling polcy"]
    # Which will be passed to command line parameter parsing utilities to
    # handle.
    def getExpandedList(self):
        ret_list = [
            self._shortOptionString(),
            self._optionString(),
            self._mNumArgs,
        ]
        if self._mAdditionalArgs is not None:
            ret_list.append(self._mAdditionalArgs)
        else:
            ret_list.append({"default": argparse.SUPPRESS, "metavar": ""})
        ret_list.append(self._mHelpText)

        return ret_list

    # Get short option string to be passed to command line option parser
    #
    def _shortOptionString(self):
        if self._mShortOpt is None:
            return self._optionString()
        return self._mShortOpt

    # Get option string to be passed to command line option parser
    #
    def _optionString(self):
        opt_str = "--" + self._mName
        return opt_str

    # Get option variable to use to look up option value from parsed command
    # line result.
    def _optionVar(self):
        return self._mName

    # Resolve parameter based on command line option received.
    def resolveParameter(self, aCmdLineOptions):
        specified = False
        opt_value = aCmdLineOptions.option_def(self._optionVar())
        if opt_value is None:
            opt_value = self._getDefaultValue()
        else:
            specified = True
        return opt_value, specified


#  Class for option that is controlled by a command line option with optional
#  associated environment variable.
#
class AppCmdLineOption(CommandLineOption):
    def __init__(
        self,
        aName,
        aDefault,
        aNumArgs,
        aAdditionalArgs,
        aHelpText,
        aShortOpt=None,
        aEnvVar=None,
    ):
        super().__init__(
            aName,
            aDefault,
            aNumArgs,
            aAdditionalArgs,
            aHelpText,
            aShortOpt,
            aEnvVar,
        )
        self.mAppName = None

    # Get option string to be passed to command line option parser
    #
    def _optionString(self):
        opt_str = "--"
        if self.mAppName is not None:
            opt_str += self.mAppName + "."
        opt_str += self._mName
        return opt_str

    # Get option variable to use to look up option value from parsed command
    # line result.
    def _optionVar(self):
        opt_var = ""
        if self.mAppName is not None:
            opt_var += self.mAppName + "."
        opt_var += self._mName
        return opt_var


#  Class for AppCmdLineOption that is meant to be specifying a file path
#
class AppPathCmdLineOption(AppCmdLineOption):
    def __init__(
        self,
        aName,
        aDefault,
        aNumArgs,
        aAdditionalArgs,
        aHelpText,
        aShortOpt=None,
        aEnvVar=None,
    ):
        super().__init__(
            aName,
            aDefault,
            aNumArgs,
            aAdditionalArgs,
            aHelpText,
            aShortOpt,
            aEnvVar,
        )

    def _optPathChecks(self, aValue, aProgramPath):
        opt_value = aValue

        if (len(opt_value) > 0) and (opt_value[0] != "/"):
            opt_value = (
                PathUtils.include_trailing_path_delimiter(aProgramPath)
                + opt_value
            )
        opt_value = PathUtils.real_path(opt_value)

        if not PathUtils.valid_path(opt_value):
            raise FileNotFoundError(
                "Path resolution for [%s] failed, [%s] could not be located"
                % (self._optionString(), opt_value)
            )

        if PathUtils.check_dir(opt_value):
            opt_value = PathUtils.include_trailing_path_delimiter(opt_value)

        return opt_value

    # Resolve parameter based on command line option received.
    def resolveParameter(self, aCmdLineOptions):
        opt_value, specified = super().resolveParameter(aCmdLineOptions)

        new_opt_value = []
        if isinstance(opt_value, list):
            for value in opt_value:
                new_opt_value.append(
                    self._optPathChecks(value, aCmdLineOptions.mProgramPath)
                )
        else:
            new_opt_value = self._optPathChecks(
                opt_value, aCmdLineOptions.mProgramPath
            )

        return new_opt_value, specified


#  Base parameter processor class
#
class ParameterProcessor(object):
    def __init__(self, aAppOptions, aCmdLineOptions):
        self.mAppParameters = ApplicationParameters()
        self.mAppParameters.resolveParameters(aAppOptions, aCmdLineOptions)
        self.mAppParameters.setParameter("version", [])


#  Options only used in control item level and not the master_run command line
#  level
#
class ControlItemOption(ApplicationOption):
    def __init__(self, aNameTuple, aOtherNameTuple, aDefault, aEnvVar=None):
        preferred_name, connector = aNameTuple
        super().__init__(preferred_name, aDefault, aEnvVar)
        self._mPreferredConnector = connector
        self._mOtherName, self._mOtherConnector = aOtherNameTuple

    # Check if the option is specified in the passed in command line.
    #
    def isSpecified(self, aCmdLine):
        # Connector is either a space or '='
        check_names = [
            self._mName + self._mPreferredConnector,
            self._mOtherName + self._mOtherConnector,
        ]
        for check_name in check_names:
            if check_name is None:
                continue

            check_name = " " + check_name
            if aCmdLine.find(check_name) != -1:
                return True

        return False

    # Append the default option to passed in command line, _mName is the
    # prefered name to use here, long or short.
    #
    def formatDefaultOption(self):
        def_value = self._getDefaultValue()
        def_format = self._mName + self._mPreferredConnector
        if def_value is not None:
            def_format += str(def_value)
        return def_format
