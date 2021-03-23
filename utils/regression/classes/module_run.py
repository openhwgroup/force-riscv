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
#  @package module_run.py
#   module_run.py
#
#   Provides command line argument parsing and access as base class of
#   master_run.py and forrest_run.py
#
import sys
import os
from abc import ABC, abstractmethod
from common.path_utils import PathUtils
from common.msg_utils import Msg

#
# Parses and accesses parsed command line options. Also can be used to set
# message level... and directly query for the force directory.


class ModuleRun(ABC):
    def __init__(self, arg_msg_lev, arg_def_lev):
        self.m_app_setup = None
        self.m_app_info = None
        self.module_dir, self.module_name = PathUtils.split_path(
            PathUtils.real_path(sys.argv[0])
        )
        self.init_app_setup()
        self.load_message_levels(arg_msg_lev, arg_def_lev)

    @abstractmethod
    def init_app_setup(self):
        pass

    ##
    # resolves the desired message types
    def load_message_levels(self, arg_msg_lev, arg_def_lev):

        # load from the command line if specified or use the default
        my_lev_str = self.option_def(arg_msg_lev, arg_def_lev)

        # if a (+) or a (-) is found then the command line will be appended
        # to or demoted by
        if (my_lev_str[0] == "+") or (my_lev_str[0] == "-"):
            # use the default string to build a format string, then append
            # the passed in value
            my_fmt_str = "%s%%s" % arg_def_lev
            my_lev_str = my_fmt_str % (my_lev_str)

        my_level = Msg.translate_levelstr(my_lev_str)

        Msg.set_level(my_level)

    # return the force path
    def get_force_dir(self):
        return self.force_path

    # Reflects the supplied default value in arg_def_val if none matching the
    # arg_switch can be found in the parsed options.
    def option_def(self, aSwitch, aDefVal=None, aConversionFunc=None):
        return_value = self.m_app_info.mCmdLineOpts.option_def(
            aSwitch, aDefVal
        )
        if aConversionFunc and return_value is not None:
            try:
                return_value = aConversionFunc(return_value)
            except (TypeError, ValueError) as ex:
                Msg.warn(
                    'Invalid value "{}" provided for "{}".  '
                    "Using default.".format(repr(return_value), aSwitch)
                )
                return aDefVal

        return return_value

    def get_arguments(self):
        return self.m_app_info.mCmdLineOpts.get_arguments()

    def get_unknown_arguments(self):
        return self.m_app_info.mCmdLineOpts.get_unknown_arguments()

    def print_help(self):
        self.m_app_info.mCmdLineOpts.print_help()
