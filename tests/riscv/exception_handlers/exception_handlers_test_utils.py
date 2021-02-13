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


# Randomly initialize and reserve aGprCount randomly-selected GPRs and
# return a dictionary mapping their indices to their values.
#
#  @param aSequence The calling sequence.
#  @param aGprCount The number of GPRs to reserve.
def reserve_random_gprs(aSequence, aGprCount):
    orig_gpr_values = {}
    gpr_indices = aSequence.getRandomGPRs(aGprCount)
    for gpr_index in gpr_indices:
        gpr_name = "x%d" % gpr_index
        aSequence.randomInitializeRegister(gpr_name)

        (gpr_val, valid) = aSequence.readRegister(gpr_name)
        assert_valid_gpr_value(aSequence, gpr_index, valid)
        aSequence.reserveRegister(gpr_name)

        orig_gpr_values[gpr_index] = gpr_val

    return orig_gpr_values


#  Fail if any GPR value differs from its specified original value.
#
#  @param aSequence The calling sequence.
#  @param aOrigGprValues A dictionary mapping GPR indices to their original
#       values.
def assert_gpr_values_unchanged(aSequence, aOrigGprValues):
    for (gpr_index, gpr_val) in aOrigGprValues.items():
        assert_gpr_has_value(aSequence, gpr_index, gpr_val)


#  Fail if the GPR's value differs from its specified original value.
#
#  @param aSequence The calling sequence.
#  @param aGprIndex The index of the GPR.
#  @param aExpectedGprVal The expected GPR value.
def assert_gpr_has_value(aSequence, aGprIndex, aExpectedGprVal):
    (gpr_val, valid) = aSequence.readRegister("x%d" % aGprIndex)
    assert_valid_gpr_value(aSequence, aGprIndex, valid)

    if gpr_val != aExpectedGprVal:
        aSequence.error(
            "Value of register x%d did not match the expected value. "
            "Expected=0x%x, Actual=0x%x"
            % (aGprIndex, aExpectedGprVal, gpr_val)
        )


#  Fail if the valid flag is false.
#
#  @param aSequence The calling sequence.
#  @param aGprIndex The index of the GPR.
#  @param aValid A flag indicating whether the specified GPR has a valid value.
def assert_valid_gpr_value(aSequence, aGprIndex, aValid):
    if not aValid:
        aSequence.error("Value for register x%d is invalid" % aGprIndex)
