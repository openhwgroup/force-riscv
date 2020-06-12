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
#include <InstructionStructure.h>
#include <UtilityFunctions.h>
#include <AsmText.h>
#include <Constraint.h>
#include <Log.h>

#include <sstream>

/*!
  \file InstructionStructure.cc
  \brief File for InstructionStructure and OperandStructure and its derived classes.

*/

using namespace std;

namespace Force {

  /*!
    \class InstructionStructure

    An InstructionStructure object directly correspond to an \<I\> element in Force instruction XML files.
    The static information contained in an InstructionStructure instance can be shared among all Instruction objects that was instantiated from the same InstructionStructure object.
    Each InstructionStructure object is meant to be unique and should not be copied around.
    This data structure is meant to be accessed by Instruction class and instruction file parsing code, therefore some of the member attributes are made public.
  */

  InstructionStructure::~InstructionStructure()
  {
    for (auto opr_struct : mOperandStructures) {
      delete opr_struct;
    }

    delete mpAsmText;
  }

  /*!
    Instruction full is made up of name#form#isa.
   */
  const string InstructionStructure::FullName() const
  {
    return mName + "#" + mForm + "#" + mIsa;
  }

  void InstructionStructure::AddOperand(OperandStructure* opr_struct)
  {
    mOperandStructures.push_back(opr_struct);
    mSize += opr_struct->mSize;

    opr_struct->AddShortOperand(mShortOperandStructures);
  }

  const string InstructionStructure::ToString() const
  {
      stringstream out_str;

      out_str << "Instruction full name: " << FullName() << endl;
      for (auto oper_struct: mOperandStructures)
      {
           out_str << oper_struct->ToString() << endl;

      }

      return string(out_str.str());
  }

  void OperandStructure::SetBits(const string& bits_str)
  {
    FieldEncoding encoding_obj(&mEncodingBits);
    encoding_obj.SetBits(bits_str);
    mSize = encoding_obj.Size();
    mMask = (1 << mSize) - 1;
  }

  uint32 OperandStructure::Encoding(uint32 opr_value) const
  {
    return get_field_encoding<uint32>(mEncodingBits, opr_value);
  }

  void OperandStructure::AddShortOperand(std::map<const std::string, const OperandStructure* >& rShortStructures) const
  {
    string key = ShortName();
    if (key.empty()) {
      LOG(debug) << "No short key for the operand \"" << Name() << "\", use long key:" << Name() << endl;
      key = Name();
    }

    if (rShortStructures.find(key) != rShortStructures.end()) {
      LOG(debug) << "duplicate operand mapping: " << Name() << " -->" << key << ", use long name." << endl;
      key = Name();
    }
    rShortStructures[key] = this;

    return;
  }

  void OperandStructure::FailedTocast() const
  {
    LOG(fail) << "{OperandStructure::FailedTocast} failed to cast OperandStructure object: " << mName << endl;
    FAIL("failed-to-cast-operand-structure");
  }

  const string OperandStructure::ToString() const
  {
    stringstream out_str;

    out_str << "Name: " << mName << ", Class: " << mClass << ", Type: " << EOperandType_to_string(mType) << ", Access: " << ERegAttrType_to_string(mAccess) << ", Size: " << dec << mSize << ", Mask: 0x" << hex << mMask;

    return out_str.str();
  }

  ExcludeOperandStructure::~ExcludeOperandStructure()
  {
    delete mpExcludeConstraint;
  }

  const ConstraintSet* ExcludeOperandStructure::ExcludeConstraint() const
  {
    if (nullptr == mpExcludeConstraint) {
      mpExcludeConstraint = new ConstraintSet(0, mMask);
      ConstraintSet excludes(mExclude);
      mpExcludeConstraint->SubConstraintSet(excludes);
      // << "operand " << mName << " mask 0x" << hex << mMask << " constraint: " << mpExcludeConstraint->ToSimpleString() << endl;
    }
    return mpExcludeConstraint;
  }

  GroupOperandStructure::~GroupOperandStructure()
  {
    for (auto opr_struct : mOperandStructures) {
      delete opr_struct;
    }
  }

  void GroupOperandStructure::AddOperand(OperandStructure* opr_struct)
  {
    mOperandStructures.push_back(opr_struct);
    mSize += opr_struct->mSize;
  }

  void GroupOperandStructure::AddShortOperand(std::map<const std::string, const OperandStructure* >& rShortStructures) const
  {
    for (auto opr_struct : mOperandStructures)
      opr_struct->AddShortOperand(rShortStructures);
  }

  void LoadStoreOperandStructure::SetElementSize(uint32 eSize) const
  {
    mElementSize = eSize;
    if (mDataSize == 0) {
      mDataSize = mElementSize;
    }
    if (mAlignment == 0) {
      mAlignment = mElementSize;
    }
  }

  // TODO this need improvement
  // * Improve performance.
  bool LoadStoreOperandStructure::AtomicOrderedAccess() const
  {
    string load_type = EMemOrderingType_to_string(mMemLoadType);
    if (load_type.find("Atomic") !=string::npos || load_type.find("Ordered") != string::npos)
      return true;

    string store_type = EMemOrderingType_to_string(mMemStoreType);
    if (store_type.find("Atomic") !=string::npos || store_type.find("Ordered") != string::npos)
      return true;

    return false;
  }

  bool LoadStoreOperandStructure::SpBased() const
  {
    return mBase == "Xn|SP";
  }

  void DataProcessingOperandStructure::AddOperandRole(const string& oprName, const string& roleName)
  {
    mOperandRoles.emplace(oprName, roleName);
  }

  string DataProcessingOperandStructure::GetOperandRole(const std::string& oprName) const
  {
    string roleName;
    auto itr = mOperandRoles.find(oprName);
    if (itr != mOperandRoles.end()) {
      roleName = itr->second;
    }

    return roleName;
  }

}
