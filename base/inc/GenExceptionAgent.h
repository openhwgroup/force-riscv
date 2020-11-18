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
#ifndef Force_GenExceptionAgent_H
#define Force_GenExceptionAgent_H

#include <Defines.h>
#include <GenAgent.h>
#include <GenRequest.h>
#include <ExceptionManager.h>
#include <vector>

namespace Force {

  class GenExceptionRequest;
  class AddressTable;
  class ExceptionContext;

  /*!
    \class GenExceptionAgent
    \brief A generator agent class to handle exception related requests.
  */
  class GenExceptionAgent : public GenAgent {
  public:
    Object* Clone() const override;  //!< Return a cloned GenExceptionAgent object of the same type and content.
    const char* Type() const override { return "GenExceptionAgent"; } //!< Return type of the GenExceptionAgent object.

    GenExceptionAgent() : GenAgent(), mpExceptionRequest(nullptr) { } //!< Constructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenExceptionAgent);

    EGenAgentType GenAgentType() const override { return EGenAgentType::GenExceptionAgent; } //!< Return type of the generator agent.
    void SetGenRequest(GenRequest* genRequest) override; //!< Set pointer to GenRequest object.
    virtual EExceptionVectorType GetExceptionVectorType(const std::string& vecStr) const { return EExceptionVectorType(0); } //!< Convert string to vector type Enum.
    virtual bool IsExceptionReturn(const GenExceptionRequest* pExceptReq) const { return false; } //!< Check if this is an exception return.
    virtual bool IsLowPower(const GenExceptionRequest* pExceptReq) const {return false;} //!< Check if this is a lower Power status
    virtual bool IsSimExit(const GenExceptionRequest* pExceptReq) const {return false;} //!< Check if Sim is exited or not
    virtual bool IsFastMode() const {return false;} //!< Check if fast exception handlers are enabled
  protected:
    GenExceptionAgent(const GenExceptionAgent& rOther) : GenAgent(rOther), mpExceptionRequest(nullptr) { } //!< Copy constructor, do not copy the request pointer
    void HandleRequest() override; //!< Handle GenExceptionRequest transaction.
    void CleanUpRequest() override; //!< Clean up request item.
    void ResetRequest() override; //!< Reset request item.
    virtual void SystemCall(); //<! Handle System Call GenExecpetionRequest
    virtual void UpdateArchHandlerInfo(); //!< process UpdateArchHandlerInfo GenExceptionRequest
    virtual bool AllowExceptionInException(const GenHandleException* except_req, std::string& rErrInfo) const { return false; } //!< Allow certain exceptions to be taken inside other exceptions
    virtual bool NeedRecoveryAddress() const { return false; }
    void UpdateHandlerInfo(); //!< process UpdateArchHandlerInfo GenExceptionRequest
    void HandleException(); //!< Handle taken exception.
    virtual void RecordExceptionSpecificAddressBounds(const std::map<std::string, std::string>& myMap) { }; //!< record exception specific address bounds.
    void DiagnoseExceptionDefault(EExceptionClassType exceptClass, std::string& rMsgStr) const; //!< Diagnose exception default code.
    void ProcessHandlerBounds(cuint32 bank, const std::string& rHandlerBounds, ExceptionManager::ListOfAddressBoundaries& rBoundariesList); //!< Parse a string of exception handler address boundaries into a list of numeric address boundaries.
    virtual ExceptionContext* ExceptionContextInstance(EExceptionClassType exceptClass) const; //!< Return an ExceptionContext object.
  protected:
    GenExceptionRequest* mpExceptionRequest; //!< Pointer to GenExceptionRequest object.
  private:
    virtual void AddPostExceptionRequests() { } //!< Add necessary requests following the exception, if any.
  };

}

#endif
