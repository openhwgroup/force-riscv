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
# extract.py
# comment: this is the abstract for classes that contain the extracted results created from the
#          process log file

from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.datetime_utils import DateTime
from common.errors import *

# from classes.control_item import ControlItem, CtrlItmKeys

class Extractor( object ):

    def __init__( self ):
        super().__init()

    # laod both the command info and the result info from a specific task
    def load( self, arg_request, arg_response ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Extractor::load(...) not implemented in descendent [%s]" % ( str( type( self ))))

    # publish string that is displayed in scrolling results output
    def report( self ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Extractor::report(...) not implemented in descendent [%s]" % ( str( type( self ))))

    # publish line based on desired output directives
    def publish( self, arg_sum_lev ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Extractor::publish(...) not implemented in descendent [%s]" % ( str( type( self ))))

    # returns true if extractor detects error
    def has_error( self ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Extractor::has_error(...) not implemented in descendent [%s]" % ( str( type( self ))))

    # assembles and returns error line if extractor detects error
    def error_msg( self ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Extractor::error_msg(...) not implemented in descendent [%s]" % ( str( type( self ))))

    # returns tuple
    # first element: 1 if successful otherwise returns 0
    # second element: process message
    def result( self ):
        Msg.error_trace()
        raise AbstractionError( "Abstract Method Error: Extractor::result(...) not implemented in descendent [%s]" % ( str( type( self ))))

