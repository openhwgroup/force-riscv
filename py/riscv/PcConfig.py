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


def get_reset_pc():
    return 0x50000000


def get_initial_pc(thread_id):
    # TODO randomize
    return get_base_initial_pc() + thread_id * get_initial_pc_offset()


def get_boot_pc(thread_id):
    return get_base_boot_pc() + thread_id * get_boot_pc_offset()


def get_reset_region_size():
    return 0x40


def get_boot_region_size():
    return 0x1000


def get_base_initial_pc():
    return 0x80011000


def get_base_boot_pc():
    return 0x80000000


def get_initial_pc_offset():
    return 0x1000000


def get_boot_pc_offset():
    return 0x100000
