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
from common.data_utils import indices


## This class contains several methods used to interface with the various scm systems used by the project.
#
class VersionCtrlUtils(object):
    """This class contains several methods used to interface with the various
    scm systems used by the project.
    """

    ## Returns an initialized data structure to hold SCM version data.
    #
    @classmethod
    def get_scm_data(cls, scm_type, a_path, a_cmd, a_cwd=None):
        """Returns an initialized data structure to hold SCM version data.

        :param str scm_type: 'svn' or 'git'
        :param str a_path: Project base folder
        :param str a_cmd: Command to return scm status detail
        :return: dict containing SCM data
        :rtype: dict
        """
        version_info = {'scm_type': scm_type,
                        'status': False,
                        'error_msg': None,
                        'folder': a_path,
                        'url': ''}
        cmd_output, valid = SysUtils.get_command_output(a_cmd, arg_cwd=a_path)

        if not valid:
            version_info['error_msg'] = "Command error: %s" % cmd_output
        elif not cmd_output:
            version_info['error_msg'] = "Revision info not found"

        return version_info, cmd_output

    ## Returns a populated data structure holding svn version data.
    #
    @classmethod
    def get_svn_revision(cls, a_path):
        """Returns a populated data structure holding svn version data.

        :param str a_path: Project base folder
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

    ## Scan a_string and parse out the tags and comment
    #
    @classmethod
    def parse_git_log_line(cls, a_string):
        """Scan a_string and parse out the tags and comment

        :param str a_string: Result from 'git log' after removing the version #
        """
        data_dict = {'tags': [], 'comment': a_string}

        if '(' not in a_string or ')' not in a_string:
            return data_dict

        start = indices(a_string, '(')[0]
        end = indices(a_string, ')')[0]

        if end < start:
            return data_dict

        data_dict['tags'] = a_string[start + 1: end].split(',')
        data_dict['comment'] = a_string[end + 1:].strip()

        return data_dict

    ## Returns a populated data structure holding git version data.
    #
    @classmethod
    def get_git_revision(cls, a_path):
        """Returns a populated data structure holding git version data.

        :param str a_path: Project base folder
        :return: Data structure holding git data
        :rtype: dict
        """
        status_cmd = "git log --oneline -n1 --decorate "
        version_info, cmd_output = cls.get_scm_data('git', a_path, status_cmd, a_cwd=a_path)
        if version_info['error_msg']:
            return version_info

        # Only process first line
        line = cmd_output.split('\n')[0]
        spaces = indices(line, ' ')

        if not spaces:
            version_info['error_msg'] = "Command error: %s" % cmd_output

        version_info['version'] = line[:spaces[0]]
        parsed_data = cls.parse_git_log_line(line[spaces[0]+1:])
        version_info.update(parsed_data)
        version_info['status'] = True

        # get origin URL
        cmd_output, valid = SysUtils.get_command_output("git remote get-url origin", arg_cwd=a_path)

        if not valid:
            version_info['error_msg'] = "Command error: %s" % cmd_output
        elif not cmd_output:
            version_info['error_msg'] = "Revision origin URL not found"
        else:
            version_info['url'] = cmd_output

        return version_info

    ## Returns a populated data structure holding all valid version data from all supported SCM tools (git and svn currently.)
    #
    @classmethod
    def get_scm_revisions(cls, a_path):
        """Returns a populated data structure holding all valid version data
        from all supported SCM tools (git and svn currently.)

        :param str a_path: Project base folder
        :return: Data structure holding all valid scm data
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

    ## Returns a formatted string containing all pertinent SCM data, ready for logging.
    #
    @classmethod
    def get_version_output(cls, a_version_data):
        """Returns a formatted string containing all pertinent SCM data,
        ready for logging.

        :param list a_version_data: Result of get_scm_revisions()
        :return: Formatted string of SCM data
        """
        out_line_fmt = "scm_type: {}, revision number: {}, location: {}, url: {}"
        version_output = ""
        for item in a_version_data:
            if item.get('status', False):
                version_output += out_line_fmt.format(item['scm_type'],
                                                      str(item['version']),
                                                      item["folder"],
                                                      item['url'])
            if item.get('tags', []):
                version_output += ", tags: {}".format(item['tags'])
            if item.get('comment', []):
                version_output += ", comment: {}".format(item['comment'])

        return version_output
