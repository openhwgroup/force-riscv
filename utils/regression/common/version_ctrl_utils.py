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
from pathlib import Path
import re
from common.sys_utils import SysUtils


class VersionCtrlUtils(object):
    """This class contains several methods used to interface with the various
    scm systems used by the project."""

    @classmethod
    def get_scm_data(cls, scm_type, a_path, a_cmd):
        """Returns an initialized data structure to hold SCM version data.

        :param scm_type: 'svn' or 'git'
        :type scm_type: str
        :param a_path: Project base folder
        :type a_path: str
        :param a_cmd: Command to return scm status detail
        :type a_cmd: str
        :return: Data structure for SCM data
        :rtype: dict
        """
        version_info = {'scm_type': scm_type,
                        'status': False,
                        'error_msg': None,
                        'folder': a_path,
                        'url': ''}

        cmd_output, valid = SysUtils.get_command_output(a_cmd)

        if not valid:
            version_info['error_msg'] = "Command error: %s" % cmd_output
        elif not cmd_output:
            version_info['error_msg'] = "Revision info not found"

        return version_info, cmd_output

    @classmethod
    def get_svn_revision(cls, a_path):
        """Returns an populated data structure holding svn version data.

        :param a_path: Project base folder
        :type a_path: str
        :return: Data structure holding svn data
        :rtype: dict
        """
        svn_revision_pattern = re.compile(r'^Revision: (\d+)')

        status_cmd = "svn info %s" % a_path
        version_info, cmd_output = cls.get_scm_data('svn', a_path, status_cmd)

        if version_info['error_msg']:
            return version_info

        for line in cmd_output.split('\n'):
            if line.startswith("Revision: "):
                match_obj = re.match(svn_revision_pattern, line)
                rev_value = int(match_obj.group(1))
                version_info['version'] = rev_value
                version_info['status'] = True

        return version_info

    @classmethod
    def get_git_revision(cls, a_path):
        """Returns an populated data structure holding git version data.

        :param a_path: Project base folder
        :type a_path: str
        :return: Data structure holding git data
        :rtype: dict
        """

        status_cmd = "cd %s; git reflog | awk 'NR<2'" % a_path
        version_info, cmd_output = cls.get_scm_data('git', a_path, status_cmd)

        if version_info['error_msg']:
            return version_info

        # Only process first line
        line = cmd_output.split('\n')[0]
        words = line.split()
        # Example:
        #     words = ['4419c7a', 'HEAD@{0}:', 'commit:', 'Moved',
        #              'command-line', 'parsing', 'to', 'inside',
        #              'master_run']
        version_info['version'] = words[0]
        version_info['tags'] = []
        version_info['comment'] = []

        if 'commit:' in words[1:]:
            i = words.index('commit:')
            version_info['tags'].extend(words[1:i])
            version_info['comment'] = ' '.join(words[i+1:])

        version_info['status'] = True

        # get origin URL
        cmd_output, valid = SysUtils.get_command_output("cd %s; git remote get-url origin" % a_path)

        if not valid:
            version_info['error_msg'] = "Command error: %s" % cmd_output
        elif not cmd_output:
            version_info['error_msg'] = "Revision origin URL not found"
        else:
            version_info['url'] = cmd_output

        return version_info

    @classmethod
    def get_scm_revisions(cls, a_path):
        """Returns an populated data structure holding all valid version data
        from all supported SCM tools (git and svn currently.)

        :param a_path: Project base folder
        :type a_path: str
        :return: Data structure holding all valid scm data
        :rtype: list
        """

        version_info = []

        # Get SVN info
        p = Path('{}/{}'.format(a_path, '.svn'))
        if p.exists() and p.is_dir():
            l_ver = cls.get_svn_revision(a_path)
            if l_ver:
                version_info.append(l_ver)

        # Get GIT info
        p = Path('{}/{}'.format(a_path, '.git'))
        if p.exists() and p.is_dir():
            l_ver = cls.get_git_revision(a_path)
            if l_ver:
                version_info.append(l_ver)

        return version_info

    @classmethod
    def get_version_output(cls, version_data):
        """Returns a formatted string containing all pertinent SCM data,
        ready for logging.

        :param version_data: Result of get_scm_revisions()
        :type version_data: list
        :return: Formatted string of SCM data
        :rtype: str
        """
        out_line_fmt = "{}, scm_type: {}, revision number: {}, location: {}, url: {}"
        version_output = ""
        for item in version_data:
            if item.get('status', False):
                version_output += out_line_fmt.format('RTL',
                                                      item['scm_type'],
                                                      str(item['version']),
                                                      item["folder"],
                                                      item['url'])
        return version_output
