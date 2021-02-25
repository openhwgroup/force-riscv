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
import importlib
import os
import sys
from importlib.machinery import SourceFileLoader

from classes.ApplicationsInfo import ApplicationsInfo, ApplicationConfig
from common.cmdline_utils import CmdLineOpts
from common.msg_utils import Msg
from master_init import Defaults


#  Manage applications that make up the master run workflow.
#
class ApplicationsSetup(object):
    """Manage applications that make up the master run workflow."""

    def __init__(
        self,
        aCmdLineParameters,
        aSysArgs,
        aConfigPath=None,
        aSingleRun=False,
        aAddOpts=False,
    ):
        """Initialize ApplicationSetup object."""
        lsf_info = self._getAppConfig("lsf")
        self._mAppsInfo = ApplicationsInfo(lsf_info)

        # Setting the config path for access in master run (so we can pass it
        # to forrest run)
        self._mAppsInfo.mConfigPath = aConfigPath

        # Getting workflow lists
        (single_run_app_opts, sequence_app_opts) = self._getWorkflow(
            aConfigPath
        )

        # Populate single run applications options
        if aSingleRun:
            for app_name, app_opts in single_run_app_opts:
                app_info = self._getAppConfig(app_name)
                self._mAppsInfo.addSingleRunApplication(app_info)
                self._addWorkflowToSysArgs(aSysArgs, app_name, app_opts)

        # Populate sequence applications options
        for app_name, app_opts in sequence_app_opts:
            app_info = self._getAppConfig(app_name)
            self._mAppsInfo.addSequenceApplication(app_info)
            if (
                aSingleRun
            ):  # We don't need to add these to forrest run's command line
                self._addWorkflowToSysArgs(aSysArgs, app_name, app_opts)

        # Add options from applications if necessary
        if aAddOpts:
            self._addAppOptions(aCmdLineParameters)

        self._mAppsInfo.mCmdLineOpts = CmdLineOpts(
            aCmdLineParameters, aSysArgs
        )

        # Let applications that have added parameters receive the parsed
        # result.
        if aAddOpts:
            self._resolveParameters()

    # Adds options to provided system arguments to resolve later
    #
    def _addWorkflowToSysArgs(self, aSysArgs, aAppName, aAppOpts):
        """Adds options to provided system arguments to resolve later"""
        for option, value in aAppOpts.items():
            argument = "--%s.%s" % (aAppName, option)
            if argument in aSysArgs:
                if value is not None:
                    option_index = aSysArgs.index(argument) + 1
                    new_argument = [aSysArgs[option_index]]
                    # Add new argument to existing options
                    if isinstance(value, list):
                        new_argument.extend(value)
                        aSysArgs[option_index] = new_argument
                    else:
                        new_argument.append(value)
                        aSysArgs[option_index] = new_argument
            else:  # Argument does not exist yet
                aSysArgs.append(argument)
                if value is not None:
                    aSysArgs.append(value)

    # Retrieve workflow from the provided path to add to aSysArgs to parse
    # when building mCmdLineOpts later
    #
    def _getWorkflow(self, aConfigPath):
        """Retrieve workflow from the provided path to add to aSysArgs to
        parse when building mCmdLineOpts later"""
        # Check workflow in config file first
        try:
            Msg.info("Using workflow from: %s" % aConfigPath)
            config_module = SourceFileLoader(
                "config", aConfigPath
            ).load_module()
            return (
                config_module.single_run_app_opts,
                config_module.sequence_app_opts,
            )
        except AttributeError:
            # aConfigPath is None or the specified config file does not
            # contain workflow
            if aConfigPath:
                Msg.info("Workflow improperly defined in %s" % aConfigPath)
            else:
                Msg.info("Config not specified.")

        # Check environment variable next
        try:
            Msg.info(
                "Attempting to use MASTER_RUN_CONFIG environment variable."
            )
            config_module = SourceFileLoader(
                "config", os.environ.get("MASTER_RUN_CONFIG")
            ).load_module()
            return (
                config_module.single_run_app_opts,
                config_module.sequence_app_opts,
            )
        except AttributeError:
            if os.environ.get("MASTER_RUN_CONFIG"):
                Msg.info(
                    "Workflow improperly defined in MASTER_RUN_CONFIG: "
                    "%s" % os.environ.get("MASTER_RUN_CONFIG")
                )
            else:
                Msg.info("MASTER_RUN_CONFIG environment variable is not set.")
        except FileNotFoundError:  # MASTER_RUN_CONFIG environment variable
            # is set, but cannot be found
            Msg.err(
                "MASTER_RUN_CONFIG is currently set to %s. "
                "Please ensure that it exists."
                % os.environ.get("MASTER_RUN_CONFIG")
            )
            sys.exit(1)  # Assume typo so quit

        # Use default last
        try:
            default_config_file = "%s/config/%s" % (
                sys.path[0],
                Defaults.fcfg_name,
            )
            Msg.info(
                "Using workflow from default config file: %s"
                % default_config_file
            )
            config_module = SourceFileLoader(
                "config", default_config_file
            ).load_module()
            return (
                config_module.single_run_app_opts,
                config_module.sequence_app_opts,
            )
        except FileNotFoundError:  # default config file cannot be found
            Msg.err("Please ensure the default config file exists.")
            sys.exit(1)  # Assume typo so quit

    # Return the ApplicationsInfo object
    #
    def getApplicationsInfo(self):
        """Return the ApplicationsInfo object"""
        return self._mAppsInfo

    # Import and create instance of ApplicationConfig for the requested
    # application
    #
    def _getAppConfig(self, aAppName):
        """Import and create instance of ApplicationConfig for the requested
        application"""
        app_module = importlib.import_module("." + aAppName, "applications")
        app_cfg = ApplicationConfig(aAppName, app_module)
        return app_cfg

    # Iterate through all ApplicationConfig objects to add application
    # specific command line parameters
    #
    def _addAppOptions(self, aCmdLineParameters):
        """Iterate through all ApplicationConfig objects to add application
        specific command line parameters"""
        for app_cfg in self._mAppsInfo.mAllAppsOrder:
            app_cmdline_opts = app_cfg.getCmdLineOptions()
            if app_cmdline_opts is not None:
                aCmdLineParameters.group_names.append(
                    app_cmdline_opts.cGroupName
                )
                aCmdLineParameters.group_descriptions.append(
                    app_cmdline_opts.cGroupDescription
                )
                app_options_expanded = list()
                for app_option in app_cmdline_opts.cOptions:
                    app_option.mAppName = app_cfg.mName
                    app_options_expanded.append(app_option.getExpandedList())
                aCmdLineParameters.group_parameters.append(
                    app_options_expanded
                )
                aCmdLineParameters.parameters.extend(app_options_expanded)

    # Iterate through all ApplicationConfig objects to pass back parsed
    # command line options and resolve application parameters.
    #
    def _resolveParameters(self):
        """Iterate through all ApplicationConfig objects to pass back parsed
        command line options and resolve application parameters."""
        for app_cfg in self._mAppsInfo.mAllAppsOrder:
            app_parms_processor_cls = app_cfg.getParametersProcessorClass()
            if app_parms_processor_cls is not None:
                app_parms_processor = app_parms_processor_cls(
                    self._mAppsInfo.mCmdLineOpts
                )
                app_cfg.mAppParameters = app_parms_processor.mAppParameters
