#!/usr/bin/env python3
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

import sys, os
from common.sys_utils import SysUtils
from common.version_ctrl_utils import VersionCtrlUtils
from common.path_utils import PathUtils

## RunWithReportParms class.
#
# Basically uses same arguments from force_regr.pl script
# Not supported switches:
# -cov (coverage)
# -e (extra delay)
# -mp (P test)
# -np (no preload)

class RunWithReportParms(object):

    # pass this object to CmdLineParser and the parameters will be populated by its set_parameters method
    # parameters:
    # self.report_dir
    # self.summary_only
    # self.dump_xml
    def __init__(self):
        pass

class RunWithReport(object):

    def __init__(self, aRunParms, aDirName):
        self._mRunParameters = aRunParms
        self._mDirName = aDirName
        OUTPUT_SUBDIR = "/output/regression"

        # find Force revision
        #self._mRtlRoot = self._getRtlRoot(self._mRunParameters.rtl_root)
        self._mTestDir = self._getTestDir()
        self._mRtlRev = self._getSvnRev(self._mTestDir + OUTPUT_SUBDIR, "rtl", "revision number:")
        self._mForceRev = self._getSvnRev(self._mTestDir + OUTPUT_SUBDIR, "force", "revision number:")
        self._mReportName = self._getReportName()
        self._mReportCommand = self._getReportCommand()
        self._mGatherCommand = self._getGatherCommand()

    def printGatherConfig(self):
        print ("Send report config.")
        print("RTL rev    : %d" % self._mRtlRev)
        print("TEST dir   : %s" % self._mTestDir)
        print("Gather cmd : %s" % self._mGatherCommand)

    def printReportConfig(self):
        print ("Produce report config.")
        print("RTL rev    : %d" % self._mRtlRev)
        print("TEST dir   : %s" % self._mTestDir)
        print("Report cmd : %s" % self._mReportCommand)
        
    def summaryOnly(self):
        return self._mRunParameters.summary_only

    def _getSvnRev( self, aPath, aClue, aRevisionHeading):
        rev = None
        version_info_filename = os.path.join(aPath, "version_info.txt")
        print("Filename is: " + str(version_info_filename))
        if os.path.isfile(version_info_filename):
            with open(version_info_filename, "r") as infile:
                for line in infile:
                    if aClue in line:
                        chunks = line.split(",")
                        for chunk in chunks:
                            if aRevisionHeading in chunk:
                                rev = int(chunk[len(aRevisionHeading)+1:])                   
        print("Got version number: " + str(rev))
        return rev

    def _getTestDir(self):
        if self._mRunParameters.report_dir is not None:
            return os.path.realpath(self._mRunParameters.report_dir)
    
    def _getDateTimeStr(self):
        from datetime import datetime
        my_utcdt = datetime.utcnow()
        return "%0.4d%0.2d%0.2d_%0.2d%0.2d" % ( my_utcdt.year, my_utcdt.month, my_utcdt.day, my_utcdt.hour, my_utcdt.minute )

    def _getReportName(self):
        report_name = self._mRunParameters.name #+"_r" + str(self._mForceRev)
        return report_name

    def _getReportCommand(self):
        report_cmd =  self._mDirName + "/../misc/gen_report_force.pl -m regress_sim -l %s -n regress_sim -v off -u top -s -svn %d --report_name %s" % (self._mTestDir, self._mRtlRev, self._mReportName)
        return report_cmd

    def runReportCommand(self):
        os.system(self._mReportCommand)

    def _getGatherCommand(self):
        #Always XML dump regardless of command line arguments for now. Preventing slip-ups while this is still in development.
        gather_cmd = self._mDirName + "/../misc/gather_force.pl -u top %s/top_regress_sim.rpt -q" % self._mTestDir
        if self._mRunParameters.dump_xml:
            gather_cmd += " --dump"

        # TODO use this one instead when confident the gather flow is working correctly
        #gather_cmd = self._mDirName + "/../misc/gather_force.pl -u top %s/top_regress_sim.rpt -q" % self._mTestDir
        return gather_cmd

    def runGatherCommand(self):
        os.system(self._mGatherCommand)

    def dump(self):
        print(vars(self))

## MAIN FUNCTION of the module
#
def run_with_report(aRunParameters, aDirName):
    run_obj = RunWithReport(aRunParameters, aDirName)

    if run_obj.summaryOnly():
        run_obj.printReportConfig()
        run_obj.runReportCommand()
    else:
        run_obj.printReportConfig()
        run_obj.runReportCommand()
        run_obj.printGatherConfig()
        run_obj.runGatherCommand()
        
class CommandLineParameters(object):

    usage = """
  To generate a report from master_run regression output.

  Example:

    %s -P /path/to/regression/output 
""" % sys.argv[0]

    # save and pass on remainder
    pass_remainder = True

    # do not allow abbrev parameters, only in Python >3.5 
    # allow_abbrev = False 
    
    parameters = [
        # "short option"     "number of additonal args" 
        # |      "long option"    |   "additional specifications"
        # |      |                |   |
        # |      |                |   |
        [ "-p", "--report-dir",   1,  { "required":True },
          "directory to generate report from, this should contain a subdirectory output/regression"
        ],
        [ "-s", "--summary-only", 0,  { "action":"store_true" },
          "only produce and NOT send report"
        ],
        [ "-x", "--dump-xml",     0,  { "action":"store_true" },
          "only dump the XML file but not send to MySQL database"
        ],
        [ "-n", "--name", 1, { "default":"master_run" },
          "sets the name of the regression report"
        ]
        # -h and --help is not needed, provided by default.
    ]

def callFromPython(aCommandLineString):
    from common.cmdline_utils import CmdLineParser
    command_list = aCommandLineString.split(" ")

    cmd_line_parser = CmdLineParser(CommandLineParameters, add_help=True)
    args = cmd_line_parser.parse_args(command_list[1:])
    run_with_report_parms = RunWithReportParms()
    cmd_line_parser.set_parameters(run_with_report_parms)
    dir_name = os.path.dirname(command_list[0])
    
    #print ("args now: %s, dir name %s" % (str(args), dir_name))
    run_with_report(run_with_report_parms, dir_name)

if __name__ == "__main__":
    from common.cmdline_utils import CmdLineParser

    cmd_line_parser = CmdLineParser(CommandLineParameters, add_help=True)
    args = cmd_line_parser.parse_args(sys.argv[1:])
    run_with_report_parms = RunWithReportParms()
    cmd_line_parser.set_parameters(run_with_report_parms)
    dir_name = os.path.dirname(sys.argv[0])
    
    #print ("args now: %s, dir name %s" % (str(args), dir_name))
    run_with_report(run_with_report_parms, dir_name)
