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
#include <PteAttribute.h>
#include <PteStructure.h>
#include <Random.h>
#include <Page.h>
#include <GenRequest.h>
#include <Constraint.h>
#include <VmAddressSpace.h>
#include <PagingChoicesAdapter.h>
#include <Choices.h>
#include <UtilityFunctions.h>
#include <Log.h>

#include <memory>

using namespace std;

namespace Force {

  PteAttribute::PteAttribute()
    : Object(), mpStructure(nullptr), mValue(0)
  {

  }

  PteAttribute::PteAttribute(const PteAttribute& rOther)
    : Object(rOther), mpStructure(rOther.mpStructure), mValue(0)
  {

  }

  PteAttribute::~PteAttribute()
  {

  }

  Object* PteAttribute::Clone() const
  {
    return new PteAttribute(*this);
  }

  const string PteAttribute::ToString() const
  {
    return Type();
  }

  EPteAttributeType PteAttribute::PteAttributeType() const
  {
    return mpStructure->mType;
  }

  uint32 PteAttribute::Size() const
  {
    return mpStructure->mSize;
  }

  void PteAttribute::Initialize(const PteAttributeStructure* pPteAttrStruct)
  {
    mpStructure = pPteAttrStruct;
  }

  void PteAttribute::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    bool value_forced = false;
    auto pte_constr = ValueForced(rPagingReq, value_forced);
    if (value_forced)
      return;

    auto choices_adapter = rVmas.GetChoicesAdapter();
    if (nullptr != pte_constr) {
      auto choices_tree = choices_adapter->GetPagingChoiceTree(mpStructure->mTypeText);
      std::unique_ptr<Choice> choices_tree_storage(choices_tree);
      mValue = choices_tree->ChooseValueWithHardConstraint(*pte_constr);
    }
    else {
      mValue = choices_adapter->GetPagingChoice(mpStructure->mTypeText);
    }
    // << mpStructure->mTypeText << " gotten value 0x" << hex << mValue << endl;
  }

  const ConstraintSet* PteAttribute::ValueForced(const GenPageRequest& rPagingReq, bool &rValForced)
  {
    rValForced = false;
    auto pte_constr = rPagingReq.PteAttributeConstraint(PteAttributeType());
    if ((nullptr != pte_constr) and (pte_constr->Size() == 1)) {
      mValue = pte_constr->OnlyValue();
      rValForced = true;
      // << "PTE " << EPteAttributeType_to_string(PteAttributeType()) << " constraint resulted forced to value 0x" << hex << mValue << endl;
    }
    return pte_constr;
  }

  uint64 PteAttribute::Encoding() const
  {
    return mpStructure->Encoding(mValue);
  }

  RandomPteAttribute::RandomPteAttribute()
    : PteAttribute()
  {

  }

  RandomPteAttribute::RandomPteAttribute(const RandomPteAttribute& rOther)
    : PteAttribute(rOther)
  {

  }

  RandomPteAttribute::~RandomPteAttribute()
  {

  }

  Object* RandomPteAttribute::Clone() const
  {
    return new RandomPteAttribute(*this);
  }

  void RandomPteAttribute::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    mValue = Random::Instance()->Random64() & mpStructure->Mask();
  }

  ValuePteAttribute::ValuePteAttribute()
    : PteAttribute()
  {

  }

  ValuePteAttribute::ValuePteAttribute(const ValuePteAttribute& rOther)
    : PteAttribute(rOther)
  {

  }

  ValuePteAttribute::~ValuePteAttribute()
  {

  }

  Object* ValuePteAttribute::Clone() const
  {
    return new ValuePteAttribute(*this);
  }

  void ValuePteAttribute::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    mValue = mpStructure->Value();
  }

  AddressPteAttribute::AddressPteAttribute()
    : PteAttribute()
  {

  }

  AddressPteAttribute::AddressPteAttribute(const AddressPteAttribute& rOther)
    : PteAttribute(rOther)
  {

  }

  AddressPteAttribute::~AddressPteAttribute()
  {

  }

  Object* AddressPteAttribute::Clone() const
  {
    return new AddressPteAttribute(*this);
  }

  void AddressPteAttribute::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    mValue = (rPte.PhysicalLower() >> mpStructure->Lsb()) & mpStructure->Mask();
    // << "generating address attribte PTE PA 0x" << hex << rPte.PhysicalLower() << " LSB " << dec << mpStructure->Lsb() << " mask 0x" << hex << mpStructure->Mask() << " resulting value 0x" << mValue << endl;
  }

  ConstraintPteAttribute::ConstraintPteAttribute()
    : PteAttribute()
  {

  }

  ConstraintPteAttribute::ConstraintPteAttribute(const ConstraintPteAttribute& rOther)
    : PteAttribute(rOther)
  {

  }

  ConstraintPteAttribute::~ConstraintPteAttribute()
  {

  }

  void ConstraintPteAttribute::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    bool value_forced = false;
    auto user_constr = ValueForced(rPagingReq, value_forced);

    //Page* page_cast = dynamic_cast<Page*> (&rPte); // DEBUG
    //if (nullptr != page_cast) { // DEBUG
    //<< "Constrained PTE generateing for 0x" << hex << page_cast->Lower() << " value forced? " << value_forced << " value " << mValue << " type: " << Type() << endl; // DEBUG

    if (value_forced)
      return SetPteGenAttribute(rPagingReq, rPte);

    auto choices_adapter = rVmas.GetChoicesAdapter();
    auto choices_tree = choices_adapter->GetPagingChoiceTree(mpStructure->TypeText());
    std::unique_ptr<Choice> choices_tree_storage(choices_tree);

    ConstraintSet value_constr;
    bool hard_constr = GetValueConstraint(rPagingReq, value_constr);

    if (nullptr != user_constr) {
      // when user constraint present.
      value_constr.ApplyConstraintSet(*user_constr);
      uint32 constr_size = value_constr.Size();
      switch (constr_size) {
      case 0:
        mValue = choices_tree->ChooseValueWithHardConstraint(*user_constr);
        break;
      case 1:
        mValue = value_constr.OnlyValue();
        break;
      default:
        mValue = choices_tree->ChooseValueWithHardConstraint(value_constr);
      }
      // << "\"" << choices_adapter->PagingChoicesName(mpStructure->TypeText()) << "\" with user constraint: " << user_constr->ToSimpleString() << " gotten value: 0x" << hex << mValue << " combined constraint: " << value_constr.ToSimpleString() << endl;
      return SetPteGenAttribute(rPagingReq, rPte); // done with this block and return
    }

    bool has_choice = false;
    uint64 fall_back_value = 0;
    mValue = choices_tree->ChooseValueWithConstraint(value_constr, has_choice, fall_back_value);
    if (not has_choice) {
      if (hard_constr) {
        LOG(fail) << "{ConstraintPteAttribute::Generate} no choice available with hard value constraint: " << value_constr.ToSimpleString() << endl;
        FAIL("no-choice-with-hard-constraint");
      }
      else {
        // no choice available, if not solution with hard value constr, ignore the value constr and use the fall back value.
        mValue = fall_back_value;
      }
    }
    // << "\"" << choices_adapter->PagingChoicesName(mpStructure->TypeText()) << "\" gotten value: 0x" << hex << mValue << " has choice? " << has_choice << " fall back: 0x" << fall_back_value << " value constraint: " << value_constr.ToSimpleString() << endl;
    SetPteGenAttribute(rPagingReq, rPte);
  }

  void ConstraintPteAttribute::GetDefaultConstraint(ConstraintSet& rValueConstr) const
  {
    uint64 max_value =  get_mask64(Size());
    rValueConstr.AddRange(0, max_value);
  }

  ExceptionConstraintPteAttribute::ExceptionConstraintPteAttribute()
    : ConstraintPteAttribute()
  {

  }

  ExceptionConstraintPteAttribute::~ExceptionConstraintPteAttribute()
  {

  }

  ExceptionConstraintPteAttribute::ExceptionConstraintPteAttribute(const ExceptionConstraintPteAttribute& rOther)
    : ConstraintPteAttribute(rOther)
  {

  }

  bool ExceptionConstraintPteAttribute::GetValueConstraint(const GenPageRequest& rPagingReq, ConstraintSet& rExceptConstr) const
  {
    bool hard_constr = false;
    EExceptionConstraintType except_constr_type = rPagingReq.GetExceptionConstraint(GetExceptionType(rPagingReq));
    LOG(info) << "{ExceptionConstraintPteAttribute::GetValueConstraint} exception type: " << EPagingExceptionType_to_string(GetExceptionType(rPagingReq)) << " constraint type: " << EExceptionConstraintType_to_string(except_constr_type) << endl;
    switch (except_constr_type) {
    case EExceptionConstraintType::TriggerHard:
      hard_constr = true; // fall through
    case EExceptionConstraintType::Trigger:
      ExceptionTriggeringConstraint(rPagingReq, rExceptConstr);
      break;
    case EExceptionConstraintType::PreventHard:
      hard_constr = true; // fall through
    case EExceptionConstraintType::Prevent:
      ExceptionPreventingConstraint(rPagingReq, rExceptConstr);
      break;
    case EExceptionConstraintType::Allow:
      GetDefaultConstraint(rExceptConstr);
      break;
    default:
      LOG(fail) << "{ExceptionConstraintPteAttribute::GetValueConstraint} unexpected exception constraint type: " << EExceptionConstraintType_to_string(except_constr_type) << endl;
      FAIL("unexpected-exception-constraint-type");
    }
    return hard_constr;
  }

}
