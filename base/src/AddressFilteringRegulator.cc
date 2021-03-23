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
#include <AddressFilteringRegulator.h>
#include <AddressSolutionFilter.h>
#include <AddressSolver.h>
#include <GenRequest.h>
#include <Log.h>

/*!
  \file AddressFilteringRegulator.cc
  \brief Code for managing which address filtering regulator to apply.
*/

using namespace std;

namespace Force {

  AddressFilteringRegulator::AddressFilteringRegulator()
    : Object(), mAddressSolutionFilters()
  {
  }

  AddressFilteringRegulator::AddressFilteringRegulator(const AddressFilteringRegulator& rOther)
    : Object(rOther), mAddressSolutionFilters()
  {
    for (auto addr_filter : rOther.mAddressSolutionFilters) {
      AddressSolutionFilter* clone_ptr = nullptr;
      if (nullptr != addr_filter) {
        clone_ptr = dynamic_cast<AddressSolutionFilter* >(addr_filter->Clone());
      }
      mAddressSolutionFilters.push_back(clone_ptr);
    }
  }

  AddressFilteringRegulator::~AddressFilteringRegulator()
  {
    for (auto addr_filter : mAddressSolutionFilters) {
      delete addr_filter;
    }
  }

  Object* AddressFilteringRegulator::Clone() const
  {
    return new AddressFilteringRegulator(*this);
  }

  const string AddressFilteringRegulator::ToString() const
  {
    return Type();
  }

  void AddressFilteringRegulator::Setup(const Generator* pGen)
  {
    for (auto as_ptr : mAddressSolutionFilters) {
      as_ptr->Setup(pGen);
    }
  }

  void AddressFilteringRegulator::GetAddressSolutionFilters(const AddressingMode& rAddrMode, std::vector<AddressSolutionFilter* >& rSolutionFilters) const
  {
    rSolutionFilters.push_back(mAddressSolutionFilters[uint32(EAddressSolutionFilterType::SpAlignment)]);
    rSolutionFilters.push_back(mAddressSolutionFilters[uint32(EAddressSolutionFilterType::BaseDependency)]);

    if (rAddrMode.ShouldApplyIndexFilters()) {
      rSolutionFilters.push_back(mAddressSolutionFilters[uint32(EAddressSolutionFilterType::IndexDependency)]);
    }
  }

  void AddressFilteringRegulator::GetVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints) const
  {
    bool is_instr = false;
    rPageReq.GetGenBoolAttribute(EPageGenBoolAttrType::InstrAddr, is_instr);

    if (is_instr) {
      GetInstrVmConstraints(rPageReq, rVmMapper, rVmConstraints);
    }
    else {
      GetDataVmConstraints(rPageReq, rVmMapper, rVmConstraints);
    }
  }

}
