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
import sys
from io import StringIO


## Outputs obj representation into a readable format.
#
def dump_repr(obj, lvl=0, file=sys.stdout):
    """Outputs obj representation into a readable format.

    :param object obj: Object to be displayed hierarchically
    :param int lvl: Level of indention.  Mostly used internally.
    :param io.TextIOWrapper file: FileHandle for output
    """
    indent = '   '
    if isinstance(obj, dict):
        print('%s{' % (lvl * indent), file=file)
        for key, val in obj.items():
            if hasattr(val, '__iter__') and not isinstance(val, str):
                print('%s%s:' % ((lvl + 1) * indent, repr(key)), file=file)
                dump_repr(val, lvl + 1, file=file)
            else:
                print('%s%s: %s,' %
                      ((lvl + 1) * indent, repr(key), repr(val)), file=file)
        print('%s},' % (lvl * indent), file=file)
    elif isinstance(obj, list):
        complex_data = False
        for val in obj:
            if hasattr(val, '__iter__'):
                complex_data = True
                break
        if complex_data:
            print('%s[' % (lvl * indent), file=file)
            for val in obj:
                if hasattr(val, '__iter__'):
                    dump_repr(val, lvl + 1, file)
                else:
                    print('%s%s,' % (
                        (lvl + 1) * indent, repr(val)), file=file)
            print('%s],' % (lvl * indent), file=file)
        else:
            print('%s%s' % ((lvl + 1) * indent, obj), file=file)
    else:
        print('%s%s,' % (lvl * indent, repr(obj)), file=file)


## Return a str with output from dump_repr()
#
def dump_str(obj):
    """Return a str with output from dump_repr()

    :param obj: Object to display hierarchically.
    :type obj: object
    :return: str
    """
    with StringIO() as buf:
        dump_repr(obj, file=buf)
        return buf.getvalue()


## Returns a list containing the indices of val in the list.
#
def indices(a_list, val):
    """Returns a list containing the indices of val in the list.

    :param list|str a_list: List to be searched.
    :param object val: Item to be found in a_list
    :return: List of integer indexes of val in a_list
    :rtype: list
    """
    return [index for index, value in enumerate(a_list) if value == val]
