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
from common.msg_utils import Msg
from datetime import datetime
from common.path_utils import PathUtils
import report
import os.path

class RtlReporter(object):
    def __init__(self):
        self.mReportNameBase = None #"name of top level control file"
        self.mForceScmVersion = None #"query for parameter version from dictionary in shared object"
        self.mRtlWasEnabled = None #"even though this was a standard module, was it actually used?"
        self.mReportingWasSkipped = None #"if the user deliberately said no reporting?"
        self.mReportDumpXml = None #"do we want an xml dump instead of an upload?"
        self.mRegressionOutputDirectory = None #"where is that data we are using to generate the report"
        self.mTotalCycleCount = None 
        self.mTotalInstructionCount = None

    # Resolve the values of the member variables using information available from the top level inter module shared object and perform reporting
    #
    def report(self, aAppsInfo, aMyTag):

        mr_info = aAppsInfo.mTagToReportInfo.get("master_run", None)
        if mr_info:
            self.mTotalCycleCount = mr_info.get("total_cycle_count", None)
            self.mTotalInstructionCount = mr_info.get("total_instruction_count", None)
            self.mReportBaseName = mr_info.get("initial_control_file", None)
            self.mRegressionOutputDirectory = mr_info.get("output_dir", None)

        force_info = aAppsInfo.mTagToReportInfo.get("generator", None)
        if force_info:  # force info was in the report info dictionary
            self.mForceScmVersion = force_info.get("version", None)
        else:  # force info was obtainable from the config object
            force_info = aAppsInfo.mTagToApp.get('generator')
            self.mForceScmVersion = force_info.parameter('version')

        rtl_config = aAppsInfo.mTagToApp.get('rtl', None)
        if rtl_config:
            self.mRtlWasEnabled = rtl_config.parameter('rtl')
        
            # If the rtl app was active, check if the user disabled reporting
            if self.mRtlWasEnabled:
                self.mReportingWasSkipped = rtl_config.parameter('rtl.report.skip')

                # If reporting was enabled, continue to resolve information
                if not self.mReportingWasSkipped:
                    self.mReportDumpXml = rtl_config.parameter('rtl.report.xml')
                    rtl_report_name = rtl_config.parameter('rtl.report.name')

                    if rtl_report_name == "master_run" and self.mReportBaseName:
                        rtl_report_name = self.mReportBaseName.strip(".py")

                    time_stamp = datetime.utcnow()
                    time_string = "_%0.4d%0.2d%0.2d_%0.2d%0.2d" % \
                                  (time_stamp.year,
                                   time_stamp.month,
                                   time_stamp.day,
                                   time_stamp.hour,
                                   time_stamp.minute)

                    version = ''
                    revision_stamp = ""

                    for item in self.mForceScmVersion:
                        if item.get('status', False):
                            version = item['version']
                            # prefer git data, otherwise keep looking for valid version
                            if item['scm_type'] == 'git':
                                break
                    if version:
                        revision_stamp = "_r" + str(version)

                    rtl_report_name = rtl_report_name + time_string + revision_stamp

                    rtl_control_data = {}
                    rtl_config.processControlData(rtl_control_data)
                    report_script_path = rtl_control_data.get('report_script_path', None)
                    report_script_path = PathUtils.real_path(aAppsInfo.mMainAppPath + "/" + report_script_path)
                    report_directory = PathUtils.real_path(self.mRegressionOutputDirectory + "/../../")

                    # What about the report path?  We get that from the master_run output_dir
                    report_command = report_script_path + " --report-dir " + report_directory + " --name " + rtl_report_name
                    if self.mReportDumpXml:
                        report_command += " --dump-xml"

                    Msg.dbg("Main app path: " + str(aAppsInfo.mMainAppPath))
                    Msg.dbg("Report script path: " + str(report_script_path))
                    Msg.dbg("Report directory: " + str(report_directory))
                    Msg.dbg("Reporting command: " + str(report_command))
        
                    # actually execute the report script
                    report.callFromPython(report_command)

                    # create or append metrics file
                    rtl_metrics_path = rtl_control_data.get('rtl_metrics_path', None)
                    home = os.path.expanduser('~')
                    rtl_metrics_path = rtl_metrics_path.replace('~', home) 

                    if not os.path.isfile(rtl_metrics_path):
                        with open(rtl_metrics_path, 'w+') as outfile:
                            outfile.write("||---------------------------------------------|--------------------|--------------------||\n")
                            outfile.write("||                                   Regression|        Total Cycles|  Total Instructions||\n")
                            outfile.write("||---------------------------------------------|--------------------|--------------------||\n")

                    with open(rtl_metrics_path, 'a') as outfile:
                        outfile.write("||")
                        for space in range(45 - len(rtl_report_name)):
                            outfile.write(" ")
                        outfile.write(rtl_report_name + "|")
                        for space in range(20 - len(str(self.mTotalCycleCount))):
                            outfile.write(" ")
                        outfile.write(str(self.mTotalCycleCount) + "|" )
                        for space in range(20 - len(str(self.mTotalInstructionCount))):
                            outfile.write(" ")
                        outfile.write(str(self.mTotalInstructionCount) + "||\n")                                                 
