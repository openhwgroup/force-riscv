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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from base.InstructionMap import InstructionMap
from DV.riscv.trees.instruction_tree import *
from base.ChoicesModifier import ChoicesModifier
from DV.riscv.trees.instruction_tree import *

class MainSequence(Sequence):
    """
    This test case uses the genFreePagesRange API to generate multiple consecutive 
    virtual pages.

    Self-checking
    1) If the 1st element of the returned tuple is a boolean 'is_valid' indicator.
       If the value is false, then the test case is failed.
    2) Using the returned start and stop address range string to calculate the total 
       size of the created virtual pages and checking that with the expected size 
       based on the pages requested.
    3) Generated ldst_instructions using the addresses in the virtual pages created.
       If there is no translation support for the returned address the generation of 
       the ldst_instrucion will fail to gen.

    """


    def generate(self, **kwargs):

        if self.getGlobalState('AppRegisterWidth') == 32:
            available_page_sizes = ( '4K', '4M' )
            page_size_dict = { '4K': 0x1000, '4M': 0x400000 }            
        else:
            available_page_sizes = ( '4K', '2M', '1G', '512G')
            page_size_dict = { '4K': 0x1000, '2M': 0x200000, '1G': 0x40000000, '512G': 0x8000000000 }

        for number_of_pages in range(2,20):

            requested_page_sizes_string = ''
            total_page_bytes = 0

            for i in range(number_of_pages):
                page_size_list = (self.sample(available_page_sizes, 1))
                total_page_bytes += page_size_dict[page_size_list[0]]
                requested_page_sizes_string += (page_size_list[0] + ',')

            self.notice("")
            self.notice(">>>>>>>>   number_of_pages is:  {}".format(
                    number_of_pages))
            self.notice(">>>>>>>>   req_page_size   is:  {}".format(
                    requested_page_sizes_string))
            self.notice(">>>>>>>> total_page_bytes  is:  {:X}".format(
                    total_page_bytes))

            # return format is a tuple with these elements:  
            #   is_valid, starting_address, start_end_range, 
            #   page_size[0], page_size[1]  [,page_size[n] ]
            result = self.genFreePagesRange(Number=number_of_pages, 
                     PageSize=requested_page_sizes_string)



            # Checking  ================================
            #    Check the "is_valid"
            if result[0]:
                self.notice(">>>>>>>>>>>>>>>>>  Valid result returned \
                            from genFreePagesRange")
                self.notice(">>>>>>>>>>>>>>>>>  starting_address:  {}".format(
                            result[1]))
                self.notice(">>>>>>>>>>>>>>>>>  start_end_range:   {}".format(
                            result[2]))
                for element in result[3:]:
                    self.notice(">>>>>>>>>>>>>>>>>  page size:     {}".format(
                            element))
            else:
                self.error("The self.genFreePagesRange(Number={}) failed \
                            to generate.".format(number_of_pages))


            # Parse the start_end_range for the start address and end address 
            # (both in hex strings) and convert to integers.
            # Subtract the start address from the end address and add 1 to 
            # get the address_range_size (in bytes).
            address_start_n_stop = result[2].split('-')
            starting_address = int(address_start_n_stop[0], 16)
            ending_address = int(address_start_n_stop[1], 16)
            address_range_size = ending_address - starting_address + 1
            start_end_range = result[2]

            if address_range_size == total_page_bytes:  
                self.notice(">>>>>>>>>>>>  Range Size Check passed. <<<<<<<<<")
            else:  
                self.error(">>>>>>>>>>>>>> Expected Range Size of:  {:12X}   \
                           The actual was:  {:12X}".format(
                           total_page_bytes, address_range_size))

            self.notice(">>>>>>>>>>>>   Address Range:   {}".format(
                    start_end_range))

            target_address = self.genVA(Size=64, Align=64, 
                    Range=start_end_range, Type='D', Shared=0)
            self.notice(">>>>>>>>>>>>   LSTarget address:  {:#x}".format(
                    target_address))

            if self.getGlobalState('AppRegisterWidth') == 32:
                the_instruction = self.pickWeighted(LDST32_All_instructions)
            else:
                the_instruction = self.pickWeighted(LDST_All_instructions)
            self.genInstruction(the_instruction, {'LSTarget': target_address})
            self.notice(">>>>>>>>>>>>   Generated instruction is {}".format(
                    the_instruction))


# Set up the valid paging choices, in case they are not enabled by default
def gen_thread_initialization(gen_thread):

    satp_info = gen_thread.getRegisterInfo("satp", gen_thread.getRegisterIndex('satp') )
    rv32 = satp_info['Width'] == 32
    
    choices_mod = ChoicesModifier(gen_thread)

    # Make the pages small, so it is easy to generate a mapping that crosses 
    # page boundaries

    if rv32:
        choices_mod.modifyPagingChoices('Page size#4K granule#S#stage 1', {'4K': 25, '4M': 25} )
    else:
        choices_mod.modifyPagingChoices('Page size#4K granule#S#stage 1', {'4K': 25, '2M': 25, '1G': 25, '512G': 25} )
            
    choices_mod.commitSet()


GenThreadInitialization = gen_thread_initialization

## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

