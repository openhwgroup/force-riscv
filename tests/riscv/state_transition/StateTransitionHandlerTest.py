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
from base.StateTransitionHandler import StateTransitionHandler

## A test StateTransitionHandler.
class StateTransitionHandlerTest(StateTransitionHandler):

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        return True
