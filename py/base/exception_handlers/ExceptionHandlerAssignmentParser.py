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
import json
import os

## This class parses JSON data files that map exception classes to default exception handler
# classes.
class ExceptionHandlerAssignmentParser(object):

    ## Parse the specified file and return a map from an exception class name and optional
    # subexception class name to a tuple consisting of the module name and class name of an
    # exception handler. Only two levels of exception codes are currently supported; files
    # specifying more than two levels will not parse correctly.
    #
    #  @param aAssignmentFilePath The path to the file to parse.
    def parseHandlerAssignments(self, aAssignmentFilePath):
        handler_assignments = {}
        with open(aAssignmentFilePath) as assignment_file:
            assignments = json.loads(assignment_file.read())

            # We skip the first element, which is the license string
            for assignment in assignments[1:]:
                if 'ExceptionSubhandlers' in assignment:
                    subassignment_file_path = os.path.join(os.path.dirname(aAssignmentFilePath), assignment['ExceptionSubhandlers'])
                    handler_subassignments = self.parseHandlerAssignments(subassignment_file_path)

                    exception_class_name = assignment['ExceptionCode']
                    for ((subexception_class_name, _), (handler_module_name, handler_class_name)) in handler_subassignments.items():
                        handler_assignments[(exception_class_name, subexception_class_name)] = (handler_module_name, handler_class_name)
                else:
                    exception_class_name = assignment['ExceptionCode']
                    handler_module_name = assignment['ExceptionHandler']['ExceptionHandlerModule']
                    handler_class_name = assignment['ExceptionHandler']['ExceptionHandlerClass']
                    handler_assignments[(exception_class_name, None)] = (handler_module_name, handler_class_name)

        return handler_assignments
