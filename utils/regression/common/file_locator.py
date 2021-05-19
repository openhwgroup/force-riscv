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
from common.path_utils import PathUtils


#  A class that contains some related convenience functions for using
#  PathUtils for resolving file locations
#
class FileLocator(object):

    # Join the members of a list into a single string using appropriate path
    # separators and then check if it exists.
    #  input: aPathList - a list of strings
    #
    #  output: tuple(string, boolean) - a joined path, the result of testing
    #       the filepath's existence
    #
    @classmethod
    def checkNestedPath(cls, aPathList):
        # validate the input
        if not isinstance(aPathList, list):
            raise Exception("Input to FileLocator.checkNestedPath must be a list")
        if len(aPathList) < 2:
            raise Exception(
                "Input to FileLocator.checkNesterPath needs to be a list" "more than 1 item long"
            )

        joined_path = str(aPathList[0])

        for path in aPathList[1:]:
            joined_path = PathUtils.real_path(PathUtils.append_path(joined_path, path))

        # validate the joined path
        if not PathUtils.check_found(joined_path):
            return joined_path, False
        else:
            return joined_path, True

    # Determine if the user specified a usable absolute or relative filepath
    #  input: aInputString - a string parsed from the users options or
    #       onfiguration, may be an absolute path or a relative path that
    #       makes sense when used with one of the prefix paths
    #  input: aPrefixDirectoryList - a list of absolute paths to prepend to
    #       the aInputString in sequence to search for an existing resolvable
    #       filepath
    #
    #  output: tuple(string, boolean) - best filepath candidate and a status
    #       flag that indicates if the filepath could be resolved.
    #
    @classmethod
    def resolveFileLocation(cls, aInputString, aPrefixDirectoryList):
        # validate the input
        if not isinstance(aPrefixDirectoryList, list):
            raise Exception("Parameter aPrefixDirectoryList needs to be a list of strings")

        # is the input string itself a usable absolute path?
        if aInputString.startswith("/") and PathUtils.check_found(aInputString):
            return aInputString, True

        # is the input string a relative path meaningful with one of the
        # prefix directories prepended?
        intermed_dir, file_name = PathUtils.split_path(aInputString)
        for directory in aPrefixDirectoryList:
            joined_path, outcome = (
                FileLocator.checkNestedPath([directory, intermed_dir, file_name])
                if intermed_dir is not None
                else FileLocator.checkNestedPath([directory, file_name])
            )
            if outcome:
                return joined_path, True

        # the input string does not form an existing filepath
        return aInputString, False
