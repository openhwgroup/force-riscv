#!/usr/bin/env python3
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
import shutil
import sys


#  Prints a control dictionary out in a text form interpretable directly as a
#  Python dictionary, unlike the default string output.
#
def print_vals(aObj, aIndent=""):
    my_str = ""
    if isinstance(aObj, dict):
        my_sep = "{ "
        my_term = ""
        for my_key in aObj:
            my_str += my_term
            if aObj[my_key] == {}:
                my_str += "%s%s'%s': {}" % (aIndent, my_sep, my_key)
            elif aObj[my_key] is None:
                my_str += "%s%s'%s': None" % (aIndent, my_sep, my_key)
            elif isinstance(aObj[my_key], dict):
                my_str += "%s%s'%s':\n" % (aIndent, my_sep, str(my_key))
                my_str += print_vals(aObj[my_key], "\t%s" % (aIndent))
            elif isinstance(aObj[my_key], (int, bool)):
                my_str += "%s%s'%s': %s" % (
                    aIndent,
                    my_sep,
                    my_key,
                    str(aObj[my_key]),
                )
            else:
                my_str += "%s%s'%s': '%s'" % (
                    aIndent,
                    my_sep,
                    my_key,
                    str(aObj[my_key]),
                )
            my_sep = "  "
            my_term = ",\n"
        my_str += "\n%s}" % (aIndent)
    return my_str


#  Takes whatever _def_frun.py file exists in the working directory and
#  extracts a control file line item from it, saving to a new file.
#
def main():
    input_filename = "/_def_frun.py"
    output_filename = "./control_line_item"

    # result_t = copyfile(input_filename, output_filename)
    try:
        sys.path.append(os.path.abspath(os.getcwd()))
        import _def_frun

        # extract the dictionary item from the list
        control_line_item = _def_frun.control_items[0]

        # extract the nested options dictionary and unrestrict the seed value
        options_dict = control_line_item.get("options", {})
        options_dict.update({"seed": None})

        # modify the top level control line item dictionary and render to
        # string, removing special characters and excessive spaces.
        control_line_item.update({"options": options_dict})
        control_line_item.update(
            {"fruntoctrl": {}}
        )  # Don't automatically run fruntoctrl again
        string_form = (
            print_vals(control_line_item)
            .replace("\n", "")
            .replace("\t", " ")
            .replace("  ", " ")
            .replace("  ", " ")
        )

        # create and write the control line item to file
        with open(output_filename, "w+") as outfile:
            outfile.write(string_form)

        # Cleanup the pycache directory because master run can't do it
        shutil.rmtree(os.getcwd() + "/__pycache__")
    except BaseException:
        print("[frun_to_ctrl]: Error, frun_to_ctrl", file=sys.stderr)
        sys.exit(1)

    print("[frun_to_ctrl]: Success control line item created")
    sys.exit(0)


if __name__ == "__main__":
    main()
