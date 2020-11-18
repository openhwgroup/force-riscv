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
#ifndef Force_VaGenerator_H
#define Force_VaGenerator_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <string>
#include <vector>

namespace Force {

  class VmMapper;
  class ConstraintSet;
  class GenPageRequest;
  class Operand;
  class VmConstraint;
  class AddressReuseMode;

  class VaGenerator {
  public:
    explicit VaGenerator(VmMapper* vmMapper, const GenPageRequest* pPageReq=nullptr, const ConstraintSet* pTargetConstr=nullptr, bool isOperand=true, const AddressReuseMode* pAddrReuseMode=nullptr); //!< Most used constructor.
    ~VaGenerator(); //!< Destructor.
    COPY_CONSTRUCTOR_ABSENT(VaGenerator);
    ASSIGNMENT_OPERATOR_ABSENT(VaGenerator);

    uint64 GenerateAddress(uint64 align, uint64 size, bool isInstr, EMemAccessType memAccess, const ConstraintSet *pRangeConstraint = nullptr); //!< Generate an address.
    void SetReachConstraint(const std::string& rCallerName, ConstraintSet* pOprConstr, bool isOperand = true ); //!< Set operand constraint.
    void SetAccurateBranch(uint32 brSize) { mAccurateBranch = true; mBranchSize = brSize; } //!< Set accurate branch state.
    inline bool NewPagesAdded() const { return mNewPagesAdded; } //!< Return whether new pages were added during the address generation.
    inline void ResetNewPagesAdded() const { mNewPagesAdded = false;} //!< Reset flag
    uint64 GenerateAddressWithRangeConstraintBaseValue(uint64 align, uint64 size, bool isInstr, EMemAccessType memAccess, const ConstraintSet *pRangeConstraint, const uint64 base_value);
    uint64 GenerateConstrainedAddressWithAilgnOffset(uint64 base_value);
  protected:
    VaGenerator(); //!< Default constructor.
    void ApplyPcConstraint(ConstraintSet* pConstr) const; //!< Apply PC constraint onto the passed in ConstraintSet object.
    void ApplyTargetConstraint(ConstraintSet* pConstr) const; //!< Merge target constraint.
    bool TargetAddressForced(uint64& rAddress) const; //!< Return true if target address is forced.
    uint64 GenerateConstrainedAddress(); //!< Generate constrained address.
    bool MapAddressRange(cuint64 addr) const; //!< Maps the virtual address range to a physical address range. Returns false if the mapped-to physical address range is unusable.
    void MergeAddressErrorConstraint(ConstraintSet* pConstr) const; //!< merge address error constraint that range constraint contained.
    void ApplyHardConstraints(ConstraintSet* pConstr) const; //!< Apply hard VM constraint.

  protected:
    VmMapper* mpVmMapper; //!< Pointer to the virtual memory mapper object.
    std::string mCallerName ; //!< Operand name to be use in error cases.
    const GenPageRequest* mpPageRequest; //!< Pointer to optional page request object.
    const ConstraintSet* mpTargetConstraint; //!< Target ConstraintSet if any.
    const ConstraintSet* mpRangeConstraint; //!< Range ConstraintSet if any.
    GenPageRequest* mpLocalPageRequest; //!< Local page request object.
    ConstraintSet* mpOperandConstraint; //!< Operand ConstraintSet if any.
    uint64 mAlignMask; //!< Alignment mask.
    uint64 mSize; //!< Mapping size.
    uint32 mAlignShift; //!< Alignment shift.
    uint32 mBranchSize; //!< Branch instruction size if applicable.
    bool mIsInstruction; //!< Indicate if this is generating for instruction usage.
    bool mIsOperand; //!< Indicate if this is generating for instruction usage.
    bool mAccurateBranch; //!< Indicate if this is generating for accurate branch.
    mutable bool mNewPagesAdded; //!< New pages added in the process.
    std::vector <VmConstraint* > mHardVmConstraints; //!< Hard VM constraints if any.
  private:
    AddressReuseMode* mpAddrReuseMode; //!< Address reuse configuration.
  };
}

#endif
