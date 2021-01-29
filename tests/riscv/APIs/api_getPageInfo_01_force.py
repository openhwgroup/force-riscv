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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence

class MainSequence(Sequence):
    """Exercise different combinations of values for the parameters for the genPA instruction.
       Focus in this test is to try values of the Size, Align and CanAlias parameters. 
       Type is always 'D'; Bank is always '0'.
    """

    def generate(self, **kargs):

        ldstr_byte_ops   = ['LB##RISCV', 'SB##RISCV']
        ldstr_half_ops   = ['LH##RISCV', 'SH##RISCV']
        ldstr_word_ops   = ['LW##RISCV', 'SW##RISCV']
        ldstr_double_ops = ['LD##RISCV', 'SD##RISCV']

        theType = 'D'
        theBank = 0
        theCanAlias = 0
        loopCount = 2


        # Iterate through Size and Align values.  Force requires Align to be a power of 2.
        # This 1st block tests smaller values of size - 1 byte to 32 bytes.
        for theSize in [2 ** x for x in range(0, 5)]:

            for theAlign in [2 ** x for x in range(0, 6)]:

                if theAlign < theSize: continue

                for _ in range(loopCount):

                    rand_PA = self.genPA(Size=theSize, Align=theAlign, Type=theType, Bank=theBank, CanAlias=theCanAlias)
                    rand_VA = self.genVAforPA(PA=rand_PA, Bank=theBank, FlatMap=0, Type=theType, Size=theSize)
                    self.notice(">>>>>> Requested Alignment:  {:6d}     Requested Size:  {:6d}     PA target= {:16X}     VA target= {:16X}".format(theAlign, theSize, rand_PA, rand_VA))


                    # Bank argument must be 0 now as the 3rd argument.  May not be required at some point.
                    page_info = self.getPageInfo(rand_VA, 'VA', 0)

                    # This section displays the keys and values for the second and third level dictionaries.
                    if 'Page' in page_info.keys():
                        self.notice('>>>>>>>>>>  VA Page info   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<')
                        for k in page_info['Page']:
                            if k != 'DescriptorDetails':
                                if k == 'MemoryType' or k == 'MemoryAttr':
                                    self.notice('>>>>>>>>>>  Key:  {:15}   Value:  {}'.format(k, page_info['Page'][k]))
                                else:
                                    self.notice('>>>>>>>>>>  Key:  {:15}   Value:  0x{:x}'.format(k, page_info['Page'][k]))
                            else:
                                for j in page_info['Page'][k]:          # Descriptor details are in 3rd level dict in page_info object
                                    self.notice('>>>>>>>>>>  DescriptorDetails:  Key:  {:22}   Value:  {}'.format(j, page_info['Page'][k][j]))
                    else:
                        self.error('>>>>>>>>>>  VA Page info:  Nothing returned from getPageInfo "VA"')


                    if 'Table' in page_info.keys():
                        self.notice('>>>>>>>>>>  VA Table info   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<')
                        for k in page_info['Table']:
                            self.notice('>>>>>>>>>>  Key:  {:12}   Value:  {}'.format(k, page_info['Table'][k]))
                    else:
                        self.notice('>>>>>>>>>>  VA Table info:  No Table info returned from getPageInfo "VA"')


                    # Just making sure we can actually generate an instruction with the rand_VA determined above.
                    instr_id = self.genInstruction(self.choice(ldstr_byte_ops), {'LSTarget':rand_VA})



        # Iterate through Size and Align values.  Force requires Align to be a power of 2.
        # This 2nd block tests larger values of size - 32K to 8M.
        for theSize in [2 ** x for x in range(15, 17)]:

            for theAlign in [2 ** x for x in range(15, 18)]:

                if theAlign < theSize: continue

                for _ in range(loopCount):

                    rand_PA = self.genPA(Size=theSize, Align=theAlign, Type=theType, Bank=theBank, CanAlias=theCanAlias)
                    rand_VA = self.genVAforPA(PA=rand_PA, Bank=theBank, FlatMap=0, CanAlias=0, ForceNewAddress=1, Type=theType, Size=theSize)
                    self.notice(">>>>>> Requested Alignment:  {:6d}     Requested Size:  {:6d}     PA target= {:16X}     VA target= {:16X}".format(theAlign, theSize, rand_PA, rand_VA))

                    instr_id = self.genInstruction(self.choice(ldstr_byte_ops), {'LSTarget':rand_VA})
                 #  if theAlign % 2 == 0: instr_id = self.genInstruction(self.choice(ldstr_half_ops), {'LSTarget':rand_PA})
                 #  if theAlign % 4 == 0: instr_id = self.genInstruction(self.choice(ldstr_word_ops), {'LSTarget':rand_PA})
                 #  if theAlign % 8 == 0: instr_id = self.genInstruction(self.choice(ldstr_double_ops), {'LSTarget':rand_PA})





MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

