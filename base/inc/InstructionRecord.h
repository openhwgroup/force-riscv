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
#ifndef Force_InstructionRecord_H
#define Force_InstructionRecord_H

#include <map>
#include <string>

namespace Force {

   using namespace std;

   class InstructionResults;

  /*!
    \class InstructionRecord
    \brief Class to hold an instruction record
  */

  class InstructionRecord {
  public:
    InstructionRecord() { }
    virtual ~InstructionRecord() { } //!< Virtual destructor to ensure orderly destruction of derived classes.

    const string& Name() const { return mName; }       //!< Return instruction full-name
    const uint64 Opcode() const { return mOpcode; }         //!< Return instruction Opcode
    const uint64 VA() const { return mVA; }                 //!< return instruction virtual address
    const uint64 PA() const { return mPA; }                 //!< Return instruction physical address
    const uint64 IPA() const { return mIPA; }               //!< Return instruction IPA
    const uint64 LSTarget() const { return mLSTarget; }     //!< Return load store instruction target
    const uint64 BRTarget() const { return mBRTarget; }     //!< Return branch isntruction target
    const string& Group() const { return mGroup; }                         //!< Return instruction group name
    const map<string, uint32>& Dests() const { return mDests; }           //!< Return instruction dest register list
    const map<string, uint32>& Srcs() const { return mSrcs; }             //!< Return instruction src register list
    const map<string, uint32>& Imms() const { return mImms; }             //!< Return instruction immediate list
    const map<string, string>& AddressingName() const { return mAddressingName; }
    //!< Return addressing operand name list
    const map<string, uint32>& AddressingIndex() const { return mAddressingIndex; }
    //!< Return addressing operand index value list
    const map<string, uint32>& Status() const { return mStatus; }     //!< Return instruction operand (load store operand) status information

  //protected:
    void SetName (const string& name) { mName = name; }                   //!< Set instruction full-name
    void SetOpcode (uint64 opcode) { mOpcode = opcode; }                  //!< Set instruction Opcode
    void SetVA(uint64 va) { mVA = va; }                                   //!< Set instruction virtual address
    void SetPA(uint64 pa) { mPA = pa; }                                   //!< Set instruction physical address
    void SetIPA(uint64 ipa) { mIPA = ipa; }                               //!< Set instruction IPA
    void SetLSTarget(uint64 lsTarget) { mLSTarget = lsTarget; }           //!< Set load store instruction target
    void SetBRTarget(uint64 brTarget) { mBRTarget = brTarget; }           //!< Set branch instruction target
    void SetGroup(const string& group) { mGroup = group; }                //!< Set instruction group name
    void AddDests(const string& name, uint32 value) { mDests[name] = value; }
    void AddSrcs(const string& name, uint32 value) { mSrcs[name] = value; }
    void AddImms(const string& name, uint32 value) { mImms[name] = value; }
    void AddAddressingName (const string& key, const string& name)
    {
        mAddressingName[key] = name;
    }
    void AddAddressingIndex (const string& key, uint32 val)
    {
        mAddressingIndex[key] = val;
    }
    void AddStatus (const string& name, uint32 value)
    {
        mStatus[name] = value;
    }

    void Clear()        //!< Clear instruction record
    {
        mName = string();
        mOpcode = 0;
        mVA = 0;
        mPA = 0;
        mIPA = 0;
        mLSTarget = 0;
        mBRTarget = 0;
        mGroup = string();
        mDests.clear();
        mSrcs.clear();
        mImms.clear();
        mAddressingName.clear();
        mAddressingIndex.clear();
        mStatus.clear();
    }
  private:
    InstructionRecord(const InstructionRecord& rOther) { }

    string mName;       //!< instruction full-name
    uint64 mOpcode;     //!< instruction Opcode
    uint64 mVA;         //!< instruction virtual address
    uint64 mPA;         //!< instruction physical address
    uint64 mIPA;        //!< instruction IPA, if ncessary, otherwise, it is 0
    uint64 mLSTarget;   //!< load store instruction target, otherwise, it is 0
    uint64 mBRTarget;   //!< branch instruction target, otherwise, it is 0
    string mGroup;      //!< instruction group name
    map<string, uint32> mDests;     //!< instruction desternation register list
    map<string, uint32> mSrcs;      //!< instruction source register list
    map<string, uint32> mImms;      //!< instruction immediate operand list
    map<string, string> mAddressingName;    //!< addressing operand list (name)
    map<string, uint32> mAddressingIndex;   //!< coresponding operand index
    map<string, uint32> mStatus;
  public:
    //friend class InstructionResults;
  };

}

#endif
