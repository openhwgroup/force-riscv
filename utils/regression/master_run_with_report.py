#!/usr/bin/env python3
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

import sys, os
import shlex
import subprocess
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
    # self.ctrl_file
    # self.test_dir
    # self.force_path
    # self.label
    # self.report_only
    # self.summary_only
    # self.rtl_root
    # self.tag
    def __init__(self):
        pass

class RunWithReport(object):

    def __init__(self, aRunParms, aDirName):
        self._mRunParameters = aRunParms
        self._mDirName = aDirName

        # find Force revision
        self._mRtlRoot = self._getRtlRoot(self._mRunParameters.rtl_root)
        self._mRtlRev = self._getScmRev(self._mRtlRoot)
        self._mTestDir = self._getTestDir()
        self._mReportName = self._getReportName()
        self._mReportCommand = self._getReportCommand()
        self._mGatherCommand = self._getGatherCommand()
        self._mMasterRunCommand = None

    def printMasterRunConfig(self):
        print("FORCE path : %s" % self._mRunParameters.force_path)
        print("FORCE rev  : %d" % self._getForceRev())
        print("RTL root   : %s" % self._mRtlRoot)
        print("RTL rev    : %d" % self._mRtlRev)
        print("TEST dir   : %s" % self._mTestDir)
        print("Master Run : %s" % self._mMasterRunCommand)

    def printGatherConfig(self):
        print ("Send report only.")
        print("RTL root   : %s" % self._mRtlRoot)
        print("RTL rev    : %d" % self._mRtlRev)
        print("TEST dir   : %s" % self._mTestDir)
        print("Gather cmd : %s" % self._mGatherCommand)

    def printReportConfig(self):
        print ("Produce report only.")
        print("RTL root   : %s" % self._mRtlRoot)
        print("RTL rev    : %d" % self._mRtlRev)
        print("TEST dir   : %s" % self._mTestDir)
        print("Report cmd : %s" % self._mReportCommand)
        
    def _getForceRev(self):
        if self._mRunParameters.force_path is None:
            print ("ERROR: FORCE path is not specified.")
            sys.exit(1)
        force_rev = self._getScmRev(self._mRunParameters.force_path)
        return force_rev
        
    def reportOnly(self):
        return self._mRunParameters.report_only

    def summaryOnly(self):
        return self._mRunParameters.summary_only

    def masterOnly(self):
        return self._mRunParameters.master_only
        
    def _getScmRev(self, aPath):
        rev, err_msg = VersionCtrlUtils.get_svn_revision(aPath)
        if err_msg:
            print("ERROR: failed to get SVN version for %s : %s" % (aPath, err_msg))
            sys.exit(1)

        return rev

    def _getRtlRoot( self, aPath ):
        if aPath is not None:
            return aPath

        return self._getEnvVar("PROJ_ROOT")

    def _getEnvVar( self, var_name):
        try:
            ret_val = os.environ[var_name]
        except KeyError as ke:
            print("ERROR: Environment variable \"%s\" not defined." % var_name)
            sys.exit(1)

        return ret_val

    def _getTestDir(self):
        if self.summaryOnly() or self.reportOnly():
            if self._mRunParameters.report_dir is not None:
                return self._mRunParameters.report_dir
        
        if self._mRunParameters.test_dir is None:
            test_dir = PathUtils.current_dir() + "/"
        else:
            test_dir = self._mRunParameters.test_dir + "/"

        if self._mRunParameters.label is None:
            label_str = "general"
        else:
            label_str = self._mRunParameters.label

        if self._mRunParameters.tag is None:
            tag_str = self._getDateTimeStr()
        else:
            tag_str = self._mRunParameters.tag

        test_dir += "_" + label_str + "_" + tag_str + "_r%d" % self._getForceRev()
        return test_dir

    def _getDateTimeStr(self):
        from datetime import datetime
        my_utcdt = datetime.utcnow()
        return "%0.4d%0.2d%0.2d_%0.2d%0.2d" % ( my_utcdt.year, my_utcdt.month, my_utcdt.day, my_utcdt.hour, my_utcdt.minute )

    def _getMasterRunCommand(self):
        if self._mRunParameters.ctrl_file is None:
            print ("ERROR: A control file should be specified to run master_run")
            sys.exit(1)

        mr_command = self._mDirName + "/master_run.py --target-dir %s -f %s" % (self._mTestDir, self._mRunParameters.ctrl_file)
        extra_parms = " ".join(self._mRunParameters.xargs)
        if len(extra_parms):
            mr_command += " " + extra_parms
        return mr_command

    def runMasterRun(self):
        self._mMasterRunCommand = self._getMasterRunCommand()
        self.printMasterRunConfig()
        subprocess.run(shlex.split(self._mMasterRunCommand))

    def _getReportName(self):
        report_name = "top_force_" + PathUtils.base_name(self._mTestDir)
        return report_name

    def _getReportCommand(self):
        report_cmd = self._mDirName + "/../misc/gen_report_force.pl -m regress_sim -l %s -n regress_sim -v off -u top -s -svn %d --report_name %s" % (self._mTestDir, self._mRtlRev, self._mReportName)
        return report_cmd

    def runReportCommand(self):
        subprocess.run(shlex.split(self._mReportCommand))

    def _getGatherCommand(self):
        gather_cmd = self._mDirName + "/../misc/gather_force.pl -u top %s/top_regress_sim.rpt -q" % self._mTestDir
        if self._mRunParameters.dump_xml:
            gather_cmd += " --dump"

        # TODO use this one instead when confident the gather flow is working correctly
        #gather_cmd = self._mDirName + "/../misc/gather_force.pl -u top %s/top_regress_sim.rpt -q" % self._mTestDir
        return gather_cmd

    def runGatherCommand(self):
        subprocess.run(shlex.split(self._mGatherCommand))

    def dump(self):
        print(vars(self))

## MAIN FUNCTION of the module
#
def run_with_report(aRunParameters, aDirName):
    run_obj = RunWithReport(aRunParameters, aDirName)

    if run_obj.reportOnly():
        run_obj.printGatherConfig()
        run_obj.runGatherCommand()
        pass
    elif run_obj.summaryOnly():
        run_obj.printReportConfig()
        run_obj.runReportCommand()
    elif run_obj.masterOnly():
        run_obj.runMasterRun()
    else:
        run_obj.runMasterRun()
        run_obj.runReportCommand()
        run_obj.runGatherCommand()

        
class CommandLineParameters(object):

    usage = """
  Run master_run with regression result reporting (A temporary solution to utilize existing Perl reporting scripts)

  Example:

    %s -c control_file_fctrl.py
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
        [ "-f",  "--ctrl-file",   1,  {},
          "specify control file"
        ],
        [ "-D",  "--test-dir",    1,  {},
          "test directory"
        ],
        [ "-F",  "--force-path",  1,  {},
          "force path"
        ],
        [ "-L",  "--label",       1,  {},
          "label to use"
        ],
        [ "-M",  "--master-only", 0,  { "action":"store_true" },
          "master run only"
        ],
        [ "-O",  "--report-only", 0,  { "action":"store_true" },
          "only send report to database"
        ],
        [ "-P", "--report-dir",   1,  {},
          "directory to generate report from, only effective when using togather with --report-only or --summary-only"
        ],
        [ "-R", "--rtl-root",     1,  {},
          "specify RTL root directory"
        ],
        [ "-S", "--summary-only", 0,  { "action":"store_true" },
          "only produce and NOT send report"
        ],
        [ "-T", "--tag",          1,  {},
          "tag to use"
        ],
        [ "-X", "--dump-xml",     0,  { "action":"store_true" },
          "only dump the XML file but not send to MySQL database"
        ],
        # -h and --help is not needed, proved by default.
    ]

if __name__ == "__main__":
    from common.cmdline_utils import CmdLineParser

    cmd_line_parser = CmdLineParser(CommandLineParameters, add_help=True)
    args = cmd_line_parser.parse_args(sys.argv[1:])
    run_with_report_parms = RunWithReportParms()
    cmd_line_parser.set_parameters(run_with_report_parms)
    dir_name = os.path.dirname(sys.argv[0])
    
    #print ("args now: %s, dir name %s" % (str(args), dir_name))
    run_with_report(run_with_report_parms, dir_name)
