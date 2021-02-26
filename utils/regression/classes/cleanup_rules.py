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
from common.path_utils import PathUtils

# The keepRequest could be
# True: keep everything
# False: don't keep anything
# A string, it will specify what type of files to keep only


class CleanUpRules(object):
    def __init__(self, keepRequest):
        self.keepAll = False
        self.keepNone = False
        self.baseNamesToKeep = []
        self.typesToKeep = None
        self.fileBaseName = None

        if keepRequest == "all":
            self.keepAll = True
        elif len(keepRequest) == 0:
            self.keepNone = True
        else:
            self.typesToKeep = keepRequest.split(",")

    def shouldKeepAll(self):
        return self.keepAll

    def shouldKeepFile(self, fileName):
        self.fileBaseName = PathUtils.base_name(fileName)
        ret_val = False

        if self.fileBaseName in self.baseNamesToKeep:
            ret_val = True

        if self.keepNone:
            ret_val = False  # keep none of the rest

        if self.keepAll:
            ret_val = True
        
        if self.typesToKeep is not None:
            for typeToKeep in self.typesToKeep:
                if self.fileBaseName.endswith(typeToKeep):
                    ret_val = True
                    break

        return ret_val 

    def setBaseNamesToKeep(self, baseNames):
        self.baseNamesToKeep = baseNames

    def lastBaseName(self):
        return self.fileBaseName
