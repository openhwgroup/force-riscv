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


def get_force_path():
    try:
        force_path = os.environ["FORCE_PATH"]
    except KeyError as ke:
        force_path = locate_force_path()
        print("Default FORCE_PATH=%s" % force_path)

    return force_path


def locate_force_path():
    my_path = os.path.realpath(__file__)
    print("My path: %s" % my_path)
    while True:
        head, tail = os.path.split(my_path)
        if os.path.exists(head + "/config/riscv_rv64.config"):
            return str(head)
        # not found has the root been reached?
        if head == "/":
            raise Exception("Reached the system root, force root not found")
        # not found and not at root move to the next directory up the chain
        my_path = head
