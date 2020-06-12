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
#ifndef Fpix_Event_H
#define Fpix_Event_H

#include <Object.h>
#include <SimAPI.h>
#include <PluginManager.h>
#include <EnumsFPIX.h>

using namespace std;

namespace Force {

  /*!
   \class SimThreadEvent
   \brief Simulator thread event class.
   */

  class SimThread;

  class SimThreadEvent : public Object {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(SimThreadEvent);

    SimThreadEvent() :
      mEventType(ESimThreadEventType::UNKNOWN_EVENT),
      mCpuId(0),
      mSimPtr(nullptr),
      mPluginsMgr(nullptr),
      mRegUpdates(nullptr),
      mMemUpdates(nullptr),
      mMmuEvents(nullptr),
      mExcUpdates(nullptr),
      mRegUpdate(nullptr),
      mExcUpdate(nullptr),
      mMemUpdate(nullptr),
      mMmuEvent(nullptr),
      mReturnCode(0)
    {Init(); };

    SimThreadEvent(SimThreadEvent *src) : 
      mEventType(ESimThreadEventType::UNKNOWN_EVENT),
      mCpuId(0),
      mSimPtr(nullptr),
      mPluginsMgr(nullptr),
      mRegUpdates(nullptr),
      mMemUpdates(nullptr),
      mMmuEvents(nullptr),
      mExcUpdates(nullptr),
      mRegUpdate(nullptr),
      mExcUpdate(nullptr),
      mMemUpdate(nullptr),
      mMmuEvent(nullptr),
      mReturnCode(0)
    { Init(src->mEventType, src->mCpuId, src->mSimPtr, src->mPluginsMgr); };

    SimThreadEvent(SimThreadEvent &src) : 
      mEventType(ESimThreadEventType::UNKNOWN_EVENT),
      mCpuId(0),
      mSimPtr(nullptr),
      mPluginsMgr(nullptr),
      mRegUpdates(nullptr),
      mMemUpdates(nullptr),
      mMmuEvents(nullptr),
      mExcUpdates(nullptr),
      mRegUpdate(nullptr),
      mExcUpdate(nullptr),
      mMemUpdate(nullptr),
      mMmuEvent(nullptr),
      mReturnCode(0)
    { Init(src.mEventType, src.mCpuId, src.mSimPtr, src.mPluginsMgr); };

    SimThreadEvent(const SimThreadEvent &src) : 
      mEventType(ESimThreadEventType::UNKNOWN_EVENT),
      mCpuId(0),
      mSimPtr(nullptr),
      mPluginsMgr(nullptr),
      mRegUpdates(nullptr),
      mMemUpdates(nullptr),
      mMmuEvents(nullptr),
      mExcUpdates(nullptr),
      mRegUpdate(nullptr),
      mExcUpdate(nullptr),
      mMemUpdate(nullptr),
      mMmuEvent(nullptr),
      mReturnCode(0)
    { Init(src.mEventType, src.mCpuId, src.mSimPtr, src.mPluginsMgr); };

    virtual ~SimThreadEvent() {};

    virtual int Process() { return mReturnCode; };
    virtual bool Done() const { return true; };

    //!< init all private data members
    void Init(ESimThreadEventType eventType = ESimThreadEventType::UNKNOWN_EVENT, uint32_t pCpuId = 0, SimAPI *pSimPtr = nullptr, PluginManager *pPluginsMgr = nullptr) {
      mEventType = eventType;
      mCpuId = pCpuId;
      mSimPtr = pSimPtr;
      mPluginsMgr = pPluginsMgr;
      mRegUpdates = nullptr;
      mMemUpdates = nullptr;
      mMmuEvents = nullptr;
      mExcUpdates = nullptr;
      mRegUpdate = nullptr;
      mExcUpdate = nullptr;
      mMemUpdate = nullptr;
      mMmuEvent = nullptr;
      mReturnCode = 0;
    };

    //!< initialize from SimThread object
    void Init(ESimThreadEventType eventType, SimThread &pST); //!< init common SimThreadEvent variables in context of thread

    //!< initialize from SimThreadEvent
    void Init(ESimThreadEventType eventType, SimThreadEvent &src) {
      Init(eventType, src.mCpuId, src.mSimPtr, src.mPluginsMgr); 
    };

    ESimThreadEventType Type() { return mEventType; };
    uint32_t CpuId() const { return mCpuId; };
    int ReturnCode() const { return mReturnCode; };
    void SetReturnCode(int pReturnCode) { mReturnCode = pReturnCode; };

    SimAPI *SimPtr() const { return mSimPtr; };
    PluginManager *PluginsMgr() const { return mPluginsMgr; };

    virtual Object* Clone() const override { return new SimThreadEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadEvent"; };
    virtual const char* Type() const override { return "SimThreadEvent"; };

    std::vector<RegUpdate> *RegisterUpdates() const { return mRegUpdates; };
    std::vector<MemUpdate> *MemoryUpdates() const { return mMemUpdates; };
    std::vector<MmuEvent> *MmuEvents() const { return mMmuEvents; };
    std::vector<ExceptionUpdate> *ExceptionUpdates() const { return mExcUpdates; };

    RegUpdate *RegisterUpdate() const { return mRegUpdate; };
    ExceptionUpdate *ExceptUpdate() const { return mExcUpdate; };
    MemUpdate *MemoryUpdate() const { return mMemUpdate; };
    MmuEvent *MMUEvent() const { return mMmuEvent; };

  protected:
    ESimThreadEventType mEventType;
    uint32_t mCpuId;
    SimAPI *mSimPtr;
    PluginManager *mPluginsMgr;

    std::vector<RegUpdate> *mRegUpdates;
    std::vector<MemUpdate> *mMemUpdates;
    std::vector<MmuEvent> *mMmuEvents;
    std::vector<ExceptionUpdate> *mExcUpdates;

    RegUpdate *mRegUpdate;
    ExceptionUpdate *mExcUpdate;
    MemUpdate *mMemUpdate;
    MmuEvent *mMmuEvent;

    int mReturnCode;
  };

  class SimThreadStartTestEvent:public SimThreadEvent {
  public:
    SimThreadStartTestEvent(SimThread &pST) { Init(ESimThreadEventType::START_TEST, pST); };
    int Process();

    Object* Clone() const override { return new SimThreadStartTestEvent(*this); };
    const std::string ToString() const override { return "SimThreadStartTestEvent"; };
    const char* Type() const override { return "SimThreadStartTestEvent"; };
  };

  class SimThreadEndTestEvent:public SimThreadEvent {
  public:
    SimThreadEndTestEvent(SimThread &pST) { Init(ESimThreadEventType::END_TEST, pST); };
    int Process();

    virtual Object* Clone() const override { return new SimThreadEndTestEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadEndTestEvent"; };
    virtual const char* Type() const override { return "SimThreadEndTestEvent"; };
  };

  class SimThreadResetEvent:public SimThreadEvent {
  public:
    SimThreadResetEvent(SimThread &pST) { Init(ESimThreadEventType::RESET, pST); };
    int Process();

    virtual Object* Clone() const override { return new SimThreadResetEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadResetEvent"; };
    virtual const char* Type() const override { return "SimThreadResetEvent"; };
  };

  class SimThreadBootEvent:public SimThreadEvent {
  public:
    SimThreadBootEvent(SimThread &pST) { Init(ESimThreadEventType::BOOT_CODE, pST); };
    int Process();

    virtual Object* Clone() const override { return new SimThreadBootEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadBootEvent"; };
    virtual const char* Type() const override { return "SimThreadBootEvent"; };
  };

  class SimThreadFirstInstrEvent:public SimThreadEvent {
  public:
    SimThreadFirstInstrEvent(SimThread &pST) { Init(ESimThreadEventType::FIRST_INSTRUCTION, pST); };
    int Process();

    virtual Object* Clone() const override { return new SimThreadFirstInstrEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadFirstInstrEvent"; };
    virtual const char* Type() const override { return "SimThreadFirstInstrEvent"; };
  };

  //!< need copy constructor to allow some event to be inserted
  //!< in the 'middle'...

  //!< step event - 
  class SimThreadStepEvent:public SimThreadEvent {
  public:
  SimThreadStepEvent(SimThread &pST,int max_count, bool lowpower_nop,int requested_step_count = -1) 
    : mStepCnt(0), mMaxCount(max_count), mLowPowerNop(lowpower_nop),mRequestedCnt(requested_step_count) { Init(ESimThreadEventType::STEP, pST); };

    int Count() const { return mStepCnt; };
    int SetRequestCount(int requested_count) { mRequestedCnt = requested_count; return requested_count;  };
    int AdvanceCount(int count_increment) { mStepCnt += count_increment; return mStepCnt; };
    int Process();
    bool Done() const;

    virtual Object* Clone() const override { return new SimThreadStepEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadStepEvent"; };
    virtual const char* Type() const override { return "SimThreadStepEvent"; };

  private:
   int  mStepCnt;      //!< step count
   int  mMaxCount;     //!< max step count
   bool mLowPowerNop;       //!< if true, then watch for and exit low power mode
   int  mRequestedCnt; //!< only step this many times
  };

  class SimThreadPreStepEvent:public SimThreadEvent {
  public:
    SimThreadPreStepEvent(SimThread &pST) { Init(ESimThreadEventType::PRE_STEP, pST); };
    SimThreadPreStepEvent(SimThreadStepEvent &src) { Init(ESimThreadEventType::PRE_STEP, src); };

    virtual Object* Clone() const override { return new SimThreadPreStepEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadPreStepEvent"; };
    virtual const char* Type() const override { return "SimThreadPreStepEvent"; };
  };

  class SimThreadRegisterUpdateEvent:public SimThreadEvent {
  public:
    SimThreadRegisterUpdateEvent(SimThreadStepEvent &src, RegUpdate *pRegUpdate) { 
      Init(ESimThreadEventType::REGISTER_UPDATE, src);
      mRegUpdate = pRegUpdate;
    };

    virtual Object* Clone() const override { return new SimThreadRegisterUpdateEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadRegisterUpdateEvent"; };
    virtual const char* Type() const override { return "SimThreadRegisterUpdateEvent"; };
  };

  class SimThreadMemoryUpdateEvent:public SimThreadEvent {
  public:
    SimThreadMemoryUpdateEvent(SimThreadEvent &src, MemUpdate *pMemUpdate) { 
      Init(ESimThreadEventType::MEMORY_UPDATE, src);
      mMemUpdate = pMemUpdate;
    };

    virtual Object* Clone() const override { return new SimThreadMemoryUpdateEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadMemoryUpdateEvent"; };
    virtual const char* Type() const override { return "SimThreadMemoryUpdateEvent"; };
  };

  class SimThreadMmuEvent:public SimThreadEvent {
  public:
    SimThreadMmuEvent(SimThreadEvent &src, MmuEvent *pMmuEvent) { 
      Init(ESimThreadEventType::MMU_EVENT, src);
      mMmuEvent = pMmuEvent;
    };

    virtual Object* Clone() const override { return new SimThreadMmuEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadMmuEvent"; };
    virtual const char* Type() const override { return "SimThreadMmuEvent"; };
  };

  class SimThreadExceptionEvent:public SimThreadEvent {
  public:
    SimThreadExceptionEvent(SimThreadEvent &src,ExceptionUpdate *pExcUpdate) {
      Init(ESimThreadEventType::EXCEPTION_EVENT, src);
      mExcUpdate = pExcUpdate; 
    };

    virtual Object* Clone() const override { return new SimThreadExceptionEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadExceptionEvent"; };
    virtual const char* Type() const override { return "SimThreadExceptionEvent"; };
  };

  class SimThreadPostStepEvent:public SimThreadEvent {
  public:
    SimThreadPostStepEvent(SimThreadEvent &src, std::vector<RegUpdate> *pRegUpdates, std::vector<MemUpdate> *pMemUpdates, 
                           std::vector<MmuEvent> *pMmuEvents, std::vector<ExceptionUpdate> *pExcUpdates) {
      Init(ESimThreadEventType::POST_STEP, src);
      mRegUpdates = pRegUpdates;
      mMemUpdates = pMemUpdates;
      mMmuEvents = pMmuEvents;
      mExcUpdates = pExcUpdates;
    };

    virtual Object* Clone() const override { return new SimThreadPostStepEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadPostStepEvent"; };
    virtual const char* Type() const override { return "SimThreadPostStepEvent"; };
  };

  class SimThreadDelayEvent:public SimThreadEvent {
  public:
    SimThreadDelayEvent(SimThread &pST, int delay_count) : mDelayCnt(delay_count) { Init(ESimThreadEventType::DELAY, pST); };

    int Count() const { return mDelayCnt; };
    int UpdateCount(int new_count=1) { mDelayCnt -= new_count; ; return mDelayCnt; }
    int Process() { mDelayCnt--; return true; };

    virtual Object* Clone() const override { return new SimThreadDelayEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadDelayEvent"; };
    virtual const char* Type() const override { return "SimThreadDelayEvent"; };
  private:
    int mDelayCnt;  //!< delay as long as > 0
  };

  //!< delay for insert-interrupt is # of instructions. the delay should be used to
  //!< schedule the insert-interrupt event (the delay count then is NOT maintained
  //!< as part of this event...

  class SimThreadInsertInterruptEvent:public SimThreadEvent {
  public:
    SimThreadInsertInterruptEvent(SimThread &pST, int pInterruptType) : mInterruptType(pInterruptType) { Init(ESimThreadEventType::INTERRUPT, pST);};

    virtual Object* Clone() const override { return new SimThreadInsertInterruptEvent(*this); };
    virtual const std::string ToString() const override { return "SimThreadInsertInterruptEvent"; };
    virtual const char* Type() const override { return "SimThreadInsertInterruptEvent"; };
    int Process();
  private:
    int mInterruptType; //!< which interrupt to cause?
  };

}

#endif
