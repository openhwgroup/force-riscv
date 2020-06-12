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
#ctrl file template strings
ctrl_items_template = 'control_items = [ '
fname_template = '"fname":"{}*_force.py"'
generator_template = ',"generator":{{{}}}'
options_template = ',"options":{{{}}}'
performance_template = ',"performance":{{{}}}'

#option strings
noiss = '"--noiss":None,'
nosim = '"no-sim":True,'
group = '"group":"{}",'
options = '"--options":"\\"{}\\"",'

#misc strings
ctrl_item_separator ='\n                  '

#arch specific strings
arch_genopts ='MMU=1,all_cacheable=1'
