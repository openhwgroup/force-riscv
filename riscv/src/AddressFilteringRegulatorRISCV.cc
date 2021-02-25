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
#include <AddressFilteringRegulatorRISCV.h>
#include <VmConstraint.h>
#include <Constraint.h>
#include <GenRequest.h>
#include <VmMapper.h>
#include <Log.h>

/*!
  \file AddressFilteringRegulatorRISCV.cc
  \brief AddressFilteringRegulator RISCV layer code.
*/

using namespace std;

namespace Force {

  AddressFilteringRegulatorRISCV::AddressFilteringRegulatorRISCV()
    : AddressFilteringRegulator()
  {

  }

  AddressFilteringRegulatorRISCV::AddressFilteringRegulatorRISCV(const AddressFilteringRegulatorRISCV& rOther)
    : AddressFilteringRegulator(rOther)
  {

  }

  AddressFilteringRegulatorRISCV::~AddressFilteringRegulatorRISCV()
  {

  }

  Object* AddressFilteringRegulatorRISCV::Clone() const
  {
    return new AddressFilteringRegulatorRISCV(*this);
  }

  void AddressFilteringRegulatorRISCV::Setup(const Generator* pGen)
  {
    AddressFilteringRegulator::Setup(pGen);
  }

  void AddressFilteringRegulatorRISCV::GetInstrVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, vector<VmConstraint* >& rVmConstraints) const
  {
    const map<EPagingExceptionType, EExceptionConstraintType>& excep_constraints = rPageReq.GetExceptionConstraints();

    for (auto vm_constr_item : excep_constraints)
    {
      switch (vm_constr_item.first)
      {
        case EPagingExceptionType::InstructionAccessFault:
          GetInstrAccessVmConstraints(rPageReq, rVmMapper, rVmConstraints, vm_constr_item.second);
          break;
        case EPagingExceptionType::InstructionPageFault:
          GetInstrPageFaultVmConstraints(rPageReq, rVmMapper, rVmConstraints, vm_constr_item.second);
          break;
        default:
          break;
      }
    }
  }

  void AddressFilteringRegulatorRISCV::GetDataVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, vector<VmConstraint* >& rVmConstraints) const
  {
    const map<EPagingExceptionType, EExceptionConstraintType>& excep_constraints = rPageReq.GetExceptionConstraints();

    for (auto vm_constr_item : excep_constraints)
    {
      switch (vm_constr_item.first)
      {
        case EPagingExceptionType::LoadAccessFault:
        case EPagingExceptionType::StoreAmoAccessFault:
          GetDataAccessVmConstraints(rPageReq, rVmMapper, rVmConstraints, vm_constr_item.second);
          break;
        case EPagingExceptionType::LoadPageFault:
        case EPagingExceptionType::StoreAmoPageFault:
          GetDataPageFaultVmConstraints(rPageReq, rVmMapper, rVmConstraints, vm_constr_item.second);
          break;
        default:
          break;
      }
    }
  }

  void AddressFilteringRegulatorRISCV::GetInstrAccessVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, vector<VmConstraint* >& rVmConstraints, EExceptionConstraintType permConstrType) const
  {
    switch (permConstrType)
    {
      case EExceptionConstraintType::PreventHard:
        {
          /*bool privileged = false;
          rPageReq.GetGenBoolAttribute(EPageGenBoolAttrType::Privileged, privileged);
          if (privileged)
          {
            const ConstraintSet* vm_constr = rVmMapper.GetVmConstraint(EVmConstraintType::PrivilegedNoExecute);
            if (vm_constr != nullptr)
            {
              rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::PrivilegedNoExecute, vm_constr));
            }
          }*/
        }
        break;
      default:
        break;
    }
  }

  void AddressFilteringRegulatorRISCV::GetInstrPageFaultVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, vector<VmConstraint* >& rVmConstraints, EExceptionConstraintType constrType) const
  {
    switch (constrType)
    {
      case EExceptionConstraintType::PreventHard:
        {
          if (rPageReq.PrivilegeLevelSpecified() and (rPageReq.PrivilegeLevel() == EPrivilegeLevelType::U))
          {
            const ConstraintSet* unpriv_constr = rVmMapper.GetVmConstraint(EVmConstraintType::UnprivilegedNoExecute);
            if (unpriv_constr != nullptr)
            {
              rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::UnprivilegedNoExecute, unpriv_constr));
            }
          }
          else
          {
            bool privileged = rPageReq.GenBoolAttributeDefaultFalse(EPageGenBoolAttrType::Privileged);
            if (privileged)
            {
              const ConstraintSet* priv_constr = rVmMapper.GetVmConstraint(EVmConstraintType::PrivilegedNoExecute);
              if (priv_constr != nullptr)
              {
                rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::PrivilegedNoExecute, priv_constr));
              }
            }
          }

          const ConstraintSet* pf_constr = rVmMapper.GetVmConstraint(EVmConstraintType::PageFault);
          if (pf_constr != nullptr)
          {
            rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::PageFault, pf_constr));
          }
        }
        break;
      default:
        break;
    }
  }

  void AddressFilteringRegulatorRISCV::GetDataAccessVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, vector<VmConstraint* >& rVmConstraints, EExceptionConstraintType permConstrType) const
  {
    switch (permConstrType)
    {
      case EExceptionConstraintType::PreventHard:
        {
        }
        break;
      default:
        break;
    }
  }

  void AddressFilteringRegulatorRISCV::GetDataPageFaultVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, vector<VmConstraint* >& rVmConstraints, EExceptionConstraintType constrType) const
  {
    switch (constrType)
    {
      case EExceptionConstraintType::PreventHard:
        {
          if (rPageReq.PrivilegeLevelSpecified() && (rPageReq.PrivilegeLevel() == EPrivilegeLevelType::U))
          {
            const ConstraintSet* no_user_constr = rVmMapper.GetVmConstraint(EVmConstraintType::NoUserAccess);
            if (no_user_constr != nullptr)
            {
              rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::NoUserAccess, no_user_constr));
            }
          }
          else
          {
            const ConstraintSet* user_constr = rVmMapper.GetVmConstraint(EVmConstraintType::UserAccess);
            if (user_constr != nullptr)
            {
              rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::UserAccess, user_constr));
            }
          }

          auto mem_access = rPageReq.MemoryAccessType();
          if (EMemAccessTypeBaseType(mem_access) & EMemAccessTypeBaseType(EMemAccessType::Write))
          {
            const ConstraintSet* ro_constr = rVmMapper.GetVmConstraint(EVmConstraintType::ReadOnly);
            if (ro_constr != nullptr)
            {
              rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::ReadOnly, ro_constr));
            }

            const ConstraintSet* not_dirty_constr = rVmMapper.GetVmConstraint(EVmConstraintType::NotDirty);
            if (not_dirty_constr != nullptr)
            {
              rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::NotDirty, not_dirty_constr));
            }
          }

          const ConstraintSet* not_accessed_constr = rVmMapper.GetVmConstraint(EVmConstraintType::NotAccessed);
          if (not_accessed_constr != nullptr)
          {
            rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::NotAccessed, not_accessed_constr));
          }

          const ConstraintSet* no_data_access_constr = rVmMapper.GetVmConstraint(EVmConstraintType::NoDataAccess);
          if (no_data_access_constr != nullptr)
          {
            rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::NoDataAccess, no_data_access_constr));
          }

          const ConstraintSet* pf_constr = rVmMapper.GetVmConstraint(EVmConstraintType::PageFault);
          if (pf_constr != nullptr)
          {
            rVmConstraints.push_back(new VmNotInConstraint(EVmConstraintType::PageFault, pf_constr));
          }
        }
        break;
      default:
        break;
    }
  }

}
