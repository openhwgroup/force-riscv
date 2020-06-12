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
import datetime
import time

#from time import time
from datetime import date, datetime, timedelta, timezone, tzinfo
from common.sys_utils import SysUtils


class DateTime( object ):

    @classmethod
    def YMD( arg_class, arg_utcdt = None ):
        my_utcdt = SysUtils.ifthen( arg_utcdt is None, datetime.utcnow(), arg_utcdt )
        return "%0.4d%0.2d%0.2d" % ( my_utcdt.year, my_utcdt.month, my_utcdt.day )

    @classmethod
    def HMS( arg_class, arg_utcdt = None ):
        my_utcdt = SysUtils.ifthen( arg_utcdt is None, datetime.utcnow(), arg_utcdt )
        return "%0.2d%0.2d%0.2d" % ( my_utcdt.hour, my_utcdt.minute, my_utcdt.second )

    @classmethod
    def YMD_HMS( arg_class, arg_utcdt = None ):
        my_utcdt = SysUtils.ifthen( arg_utcdt is None, datetime.utcnow(), arg_utcdt )
        return "%0.4d%0.2d%0.2d-%0.2d%0.2d%0.2d" % ( my_utcdt.year
                                                   , my_utcdt.month
                                                   , my_utcdt.day
                                                   , my_utcdt.hour
                                                   , my_utcdt.minute
                                                   , my_utcdt.second
                                                   )
    @classmethod
    def DateAsStr( arg_class, arg_utcdt = None ):
        my_utcdt = SysUtils.ifthen( arg_utcdt is None, datetime.utcnow(), arg_utcdt )
        return str( my_utcdt.date())

    @classmethod
    def TimeAsStr( arg_class, arg_utcdt = None ):
        my_utcdt = SysUtils.ifthen( arg_utcdt is None, datetime.utcnow(), arg_utcdt )
        return str( my_utcdt.time())

    @classmethod
    def UTCNow( arg_class ):
        return datetime.utcnow()

    @classmethod
    def Time( arg_class ):
        return time.time()

    @classmethod
    def DateDelta( arg_delta = 1 ):
        return datetime.utcnow() - timedelta( days=arg_delta)


