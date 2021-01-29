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
import os
import shlex
import signal
import socket
import sys
import time

from common.msg_utils import Msg


class SysType:
    Unknown = 0
    String = 1
    Integer = 2
    Float = 3
    Tuple = 4
    List = 5
    Dictionary = 6


# Series of system utilities
class SysUtils:
    PROCESS_TIMEOUT = -5
    PROCESS_UNABLE_TO_SPAWN = -4

    # mimics the c++ conditional statement
    @classmethod
    def ifthen(cls, arg_test, arg_true_val, arg_false_val):

        if arg_test:
            return arg_true_val
        return arg_false_val

    @classmethod
    def ifkey(cls, arg_key, arg_dict, arg_def_val):
        if arg_key in arg_dict:
            return arg_dict[arg_key]
        return arg_def_val

    @classmethod
    def percent(cls, arg_num, arg_denom):
        if bool(arg_denom):
            return 100 * arg_num / arg_denom
        return 0.0

    @classmethod
    def envar(cls, arg_key, arg_defval=None, arg_def_warn=True):
        try:
            return os.environ[arg_key]
        except KeyError as arg_ex:
            if arg_defval is None:
                Msg.warn(
                    arg_key + " Not found in environmental variables, "
                    "NO DEFAULT VALUE SPECIFIED!"
                )
            elif arg_def_warn:
                Msg.warn(
                    arg_key + " Not found in environmental variables, "
                    'using default value "' + str(arg_defval) + '"'
                )
        return arg_defval

    @classmethod
    def envar_set(cls, arg_var, arg_val):
        os.environ[arg_var] = arg_val
        return SysUtils.envar(arg_var, arg_val) == arg_val

    @classmethod
    def envar_unset(cls, arg_var):
        del os.environ[arg_var]
        return SysUtils.envar(arg_var, "default") == "default"

    @classmethod
    def check_host(cls, arg_teststr):
        my_hostname = socket.gethostname()
        return my_hostname.find(arg_teststr) == 0

    @classmethod
    def is_numeric(cls, arg_var, arg_no_complex=True):
        if arg_no_complex:
            return isinstance(arg_var, int) or isinstance(arg_var, float)
        return (
            isinstance(arg_var, int)
            or isinstance(arg_var, float)
            or isinstance(arg_var, complex)
        )

    @classmethod
    def sleep(cls, arg_interval):
        time.sleep(arg_interval / 1000)

    @classmethod
    def found(cls, arg_pos):
        return arg_pos >= 0

    @classmethod
    def success(cls, arg_retcode):
        return arg_retcode is not None and int(arg_retcode) == int(0)

    @classmethod
    def failed(cls, arg_retcode):
        return not SysUtils.success(arg_retcode)
