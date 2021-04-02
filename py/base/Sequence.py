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
#  Sequence class
#  Base sequence class, pass on API calls to GenThread class
from base.ItemMap import ItemMap
from base.Macro import Macro
import base.UtilityFunctions as UtilityFunctions
from base.TestUtils import get_stack_frame_string
from Config import Config
from Enums import EGlobalStateType, ELimitType
import Log
import RandomUtils
import warnings
from inspect import signature


class Sequence(object):
    def __init__(self, gen_thread, name=None):
        self.genThread = gen_thread
        self.name = name
        self.entryFunction = None
        self.globalEretPreambleSequence = None

    def setup(self, **kargs):
        pass

    def generate(self, **kargs):
        pass

    def cleanUp(self, **kargs):
        pass

    def run(self, **kargs):
        if self.entryFunction is None:
            self.setup(**kargs)
            self.generate(**kargs)
            self.cleanUp(**kargs)
        else:
            entry_args = self._getEntryFunctionPositionalArgs(**kargs)
            self.entryFunction(*entry_args, **kargs)

    def getResult(self):
        return None

    def setSharedThreadObject(self, name, a_object):
        self.genThread.setSharedThreadObject(name, a_object)

    def getSharedThreadObject(self, name):
        return self.genThread.getSharedThreadObject(name)

    def beginLoop(self, loop_count, kargs):
        return self.genThread.beginLoop(loop_count, kargs)

    def endLoop(self, loop_id):
        self.genThread.endLoop(loop_id)

    def reportLoopReconvergeAddress(self, loop_id, address):
        self.genThread.reportLoopReconvergeAddress(loop_id, address)

    def reportPostLoopAddress(self, loop_id, address):
        self.genThread.reportPostLoopAddress(loop_id, address)

    def beginLinearBlock(self):
        return self.genThread.beginLinearBlock()

    def endLinearBlock(
        self, block_id, execute=True, max_re_execution_instructions=0
    ):
        self.genThread.endLinearBlock(
            block_id, execute, max_re_execution_instructions
        )

    def genMacro(self, item, param):
        return self._genInstructionSequenceFromMacro(Macro(item, param))

    def genInstrOrSequence(self, item, kargs=dict()):
        try:
            if isinstance(item, str):
                return self.genInstruction(item, kargs)
            else:
                return self._genInstructionSequence(item, kargs)
        except TypeError:
            self.error(
                "genInstrOrSequence: given item type or format is incorrect"
            )

        return None

    def genInstruction(self, instr_name, kargs=dict()):
        return self.genThread.genInstruction(instr_name, kargs)

    def genMetaInstruction(self, instr_name, kargs=dict()):
        # enable speculative bnt by default
        sp_key = "SpeculativeBnt"
        if sp_key not in kargs:
            kargs[sp_key] = True
        return self.genThread.genMetaInstruction(instr_name, kargs)

    def queryInstructionRecord(self, rec_id):
        return self.genThread.queryInstructionRecord(rec_id)

    def queryExceptionRecordsCount(self, exception_id):
        return self.genThread.queryExceptionRecordsCount(exception_id)

    def queryExceptionRecords(self, exception_id):
        return self.genThread.queryExceptionRecords(exception_id)

    def queryExceptions(self, **kargs):
        return self.genThread.queryExceptions(kargs)

    def getVmContextDelta(self, **kargs):
        return self.genThread.getVmContextDelta(kargs)

    def genVmContext(self, **kargs):
        return self.genThread.genVmContext(kargs)

    def getVmCurrentContext(self, **kargs):
        return self.genThread.getVmCurrentContext(kargs)

    def updateVm(self, **kargs):
        return self.genThread.updateVm(kargs)

    def queryHandlerSetMemory(self, bank):
        return self.genThread.queryHandlerSetMemory(bank)

    def queryResourceEntropy(self, resource_type):
        return self.genThread.queryResourceEntropy(resource_type)

    def queryExceptionVectorBaseAddress(self, vector_type):
        return self.genThread.queryExceptionVectorBaseAddress(vector_type)

    def setGlobalState(self, state_name, state_value):
        config = Config.getInstance()
        state_type = self._convertStringToEnum(EGlobalStateType, state_name)
        config.setGlobalState(state_type, state_value)

    def getGlobalState(self, state_name):
        config = Config.getInstance()
        state_type = self._convertStringToEnum(EGlobalStateType, state_name)
        return config.getGlobalState(state_type)

    def getLimitValue(self, limit_name):
        config = Config.getInstance()
        limit_type = self._convertStringToEnum(ELimitType, limit_name)
        return config.getLimitValue(limit_type)

    def setPEstate(self, state_name, state_value):
        self.genThread.setPEstate(state_name, state_value)

    def getPEstate(self, state_name):
        return self.genThread.getPEstate(state_name)

    def initializeMemory(self, addr, bank, size, data, is_instr, is_virtual):
        self.genThread.initializeMemory(
            addr, bank, size, data, is_instr, is_virtual
        )

    # Page related API
    def genPA(self, **kargs):
        return self.genThread.genPA(kargs)

    def genVA(self, **kargs):
        return self.genThread.genVA(kargs)

    def genVMVA(self, **kargs):
        return self.genThread.genVMVA(kargs)

    def genVAforPA(self, **kargs):
        return self.genThread.genVAforPA(kargs)

    # Random module API
    # val = self.random32(min_v=0, max_v=MAX_UINT32)
    def random32(self, min_v=0, max_v=0xFFFFFFFF):
        warnings.warn(
            "Sequence.random32() is deprecated; "
            "please use RandomUtils.random32()",
            DeprecationWarning,
        )

        return RandomUtils.random32(min_v, max_v)

    def random64(self, min_v=0, max_v=0xFFFFFFFFFFFFFFFF):
        warnings.warn(
            "Sequence.random64() is deprecated; "
            "please use RandomUtils.random64()",
            DeprecationWarning,
        )

        return RandomUtils.random64(min_v, max_v)

    def shuffleList(self, myList):
        warnings.warn(
            "Sequence.shuffleList() is deprecated; "
            "please use UtilityFunctions.shuffle_list()",
            DeprecationWarning,
        )

        return UtilityFunctions.shuffle_list(myList)

    # replace Python random.sample to assure the random normalization!
    def sample(self, items, sample_size):
        return self.genThread.sample(items, sample_size)

    # Random choose one item from given item list
    # the item list could be dictionary, list, str, or tuple
    def choice(self, items):
        return self.genThread.choice(items)

    # Permute the given item list and return one item at a time
    # the item list could be dictionary, list, str, or tuple
    def choicePermutated(self, items):
        item_list = items
        if isinstance(items, dict):
            item_list = list(sorted(items.items()))

        index_list = list(range(len(item_list)))
        shuffled_index_list = self.shuffleList(index_list)

        for i in shuffled_index_list:
            yield item_list[i]

    # Register module API
    # (reg1, reg2, reg3) = self.getRandomRegisters(Number=3, Type="GPR",
    # Exclude="31"), Exclude is optional, default is empty string ""
    def getRandomRegisters(self, number, reg_type, exclude="", no_skip=False):
        reg_indices = self.genThread.getRandomRegisters(
            number, reg_type, exclude
        )
        if no_skip and (reg_indices is None):
            self.error(
                "Failed to obtain %d random %s register(s)"
                % (number, reg_type)
            )

        return reg_indices

    def getRandomRegistersForAccess(
        self, number, reg_type, access, exclude="", no_skip=False
    ):
        reg_indices = self.genThread.getRandomRegistersForAccess(
            number, reg_type, access, exclude
        )
        if no_skip and (reg_indices is None):
            self.error(
                "Failed to obtain %d random %s register(s)"
                % (number, reg_type)
            )

        return reg_indices

    def getRandomGPRs(self, number, exclude="", no_skip=False):
        if len(exclude):
            exclude += ",31"
        else:
            exclude = "31"

        return self.getRandomRegisters(number, "GPR", exclude, no_skip)

    def getRandomGPRsForAccess(
        self, number, access, exclude="", no_skip=False
    ):
        if len(exclude):
            exclude += ",31"
        else:
            exclude = "31"

        return self.getRandomRegistersForAccess(
            number, "GPR", access, exclude, no_skip
        )

    def getRandomGPR(self, exclude="", no_skip=False):
        return self.getRandomGPRs(1, exclude, no_skip)[0]

    def getRandomGPRForAccess(self, access, exclude="", no_skip=False):
        return self.getRandomGPRsForAccess(1, access, exclude, no_skip)[0]

    def isRegisterReserved(self, name, access="Write", resv_type="User"):
        return self.genThread.isRegisterReserved(name, access, resv_type)

    def reserveRegisterByIndex(self, size, index, reg_type, access="Write"):
        self.genThread.reserveRegisterByIndex(
            size, index, reg_type, access
        )  # input name

    def reserveRegister(self, name, access="Write"):
        self.genThread.reserveRegister(name, access)

    def unreserveRegisterByIndex(self, size, index, my_type, access="Write"):
        self.genThread.unreserveRegisterByIndex(
            size, index, my_type, access
        )  # input index

    def unreserveRegister(self, name, access="Write", reserve_type="User"):
        self.genThread.unreserveRegister(name, access, reserve_type)

    def readRegister(self, name, field=""):
        return self.genThread.readRegister(name, field)

    def writeRegister(self, name, value, field="", update=False):
        self.genThread.writeRegister(name, field, value, update)

    def initializeRegister(self, name, value, field=""):
        self.genThread.initializeRegister(name, field, value)

    def initializeRegisterFields(self, register_name, field_value_map):
        self.genThread.initializeRegisterFields(register_name, field_value_map)

    def randomInitializeRegister(self, name, field=""):
        self.genThread.randomInitializeRegister(name, field)

    def randomInitializeRegisterFields(self, register_name, field_list):
        self.genThread.randomInitializeRegisterFields(
            register_name, field_list
        )

    def getRegisterFieldMask(self, register_name, field_list):
        return self.genThread.getRegisterFieldMask(register_name, field_list)

    # self.genSequence()
    def genSequence(self, my_type, kargs={}):
        self.genThread.genSequence(my_type, kargs)

    # get register information
    def getRegisterInfo(self, name, index):
        return self.genThread.getRegisterInfo(name, index)

    # get page information
    def getPageInfo(self, addr, addr_type, bank):
        return self.genThread.getPageInfo(addr, addr_type, bank)

    # get max physical address
    def getMaxPhysicalAddress(self):
        return self.genThread.getMaxAddress("Physical")

    # get max virtual address
    def getMaxVirtualAddress(self):
        return self.genThread.getMaxAddress("Virtual")

    # get max address
    def getMaxAddress(self, addr_type="Virtual"):
        return self.genThread.getMaxAddress(addr_type)

    # get branch offset
    def getBranchOffset(self, br_addr, target_addr, offset_size, shift):
        return self.genThread.getBranchOffset(
            br_addr, target_addr, offset_size, shift
        )

    def validAddressMask(self, addr, is_instr):
        # Tuple of (is_instr, addr)
        return self.genThread.validAddressMask(
            "ValidAddressMask", addr, is_instr
        )

    def pickWeighted(self, weighted_dict):
        return self.genThread.pickWeighted(weighted_dict)

    def pickWeightedValue(self, weighted_dict):
        return self.genThread.pickWeightedValue(weighted_dict)

    # get permuted item
    def getPermutated(self, weighted_dict, skip_weight_check=False):
        # verify all items are of supported types
        if not all(
            map(
                lambda item: isinstance(item, (str, ItemMap, Macro)),
                weighted_dict,
            )
        ):
            raise TestException("Picked unsupported object in getPermutated.")

        # first shuffle the given dictionary
        sorted_items = sorted(weighted_dict.items())
        if not skip_weight_check:
            sorted_items = filter(
                lambda item, weight: weight > 0, sorted_items
            )

        permuted_list = list(sorted_items)
        index_list = list(range(len(permuted_list)))
        shuffled_index_list = self.shuffleList(index_list)

        # iterate the shuffled list
        for index in shuffled_index_list:
            yield from self._getPermutatedItems(
                permuted_list[index][0], skip_weight_check
            )

    # self.error("failed to setup scenario at address 0x%x" % addr)
    def error(self, err_msg):
        stack_frame_str = get_stack_frame_string()
        Log.fail("%s\n%s" % (err_msg, stack_frame_str))

    def notice(self, msg):
        warnings.warn(
            "Sequence.notice() is deprecated; please use Log.notice()",
            DeprecationWarning,
        )

        Log.notice(msg)

    def warn(self, msg):
        warnings.warn(
            "Sequence.warn() is deprecated; please use Log.warn()",
            DeprecationWarning,
        )

        Log.warn(msg)

    def debug(self, msg):
        warnings.warn(
            "Sequence.debug() is deprecated; please use Log.debug()",
            DeprecationWarning,
        )

        Log.debug(msg)

    def info(self, msg):
        warnings.warn(
            "Sequence.info() is deprecated; please use Log.info()",
            DeprecationWarning,
        )

        Log.info(msg)

    def trace(self, msg):
        warnings.warn(
            "Sequence.trace() is deprecated; please use Log.trace()",
            DeprecationWarning,
        )

        Log.trace(msg)

    # self.getOption("enable_paging")
    def getOption(self, optName):
        return self.genThread.getOption(optName)

    # self.getRegisterIndex(registerName)
    def getRegisterIndex(self, registerName):
        return self.genThread.getRegisterIndex(registerName)

    # self.getRegisterReloadValue(registerName, fieldConstraints)
    def getRegisterReloadValue(self, registerName, fieldConstraints={}):
        return self.genThread.getRegisterReloadValue(
            registerName, fieldConstraints
        )

    # self.genRegisterFieldInfo(registerName, fieldConstraints)
    def genRegisterFieldInfo(self, registerName, fieldConstraints):
        return self.genThread.getRegisterFieldInfo(
            registerName, fieldConstraints
        )

    # self.virtualMemoryRequest(requestName, parameters)
    def virtualMemoryRequest(self, requestName, parameters={}):
        return self.genThread.virtualMemoryRequest(requestName, parameters)

    # self.systemCall(params)
    def systemCall(self, callDetails):
        self.genThread.systemCall(callDetails)

    def selfHostedDebug(self, params):
        return self.genThread.selfHostedDebug(params)

    def exceptionRequest(self, requestName, kargs):
        return self.genThread.exceptionRequest(requestName, kargs)

    def reserveMemory(self, Name, Range, Bank, IsVirtual=False):
        self.genThread.reserveMemory(Name, Range, Bank, IsVirtual)

    def reserveMemoryRange(self, Name, Start, Size, Bank, IsVirtual=False):
        self.genThread.reserveMemoryRange(Name, Start, Size, Bank, IsVirtual)

    def unreserveMemory(self, Name, Range="", Bank=0, IsVirtual=False):
        self.genThread.unreserveMemory(Name, Range, Bank, IsVirtual)

    def unreserveMemoryRange(self, Name, Start, Size, Bank, IsVirtual=False):
        self.genThread.unreserveMemoryRange(Name, Start, Size, Bank, IsVirtual)

    def modifyVariable(self, name, value, var_type):
        self.genThread.modifyVariable(name, value, var_type)

    def getVariable(self, name, var_type):
        return self.genThread.getVariable(name, var_type)

    def dumpPythonObject(self, a_object, a_hex=False):
        if isinstance(a_object, dict):
            self._dumpPythonDictionary(a_object, a_hex)
        elif isinstance(a_object, list):
            self._dumpPythonList(a_object, a_hex)
        else:
            self.error("dumpPythonObject: object not dict or list")

    # set bnt sequence class
    def setBntHook(self, **kargs):
        return self.genThread.setBntHook(**kargs)

    # revert bnt sequence
    def revertBntHook(self, bnt_id=0):
        return self.genThread.revertBntHook(bnt_id)

    def setEretPreambleSequence(self, eretSequence, eretGlobal=True):
        self.modifyVariable(
            "Eret Preamble Sequence Class", eretSequence, "String"
        )
        if eretGlobal:
            self.globalEretPreambleSequence = eretSequence

    def revertEretPreambleSequence(self, eretGlobal=True):
        if eretGlobal:
            self.globalEretPreambleSequence = "default"
        self.modifyVariable(
            "Eret Preamble Sequence Class",
            self.globalEretPreambleSequence,
            "String",
        )

    def verifyVirtualAddress(self, vaddr, size, is_instr):
        return self.genThread.verifyVirtualAddress(vaddr, size, is_instr)

    def confirmSpace(self, size):
        self.genThread.confirmSpace(size)

    # query thread group info by group id. If group id is none, return all
    # thread group info
    def queryThreadGroup(self, aGroupId=None):
        return self.genThread.queryThreadGroup(aGroupId)

    # get main thread group ID the thread belongs to.
    def getThreadGroupId(self, aThreadId=None):
        if aThreadId is None:
            aThreadId = self.genThread.genThreadID

        return self.genThread.getThreadGroupId(aThreadId)

    def getFreeThreads(self):
        return self.genThread.getFreeThreads()

    def partitionThreadGroup(self, aPolicy, **kargs):
        self.genThread.partitionThreadGroup(aPolicy, kargs)

    def setThreadGroup(self, aId, aJob, aThreads):
        self.genThread.setThreadGroup(aId, aJob, aThreads)

    def getThreadNumber(self):
        return (
            self.genThread.numberOfChips()
            * self.genThread.numberOfCores()
            * self.genThread.numberOfThreads()
        )

    def lockThreadScheduler(self):
        self.genThread.lockThreadScheduler()

    def unlockThreadScheduler(self):
        self.genThread.unlockThreadScheduler()

    def hasSharedThreadObject(self, name):
        return self.genThread.hasSharedThreadObject(name)

    def threadLockingContext(self):
        # A nested thread locking context manager class to ensure the locking
        # and unlocking happen as a balanced pair.
        #
        class ThreadLockingContext:
            def __init__(self, aGenThread):
                self.mGenThread = aGenThread

            def __enter__(self):
                self.mGenThread.lockThreadScheduler()
                return self

            def __exit__(self, *aUnused):
                self.mGenThread.unlockThreadScheduler()
                return False

        return ThreadLockingContext(self.genThread)

    def genData(self, kargs):
        return self.genThread.genData(kargs)

    def genFreePagesRange(self, **kargs):
        return self.genThread.genFreePagesRange(kargs)

    def synchronizeWithBarrier(self, **kargs):
        return self.genThread.synchronizeWithBarrier(kargs)

    def _getEntryFunctionPositionalArgs(self, **kargs):
        entry_args = []
        for param in signature(self.entryFunction).parameters.values():
            try:
                if param.kind == param.POSITIONAL_ONLY:
                    entry_args.append(kargs[param.name])
            except KeyError:
                self.notice("missing entry point argument %s" % (param.name))

        return entry_args

    def _genInstructionSequence(self, item, kargs):
        if isinstance(item, Macro):
            return self._genInstructionSequenceFromMacro(item)
        elif isinstance(item, Sequence):
            item.run(**kargs)
            return item
        elif issubclass(item, Sequence):
            myInstance = item(self.genThread)
            myInstance.run(**kargs)
            return myInstance

    def _genInstructionSequenceFromMacro(self, macro):
        objName = macro.obj()
        objVal = macro.value()
        if issubclass(objName, Sequence):
            myInstance = objName(self.genThread)

            args = {}
            if isinstance(objVal, str):
                args = dict(e.split("=") for e in objVal.split(","))
            elif isinstance(objVal, dict):
                args = objVal

            myInstance.run(**args)
            return myInstance

        return None

    def _convertStringToEnum(self, aEnumClass, aString):
        enum_val = None
        try:
            enum_val = aEnumClass.__members__[aString]
        except KeyError:
            self.error(
                "%s is not a valid value of %s"
                % (aString, aEnumClass.__name__)
            )

        return enum_val

    def _getPermutatedItems(self, base_item, skip_weight_check):
        if isinstance(base_item, str) or isinstance(base_item, Macro):
            yield base_item
        elif isinstance(base_item, ItemMap):
            yield from base_item.getPermutated(self, skip_weight_check)

    def _dumpPythonDictionary(self, a_dict, a_hex):
        for (key, value) in sorted(a_dict.items()):
            if isinstance(value, int):
                self._dumpPythonContainerEntry(key, value, a_hex)
            elif isinstance(value, (dict, list)):
                self.dumpPythonObject(value, a_hex)
            else:
                self.notice("%s:%s" % (key, value))

    def _dumpPythonList(self, a_list, a_hex):
        for (i, item) in enumerate(a_list):
            if isinstance(item, int):
                self._dumpPythonContainerEntry(i, item, a_hex)
            elif isinstance(item, (dict, list)):
                self.dumpPythonObject(item, a_hex)
            else:
                self.notice("%s:%s" % (i, item))

    def _dumpPythonContainerEntry(self, a_key, a_value, a_hex):
        if a_hex:
            self.notice("%s:%x" % (a_key, a_value))
        else:
            self.notice("%s:%d" % (a_key, a_value))
