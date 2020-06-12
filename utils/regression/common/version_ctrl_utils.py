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
from common.path_utils import PathUtils
from common.sys_utils import SysUtils
import re

class VersionCtrlUtils(object):

    svn_revision_pattern = re.compile(r'^Revision: (\d+)')

    # Get SVN version of the specified path, return error message if encountered error
    @classmethod
    def get_svn_revision( arg_class, arg_path ):
        if not PathUtils.check_dir(arg_path):
            return 0, "Invalid path: %s" % arg_path

        cmd_output, valid = SysUtils.get_command_output("svn info %s" % arg_path)
        if not valid:
            return 0, "Command error: %s" % cmd_output

        for line in cmd_output.split('\n'):
            if line.startswith("Revision: "):
                match_obj = re.match(VersionCtrlUtils.svn_revision_pattern, line)
                rev_value = int(match_obj.group(1))
                return rev_value, None

        return 0, "Revision info not found"
