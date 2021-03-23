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


#  A minimal object to hold the application parameters
#
class ApplicationParameters(object):
    def __init__(self):
        self._mParameters = dict()

    def resolveParameters(self, aAppOptions, aCmdLineOptions):
        for app_option in aAppOptions:
            opt_value, opt_specified = app_option.resolveParameter(
                aCmdLineOptions
            )
            self._mParameters[app_option.name()] = opt_value
            self._mParameters[
                app_option.name() + " from cmdline"
            ] = opt_specified

    def parameter(self, aParmName):
        return self._mParameters[aParmName]

    def setParameter(self, aParmName, aParmValue):
        self._mParameters[aParmName] = aParmValue

    def commandLineSpecified(self, aParmName):
        return self._mParameters[aParmName + " from cmdline"]


#  Configuration information for individual application
#
class ApplicationConfig(object):
    def __init__(self, aName, aAppModule):
        self.mName = aName
        self.mModule = aAppModule
        if hasattr(self.mModule, "Tag"):
            self._mTag = self.mModule.Tag
        else:
            self._mTag = self.mName

        self.mAppParameters = None

    # Return application name
    #
    def name(self):
        return self.mName

    # Return application config's tag
    #
    def tag(self):
        return self._mTag

    # Return application specific command line options if any.
    #
    def getCmdLineOptions(self):
        return self.mModule.CmdLineOptions

    # Return application specific parameters processor if any.
    #
    def getParametersProcessorClass(self):
        return self.mModule.ParametersProcessorClass

    # Return certain application parameter value.
    #
    def parameter(self, aParmName):
        return self.mAppParameters.parameter(aParmName)

    # Call upon application specific facility to process control data
    #
    def processControlData(self, aControlData):
        self.mModule.ProcessControlData(aControlData, self.mAppParameters)

    # Create executor
    #
    def createExecutor(self):
        return self.mModule.ExecutorClass()

    # Create reporter
    #
    def createReporter(self):
        return self.mModule.ReporterClass()


#  Container of all application components.
#
class ApplicationsInfo(object):

    # Init with LSF info passed in.
    #
    def __init__(self, aLsfInfo):
        self.mLsfInfo = aLsfInfo
        self.mAllAppsOrder = list()
        self.mAllAppsOrder.append(self.mLsfInfo)
        self.mSingleRunApps = list()
        self.mSequenceApps = list()
        self.mCmdLineOpts = None
        self.mTagToApp = dict()
        self._registerTag(aLsfInfo)
        self.mMainAppPath = None
        self.mProcessMax = None
        self.mTestBaseDir = None
        self.mToolPath = None
        self.mMode = None
        self.mNumTestsCount = 0
        self._mNamesWithIndex = dict()
        self.mTagToReportInfo = dict()
        self.mConfigPath = None

    # Add single run application
    #
    def addSingleRunApplication(self, aSingleRunApp):
        self.mSingleRunApps.append(aSingleRunApp)
        self._addApplication(aSingleRunApp)

    # Add application
    def _addApplication(self, aApp):
        self.mAllAppsOrder.append(aApp)
        self._registerTag(aApp)

    # Add sequence application.
    #
    def addSequenceApplication(self, aSeqApp):
        self.mSequenceApps.append(aSeqApp)
        self._addApplication(aSeqApp)

    # Register the applications' tag.
    def _registerTag(self, aAppCfg):
        if aAppCfg.tag() in self.mTagToApp.keys():
            raise Exception(
                "Registering application %s with tag %s that already exists."
                % (aAppCfg.name(), aAppCfg.tag())
            )
        self.mTagToApp[aAppCfg.tag()] = aAppCfg

    # Look up an application config by using tag.
    #
    def getAppConfig(self, aAppTag):
        return self.mTagToApp[aAppTag]

    # Increment the test count for use with count mode
    #
    #  Note, this is not an atomic add because we stay single threaded in
    #  count mode
    #
    def incrementTestCount(self):
        self.mNumTestsCount += 1

    # Mainly used in creating indices for sub-tasks, but somewhat generalized
    # to handle names with index.
    #
    def getNextIndex(self, aName):
        next_index = 0
        if aName in self._mNamesWithIndex:
            next_index = self._mNamesWithIndex[aName] + 1

        # update saved index.
        self._mNamesWithIndex[aName] = next_index
        return next_index
