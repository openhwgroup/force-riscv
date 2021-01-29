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

## This class is intended to serve as a base class for creating sequences that can be generated once
# and called as routines and/or branched to from multiple locations. The caller must ensure that the
# routine has been generated prior to attempting to call it or branch to it.
class ReusableSequence(Sequence):

    def __init__(self, aGenThread, aFactory, aStack):
        super().__init__(aGenThread)

        self.mFactory = aFactory
        self.mStack = aStack
        self.mAssemblyHelper = self.mFactory.createAssemblyHelper(self)
        self._mRoutineStartAddresses = {}

    ## Generate the specified routine and record its starting address. The routine will only be
    # generated once, even if this method is called multiple times. If the specified routine is
    # dependent on any other routines being generated first, this method will also generate those
    # routines.
    #
    #  @param aRoutineName The name of the routine to generate.
    #  @param kwargs The arguments to pass to the generate<RoutineName>() method.
    def generateRoutine(self, aRoutineName, **kwargs):
        routine_names = [aRoutineName]
        for routine_name in routine_names:
            for prereq_routine_name in self.getPrerequisiteRoutineNames(routine_name):
                if prereq_routine_name not in routine_names:
                    routine_names.append(prereq_routine_name)

        # Generate with the prerequisite routines first
        for routine_name in reversed(routine_names):
            if routine_name not in self._mRoutineStartAddresses:
                self._generateValidatedRoutine(routine_name, **kwargs)

    ## Call a routine that has previously been generated. A branch with link instruction will be
    # generated to jump to the routine.
    #
    #  @param aRoutineName The name of the routine to call.
    #  @param aSaveRegIndices The indices of any registers that need to be preserved. The link
    #       register will always be preserved, so it should not be specified.
    def callRoutine(self, aRoutineName, aSaveRegIndices=None):
        save_reg_indices = []
        if aSaveRegIndices:
            save_reg_indices = aSaveRegIndices

        routine_start_addr = self._mRoutineStartAddresses.get(aRoutineName)
        if routine_start_addr is not None:
            self.mStack.newStackFrame(save_reg_indices)
            self.mAssemblyHelper.genRelativeBranchWithLinkToAddress(routine_start_addr)
            self.mStack.freeStackFrame()
        else:
            raise ValueError('Routine %s has not been generated' % aRoutineName)

    ## Jump to a routine that has previously been generated. A branch instruction will be generated
    # to jump to the routine.
    #
    #  @param aRoutineName The name of the routine to jump to.
    def jumpToRoutine(self, aRoutineName):
        routine_start_addr = self._mRoutineStartAddresses.get(aRoutineName)
        if routine_start_addr:
            self.mAssemblyHelper.genRelativeBranchToAddress(routine_start_addr)
        else:
            raise ValueError('Routine %s has not been generated' % aRoutineName)

    ## Return whether the specified routine has been generated
    #
    #  @param aRoutineName The name of the routine.
    def hasGeneratedRoutine(self, aRoutineName):
        return (aRoutineName in self._mRoutineStartAddresses)

    ## Return whether this object defines the specified routine.
    #
    #  @param aRoutineName The name of the routine.
    def hasRoutine(self, aRoutineName):
        return (hasattr(self, ('generate%s' % aRoutineName)))

    ## Return the names of all routines that the specified routine directly depends on. This method
    # is used to determine whether any other routines need to be generated prior to generating the
    # specified routine.
    def getPrerequisiteRoutineNames(self, aRoutineName):
        return tuple()

    ## Generate the specified routine and record its starting address. This method should only be
    # called after ensuring the routine has not been previously generated and ensuring all
    # prerequisite routines have already been generated.
    #
    #  @param aRoutineName The name of the routine to generate.
    #  @param kwargs The arguments to pass to the generate<RoutineName>() method.
    def _generateValidatedRoutine(self, aRoutineName, **kwargs):
        self._mRoutineStartAddresses[aRoutineName] = self.getPEstate('PC')
        routine_gen_method = self.__getattribute__('generate%s' % aRoutineName)
        routine_gen_method(**kwargs)
