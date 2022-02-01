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
from common.msg_utils import Msg
from datetime import datetime
from common.path_utils import PathUtils
import report
import os.path


class RtlReporter(object):
    def __init__(self):
        self.m_report_name_base = None  # "name of top level control file"

        # "query for parameter version from dictionary in shared object"
        self.m_force_scm_version = None

        # "even though this was a standard module, was it actually used?"
        self.m_rtl_was_enabled = None

        self.m_reporting_was_skipped = None  # "if the user deliberately said no reporting?"
        self.m_report_dump_xml = None  # "do we want an xml dump instead of an upload?"
        self.m_regression_output_directory = (
            None  # "where is that data we are using to generate the report"
        )
        self.m_total_cycle_count = None
        self.m_total_instruction_count = None
        self.rtl_config = None
        self.rtl_report_name = None

    def setup_report(self, a_apps_info):
        mr_info = a_apps_info.mTagToReportInfo.get("master_run", None)
        if mr_info:
            self.m_total_cycle_count = mr_info.get("total_cycle_count", None)
            self.m_total_instruction_count = mr_info.get("total_instruction_count", None)
            self.m_report_name_base = mr_info.get("initial_control_file", None)
            self.m_regression_output_directory = mr_info.get("output_dir", None)

        force_info = a_apps_info.mTagToReportInfo.get("generator", None)
        if force_info:  # force info was in the report info dictionary
            self.m_force_scm_version = force_info.get("version", None)
        else:  # force info was obtainable from the config object
            force_info = a_apps_info.mTagToApp.get("generator")
            self.m_force_scm_version = force_info.parameter("version")

        self.rtl_config = a_apps_info.mTagToApp.get("rtl", None)
        if not self.rtl_config:
            return False

        self.m_rtl_was_enabled = self.rtl_config.parameter("rtl")

        # If the rtl app was active, check if the user disabled reporting
        if not self.m_rtl_was_enabled:
            return False

        self.m_reporting_was_skipped = self.rtl_config.parameter("rtl.report.skip")

        # If reporting was enabled, continue to resolve information
        if self.m_reporting_was_skipped:
            return False

        self.m_report_dump_xml = self.rtl_config.parameter("rtl.report.xml")
        self.rtl_report_name = self.rtl_config.parameter("rtl.report.name")

        if self.rtl_report_name == "master_run" and self.m_report_name_base:
            self.rtl_report_name = self.m_report_name_base.strip(".py")

        return True

    # Resolve the values of the member variables using information available
    # from the top level inter module shared object and perform reporting
    def report(self, a_apps_info, a_my_tag):

        if not self.setup_report(a_apps_info):
            return

        time_stamp = datetime.utcnow()
        time_string = "_%0.4d%0.2d%0.2d_%0.2d%0.2d" % (
            time_stamp.year,
            time_stamp.month,
            time_stamp.day,
            time_stamp.hour,
            time_stamp.minute,
        )

        version = ""
        revision_stamp = ""

        for item in self.m_force_scm_version:
            if item.get("status", False):
                version = item["version"]
                # prefer git data, otherwise keep looking for
                # valid version
                if item["scm_type"] == "git":
                    break
        if version:
            revision_stamp = "_r" + str(version)

        rtl_report_name = self.rtl_report_name + time_string + revision_stamp

        rtl_control_data = {}
        self.rtl_config.processControlData(rtl_control_data)
        report_script_path = rtl_control_data.get("report_script_path", None)
        report_script_path = PathUtils.real_path(
            a_apps_info.mMainAppPath + "/" + report_script_path
        )
        report_directory = PathUtils.real_path(self.m_regression_output_directory + "/../../")

        # What about the report path?  We get that from the
        # master_run output_dir
        report_command = (
            report_script_path + " --report-dir " + report_directory + " --name " + rtl_report_name
        )
        if self.m_report_dump_xml:
            report_command += " --dump-xml"

        Msg.dbg("Main app path: " + str(a_apps_info.mMainAppPath))
        Msg.dbg("Report script path: " + str(report_script_path))
        Msg.dbg("Report directory: " + str(report_directory))
        Msg.dbg("Reporting command: " + str(report_command))

        # actually execute the report script
        report.call_from_python(report_command)

        # create or append metrics file
        rtl_metrics_path = rtl_control_data.get("rtl_metrics_path", None)
        home = os.path.expanduser("~")
        rtl_metrics_path = rtl_metrics_path.replace("~", home)

        if not os.path.isfile(rtl_metrics_path):
            with open(rtl_metrics_path, "w+") as outfile:
                outfile.write(
                    "||------------------------------------------"
                    "---|--------------------|-------------------"
                    "-||\n"
                )
                outfile.write(
                    "||                                   "
                    "Regression|        Total Cycles|  Total "
                    "Instructions||\n"
                )
                outfile.write(
                    "||-------------------------------------------"
                    "--|--------------------|-------------------"
                    "-||\n"
                )

        with open(rtl_metrics_path, "a") as outfile:
            outfile.write("||")
            for _ in range(45 - len(rtl_report_name)):
                outfile.write(" ")
            outfile.write(rtl_report_name + "|")
            for _ in range(20 - len(str(self.m_total_cycle_count))):
                outfile.write(" ")
            outfile.write(str(self.m_total_cycle_count) + "|")
            for _ in range(20 - len(str(self.m_total_instruction_count))):
                outfile.write(" ")
            outfile.write(str(self.m_total_instruction_count) + "||\n")
