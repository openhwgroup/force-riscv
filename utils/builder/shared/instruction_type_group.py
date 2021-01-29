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
import os.path
import sys
import subprocess

import time

class IGroupByType(object):
    def __init__(self, type_name):
        super(IGroupByType, self).__init__()
        self.name = type_name
        self.instructions  = list()

    def add_instruction(self, instr, instr_type):
        if instr_type != self.name:
            print(" Failed to add instruction %s with type %s" % (instr.name, instr_type))
        self.instructions.append(instr)
    
    def print_test(self, force_path):
        instrs_list = list()
        for instr in self.instructions:
            instrs_list.append("\'" + instr.get_full_ID() + "\'" + " : 10")
        instr_names = ",\n                      ".join(instrs_list)  
        instrs_weight_str = "        self." + self.name+"_dict = {%s}\n" % instr_names
        with open(force_path + "/py/riscv/InstructionsWeight.py", "a") as instrs_weight_handle:
            instrs_weight_handle.write(instrs_weight_str)

        template_name = "T%d-" % len(self.instructions) + self.name + "_force.py"
        template_name = template_name.replace("|", "Or") # | will be mistaken as file pipe in bash
        with open(template_name, "w") as output_handle:
            from perf_test_template import test_template_str
            test_str = test_template_str % self.name
            output_handle.write(test_str)
        
        return template_name
 
class InstructionTypeGroup(object):
    def __init__(self):
        self.IDict = dict()  # <Key : Value> = <Type, IGroupByType>
        self.run_list = list()   # each element is a tuple <test_name, instr_num>

    def add_instruction(self, instr):
        instr_type = instr.group
        if instr_type in self.IDict:
            igrp = self.IDict[instr_type]
        else:
            igrp = IGroupByType(instr_type)
            self.IDict[instr_type] = igrp

        igrp.add_instruction(instr, instr_type)

    def print_tests(self, force_path):
        with open(force_path + "/py/riscv/InstructionsWeight.py", "w") as instrs_weight_handle:
            from instrs_weight_template import instrs_weight_str
            instrs_weight_handle.write(instrs_weight_str)

        for key, igrp in sorted(self.IDict.items()):
            test_name = igrp.print_test(force_path)
            instr_num = len(igrp.instructions)
            self.run_list.append((test_name, instr_num))

    def run_tests(self, force_path, test_dir):
        print ("Entering: InstructionTypeGroup::run_tests" )
        print( "Force Path: " + str( force_path ))
        total_time = 0
        total_instr_num = 0
        
        sys.path.append(force_path + "/utils")

        #from summary_classes import InstructionGroupSummary, PerformanceSummary, PathUtils
        #from shared.path_utils import PathUtils
        
        my_force_path = force_path
        if not my_force_path.endswith( "/" ):
            my_force_path += "/" 
        
        my_testpath = my_force_path + test_dir 
        
        #my_perf_summary = PerformanceSummary( my_testpath )
        
        for (test_name, instr_num) in self.run_list:
            #my_instr_grp_summ = InstructionGroupSummary( my_testpath, test_name )  
            print("Running test case \"%s\"" % test_name)
            start_time = time.time()
            # my_instr_grp_summ.start(time.time()) 
            
            
            
            my_retcode = os.system( my_force_path  \
                     + "utils/regression/quick_run.py --no-sim --no-asm --test-only --force-path %s --dir=%s --test=%s" \
                     % (force_path, test_dir, test_name))
                     
            end_time = time.time()
            # my_instr_grp_summ.end(time.time()) 
            print("Time elapsed in seconds:%.2f" % (end_time - start_time))
            print("IPS %.2f for test \"%s\"" % (instr_num/(end_time - start_time),  test_name))
          
            total_time += end_time - start_time
            total_instr_num += instr_num
            # my_perf_summary.append( my_instr_grp_summ )
            
        # my_perf_summary.summarize()    
        print("IPS %.2f for all test" % (total_instr_num/total_time) )
        print ("Leaving: InstructionTypeGroup::run_tests" )
         

        
