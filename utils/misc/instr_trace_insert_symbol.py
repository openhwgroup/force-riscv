#!/usr/bin/env python3
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

import getopt, sys
import os
import os.path
import re

def usage():
    usage_str = """Insert symbols into simulator instruction trace file
  -l, --log specify the generator log file
  -t, --trace specify the name of the simulator instruction trace file.
  -h, --help print this help message

Example:

%s -l gen.log -t fpix_sim.log

  For this example, the output file will be: fpix_sim.log.symbol
""" % sys.argv[0]
    print(usage_str)


pc_re = re.compile(r" PC\(VA\) 0x([0-9a-f]+) ")

# find the PC match in a supposedly PC line
def get_pc(simtrace_line, symbols_dict):
    pc_match = re.search(pc_re, simtrace_line)
    if pc_match:
        pc_value = int(pc_match.group(1), 16)
        if pc_value in symbols_dict:
            return symbols_dict[pc_value]
    return None

symbol_re = re.compile(r" ([0-9a-f]+) (.+)$")

# collect symbols from a generator log file
def collect_symbols(gen_log_file, symbols_dict):
    with open(gen_log_file) as gen_log:
        for line in gen_log:
            if line.find('Front-end: __SYMBOL__ ') != -1:
                line = line[:-1] # remove line return
                line_match = re.search(symbol_re, line)
                addr = int(line_match.group(1), 16)
                symbol_text = '__SYMBOL__: ' + line_match.group(2)
                if addr in symbols_dict:
                    symbols_dict[addr] += '\n' + symbol_text
                else:
                    symbols_dict[addr] = symbol_text

# Read simtrace, insert symbol lines before corresponding lines with the specified PC
def insert_symbols(simtrace_trace, symbols_dict):
    with open(simtrace_trace) as simtrace_input:
        output_file_name = simtrace_trace + ".symbol"
        with open(output_file_name, 'w') as simtrace_output:
            for line in simtrace_input:
                if line.find(' PC(VA) ') != -1:
                    symbols_to_add = get_pc(line, symbols_dict)
                    if symbols_to_add is not None:
                        simtrace_output.write(symbols_to_add + "\n")
                    
                simtrace_output.write(line)


def simtrace_insert_symbol():

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hl:t:", ["help", "log=", "simtrace="])
    except getopt.GetoptError as err:
        print (err)
        usage()
        sys.exit(1)

    gen_log_file = "gen.log"
    simtrace_trace = "fpix_sim.log"

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-l", "--log"):
            gen_log_file = a
        elif o in ("-t", "--simtrace"):
            simtrace_trace = a
        else:
            raise ValueError("Unhandled option.")

    symbols_dict = dict()

    collect_symbols(gen_log_file, symbols_dict)

    if len(symbols_dict) == 0:
        print('No symbols found.')
        sys.exit(0)

    insert_symbols(simtrace_trace, symbols_dict)

if __name__ == "__main__":
    simtrace_insert_symbol()
