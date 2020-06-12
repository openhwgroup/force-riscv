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
#include <iostream>
#include <SimEvent.h>
#include <SimThread.h>
#include <GenException.h>
#include <Log.h>

#include <sstream>

using namespace std;

namespace Force {

//!< initialize 'core' SimThreadEvent variables from SimThread...

void SimThreadEvent::Init(ESimThreadEventType eventType, SimThread &pST) { 
  Init(eventType, pST.CpuId(), pST.SimPtr(), pST.PluginsMgr());
}


  SimThread::SimThread(int cpu_id, ConfigFPIX *sim_cfg, SimAPI *sim_ptr, uint64_t entry_point) 
    : mCpuId(cpu_id), mSimCfg(sim_cfg), mSimPtr(sim_ptr), mPluginsMgr(NULL), mEvents(), mCurrentPC(entry_point), mInstrCount(0),
      mPastBootCode(false),mPastFirstInstr(false), mReturnCode(0), mHitEndTest(false), mAllStop(false)
  {
    Init();
  }

  SimThread::~SimThread()
  {
    delete mPluginsMgr;

    for (auto event_ptr : mEvents) {
      delete event_ptr;
    }
  }
  
//!< sim-thread initialization...

void SimThread::Init() {
  // setup event-types list to inform plugin mgr which plugin events to signal on...

  vector<ESimThreadEventType> event_types {
                              ESimThreadEventType::START_TEST, ESimThreadEventType::END_TEST,
                              ESimThreadEventType::RESET, ESimThreadEventType::BOOT_CODE,
                              ESimThreadEventType::FIRST_INSTRUCTION, ESimThreadEventType::PRE_STEP,
                              ESimThreadEventType::STEP, ESimThreadEventType::POST_STEP,
                              ESimThreadEventType::MEMORY_UPDATE, ESimThreadEventType::MMU_EVENT,
                              ESimThreadEventType::REGISTER_UPDATE, ESimThreadEventType::DELAY,
                              ESimThreadEventType::INTERRUPT, ESimThreadEventType::EXCEPTION_EVENT
                              };

  mPluginsMgr = new PluginManager(event_types);

  // at start, these are the only known events. Other events may be inserted during simulation...
  mEvents.push_back( new SimThreadStartTestEvent( *this ) );
  mEvents.push_back( new SimThreadStepEvent(*this, mSimCfg->MaxInsts(), mSimCfg->TreatLowPowerAsNOP() ) );
  mEvents.push_back( new SimThreadEndTestEvent( *this ) );

  // set simulator PC to starting pc, other init related stuff...

  mSimPtr->WriteRegister(mCpuId, "PC", mCurrentPC, -1ull); //!< write PC value for this cpuID to simulator

  if (SimCfg()->EnableDecoding())
    Force::SimUtils::DumpInstructionTrace( CpuId(), mCurrentPC, 0); // output similar to what iss produces by default
}

//!< process next item on event-queue, 'til queue is empty...

int SimThread::ProcessNextEvent(bool pAllStop) {
  LOG(debug) << "SimThread: Processing next event, all-stop? " << pAllStop << "..." << endl;

  mAllStop = pAllStop; // see UpdateEventSchedule

  try {

    if ( Done() ) {
      // this thread is done...
    } 
    else if ( mAllStop && ((mEvents.front())->Type() == ESimThreadEventType::STEP) ) {
      // if all-stop, then don't process Step event. Will ASSUME update-event-schedule will cancel step...
      UpdateEventSchedule(); // should catch the all-stop signal and cancel step
    } 
    else {
      mReturnCode = (mEvents.front())->Process();   // the outcome of processing an event
      UpdateEventSchedule();                        //   may cause changes to the overall event schedule...
    }

  }
  catch (const SimulationError& rSimError) {
    SimPtr()->RecordTermination(mCpuId, 1, rSimError.what());
    mReturnCode = 1;
  }
      
  return mReturnCode;
}

//!< return true if no more events to process

bool SimThread::Done() const { 
  LOG(debug) << "SimThread: Done? " << mEvents.empty() << " (# of events yet to be processed: " << mEvents.size() << ")" << endl;
  return mEvents.empty(); 
}; 

bool SimThread::EndTestReached() const { 
  LOG(debug) << "SimThread: End test end? " << mHitEndTest << endl;
  return mHitEndTest; 
}; 

//!< based on conditions, update event queue...

int SimThread::UpdateEventSchedule() {
  //!< is it 'time' to schedule some special event (insert-interrupt? delay? tbd?...

  if ( (mEvents.front())->Done() ) {
    // the current event, ie, the event just processed, is done. remove it from the events list...
    mReturnCode = mEvents.front()->ReturnCode();
    delete mEvents.front();
    mEvents.erase(mEvents.begin());
  }
  else if ( (mEvents.front())->Type() == ESimThreadEventType::STEP ) {
    uint64 PC = 0;
    uint64 mask = 0;
    SimPtr()->ReadRegister(mCpuId,"PC", &PC, &mask);

    if (EndTest(PC))
      mHitEndTest = true;
    if ( mHitEndTest && mAllStop ) {
      // we've reached end-test, and have the 'all-stop' signal. Remove the step event...
      delete mEvents.front();
      mEvents.erase(mEvents.begin());
    }

    if (!mHitEndTest && SimCfg()->EnableDecoding())
      SimUtils::DumpInstructionTrace( CpuId(), PC, ++mInstrCount ); // output similar to what iss produces by default
  }

  return mReturnCode;
}

//!< insert interrupt of specified type after some specified delay (specified in terms of # of instrs)

//!< NOTE: can't actually schedule this event 'til step is at head of events queue!!!

void SimThread::InsertInterrupt(int /* interrupt_type */, int /* delay */) {
  /* TBD...
  // setup a 'pre-interrupt' step event, with count equal to the interrupt 'delay'...
  SimThreadStepEvent pre_interrupt_step( mEvents.front() );
  pre_interrupt_step.SetRequestedCount(delay);
  // advance the count for the existing step event by the # of instrs to delay...
  mEvents.front().AdvanceCount(delay);
  // insert (in reverse order) the interrupt event and the pre-interrupt steo event...
  mEvents.insert( mEvents.begin(), SimThreadInsertInterruptEvent(interrupt_type) );
  mEvents.insert( mEvents.begin(), pre_interrupt_step() );
  */
}

//!< at simulation start, at 1st instruction to simulate...

int SimThreadStartTestEvent::Process() {
  LOG(debug) << "SimThread: Processing Start-test event..." << endl;
  mPluginsMgr->Signal( *this ); //!< signal 'at start' to plugins...
  return ReturnCode();
}

//!< at PC where end-test condition detected...

int SimThreadEndTestEvent::Process() {
  LOG(debug) << "SimThread: Processing End-test event..." << endl;
  mPluginsMgr->Signal( *this ); //!< signal 'at end' to plugins...
  return ReturnCode();
}

//!< at first random instruction...

int SimThreadFirstInstrEvent::Process() {
  mPluginsMgr->Signal( *this ); //!< signal 'at 1st instr' to plugins...
  return ReturnCode();
}

//!< reset detected. at 1st instruction after reset...

int SimThreadResetEvent::Process() {
  mPluginsMgr->Signal( *this ); //!< signal 'reset' to plugins...
  return ReturnCode();
}

//!< at first instruction of thread-specific boot code...

int SimThreadBootEvent::Process() {
  mPluginsMgr->Signal( *this ); //!< signal 'at boot code' to plugins...
  return ReturnCode();
}

//!< step an instruction...

int SimThreadStepEvent::Process() {
  LOG(debug) << "SimThread: Processing Step event..." << endl;

  int rcode = 0;

  // signal 'pre-step' to plugins...
  SimThreadPreStepEvent pre_step_event(*this);
  mPluginsMgr->Signal( pre_step_event ); 
      
  rcode = pre_step_event.ReturnCode(); // plugins may detect some error...

  std::vector<RegUpdate> reg_updates;
  std::vector<MemUpdate> mem_updates;
  std::vector<MmuEvent> mmu_events;
  std::vector<ExceptionUpdate> exc_updates;

  // step next instruction...

  if (!rcode)
    mSimPtr->Step(mCpuId,reg_updates,mem_updates,mmu_events,exc_updates);

  // pass register update to plugins...

  for (auto reg_update = reg_updates.begin(); reg_update != reg_updates.end() && !rcode; reg_update++) {
     SimThreadRegisterUpdateEvent reg_update_event(*this,&(*reg_update));
     mPluginsMgr->Signal( reg_update_event ); 
     rcode = reg_update_event.ReturnCode();
  }

  // pass memory update to plugins...

  for (auto mem_update = mem_updates.begin(); mem_update != mem_updates.end() && !rcode; mem_update++) {
     SimThreadMemoryUpdateEvent mem_update_event(*this,&(*mem_update));
     mPluginsMgr->Signal( mem_update_event ); 
     rcode = mem_update_event.ReturnCode();
  }
  
  // pass mmu 'events' to plugins...

  for (auto mmu_event = mmu_events.begin(); mmu_event != mmu_events.end() && !rcode; mmu_event++) {
     SimThreadMmuEvent MMU_event(*this,&(*mmu_event));
     mPluginsMgr->Signal( MMU_event ); 
     rcode = MMU_event.ReturnCode();
  }
  
  // some exception related event...

  /*
    some exception IDS to expect:

         ID          meaning
       ------     ----------------------------
        0x4d       low power
        0x4c       simulator exit
        0x4e       ERET
  */

  bool low_power_mode = false;

  for (auto exc_update = exc_updates.begin(); exc_update != exc_updates.end() && !rcode; exc_update++) {
     SimThreadExceptionEvent exc_event(*this,&(*exc_update));
     mPluginsMgr->Signal( exc_event );  
     rcode = exc_event.ReturnCode();
     // watch for low power...
     if ( (*exc_update).mExceptionID == 0x4d )
       low_power_mode = true;
  }

  if (low_power_mode && mLowPowerNop ) {
    // inject to wake up core...
    SimPtr()->WakeUp(CpuId());
  }

  // signal 'post-step' to plugins...

  if (!rcode) {  
    SimThreadPostStepEvent post_step_event(*this,&reg_updates,&mem_updates,&mmu_events,&exc_updates);
    mPluginsMgr->Signal( post_step_event );
    rcode = post_step_event.ReturnCode();
  }

  mStepCnt++; // bump stepped instruction count

  return rcode;
}

//!< step event is not 'done' until the requested instruction count has been reached,
//!<  or the maximum # of instructions has been exceeded...

bool SimThreadStepEvent::Done() const {
  if ( (mRequestedCnt > 0) && (mStepCnt >= mRequestedCnt) ) 
    return true;  // the 'requested' step count has been reached

  if ( mStepCnt > mMaxCount ) {
    // max simulation count exceeded...
    stringstream err_msg;
    err_msg << "ERROR: Cpu " << dec << mCpuId << " step max count " << mMaxCount << " exceeded";
    throw SimulationError(err_msg.str());
    return true; // step is 'done' of course
  }

  return false;
}

//!< when delay-count hits zero, insert interrupt

int SimThreadInsertInterruptEvent::Process() {
  /* TBD...
   mDelayCnt--; 
   if (mDelayCnt == 0) {
     // inject interrupt...TBD...
   }

  */
  return 0; 
}

//!< return true when end-test condition reached

bool SimThread::EndTest(uint64_t nextPC) {
  if (nextPC == mCurrentPC)
    return true;
  mCurrentPC = nextPC;
  return false;
}

//!< return true when maximum step instruction count exceeded

bool SimThread::MaxCountReached(int step_count) const {
  return step_count > mSimCfg->MaxInsts();
}

}
