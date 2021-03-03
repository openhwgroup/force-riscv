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
from base.Sequence import Sequence


class InitMemoryBlock(Sequence):
    def __init__(self, gen_thread):
        super().__init__(gen_thread)
        self.startAddress = None
        self.bank = None
        self.size = None
        self.isInstruction = None
        self.isVirtual = None

    # Main entry for the class
    def initializeRandomly(self, addr, bank, size, is_instr, is_virtual):
        self.startAddress = addr
        self.bank = bank
        self.size = size
        self.isInstruction = is_instr
        self.isVirtual = is_virtual
        self.run()

    def generate(self, **kargs):
        if self.size <= 8:
            self.error(
                "Should call this API with size > 8, provided size=%d"
                % self.size
            )

        aligned_addr = self.startAddress
        aligned_size = self.size
        leading_offset = self.startAddress & 0x7
        if leading_offset != 0:
            leading_size = 8 - (leading_offset)
            init_value = self.random64()
            self.initializeMemory(
                self.startAddress,
                self.bank,
                leading_size,
                init_value,
                self.isInstruction,
                self.isVirtual,
            )
            aligned_size -= leading_size
            aligned_addr -= leading_offset

        end_addr = aligned_addr + (aligned_size - 1)

        # print ("end address 0x%x" % end_addr)
        for init_addr in range(aligned_addr, end_addr, 8):
            init_value = self.random64()
            init_size = 8
            if (init_addr + 7) > end_addr:
                init_size = (end_addr - init_addr) + 1
            self.initializeMemory(
                init_addr,
                self.bank,
                init_size,
                init_value,
                self.isInstruction,
                self.isVirtual,
            )


#  A simple register-based loop control class.
class LoopControlBase(Sequence):
    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self.mLoopRegIndex = None
        self.mLoopBackAddress = None
        self.mPostLoopAddress = None
        self.mLoopCount = None
        self.mLoopId = None
        self._mLoopRegReservationAccess = None

    # Generate a loop for the specified number of iterations. Every instruction
    # generated between the call to start() and the call to end() comprises the
    # loop body. start() will fail if called multiple times without matching
    # calls to end().
    #
    #  @param LoopReg The loop register index. A random register will be
    #       selected if none is specified.
    #  @param LoopCount The number of iterations to execute the loop.
    def start(self, LoopReg=None, LoopCount=None):
        if self.mLoopId is not None:
            self.error(
                "Unable to start a loop before ending Loop %d." % self.mLoopId
            )

        self.mLoopRegIndex = (
            LoopReg
            if LoopReg is not None
            else self.getRandomGprForLoopControl()
        )
        self.mLoopCount = LoopCount

        self._reserveLoopRegister()

        self.loadLoopRegister()

        self.beginLoopSupport()

    # Mark the end of the loop body and generate instructions to return
    # execution to the beginning of the loop. end() will fail if called before
    # start().
    def end(self):
        if self.mLoopId is None:
            self.error("Unable to end a loop before starting one.")

        self.generateLoopBack()
        self.endLoop(self.mLoopId)
        self._unreserveLoopRegister()

        self.mLoopId = None

    # Notify the back end that loop generation has been initiated.
    def beginLoopSupport(self):
        (self.mLoopId, self.mLoopBackAddress) = self.beginLoop(
            self.mLoopCount, {"LoopRegIndex": self.mLoopRegIndex}
        )

    # Prepare for and generate instructions to return execution to the
    # beginning of the loop.
    def generateLoopBack(self):
        block_id = self.beginLinearBlock()
        self.reportLoopReconvergeAddress(self.mLoopId, self.getPEstate("PC"))

        self.generateLoopBackInstructions()

        self.reportPostLoopAddress(self.mLoopId, self.mPostLoopAddress)
        self.endLinearBlock(block_id)

    # Generate instructions to load the loop register with loop count value.
    def loadLoopRegister(self):
        raise NotImplementedError

    # Generate instructions to return execution to the beginning of the loop.
    # This method must
    # update mPostLoopAddress.
    def generateLoopBackInstructions(self):
        raise NotImplementedError

    # Get the name of a general purpose register based on its index.
    #
    #  @param aGprIndex The index of the general purpose register.
    def getGprName(self, aGprIndex):
        raise NotImplementedError

    # Return the index of a register to use for register branch instructions.
    def getBranchRegisterIndex(self):
        return self.getRandomGprForLoopControl()

    # Return a random general purpose register index for use in generating loop
    # control instructions.
    def getRandomGprForLoopControl(self):
        raise NotImplementedError

    # Reserve the loop register if it has not already been reserved.
    def _reserveLoopRegister(self):
        loop_reg_name = self.getGprName(self.mLoopRegIndex)
        self._mLoopRegReservationAccess = self._getRegisterReservationAccess(
            loop_reg_name
        )
        if self._mLoopRegReservationAccess is not None:
            self.reserveRegister(
                loop_reg_name, access=self._mLoopRegReservationAccess
            )

    # Unreserve the loop register if the loop control reserved it.
    def _unreserveLoopRegister(self):
        if self._mLoopRegReservationAccess is not None:
            self.unreserveRegister(
                self.getGprName(self.mLoopRegIndex),
                access=self._mLoopRegReservationAccess,
            )
            self._mLoopRegReservationAccess = None

    # Get the register reservation access required to fully reserve the
    # specified register. For example, if the regiser is reserved for "Write",
    # "Read" is returned.
    #
    #  @param aLoopRegName The name of the register.
    def _getRegisterReservationAccess(self, aLoopRegName):
        loop_reg_reservation_access = None

        read_reserved = self.isRegisterReserved(aLoopRegName, access="Read")
        write_reserved = self.isRegisterReserved(aLoopRegName, access="Write")
        if read_reserved and write_reserved:
            loop_reg_reservation_access = None
        elif read_reserved:
            loop_reg_reservation_access = "Write"
        elif write_reserved:
            loop_reg_reservation_access = "Read"
        else:
            loop_reg_reservation_access = "ReadWrite"

        return loop_reg_reservation_access
