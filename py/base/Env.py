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
import itertools

import PyInterface
import Log
import RandomUtils

from base.GenThreadExecutor import GenThreadExecutorFactory
from base.Sequence import Sequence


#  GlobalInitSequence class
#  Base class of arch level global init sequence class.
class GlobalInitSequence(Sequence):
    def __init__(self, gen_thread, name):
        super().__init__(gen_thread, name)

    def generate(self, **kargs):
        self.setupResetRegion()
        self.allocateHandlerSetMemory()
        self.setupMemoryFillPattern()
        self.setupThreadGroup()

    def setupResetRegion(self):
        pass

    def allocateHandlerSetMemory(self):
        pass

    def setupMemoryFillPattern(self):
        pass

    def setupThreadGroup(self):
        pass


#  Env class
#  Top level class in a test template
class Env(object):
    def __init__(self, interface):
        self.interface = interface
        self.numberChips = self.interface.numberOfChips()
        self.numberCores = self.interface.numberOfCores()
        self.numberThreads = self.interface.numberOfThreads()
        self.genThreads = list()
        self.mGenMain = None
        self.beforeSequences = list()
        self.afterSequences = list()
        self.defaultGenClass = None
        self.defaultSeqClass = None
        self.defaultInitSeqClass = None
        self.genThreadInitFunc = None

        self.executor = GenThreadExecutorFactory.createGenThreadExecutor(
            self.numberChips * self.numberCores * self.numberThreads
        )

    # Configure generator memory
    def configureMemory(self, memfile_module):
        import importlib

        mem_module = importlib.import_module(memfile_module)
        mem_module.configure_memory(self.interface)

    def configureChoicesModifier(self, modfile_module):
        import importlib

        choices_module = importlib.import_module(modfile_module)
        choices_module.configure_choices(self.mGenMain)

    # Setup generator threads
    def setup(self):
        combinations = itertools.product(
            range(self.numberChips),
            range(self.numberCores),
            range(self.numberThreads),
        )
        for (i_chip, i_core, i_thread) in combinations:
            gen_thread_id = self.createBackEndGenThread(
                i_thread, i_core, i_chip
            )  # create back end generator thread
            new_gen_thread = self.createGenThread(
                gen_thread_id, i_thread, i_core, i_chip
            )  # create front end generator thread
            self.genThreads.append(new_gen_thread)
            self.setupGenThread(new_gen_thread)

        self.assignMainGen()
        self.addThreadSplitterSequence()

    # Assign a main generator for the before and after main test processing.
    def assignMainGen(self):
        num_gen = len(self.genThreads)
        if num_gen == 0:
            self.interface.error("[assignMainGen] number of threads = 0")

        gen_index = RandomUtils.random32(0, num_gen - 1)
        self.mGenMain = self.genThreads[gen_index]
        self.executor.setMainThreadId(self.mGenMain.genThreadID)
        Log.notice("Main generator is 0x%x" % self.mGenMain.genThreadID)

    # Create back end generator thread
    def createBackEndGenThread(self, i_thread, i_core, i_chip):
        ret_thread_id = self.interface.createGeneratorThread(i_thread, i_core, i_chip)
        return ret_thread_id

    # Create front end generator thread.
    def createGenThread(self, gen_thread_id, i_thread, i_core, i_chip):
        return self.defaultGenClass(gen_thread_id, self.interface)

    # Setting up newly created generator thread.
    def setupGenThread(self, gen_thread):
        main_seq = self.defaultSeqClass(gen_thread, self.defaultSeqClass.__name__)
        gen_thread.addSequence(main_seq)
        gen_thread.setGenThreadInitFunc(self.genThreadInitFunc)

    # Start all the generator threads
    def generate(self):
        for seq in self.beforeSequences:
            seq.genThread = self.mGenMain
            seq.run()

        self.mGenMain.setup()
        for gen_thread in self.genThreads:
            if gen_thread is not self.mGenMain:
                ex_mgr = self.mGenMain.exceptionHandlerManager
                gen_thread.exceptionHandlerManager = ex_mgr.createShallowCopy(gen_thread)
                at_mgr = self.mGenMain.addressTableManager
                gen_thread.addressTableManager = at_mgr.createShallowCopy(gen_thread)

        self.executor.executeGenThreads(self.genThreads)

        for seq in self.afterSequences:
            seq.genThread = self.mGenMain
            seq.run()

        self.mGenMain.genSequence("Summary")

    # set Sequence class like bnt, eret on a thread
    def setSequenceClass(self, thread_id, seq_type, sequence):
        thread_obj = self.getThreadObject(thread_id)
        if thread_obj is not None:
            if seq_type == 0:
                thread_obj.setBntSequence(sequence)
            elif seq_type == 1:
                thread_obj.setEretPreambleSequence(sequence)
            else:
                self.interface.error("invalid sequence type: %d" % seq_type)

    # run Sequence on a thread
    # TBD: to optimize thread list for better performance
    def runSequence(self, thread_id, seq_type, primary, param_dict):
        thread_obj = self.getThreadObject(thread_id)
        if thread_obj is not None:
            if seq_type == 0:
                thread_obj.runBntSequence(primary, param_dict)
            elif seq_type == 1:
                thread_obj.runEretPreambleSequence(param_dict)
            else:
                self.interface.error("invalid sequence type: %d" % seq_type)

    def getThreadObject(self, thread_id):
        for thread in self.genThreads:
            if thread.genThreadID == thread_id:
                return thread

        self.interface.error("invalid thread id: %d" % thread_id)

        return None

    # Add a sequence to be run before generating the main test.
    def addInitialSequence(self, init_class):
        if init_class is not None:
            self.beforeSequences.append(init_class(None, init_class.__name__))
        else:
            self.beforeSequences.append(
                self.defaultInitSeqClass(None, self.defaultInitSeqClass.__name__)
            )

    def addThreadSplitterSequence(self):
        raise NotImplementedError
