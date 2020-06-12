//
// Copyright (C) [2020] Futurewei Technologies, Inc.
//
// FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
// FIT FOR A PARTICULAR PURPOSE.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef Fpix_SimThread_H
#define Fpix_SimThread_H

#include <vector>
#include <ConfigFPIX.h>
#include <SimAPI.h>
#include <SimUtils.h>
#include <SimEvent.h>
#include <PluginManager.h>

using namespace std;

namespace Force {

  //!< each simulation 'thread' (cluster/core/thread combo) is represented by a SimThread object...

  class SimThread {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(SimThread);
    COPY_CONSTRUCTOR_ABSENT(SimThread);
    SimThread(int cpu_id, ConfigFPIX *sim_cfg, SimAPI *sim_ptr, uint64_t entry_point); //!< Constructor.
    ~SimThread(); //!< Destructor.

      void Init();

      bool EndTest(uint64_t nextPC); //!< return true when end-test condition reached
      bool MaxCountReached(int step_count) const; //!< return true when maximum step instruction count exceeded

      bool Done() const;            //!< return true if no more events to process
      bool EndTestReached() const;  //!<   "      "  if a thread has hit end-test condition

      bool GetPC(uint64_t &PC, std::vector<RegUpdate> &reg_updates) const {
         return SimUtils::GetRegisterUpdate(PC,reg_updates,"PC", "write");
      };

      //!< process next item on event-queue, 'til queue is empty
      int ProcessNextEvent(bool pAllStop); // all-stop set to true once all cores reach end-test.

      int UpdateEventSchedule();   //!< based on conditions, update event queue

      void InsertInterrupt(int interrupt_type, int delay); //!< insert interrupt of specified type after some specified delay

      //friend SimEvent; //!< SimEvent class gets cpu ID, SimAPI ptr, etc from SimThread

      uint32_t CpuId() const { return mCpuId; };
      ConfigFPIX *SimCfg() const { return mSimCfg; };
      SimAPI *SimPtr() const { return mSimPtr; };
      PluginManager *PluginsMgr() const { return mPluginsMgr; };

    protected:
      uint32_t                 mCpuId;          //!< assigned Cpu ID to this 'sim-thread'
      ConfigFPIX              *mSimCfg;         //<! simulation parameters
      SimAPI                  *mSimPtr;         //!< handle to simulator/simulator-api
      PluginManager           *mPluginsMgr;     //!< used to tie this sim-threads events to plugin instances
      std::vector<SimThreadEvent *> mEvents;         //!< list of simulation events for this sim-thread.
      uint64_t                 mCurrentPC;      //!< current PC value
      uint32_t                 mInstrCount;     //!< running count of stepped instructions
      bool                     mPastBootCode;   //!< true once boot-code has been reached
      bool                     mPastFirstInstr; //!<      "    1st random instr seen
      int                      mReturnCode;     //!< return code from 'main' entry points
      bool                     mHitEndTest;     //!< set to true once end-test condition reached
      bool                     mAllStop;        //!< when true and have hit end-test, can exit step event
  };

}

#endif
