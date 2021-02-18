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
import warnings

import Log
import RandomUtils

from base.Sequence import Sequence
from base.TestException import *
from base.ThreadRequest import ThreadRequestContextManager


#  GenThreadSetupSequence class
#  Generate common sequence at the beginning of each test thread.
class GenThreadSetupSequence(Sequence):
    def __init__(self, gen_thread, name):
        super().__init__(gen_thread, name)

    def generate(self, **kargs):
        self.setPEstate("BootPC", self.getBootPC())
        self.setupBootRegion()
        self.setupEssentials()
        self.setDefaultStateTransitionHandlers()
        init_pc = self.getInitialPC()
        self.setPEstate("InitialPC", init_pc)
        self.setPEstate("PC", init_pc)

    # Setup memory region for boot instructions
    def setupBootRegion(self):
        raise NotImplementedError

    # Setup essentials like key register fields, etc
    def setupEssentials(self):
        raise NotImplementedError

    # Return initial main test PC for the generator thread
    def getInitialPC(self):
        raise NotImplementedError

    # Return boot code address for the generator thread
    def getBootPC(self):
        raise NotImplementedError

    # Set default StateTransitionHandlers to process StateElements of each type
    def setDefaultStateTransitionHandlers(self):
        raise NotImplementedError


#  GenThreadEndSequence class
#  Generate common sequences at the end of every test thread.
class GenThreadEndSequence(Sequence):
    def __init__(self, gen_thread, name):
        super().__init__(gen_thread, name)

    def generate(self, **kargs):
        # generate end of test sequence
        self.genSequence("EndOfTest")
        # generate BNT code
        self.genSequence("BranchNotTaken")
        # loading registers after booting
        self.genSequence("BootLoading")
        # generate instruction summary
        self.genSequence("ThreadSummary")


class GenThread(object):
    """Base gen-thread class, holds thread ID, reference to scheduler.
    Passes on API calls to back-end scheduler"""

    sharedThreadObjects = {}

    def __init__(self, gen_id, interface):
        self.genThreadID = gen_id
        self.interface = interface
        self.sequences = list()
        self.setupModifiers = list()
        self.setupSequences = list()
        self.cleanUpSequences = list()
        self.beforeSequences = list()
        self.afterSequences = list()
        self.systemCallSequence = None
        self.selfHostedDebugSequence = None
        self.bntSequenceObject = None
        self.eretPreambleSequenceClass = None
        self.genThreadInitFunc = None
        self.setupComplete = False
        self.exceptionHandlerManager = None
        self.fastMode = None

    def addSequence(self, seq):
        self.sequences.append(seq)

    # Called to assign sequences to be run at setup stage
    def assignSetupSequences(self):
        self.addSetupSequence(GenThreadSetupSequence)

    # Called to assign sequences to be run at cleanup stage
    def assignCleanUpSequences(self):
        self.addCleanUpSequence(GenThreadEndSequence)

    def setup(self, **kargs):
        """The GenThread dispatching logic is going to invoke setup() on all
        GenThreads. However, we need to run setup() on a single GenThread
        prior to that point in order to generate the exception handlers.
        Rather than pollute the GenThread dispatching logic with this special
        case, we use a flag to indicate it has completed."""
        if self.setupComplete:
            return

        self.initializeThread()

        for mod in self.setupModifiers:
            mod.apply()

        # Write the genThreadInitFunc here for the following considerations
        # a) the function should be able to change the modifiers that
        #       has applied by the setupModifiers (after setupModifiers)
        # b) the function should be able to configure the Exception
        #       Handlers infos before generate the ExceptionHandlers
        #       (before setupSequences)
        if self.genThreadInitFunc is not None:
            self.genThreadInitFunc(self)

        self.assignSetupSequences()
        for seq in self.setupSequences:
            seq.run()

        self.setupComplete = True

    def generate(self, **kargs):
        for seq in self.beforeSequences:
            seq.run()

        for seq in self.sequences:
            seq.run()

        for seq in self.afterSequences:
            seq.run()

    def cleanUp(self, **kargs):
        self.assignCleanUpSequences()

        for seq in self.cleanUpSequences:
            seq.run()

    def setSharedThreadObject(self, name, a_object):
        with ThreadRequestContextManager():
            GenThread.sharedThreadObjects[name] = a_object

    def getSharedThreadObject(self, name):
        with ThreadRequestContextManager():
            if name in GenThread.sharedThreadObjects:
                return GenThread.sharedThreadObjects[name]
            else:
                Log.fail(
                    "[getSharedThreadObject] shared object not found name=%s"
                    % (name)
                )

    def hasSharedThreadObject(self, name):
        with ThreadRequestContextManager():
            if name in GenThread.sharedThreadObjects:
                return True
            else:
                return False

    def genInstruction(self, instr_name, kargs):
        return self.interface.genInstruction(
            self.genThreadID, instr_name, kargs
        )

    def genMetaInstruction(self, instr_name, kargs):
        return self.interface.genMetaInstruction(
            self.genThreadID, instr_name, kargs
        )

    def queryInstructionRecord(self, rec_id):
        return self.interface.query(
            self.genThreadID, "InstructionRecord", rec_id, {}
        )

    def queryExceptionRecords(self, exception_id):
        return self.interface.query(
            self.genThreadID, "SimpleExceptionsHistory", str(exception_id), {}
        )

    def queryExceptionRecordsCount(self, exception_id):
        exception_records = self.interface.query(
            self.genThreadID, "SimpleExceptionsHistory", str(exception_id), {}
        )
        return len(exception_records)

    def queryExceptions(self, kargs):
        for (key, val) in kargs.items():
            kargs[key] = str(val)

        return self.interface.query(
            self.genThreadID, "AdvancedExceptionsHistory", "", kargs
        )

    # Install a custom synchronous handler
    def registerSynchronousExceptionHandler(self, assignment_request):
        self.exceptionHandlerManager.registerSynchronousExceptionHandler(
            assignment_request
        )

    def registerAsynchronousExceptionHandler(
        self, handler_module_name, handler_class_name
    ):
        """Install a custom asynchronous handler. Unlike registering a s
        ynchronous exception handler, this method will assign the specified
        handler to handle all asynchronous exceptions in all execution states.
        """
        self.exceptionHandlerManager.registerAsynchronousExceptionHandler(
            handler_module_name, handler_class_name
        )

    def registerSynchronousExceptionDispatcher(self, dispatcher):
        """Install a custom synchronous dispatcher. The specified dispatcher
        should handle all possible synchronous exceptions."""
        self.exceptionHandlerManager.registerSynchronousExceptionDispatcher(
            dispatcher
        )

    # Specify default exception handler set to use
    def specifyExceptionHandlerSet(self, handler_set_name):
        self.exceptionHandlerManager.setDefaultExceptionHandlerSet(
            handler_set_name
        )
        self.fastMode = handler_set_name == "Fast"

    def getVmContextDelta(self, kargs):
        return self.interface.query(
            self.genThreadID, "GetVmContextDelta", "", kargs
        )

    def genVmContext(self, kargs):
        return self.interface.virtualMemoryRequest(
            self.genThreadID, "GenVmContext", kargs
        )

    def getVmCurrentContext(self, kargs):
        return self.interface.query(
            self.genThreadID, "GetVmCurrentContext", "", kargs
        )

    def updateVm(self, kargs):
        return self.interface.virtualMemoryRequest(
            self.genThreadID, "UpdateVm", kargs
        )

    def queryHandlerSetMemory(self, bank):
        return self.interface.query(
            self.genThreadID, "HandlerSetMemory", str(bank), {}
        )

    def queryExceptionVectorBaseAddress(self, vector_type):
        return self.interface.query(
            self.genThreadID, "ExceptionVectorBaseAddress", vector_type, {}
        )

    def queryResourceEntropy(self, resource_type):
        return self.interface.query(
            self.genThreadID, "ResourceEntropy", str(resource_type), {}
        )

    def setPEstate(self, state_name, state_value):
        self.interface.stateRequest(
            self.genThreadID, "Set", state_name, state_value, {}
        )

    def getPEstate(self, state_name):
        return self.interface.query(
            self.genThreadID, "GenState", state_name, {}
        )

    def initializeMemory(self, addr, bank, size, data, is_instr, is_virtual):
        self.interface.initializeMemory(
            self.genThreadID, addr, bank, size, data, is_instr, is_virtual
        )

    def addSetupModifier(self, mod_class):
        """Add a ChoicesModifier to be applied before the sequences in the
        setup stage of the generator thread"""
        self.setupModifiers.append(mod_class(self, mod_class.__name__))

    # Add a sequence to be run in the setup stage of the generator thread
    def addSetupSequence(self, setup_class):
        self.setupSequences.append(setup_class(self, setup_class.__name__))

    # Add a sequence to be run in the cleanup stage of the generator thread
    def addCleanUpSequence(self, cleanup_class):
        self.cleanUpSequences.append(
            cleanup_class(self, cleanup_class.__name__)
        )

    # Add a sequence to be run before generating the main sequences.
    def addBeforeSequence(self, before_class):
        self.beforeSequences.append(before_class(self, before_class.__name__))

    # Add a sequence to be run after generating the main sequences.
    def addAfterSequence(self, after_class):
        self.afterSequences.append(after_class(self, after_class.__name__))

    def addChoicesModification(self, choices_type, tree_name, mod_dict):
        return self.interface.addChoicesModification(
            self.genThreadID, choices_type, tree_name, mod_dict, False
        )

    def commitModificationSet(self, choices_type, set_id):
        self.interface.commitModificationSet(
            self.genThreadID, choices_type, set_id
        )

    def revertModificationSet(self, choices_type, set_id):
        self.interface.revertModificationSet(
            self.genThreadID, choices_type, set_id
        )

    def beginLoop(self, loop_count, kargs):
        return self.interface.stateRequest(
            self.genThreadID, "Push", "Loop", loop_count, kargs
        )

    def endLoop(self, loop_id):
        self.interface.stateRequest(
            self.genThreadID, "Pop", "Loop", 0, {"LoopId": loop_id}
        )

    def reportLoopReconvergeAddress(self, loop_id, address):
        self.interface.stateRequest(
            self.genThreadID,
            "Set",
            "LoopReconvergeAddress",
            address,
            {"LoopId": loop_id},
        )

    def reportPostLoopAddress(self, loop_id, address):
        self.interface.stateRequest(
            self.genThreadID,
            "Set",
            "PostLoopAddress",
            address,
            {"LoopId": loop_id},
        )

    def beginStateRestoreLoop(
        self, loopRegIndex, simCount, restoreCount, restoreExclusions
    ):
        self.interface.beginStateRestoreLoop(
            self.genThreadID,
            loopRegIndex,
            simCount,
            restoreCount,
            restoreExclusions,
        )
        return self.interface.query(
            self.genThreadID, "RestoreLoopContext", "", {}
        )

    def endStateRestoreLoop(self, loopId):
        self.interface.endStateRestoreLoop(self.genThreadID, loopId)

    def generateLoopRestoreInstructions(self, loopId):
        self.interface.generateLoopRestoreInstructions(
            self.genThreadID, loopId
        )

    def beginLinearBlock(self):
        return self.interface.stateRequest(
            self.genThreadID, "Push", "LinearBlock", 0, {}
        )

    def endLinearBlock(self, block_id, execute, max_re_execution_instructions):
        (
            block_start_addr,
            block_end_addr,
            empty,
        ) = self.interface.stateRequest(
            self.genThreadID,
            "Pop",
            "LinearBlock",
            0,
            {"BlockId": block_id, "Execute": execute},
        )
        if execute and not empty:
            pc_val = block_start_addr

            dict_arg = {
                "Address": pc_val,
                "MaxReExecutionInstructions": max_re_execution_instructions,
            }
            while pc_val != block_end_addr:
                self.interface.genSequence(
                    self.genThreadID, "ReExecution", dict_arg
                )
                pc_val = self.getPEstate("PC")

    # Page related API
    def genPA(self, kwargs):
        return self.interface.genPA(self.genThreadID, kwargs)

    def genVA(self, kwargs):
        return self.interface.genVA(self.genThreadID, kwargs)

    def genVMVA(self, kwargs):
        return self.interface.genVMVA(self.genThreadID, kwargs)

    def genVAforPA(self, kwargs):
        return self.interface.genVAforPA(self.genThreadID, kwargs)

    def genFreePagesRange(self, kwargs):
        return self.interface.genFreePagesRange(self.genThreadID, kwargs)

    # Random module API
    def sample(self, items, sample_size):
        if isinstance(items, dict):
            raise TypeError
        if sample_size > len(items):
            raise ValueError

        sample_list = []
        for index in self.interface.sample(len(items), sample_size):
            item = items[index]
            sample_list.append(item)
        return sample_list

    # Random choose one item from given item list
    # the item list could be dictionary, list, str, or tuple
    # (Note: assume the length of the item list is less than 32-bit integer)
    def choice(self, items):
        # len can operate on dictionary, list, str and tuple
        n = len(items)
        if n == 0:
            return None
        index = RandomUtils.random32(0, n - 1)
        if isinstance(items, dict):
            key = list(sorted(items.keys()))[index]
            return key, items[key]
        return items[index]

    # Register module API
    def getRandomRegisters(self, number, reg_type, exclude):
        return self.interface.getRandomRegisters(
            self.genThreadID, number, reg_type, exclude
        )

    def getRandomRegistersForAccess(self, number, access, reg_type, exclude):
        return self.interface.getRandomRegistersForAccess(
            self.genThreadID, number, access, reg_type, exclude
        )

    def isRegisterReserved(self, name, access, resv_type):
        return self.interface.isRegisterReserved(
            self.genThreadID, name, access, resv_type
        )

    def reserveRegisterByIndex(self, size, index, reg_type, access):
        self.interface.reserveRegisterByIndex(
            self.genThreadID, size, index, reg_type, access
        )

    def reserveRegister(self, name, access):
        self.interface.reserveRegister(self.genThreadID, name, access)

    def unreserveRegisterByIndex(self, size, index, reg_type, access):
        self.interface.unreserveRegisterByIndex(
            self.genThreadID, size, index, reg_type, access
        )

    def unreserveRegister(self, name, access, reserve_type):
        self.interface.unreserveRegister(
            self.genThreadID, name, access, reserve_type
        )

    def readRegister(self, name, field):
        return self.interface.readRegister(self.genThreadID, name, field)

    def writeRegister(self, name, field, value, update):
        self.interface.writeRegister(
            self.genThreadID, name, field, value, update
        )

    def initializeRegister(self, name, field, value):
        self.interface.initializeRegister(self.genThreadID, name, field, value)

    def initializeRegisterFields(self, registerName, field_value_map):
        self.interface.initializeRegisterFields(
            self.genThreadID, registerName, field_value_map
        )

    def randomInitializeRegister(self, name, field):
        self.interface.randomInitializeRegister(self.genThreadID, name, field)

    def randomInitializeRegisterFields(self, registerName, fieldList):
        self.interface.randomInitializeRegisterFields(
            self.genThreadID, registerName, fieldList
        )

    def getRegisterFieldMask(self, registerName, fieldList):
        return self.interface.getRegisterFieldMask(
            self.genThreadID, registerName, fieldList
        )

    def genSequence(self, my_type, kargs={}):
        self.interface.genSequence(self.genThreadID, my_type, kargs)

    def getRegisterInfo(self, name, index):
        myDict = {"Index": index}
        return self.interface.query(
            self.genThreadID, "RegisterInfo", name, myDict
        )

    def getPageInfo(self, addr, addr_type, bank):
        myDict = {"Addr": addr, "Type": addr_type, "Bank": bank}
        return self.interface.query(self.genThreadID, "PageInfo", "", myDict)

    def getBranchOffset(self, br_addr, target_addr, offset_size, shift):
        myDict = {
            "BranchAddress": br_addr,
            "TargetAddress": target_addr,
            "OffsetSize": offset_size,
            "Shift": shift,
        }
        return self.interface.query(
            self.genThreadID, "BranchOffset", "", myDict
        )

    def validAddressMask(self, util_name, intarg1=0, intarg2=0):
        myDict = {"arg1": intarg1, "arg2": intarg2}
        return self.interface.query(
            self.genThreadID, "ValidAddressMask", util_name, myDict
        )

    def getMaxAddress(self, util_name):
        myDict = {}
        return self.interface.query(
            self.genThreadID, "MaxAddress", util_name, myDict
        )

    def notice(self, msg):
        warnings.warn(
            "GenThread.notice() is deprecated; please use Log.notice()",
            DeprecationWarning,
        )

        Log.notice(msg)

    def getOption(self, optName):
        return self.interface.getOption(optName)

    def getRegisterIndex(self, registerName):
        return self.interface.query(
            self.genThreadID, "RegisterIndex", registerName, {}
        )

    def getRegisterReloadValue(self, registerName, fieldConstraints={}):
        return self.interface.query(
            self.genThreadID,
            "RegisterReloadValue",
            registerName,
            fieldConstraints,
        )

    def getRegisterFieldInfo(self, registerName, fieldConstraints):
        return self.interface.query(
            self.genThreadID,
            "RegisterFieldInfo",
            registerName,
            fieldConstraints,
        )

    def virtualMemoryRequest(self, requestName, parameters={}):
        return self.interface.virtualMemoryRequest(
            self.genThreadID, requestName, parameters
        )

    def systemCall(self, callDetails):
        """Issue system call (callDetails must be a string with ','
        splitter or dictionary)"""
        systemcall = self.systemCallSequence(self)
        args = {}
        if isinstance(callDetails, str):
            args = dict(e.split("=") for e in callDetails.split(","))
        elif isinstance(callDetails, dict):
            args = callDetails
        systemcall.run(**args)

    def selfHostedDebug(self, params):
        """Issue self-hosted debug event (params must be a string with ','
        splitter or dictionary)"""
        if self.selfHostedDebugSequence is not None:
            sh_debug = self.selfHostedDebugSequence(self)
            args = {}
            if isinstance(params, str):
                args = dict(e.split("=") for e in params.split(","))
            elif isinstance(params, dict):
                args = params
            sh_debug.run(**args)
            return sh_debug
        return None

    def exceptionRequest(self, requestName, kargs):
        return self.interface.exceptionRequest(
            self.genThreadID, requestName, kargs
        )

    def modifyGenMode(self, mode):
        self.interface.stateRequest(
            self.genThreadID, "Push", "GenMode", mode, {}
        )

    def revertGenMode(self, mode):
        self.interface.stateRequest(
            self.genThreadID, "Pop", "GenMode", mode, {}
        )

    def reserveMemory(self, Name, Range, Bank, IsVirtual=False):
        self.interface.reserveMemory(
            self.genThreadID, Name, Range, Bank, IsVirtual
        )

    def reserveMemoryRange(self, Name, Start, Size, Bank, IsVirtual=False):
        range_str = "0x%x-0x%x" % (Start, Start + (Size - 1))
        self.interface.reserveMemory(
            self.genThreadID, Name, range_str, Bank, IsVirtual
        )

    def unreserveMemory(self, Name, Range="", Bank=0, IsVirtual=False):
        self.interface.unreserveMemory(
            self.genThreadID, Name, Range, Bank, IsVirtual
        )

    def unreserveMemoryRange(self, Name, Start, Size, Bank, IsVirtual=False):
        range_str = "0x%x-0x%x" % (Start, Start + (Size - 1))
        self.interface.unreserveMemory(
            self.genThreadID, Name, range_str, Bank, IsVirtual
        )

    def modifyVariable(self, name, value, var_type):
        self.interface.modifyVariable(self.genThreadID, name, value, var_type)

    def getVariable(self, name, var_type):
        return self.interface.getVariable(self.genThreadID, name, var_type)

    def pickWeighted(self, weighted_dict):
        from base.ItemMap import ItemMap
        from base.Macro import Macro

        total_weight = 0
        for item, weight in weighted_dict.items():
            total_weight += weight

        if total_weight <= 0:
            raise TestException(
                "Sum of all weights in weighted-dict incorrect %d"
                % total_weight
            )

        picked_value = RandomUtils.random64(0, total_weight - 1)

        for item, weight in sorted(weighted_dict.items()):
            if picked_value < weight:
                # found the picked item (instruction)
                if isinstance(item, str) or isinstance(item, Macro):
                    return item
                elif isinstance(item, ItemMap):
                    return item.pick(self)
                else:
                    raise TestException("Picked unsupported object.")

            picked_value -= weight
        else:
            Log.fail("Error picking item from weighted dict")

    def pickWeightedValue(self, weighted_dict):
        picked_val = self.pickWeighted(weighted_dict)
        return self.interface.query(
            self.genThreadID,
            "PickedValue",
            "PickedValue",
            {"arg1": picked_val},
        )

    def getChoicesTreeInfo(self, treeName, treeType):
        myDict = {"Type": treeType}
        return self.interface.query(
            self.genThreadID, "ChoicesTreeInfo", treeName, myDict
        )

    # register modification set to control target privilege level
    def registerModificationSet(self, choice_type, set_id):
        self.interface.registerModificationSet(
            self.genThreadID, choice_type, set_id
        )

    def runBntSequence(self, function, param_dict):
        try:
            self.bntSequenceObject.setBntCallback(
                getattr(self.bntSequenceObject, function)
            )
        except AttributeError:
            Log.fail("unknown bnt function:%s" % function)

        if "BntInstructionNumber" in param_dict.keys():
            instr_num = param_dict["BntInstructionNumber"]
            if instr_num < self.bntSequenceObject.getInstrNum():
                self.notice(
                    "Not enough instruction space to generate Bnt sequence, "
                    "only generate %d instructions." % instr_num
                )
                self.bntSequenceObject.setInstrNum(instr_num)

        self.bntSequenceObject.run()

    def setBntSequence(self, bntSequence):
        bntSequenceClass = bntSequence
        self.bntSequenceObject = bntSequenceClass(self)

    def verifyVirtualAddress(self, vaddr, size, is_instr):
        return self.interface.verifyVirtualAddress(
            self.genThreadID, vaddr, size, is_instr
        )

    def confirmSpace(self, size):
        myDict = {"arg1_size": size}
        self.interface.genSequence(self.genThreadID, "ConfirmSpace", myDict)

    def initializeThread(self):
        pass

    def setGenThreadInitFunc(self, func):
        self.genThreadInitFunc = func

    def applyChoiceModifier(self, mod_class):
        mod_class(self, mod_class.__name__).apply()

    def setBntHook(self, **kargs):
        return self.interface.stateRequest(
            self.genThreadID, "Push", "BntHook", 0, kargs
        )

    def revertBntHook(self, bnt_id):
        return self.interface.stateRequest(
            self.genThreadID, "Pop", "BntHook", 0, {"bntId": bnt_id}
        )

    def softwareStepPrivLevs(self):
        """return a list of seletced PrivLevs based on current_el, current_ns,
        and TGEvalue"""
        return self.interface.query(
            self.genThreadID,
            "SoftwareStepPrivLevs",
            "SoftwareStepPrivLevs",
            {},
        )

    def softwareStepReady(self, step_privlev):
        """input a selected PrivLev, if valid return and not change it; if
        non-valid return PrivLev=0"""
        return self.interface.query(
            self.genThreadID,
            "SoftwareStepReady",
            "SoftwareStepReady",
            {"arg1": step_privlev},
        )

    def queryThreadGroup(self, aGroupId):
        return self.interface.queryThreadGroup(aGroupId)

    def getThreadGroupId(self, aThreadId):
        return self.interface.getThreadGroupId(aThreadId)

    def getFreeThreads(self):
        return self.interface.getFreeThreads()

    def partitionThreadGroup(self, aPolicy, kargs):
        self.interface.partitionThreadGroup(aPolicy, kargs)

    def setThreadGroup(self, aId, aJob, aThreads):
        self.interface.setThreadGroup(aId, aJob, aThreads)

    def numberOfChips(self):
        return self.interface.numberOfChips()

    def numberOfCores(self):
        return self.interface.numberOfCores()

    def numberOfThreads(self):
        return self.interface.numberOfThreads()

    def lockThreadScheduler(self):
        self.interface.lockThreadScheduler(self.genThreadID)

    def unlockThreadScheduler(self):
        self.interface.unlockThreadScheduler(self.genThreadID)

    # Generate data using user specifed data pattern.
    def genData(self, aPattern):
        return self.interface.query(
            self.genThreadID, "GenData", "GenData", {"arg1": aPattern}
        )

    def genSemaphore(self, aName, aCounter, aBank, aSize):
        return self.interface.genSemaphore(
            self.genThreadID, aName, aCounter, aBank, aSize
        )

    def synchronizeWithBarrier(self, kwargs):
        return self.interface.synchronizeWithBarrier(self.genThreadID, kwargs)
