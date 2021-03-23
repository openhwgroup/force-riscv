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
#ifndef Fpix_Plugin_H
#define Fpix_Plugin_H

#include <Notify.h>
#include <SimAPI.h>
#include <EnumsFPIX.h>
#include <map>

using namespace std;

namespace Force {

  /*!
   \class SimPlugin
   \brief Simulator plugin base class.
   */

  class SimPlugin : public Receiver<ESimThreadEventType> {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(SimPlugin);
    COPY_CONSTRUCTOR_ABSENT(SimPlugin);
    SimPlugin() : 
      mCpuID(0),    //!< current cpu ID, sim-ptr
      mSimPtr(0),   //!< NOTE: updated each time from HandleNotification
      mReturnCode(-1)    //!< plugin return code
    {};
    virtual ~SimPlugin() {};

    virtual const std::string Name() const { return "?"; };

    virtual bool IsSupported(ESimThreadEventType eventType) const = 0; //!< sub-class must supply this method

    virtual void parsePluginsClargs(std::vector<std::string> &plugins_cl_args) = 0; //!< sub-class must supply this method

    virtual void parsePluginsOptions(std::map<std::string, std::string> &aRPluginsOptions) = 0; //!< sub-class must supply this method

    //<! handle sim event notification...

    void HandleNotification(const Sender<ESimThreadEventType>* sender, ESimThreadEventType eventType, Object* pPayload); 

    uint32_t CpuID() const { return mCpuID; };    //!< current cpu ID
    SimAPI  *SimPtr() const { return mSimPtr; };  //!<   "     sim-ptr

    //!< plugins can optionally set a return code should some error be detected

    void SetReturnCode(int pRtnCode) { mReturnCode = pRtnCode; };

    //!< sub-class, ie, user plugin, must both implement one or more of these event methods:

    virtual void atTestStart() { };       //!< after test image has been loaded, before 1st instruction step
    virtual void onReset() { };           //!< when reset detected. PC of 1st instr after reset 
    virtual void onBootCode() { };        //!< before first thread-specific boot code instruction
    virtual void onMain() { };            //!< before 1st random instruction
    virtual void onPreStep() { };         //!< before an instruction is stepped

    //!< after stepping an instruction:
    virtual void onStep(std::vector<RegUpdate> *reg_updates,std::vector<MemUpdate> *mem_updates,
                        std::vector<MmuEvent> *mmu_events, std::vector<ExceptionUpdate> *exceptions) { };

    virtual void onRegisterUpdate(RegUpdate *reg_update) { };    //!< on register update as a result of stepping an instruction
    virtual void onMemoryUpdate(MemUpdate *mem_update) { };      //!< "  memory                   "
    virtual void onException(ExceptionUpdate *exceptions) { };   //!< on instruction that caused an exception, after step

    virtual void atTestEnd() { };         //!< at test ends, after last instruction was stepped

  private:
    uint32_t mCpuID;    //!< current cpu ID, sim-ptr
    SimAPI  *mSimPtr;   //!< NOTE: updated each time from HandleNotification

    int mReturnCode;    //!< plugin return code
  };

}

#endif
