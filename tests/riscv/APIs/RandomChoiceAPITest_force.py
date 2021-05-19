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
# [scenario]
# demonstrate and validate usage of
# -- choice and choicePermutated

# import instruction_tree module from the current directory
from DV.riscv.trees.instruction_tree import *
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


class MainSequence(Sequence):
    def generate(self, **kargs):

        instrs_tree = (
            ALU_Int32_All_instructions
            if self.getGlobalState("AppRegisterWidth") == 32
            else ALU_Int_All_instructions
        )

        # test sample function - note sample function doesn't support
        # dictionary for the input sequence!
        sample_list = []
        for (instr, weight) in sorted(instrs_tree.items()):
            sample_list.append(instr)
        samples = self.sample(sample_list, 5)
        for item in samples:
            self.notice("Samples: %s" % item)

        # obtain the start address
        cur_addr = self.getPEstate("PC")
        self.notice("Start Addr: 0x%x" % cur_addr)

        myInstrList = list()
        self.notice("Generate instructions...")

        # Use self.choice to pick from a list
        self.notice("Randomly pickup a simple instruction from list...")
        instr = self.choice(list(instrs_tree.keys()))
        instr_rec = self.genInstruction(instr)
        self.notice("%s <<== %s" % (instr_rec, instr))
        myInstrList.append(instr_rec)

        # Use self.choice to pick from a tuple
        self.notice("Randomly pickup a simple instruction from tuple...")
        instr1 = self.choice(list(instrs_tree.keys()))
        instr2 = self.choice(list(instrs_tree.keys()))
        instr_pair = (instr1, instr2)
        instr = self.choice(instr_pair)
        instr_rec = self.genInstruction(instr)
        self.notice("%s <<== %s" % (instr_rec, instr))
        myInstrList.append(instr_rec)

        # Use self.choice to pick from a dictionary
        self.notice("Randomly pick an instruction from dictionary...")
        one_instr, weight = self.choice(instrs_tree)
        instr_rec = self.genInstruction(one_instr)
        self.notice("%s <<== %s" % (instr_rec, one_instr))
        myInstrList.append(instr_rec)

        # Use self.choicePermutated to pick from a dictionary
        self.notice("Permutate from dictionary and pick one at a time...")
        for instr, weight in self.choicePermutated(instrs_tree):
            instr_rec = self.genInstruction(instr)
            self.notice("%s <<== %s" % (instr_rec, instr))
            myInstrList.append(instr_rec)

        self.notice("Query generated instruction information...")
        self.notice("!!! Randomly pick from the instruction record to print !!!")

        for instr in myInstrList:
            instr_query_result = self.queryInstructionRecord(instr)
            key, value = self.choice(instr_query_result)

            if key == "Opcode" or key == "PA" or key == "VA" or key == "IPA":
                self.notice("%s ==>> %s = 0x%x" % (instr, key, value))
            elif key == "Name" or key == "Group":
                self.notice("%s ==>> %s   = %s" % (instr, key, value))
            elif key == "LSTarget" or key == "BRTarget":
                self.notice("%s ==>>%s= 0x%x" % (instr, key, value))
            # search from Dests or Srcs list
            elif key == "Dests" or key == "Srcs":
                self.notice("%s ==>> %s  ==>>" % (instr, key))

                # value is a list
                mylist = []
                for k, v in value.items():
                    self.notice("%s:%d" % (k, v))
                    mylist.append((k, v))
                # get register information
                for (k, v) in self.choicePermutated(mylist):
                    register_id = "x{}".format(v)
                    self.notice("Register: %s:%s:%d" % (k, register_id, v))
                    reg_dict = self.getRegisterInfo(register_id, v)
                    self.notice("Register: %s:%s:%d" % (k, register_id, v))
                    for a, b in self.choicePermutated(reg_dict):
                        if a == "Type":
                            self.notice("%s:%s" % (a, b))
                        elif a == "Value":
                            self.notice("%s:0x%x" % (a, b))
                        elif a == "LargeValue":
                            for val in b:
                                self.notice("%s:0x%x" % (a, val))
                        else:
                            self.notice("%s:%d" % (a, b))
            # search from immediate list
            elif key == "Imms":
                self.notice("%s ==>> Imms   = " % instr)
                for k, v in value.items():
                    self.notice("%s:%d" % (k, v))
            # search from status list
            elif key == "Status":
                self.notice("%s ==>> Status = " % instr)
                name = ""
                index = 0
                for k, v in value.items():
                    self.notice("%s:%d" % (k, v))
                    name = k
                    index = v
                # get status register information
                if name:
                    self.notice("Randomly pick one status register information...")
                    reg_dict = self.getRegisterInfo(name, index)
                    a, b = self.choice(reg_dict)
                    if a == "Type":
                        self.notice("%s:%s" % (a, b))
                    elif a == "Value":
                        self.notice("%s:0x%x" % (a, b))
                    else:
                        self.notice("%s:%d" % (a, b))
            # search from addressing name list
            if key == "Addressing":
                self.notice("%s ==>>Addressing... = " % instr)
                for k, v in self.choicePermutated(value):
                    self.notice("%s :" % k)
                    # print(v)


#  Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

#  Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

#  Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
