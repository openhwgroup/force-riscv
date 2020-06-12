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
import sys

from random import randint

from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.path_utils import PathUtils

from executors.app_executor import *

class CompileExecutor(AppExecutor):

    def __init__(self):
        super().__init__()
        self.mMakefilePath = None # Path to the Makefile
        self.mOptions = None # Options string

        self.log = 'compile.log'
        self.elog = 'compile.err'

    def load(self, aCtrlItem):
        super().load(aCtrlItem)
        self.mMakefilePath = self.ctrl_item.compile.get('path')
        self.mOptions = self.ctrl_item.compile.get('options')

    def skip(self):
        if not self.ctrl_item.compile.get('run', False):
            Msg.user('[CompileExecutor::skip] skipping since run is not set to True...')
            return True

        Msg.user('[CompileExecutor::skip] not skipping')
        return False

    def execute(self):
        try:
            cmd = 'make compile -C %s %s' % (self.mMakefilePath, self.mOptions)
            Msg.user('Compile command = ' + str({'compile-cmd':cmd}))

            result = SysUtils.exec_process(cmd, self.log, self.elog, self.ctrl_item.timeout, True)
            Msg.user('Compile result = ' + str(result))

        except:
            print('[CompileExecutor] problem in execute method: ', sys.exc_info()[0])
            raise

        return SysUtils.success(int(result[0]))

    def pre(self):
        if self.ctrl_item.compile.get('mp'):
            cmd = '%s/script/lsu_resize.py -scd_num %d -sca_num %d -scb_starve %d' % (self.mMakefilePath, randint(1, 16), randint(1, 12), randint(0, 1))
            Msg.user('MP pre-compile command = %s' % cmd)
            result = SysUtils.exec_process(cmd, self.log, self.elog, self.ctrl_item.timeout, True)
            Msg.user('MP pre-compile command result = %s' % str(result))

    def post(self):
        if self.ctrl_item.compile.get('mp'):
            lsu_folder = PathUtils.real_path('%s/../../../rtl/lsu' % self.mMakefilePath)
            cmd = 'svn revert %s/lsu_scb_retire_ctrl.vp %s/lsu_scb_sca_array.vp %s/lsu_scb_scd_array.vp' % (lsu_folder, lsu_folder, lsu_folder)
            Msg.user('MP post-compile command = %s' % cmd)
            result = SysUtils.exec_process(cmd, self.log, self.elog, self.ctrl_item.timeout, True)
            Msg.user('MP post-compile command = %s' % str(result))

