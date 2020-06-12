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
class UsageStr(object): pass
class CmdLine (object): pass
class Defaults(object): pass

UsageStr = """
Single Task Regression and Performance Execute Utility
  -h, --help          - when present, displays this message
  -x, --process-max   - The max number of concurrent processes
  -l,--msg-lev=       - Set the output level, implemented as a bit mask example if you want warnings and debug pass 20, 4 + 16
                            "nomsg"   [0x0000]- supresses all message
                            "crit"    [0x0001]- Critical Errors Messages         "err"     [0x0002]- Non Critical Errors Messages
                            "warn"    [0x0004]- Include Warning Messages         "info"    [0x0008]- Run Info Messages
                            "dbg"     [0x0010]- Debug Information                "user"    [0x0020]- Special Messages
                            "trace"   [0x0040]- provides call stack trace info   "noinfo"  [0x0080]- supresses the info label
                        Example: "err"+"warn"+"dbg" will post messages
                            [ERROR] - Shows Non Critical Error Messages     and
                            [WARN]  - shows warning messages                and
                            [DEBUG] - Shows debug messages                  and nothing else

                        Default: "crit+err+warn+info+noinfo"
                        To use the default log level and add levels prepend that level with a (+)
                        To use the default log level and remove levels prepend that level with a (-)
                        Example:
                            [-l +trace-crit+user] will create an output level err+warn+info+noinfo+trace+user
"""

class Defaults(object):
    msg_level = "crit+err+warn+info+noinfo"
    process_max = 16
    help = False

class CmdLine(object):
    # Allowed Command Line Switches
    Switches = [ "help"            # 0
               , "process-max="    # 1
               , "msg-lev="        # 2
               ]

    # Help can triggered with a single letter
    ShortOpts = "hx:l:"

    # command Switch Index
    help         = 0
    process_max  = 1
    msg_lev      = 2

    # Short Opts
    Help       = "-h"
    MsgLevel   = "-l"
    ProcessMax = "-x"

