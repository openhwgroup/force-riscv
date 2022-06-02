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
#include "PageRequestRegulatorRISCV.h"

#include "GenRequest.h"
#include "Generator.h"
#include "InstructionStructure.h"
#include "Log.h"
#include "Register.h"

using namespace std;

/*!
  \file PageRequestRegulatorRISCV.cc
  \brief RISC-V layer paging request regulating code.
*/

namespace Force {

  Object* PageRequestRegulatorRISCV::Clone() const
  {
    return new PageRequestRegulatorRISCV(*this);
  }

  PageRequestRegulatorRISCV::PageRequestRegulatorRISCV()
    : PageRequestRegulator()
  {
    //mInstrExceptionTypes.push_back(EPagingExceptionType::InstructionAccessFault);
    mInstrExceptionTypes.push_back(EPagingExceptionType::InstructionPageFault);

    //mDataExceptionTypes.push_back(EPagingExceptionType::LoadAccessFault);
    //mDataExceptionTypes.push_back(EPagingExceptionType::StoreAmoAccessFault);
    mDataExceptionTypes.push_back(EPagingExceptionType::LoadPageFault);
    mDataExceptionTypes.push_back(EPagingExceptionType::StoreAmoPageFault);
  }

  PageRequestRegulatorRISCV::PageRequestRegulatorRISCV(const PageRequestRegulatorRISCV& rOther)
    : PageRequestRegulator(rOther)
  {

  }

  PageRequestRegulatorRISCV::~PageRequestRegulatorRISCV()
  {

  }

  void PageRequestRegulatorRISCV::Setup(const Generator* pGen)
  {
    PageRequestRegulator::Setup(pGen);
  }

  void PageRequestRegulatorRISCV::RegulateLoadStorePageRequest(const VmMapper* pVmMapper, const LoadStoreOperandStructure* pLsStruct, GenPageRequest* pPageReq) const
  {
    uint32 priv_level = mpGenerator->PrivilegeLevel();
    if (nullptr != pLsStruct)
    {
      // coming from instruction, can use current privilege level
      if (pLsStruct->Unprivileged() || priv_level == 0)
      {
        pPageReq->SetPrivilegeLevel(EPrivilegeLevelType::U);
      }
      else
      {
        pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::Privileged, 1);
      }

      if (pLsStruct->AtomicOrderedAccess())
      {
        pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::Atomic, 1);
      }
    }
    else
    {
      // not coming from instruction, therefore targetting EL might be reached via exception.
      if (pPageReq->PrivilegeLevelSpecified())
      {
        priv_level = uint32(pPageReq->PrivilegeLevel());
      }
      else if (priv_level == 0)
      {
        pPageReq->SetPrivilegeLevel(EPrivilegeLevelType::U);
      }

      if (priv_level != 0)
      {
        pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::Privileged, 1);
      }
    }

    PageRequestRegulator::RegulateLoadStorePageRequest(pVmMapper, pLsStruct, pPageReq);
  }

  void PageRequestRegulatorRISCV::RegulateBranchPageRequest(const VmMapper* pVmMapper, const BranchOperandStructure* pBrStruct, GenPageRequest* pPageReq) const
  {
    uint32 priv_level = mpGenerator->PrivilegeLevel();
    if (nullptr != pBrStruct)
    {
      // coming from instruction, can use current privilege level
      if (priv_level == 0)
      {
        pPageReq->SetPrivilegeLevel(EPrivilegeLevelType::U);
      }
      else
      {
        pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::Privileged, 1);
      }
    }
    else
    {
      // not coming from instruction, so target privilege could differ from the current privilege.
      if (pPageReq->PrivilegeLevelSpecified())
      {
        priv_level = uint32(pPageReq->PrivilegeLevel());
      }
      else if (priv_level == 0)
      {
        pPageReq->SetPrivilegeLevel(EPrivilegeLevelType::U);
      }

      if (priv_level != 0)
      {
        pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::Privileged, 1);
      }
    }

    PageRequestRegulator::RegulateBranchPageRequest(pVmMapper, pBrStruct, pPageReq);
  }

}
