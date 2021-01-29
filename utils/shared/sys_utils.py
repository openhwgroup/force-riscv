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
import subprocess
import sys
import time
from subprocess import PIPE, TimeoutExpired

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
    def open_output(cls, arg_fout=None, arg_ferr=None):

        Msg.dbg("arg_fout: %s, arg_ferr: %s" % (str(arg_fout), str(arg_ferr)))
        try:
            my_fout = SysUtils.ifthen(
                arg_fout is None, PIPE, open(arg_fout, "w")
            )

        except Exception as arg_ex:
            Msg.err("stdout: " + str(arg_ex))
            my_fout = PIPE  # SysUtils.ifthen( my_fout is None, my_fout )

        try:
            my_ferr = SysUtils.ifthen(
                arg_ferr is None, PIPE, open(arg_ferr, "w")
            )

        except Exception as arg_ex:
            Msg.err("stderr: " + str(arg_ex))
            my_ferr = PIPE  # SysUtils.ifthen( my_ferr is None, my_ferr )
        return (my_fout, my_ferr)

    @classmethod
    def exec_process(
        cls,
        arg_cmd,
        arg_fout=None,
        arg_ferr=None,
        arg_timeout=None,
        arg_kill=False,
        arg_set_process=None,
    ):

        my_process_cmd = arg_cmd
        Msg.dbg(my_process_cmd)
        my_fout, my_ferr = SysUtils.open_output(arg_fout, arg_ferr)
        my_result = None
        my_stdout = None
        my_stderr = None

        Msg.user(
            "SysUtils.exec_process( ... , arg_set_process: %s"
            % (str(bool(arg_set_process is not None))),
            "SYS-UTILS",
        )
        Msg.flush()

        try:
            from common.datetime_utils import DateTime

            my_process = subprocess.Popen(
                shlex.split(my_process_cmd), stdout=my_fout, stderr=my_ferr
            )
            my_start_time = DateTime.Time()
            my_pid = my_process.pid
            if arg_timeout is None or not SysUtils.is_numeric(arg_timeout):
                Msg.user("Exec PID[NONE]: %s" % (str(my_pid)))
                my_result = my_process.communicate()
                Msg.user("Done PID[NONE]: %s" % (str(my_pid)))
            else:
                Msg.user("Timeout: %s" % (str(arg_timeout)))
                try:
                    my_pgid = os.getpgid(my_pid)
                    my_parent_pgid = os.getpgid(0)

                    # use callback to save an instance of the process to allow
                    if arg_set_process is not None:
                        arg_set_process(my_process)

                    my_result = my_process.communicate(timeout=arg_timeout)

                except TimeoutExpired:
                    try:
                        my_parent_pgid = os.getpgid(0)
                        my_pgid = os.getpgid(my_pid)

                        # on timeout kill the spawned process and all related
                        # sub-processes
                        if arg_kill:
                            os.killpg(os.getpgid(my_pid), signal.SIGKILL)

                        # Trigger Shutdown of spawned processes
                        else:
                            Msg.error_trace()
                            Msg.err(
                                "Process Did Not Execute Properly: %s"
                                % (my_process_cmd)
                            )
                            # my_process.kill()
                            os.kill(my_pid, signal.SIGTERM)

                    except OSError as arg_ex:
                        Msg.error(str(arg_ex))
                    finally:
                        my_result = my_process.communicate()
                        Msg.flush()
                        return (
                            my_process.returncode,
                            None,
                            "Process Timeout Occurred",
                            my_start_time,
                            my_start_time,
                        )

                except Exception as arg_ex:

                    Msg.err(str(arg_ex))
                    Msg.error_trace()

                except BaseException:
                    Msg.error_trace()

                finally:
                    Msg.user("[1] SysUtils::exec_process")
                    pass

            my_end_time = DateTime.Time()

            if my_result[0] is not None:
                my_stdout = my_result[0].decode("utf-8")

            if my_result[1] is not None:
                my_stderr = my_result[1].decode("utf-8")

            return (
                my_process.returncode,
                my_stdout,
                my_stderr,
                my_start_time,
                my_end_time,
            )

        finally:

            if not (my_fout == PIPE or my_fout.closed):
                my_fout.close()
            if not (my_ferr == PIPE or my_ferr.closed):
                my_ferr.close()

            return (
                SysUtils.PROCESS_UNABLE_TO_SPAWN,
                None,
                "Unable to Spawn Process ....",
                0,
                None,
                None,
            )

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
