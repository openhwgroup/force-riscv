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
#ifndef Force_GenExceptionAgentRISCV_H
#define Force_GenExceptionAgentRISCV_H

#include <GenExceptionAgent.h>
#include <GenRequest.h>
#include <map>
#include <set>

namespace Force {

  class VmMapper;
  struct SwitchPrivilegeResultInfo;

  /*!
    \class GenExceptionAgentRISCV
    \brief A generator agent class to handle exception related requests.
  */
  class GenExceptionAgentRISCV : public GenExceptionAgent {
  public:
    GenExceptionAgentRISCV(); //!< Constructor.
    Object* Clone() const override;  //!< Return a cloned GenExceptionAgentRISCV object of the same type and content.
    const char* Type() const override { return "GenExceptionAgentRISCV"; } //!< Return type of the GenExceptionAgentRISCV object.
    EExceptionVectorType GetExceptionVectorType(const std::string& vecStr) const override; //!< Convert string to vector type Enum.
    bool IsExceptionReturn(const GenExceptionRequest* pExceptReq) const override; //!< Check if this is an exception return.
    bool IsLowPower(const GenExceptionRequest* pExceptReq) const override; //!< Check if this is a lower power status
    bool IsSimExit(const GenExceptionRequest* pExceptReq) const override; //!< Check if sim is exited or not
    bool IsFastMode() const override; //!< Check if fast exception handlers are enabled
  protected:
    GenExceptionAgentRISCV(const GenExceptionAgentRISCV& rOther); //!< Copy constructor.

    void SystemCall() override; //<! Handle System Call GenExecpetionRequest
    void UpdateArchHandlerInfo() override; //<! process UpdateHandlerInfo GenExceptionRequest
    void SwitchPrivilege(const std::map<std::string, std::string>& params_dict, std::map<std::string, uint64>& results); //!< handle switch exception level request
    uint64 BuildDataBlock(const SwitchPrivilegeResultInfo& results,  const std::map<std::string, uint64>& sysRegReloads); //!< build switch el data block
    void ParseHandlerArgumentRegisters(const std::map<std::string, std::string>& rParams); //!< parse and record the exception registers used for passing arguments
    uint64 BuildRequests(uint32 targetPrivLevel, uint64 targetAddr); //!< build a list of requests to be called later, return the requests record id
    bool AllowExceptionInException(const GenHandleException* except_req, std::string& rErrInfo) const override; //!< allow exception in exception
    bool NeedRecoveryAddress() const override; //!< need recovery address.
    void RecordExceptionSpecificAddressBounds(const std::map<std::string, std::string>& myMap) override; //!< record exception specific address bounds.
    //ExceptionContext* ExceptionContextInstance(EExceptionClassType exceptClass) const override; //!< Return an ExceptionContext object.
  private:
    //void DiagnoseUnknownReasonException(std::string& rMsgStr) const; //!< Diagnose unknown reason exception.
  private:
    std::vector<uint32> mArgRegIndices; //!< handler argument regsiters
    bool mFastMode; //!< flag indicating fast mode
    std::set<uint64> mAddrTableEC; //!< Address table supported custom error codes
  };

}

#endif
