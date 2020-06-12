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
#include <PageRequestRegulatorRISCV.h>
#include <InstructionStructure.h>
#include <Generator.h>
#include <GenRequest.h>
#include <Register.h>
#include <Log.h>

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
    //TODO setup privileged condition for RISCV if needed
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

  const char* PageRequestRegulatorRISCV::GetExceptionString(EPagingExceptionType exceptType) const
  {
    return EPagingExceptionType_to_string(exceptType).c_str();
  }

  void PageRequestRegulatorRISCV::PreventDataAbort(GenPageRequest* pPageReq) const
  {
    PageRequestRegulator::PreventDataAbort(pPageReq);
    pPageReq->SetExceptionConstraint(EPagingExceptionType::LoadPageFault, EExceptionConstraintType::PreventHard);
    pPageReq->SetGenAttributeValue(EPageGenAttributeType::Invalid, 0); // ensure Invalid attribute is set to 0.
    pPageReq->SetGenAttributeValue(EPageGenAttributeType::AddrSizeFault, 0); // Don't generate address size fault for system pages
  }

  void PageRequestRegulatorRISCV::PreventInstrAbort(GenPageRequest* pPageReq) const
  {
    PageRequestRegulator::PreventInstrAbort(pPageReq);
    pPageReq->SetExceptionConstraint(EPagingExceptionType::InstructionPageFault, EExceptionConstraintType::PreventHard);
    pPageReq->SetGenAttributeValue(EPageGenAttributeType::Invalid, 0); // ensure Invalid attribute is set to 0.
    pPageReq->SetGenAttributeValue(EPageGenAttributeType::AddrSizeFault, 0); // Don't generate address size fault for system pages
  }

}
