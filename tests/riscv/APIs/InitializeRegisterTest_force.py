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
import RandomUtils

class MainSequence(Sequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInitializedFpRegIndices = set()

    def generate(self, **kargs):
        """
        Initialize, read back and check the result for each architected register that is not reserved.
        Supported regs:  x1-x31, S0-S31, D0-D31
        Unsupported regs:  fcsr

        Each entry in reg_list is a (reg_id string, mask bit pattern).  The mask indicates which bits of reg are defined.
        after building the list of register id's, the test then intializes and reads each register and checks that the 
        returned data matches.
        """

        rv32 = self.getGlobalState('AppRegisterWidth') == 32
        
        # Build the list of register ids
        reg_list = list()

        # generate GPR id's x1, x2 ... x31.  x0 is not used.  mask=0xFFFFFFFFFFFFFFFF

        gpr_mask = 0xFFFFFFFF if rv32 else 0xFFFFFFFFFFFFFFFF
        
        for i in range(1,32):
            reg_list.append( ('x{}'.format(i), gpr_mask) )            

        # generate FPR id's D0, D1, ... D31.   mask=0xFFFFFFFFFFFFFFFF
        for i in range(0, 32):                          
            reg_list.append( ('D{}'.format(i), 0xFFFFFFFFFFFFFFFF) )

        # generate FPR id's S0, S1, ... S31.   mask=0x00000000FFFFFFFF
        for i in range(0, 32):                          
            reg_list.append( ('S{}'.format(i), 0x00000000FFFFFFFF) )


        # Do the initialize, read and compare test for each reg_id

        for (reg_id, reg_mask) in self.choicePermutated(reg_list):

            # skip initializing & checking registers that are already reserved
            if self.isRegisterReserved(reg_id, access="Write"):  continue

            # skip initializing floating point registers that have already been initialized under a
            # different name
            if self._isFloatingPointRegisterInitialized(reg_id):  continue

            initial_data = RandomUtils.random64()

            if reg_id[0] in ('x') and rv32:
                initial_data = RandomUtils.random32()
                
            self.initializeRegister(reg_id, initial_data)
            self._verifyInitialization(reg_id, reg_mask, initial_data)

    ## Return true if the specified register is a floating point register and has already been
    # partially or fully initialized. For example, this method will return true for D3 if S3 was
    # already initialized.
    #
    #  @param aRegId The name of the register.
    def _isFloatingPointRegisterInitialized(self, aRegId):
        initialized = False

        if aRegId[0] in ('D', 'S'):
            reg_index = int(aRegId[1:])

            if reg_index in self._mInitializedFpRegIndices:
                initialized = True

        return initialized

    ## Assert that the register contains the specified initial value.
    #
    #  @param aRegId The name of the register.
    #  @param aRegMask A mask representing the bits in the initial value expected to be in the
    #       register.
    #  @param aInitialData The initial register value.
    def _verifyInitialization(self, aRegId, aRegMask, aInitialData):
        readValue, validCheck = self.readRegister(aRegId)
        self.notice(">>>>>>>>>>   Register ID:  {}     Initial data:  {:016X}     Returned data:  {:016X}".format(aRegId, aInitialData, readValue))

        if validCheck == 0:  self.error('>>>>>>>>>>>>   Read of Register ID {} was not valid.'.format(aRegId))

        if readValue == (aInitialData & aRegMask):  
            self.notice(">>>>>>>>>>>>   Register {} initialized to {:016X} was read correctly.".format(aRegId, readValue))
        else:
            self.error(">>>>>>>>>>>>   Value read from Register ID {} did not match value written.    Written value: {:016X}     Read value: {:016X}".format(aRegId, aInitialData, readValue))

        if aRegId[0] in ('D', 'S'):
            self._mInitializedFpRegIndices.add(int(aRegId[1:]))


## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

