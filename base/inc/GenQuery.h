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
#ifndef Force_GenQuery_H
#define Force_GenQuery_H

#include <map>
#include <string>
#include <utility>

#include "Defines.h"
#include "Enums.h"
#include "ExceptionRecords.h"
#include "PageInfoRecord.h"
#include ARCH_ENUM_HEADER

/*!
  Declare pybind11::object type without actual definitions.
  To avoid spreading pybind11 classes to other backend modules unnecessarily.
*/
namespace pybind11 {
  class object; //!< Base pybind11 Python object class.
}
namespace py = pybind11;

namespace Force {

  class ConstraintSet;

  /*!
    \class GenQuery
    \brief Base class for all query objects to test generator thread.
   */
  class GenQuery {
  public:
    explicit GenQuery(EQueryType queryType) : mType(queryType), mPrimaryString() { }
    virtual ~GenQuery() { } //!< Destructor.

    EGenAgentType GenAgentType() const { return EGenAgentType::GenQueryAgent; } //!< Return associated GenAgent type.
    EQueryType QueryType() const { return mType; } //!< Return the query enum type.
    void SetPrimaryString(const std::string& primaryStr) { mPrimaryString = primaryStr; } //!< Set primary string, the main clue.
    const std::string& PrimaryString() const { return mPrimaryString; } //!< Return the primary string for the query.
    virtual void AddDetail(const std::string& attrName, uint64 value) {} //!< Add query detail, with integer value parameter.
    virtual void AddDetail(const std::string& attrName, const std::string& valueStr) {} //!< Add query detail, with value string parameter.
    virtual std::string ToString() const; //!< Return details of the GenQuery object.
    virtual void GetResults(py::object& rPyObject) const = 0; //!< Return query results in the passed in rPyObject.

    /*!
      Templated function so that a derived class can conveniently cast base class to the desired derived class type.
      For example, GenLoadRegister* load_reg_req = gen_req->CastInstance<GenLoadRegister>();
    */
    template<typename T>
      const T* ConstCastInstance() const
      {
        const T* cast_instance = dynamic_cast<const T* >(this);
        return cast_instance;
      }

    static GenQuery* GenQueryInstance(const std::string& queryName); //!< Return a GenQuery instance based on the queryName given.
  protected:
    void UnsupportedQueryDetailAttribute(const std::string& attrName) const; //!< Called to report error on unsupported query detail attribute.
    void UnsupportedQueryDetailValueType(const std::string& attrName, const std::string& valueType) const; //!< Called to report error on unsupported query detail value type.
  private:
    GenQuery() : mType(EQueryType::RegisterIndex), mPrimaryString() { } //!< Default constructor, not meant to be called.
    GenQuery(const GenQuery& rOther) : mType(EQueryType::RegisterIndex), mPrimaryString() { } //!< Copy constructor, not meant to be called.
  protected:
    EQueryType mType; //!< Query type enum.
    std::string mPrimaryString; //!< The primary info string for the query.
  };

  /*!
    \class GenRegisterIndexQuery
    \brief Query returns register's index from its name.
  */
  class GenRegisterIndexQuery : public GenQuery {
  public:
    explicit GenRegisterIndexQuery(EQueryType queryType) : GenQuery(queryType), mIndex(0) { } //!< Default constructor.
    void SetIndex(uint32 index) const { mIndex = index; } //!< Set return index.
    void GetResults(py::object& rPyObject) const override; //!< Return RegisterIndex query results in the passed in rPyObject.
  private:
    mutable uint32 mIndex; //!< Return the register index in this parameters.
  };

  /*!
   \class GenRegisterReloadValueQuery
   \brief Query takes user's input of custom field value constraints and return   a valid register's reload value based on internal and custom constraints.
  */
  class GenRegisterReloadValueQuery : public GenQuery {
  public:
    explicit GenRegisterReloadValueQuery(EQueryType queryType): GenQuery(queryType), mFieldConstraintMap(), mReloadValue(0) {} // !< Default constructor.
    ~GenRegisterReloadValueQuery ();  // !< Destructor.
    void SetReloadValue(const uint64 value) const {mReloadValue = value; } //!< Set return value.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add query detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add query detail, with value string parameter.
    const std::map<std::string, ConstraintSet*> FieldConstraintMap() const {return mFieldConstraintMap; } //!< Getter for mFieldConstraintMap.

    void GetResults(py::object& rPyObject) const override; //!< Return register reload value in result.
  private:
    void AddFieldConstraint(const std::string& fieldName, ConstraintSet* pConstr);
  private:
    std::map<std::string, ConstraintSet*> mFieldConstraintMap;
    mutable uint64 mReloadValue;
  };

  /*!
   \class GenRegisterFieldInfoQuery
   \brief Query takes user's input of custom field value constraints and return the register mask with field list, register value changed from field value constraints, and if register field(s) impact vm
  */
  class GenRegisterFieldInfoQuery : public GenQuery {
  public:
    explicit GenRegisterFieldInfoQuery(EQueryType queryType): GenQuery(queryType), mFieldConstraintMap(), mRegisterMask(0), mFieldsValue(0) {} // !< Default constructor.
    ~GenRegisterFieldInfoQuery ();  // !< Destructor.
    void SetRegisterFieldInfo(const uint64 mask, const uint64 value) const //!< Set return value.
    { mRegisterMask = mask; mFieldsValue = value; }
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add query detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add query detail, with value string parameter.
    const std::map<std::string, ConstraintSet*> FieldConstraintMap() const {return mFieldConstraintMap; } //!< Getter for mFieldConstraintMap.

    void GetResults(py::object& rPyObject) const override; //!< Return register reload value in result.
  private:
    void AddFieldConstraint(const std::string& fieldName, ConstraintSet* pConstr);
  private:
    std::map<std::string, ConstraintSet*> mFieldConstraintMap;
    mutable uint64 mRegisterMask;
    mutable uint64 mFieldsValue;
  };


  /*!
    \class GenRegisterInfoQuery
    \brief Query register info with given name and index
  */
  class GenRegisterInfoQuery : public GenQuery {
  public:
    explicit GenRegisterInfoQuery(EQueryType queryType) : GenQuery(queryType), mIndex(0), mValue(0), mValues(), mWidth(0), mValueValid(0), mRegisterType() { clear(); } //!< Default constructor.
    void GetResults(py::object& rPyObject) const override; //!< Return RegisterInfo
    void AddDetail(const std::string& attrName, uint64 value) override { mIndex = value; } //!< Add query detail, with integer value parameter.
    uint64 Index() const { return mIndex; }
    void SetRegisterType   (const std::string& type)    const { mRegisterType = type; }
    void SetRegisterWidth  (uint32 width)               const { mWidth = width; }
    void SetRegisterValue  (uint64 value)               const { mValue = value;  mValueValid = true; }
    void SetRegisterValues (const std::vector<uint64>& values) const { mValues = values; mValueValid = true; }
  private:
    void clear()
    {
      mValue = 0;
      mValues.clear();
      mWidth = 0;
      mValueValid = false;
      mRegisterType = std::string();
    }

    uint64 mIndex;
    mutable uint64 mValue;
    mutable std::vector<uint64> mValues;
    mutable uint32 mWidth;
    mutable bool mValueValid;
    mutable std::string mRegisterType;
  };

  /*!
    \class GenInstructionRecordQuery
    \brief Query instruction record with given record id
  */
  class GenInstructionRecordQuery : public GenQuery {
  public:
    explicit GenInstructionRecordQuery(EQueryType queryType) : GenQuery(queryType), mName(), mOpcode(0), mPA(0), mBank(0), mVA(), mIPA(), mLSTarget(), mBRTarget(), mGroup(), mDests(), mSrcs(), mImms(), mAddressingName(), mAddressingIndex(), mStatus(), mValid(true) { clear(); } //!< Default constructor.
    void GetResults(py::object& rPyObject) const override; //!< Return RegisterIndex query results in the passed in rPyObject.

    const std::string& Name() const { return mName; }       //!< Return instruction full-name
    const uint64 Opcode() const { return mOpcode; }         //!< Return instruction Opcode
    const uint64 PA() const { return mPA; }                 //!< Return instruction physical address
    const std::pair<bool, uint64>& VA()  const { return mVA; }  //!< return instruction virtual address
    const std::pair<bool, uint64>& IPA() const { return mIPA; } //!< Return instruction IPA
    const std::pair<bool, uint64>& LSTarget() const { return mLSTarget; } //!< Return load store instruction target
    const std::pair<bool, uint64>& BRTarget() const { return mBRTarget; } //!< Return branch isntruction target
    const std::string& Group() const { return mGroup; }                   //!< Return instruction group name
    const std::map<std::string, uint32>& Dests() const { return mDests; } //!< Return instruction dest register list
    const std::map<std::string, uint32>& Srcs() const { return mSrcs; }   //!< Return instruction src register list
    const std::map<std::string, uint32>& Imms() const { return mImms; }   //!< Return instruction immediate list
    const std::map<std::string, std::string>& AddressingName() const { return mAddressingName; }
    //!< Return addressing operand name list
    const std::map<std::string, uint32>& AddressingIndex() const { return mAddressingIndex; }
    //!< Return addressing operand index value list
    const std::map<std::string, uint32>& Status() const { return mStatus; }     //!< Return instruction operand (load store operand) status information
    const uint32 Bank() const { return mBank; }  //!< Return instruction record memory bank id

    void SetName (const std::string& name) const { mName = name; }     //!< Set instruction full-name
    void SetOpcode (uint64 opcode) const { mOpcode = opcode; }         //!< Set instruction Opcode
    void SetPA(uint64 pa) const { mPA = pa; }                          //!< Set instruction physical address
    void SetVA(uint64 va) const { mVA.first = true; mVA.second = va; } //!< Set instruction virtual address
    void SetIPA(uint64 ipa) const { mIPA.first = true; mIPA.second = ipa; } //!< Set instruction IPA
    void SetLSTarget(uint64 lsTarget) const { mLSTarget.first = true; mLSTarget.second = lsTarget; } //!< Set load store instruction target
    void SetBRTarget(uint64 brTarget) const { mBRTarget.first = true;  mBRTarget.second = brTarget; } //!< Set branch instruction target
    void SetGroup(const std::string& group) const { mGroup = group; } //!< Set instruction group name
    void AddDests(const std::string& name, uint32 value) const { mDests[name] = value; }
    void AddSrcs(const std::string& name, uint32 value) const { mSrcs[name] = value; }
    void AddImms(const std::string& name, uint32 value) const { mImms[name] = value; }
    void AddAddressingName (const std::string& key, const std::string& name) const
    {
        mAddressingName[key] = name;
    }
    void AddAddressingIndex (const std::string& key, uint32 val) const
    {
        mAddressingIndex[key] = val;
    }
    void AddStatus (const std::string& name, uint32 value) const
    {
        mStatus[name] = value;
    }
    void SetBank(uint32 bank) const { mBank = bank; }
    inline void SetValid(bool valid) const { mValid = valid; } //!< the accessor for mValid
    bool IsValid() const { return mValid; }
  private:
    void clear()        //!< clear instruction record
    {
        mName = std::string();
        mOpcode = 0;
        mPA = 0;
        mBank = 0;
        mVA.first = false;
        mIPA.first = false;
        mLSTarget.first = false;
        mBRTarget.first = false;
        mGroup = std::string();
        mDests.clear();
        mSrcs.clear();
        mImms.clear();
        mAddressingName.clear();
        mAddressingIndex.clear();
        mStatus.clear();
        mValid = true;
    }
    mutable std::string mName;       //!< instruction full-name
    mutable uint64 mOpcode;     //!< instruction Opcode
    mutable uint64 mPA;         //!< instruction physical address
    mutable uint32 mBank;       //!< instruction memory bank
    mutable std::pair<bool, uint64> mVA;         //!< virtual address, bool indicates valid VA or not
    mutable std::pair<bool, uint64> mIPA;        //!< intermediate physical address, bool indicates valid IPA or not
    mutable std::pair<bool, uint64> mLSTarget;   //!< load store instruction target pair. The first element shows the target is valid or not.
    mutable std::pair<bool, uint64> mBRTarget;   //!< branch instruction target pair. The first element shows the target is valid or not
    mutable std::string mGroup;      //!< instruction group name
    mutable std::map<std::string, uint32> mDests;     //!< instruction desternation register list
    mutable std::map<std::string, uint32> mSrcs;      //!< instruction source register list
    mutable std::map<std::string, uint32> mImms;      //!< instruction immediate operand list
    mutable std::map<std::string, std::string> mAddressingName;    //!< addressing operand list (name)
    mutable std::map<std::string, uint32> mAddressingIndex;   //!< coresponding operand index
    mutable std::map<std::string, uint32> mStatus;
    mutable bool mValid; //!< whether the record info is valid or not
  };

  /*!
    \class GenStateQuery
    \breif A query to the test generator thread for a certain generator state.
  */
  class GenStateQuery : public GenQuery {
  public:
    explicit GenStateQuery(EQueryType queryType) : GenQuery(queryType), mIsValue(false), mValue(0), mString() { }
    const std::string& StringVariable() const { return mString; } //!< Return string variable.
    uint64 ValueVariable() const { return mValue; } //!< Return value variable.
    void SetValue(uint64 value) const; //!< Set query value result.
    bool IsValue() const { return mIsValue; } //!< Return whether the variable is in value form.
    void GetResults(py::object& rPyObject) const override; //!< Return query results in the passed in rPyObject.
  protected:
    mutable bool mIsValue; //!< Indicate the state is a value type.
    mutable uint64 mValue; //!< State value if applicable.
    mutable std::string mString; //!< State string if applicable.
  };

  /*!
    \class GenUtilityQuery
    \brief A query to a particular utility function.
   */
  class GenUtilityQuery : public GenQuery {

  public:
    explicit GenUtilityQuery(EQueryType queryType) : GenQuery(queryType), mIntArgs(), mStrArgs(), mString(), mList() { }
    const std::string& StringVariable() const { return mString; } //!< Return utility function to call.
    const std::vector<uint64>& ListVariable() const { return mList; } //!< Return the array containing the return values from the query
    const uint64 ValueVariable (uint32 argNum) const { return (mIntArgs[argNum-1]); }
    const bool BoolValueVariable (uint32 argNum) const { return (mIntArgs[argNum-1] == 1);  }
    const std::string& StringVariable(uint32 argNum) const { return mStrArgs[argNum-1]; }
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add query detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add query detail, with string value parameter
    void SetString(const std::string& valueStr) const { mString.assign(valueStr); } //!< Set the utility function to call
    void AddValue(uint64 value) const { mList.push_back(value); }
    void GetResults(py::object& rPyObject) const override; //!< Return query results in the passed in rPyObject.
  protected:
    std::vector<uint64> mIntArgs;
    std::vector<std::string> mStrArgs;

    mutable std::string mString;
    mutable std::vector<uint64> mList;
  };

  /*!
    \class GenPageInfoQuery
    \breif A query to page information with given address and address type (VA, PA or IPA)
  */
  class GenPageInfoQuery : public GenQuery {
  public:
    explicit GenPageInfoQuery(EQueryType queryType) : GenQuery(queryType), mPageInformation(), mAddr(0), mPageType(), mBank(0), mValid(false) { }
    void GetResults(py::object& rPyObject) const override; //!< Return query results in the passed in rPyObject.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add query detail, with value string parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add query detail, with value string parameter.
    const uint64 Addr() const { return mAddr; }
    const std::string& Type() const { return mPageType; }
    const uint32 Bank() const { return mBank; }
    void Copy (const PageInformation& pageInformation) const;
    void SetValid() const { mValid = true; }
  private:
    mutable PageInformation mPageInformation;
    uint64 mAddr;
    std::string mPageType;
    uint32 mBank;
    mutable bool mValid;
  };

  /*!
    \class GenBranchOffsetQuery
    \brief A query to check if branch target can be reached with the given parameters.
  */
  class GenBranchOffsetQuery : public GenQuery {
  public:
    explicit GenBranchOffsetQuery(EQueryType queryType); //!< Constructor with query type parameter given.
    void GetResults(py::object& rPyObject) const override; //!< Return query results in the passed in rPyObject.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add query detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add query detail, with value string parameter.
    uint64 BranchAddress() const { return mBranchAddress; } //!< Return branch address.
    uint64 TargetAddress() const { return mTargetAddress; } //!< Return target address.
    uint32 OffsetSize() const { return mOffsetSize; } //!< Return the offset size.
    uint32 Shift() const { return mShift; } //!< Return the shift value.
    void SetValid(bool valid) const { mValid = valid; } //!< Set the valid state.
    void SetOffset(uint32 offset) const { mOffset = offset; } //!< Set the offset value.
    void SetHalfWords(uint32 num) const { mHalfWords = num; } //!< Set number of half words.
  private:
    uint64 mBranchAddress; //!< Address of branch instruction.
    uint64 mTargetAddress; //!< Target address.
    uint32 mOffsetSize; //!< Branch offset size.
    uint32 mShift; //!< Number of shifts.
    mutable uint32 mOffset; //!< Branch offset value.
    mutable uint32 mHalfWords; //!< Number of half words needed to load the target address.
    mutable bool mValid; //!< Indicates whether the result is valid, i.e. the target can be reached with a PC relative branch.
  };

   /*!
    \class GenChoicesTreeInfoQuery
    \breif A query to choices tree information with given tree name and tree type
  */
  class GenChoicesTreeInfoQuery : public GenQuery {
  public:
    explicit GenChoicesTreeInfoQuery(EQueryType queryType) : GenQuery(queryType), mChoicesInfo(), mChoicesTreeType() { }
    void GetResults(py::object& rPyObject) const override; //!< Return query results in the passed in rPyObject.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override { mChoicesTreeType = valueStr; } //!< Add query detail, with value string parameter.
    const std::string& ChoicesType() const { return mChoicesTreeType; }
    void AddChoiceInfo(const std::string& choiceName, uint32 weight) const; //!< add one choice info
  private:
    mutable std::map<std::string, uint32> mChoicesInfo; //!< containter for choice info
    std::string mChoicesTreeType; //!< choices tree type
  };

  /*!
    \class GenExceptionHistoryQuery
    \brief A query to a particular utility function.
   */
  class GenExceptionsHistoryQuery : public GenQuery {

  public:
    explicit GenExceptionsHistoryQuery(EQueryType queryType) : GenQuery(queryType), mInputArgs(), mErrorHistory(0) { }
    void SetErrorRecords(const std::vector<ExceptionRecord> &records) const { mErrorHistory = records; } //!< Set the number of times this exception was seen.
    void GetResults(py::object& rPyObject) const override; //!< Return query results in the passed in rPyObject.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add query detail, with string value parameter.
    const std::map<std::string, std::string> & GetInputArgs() const { return mInputArgs;} //!< return variable value of argName
    bool FindInputArgs(const std::string& argName) const { return mInputArgs.find(argName) != mInputArgs.end();}  //!< find if argName exists
  protected:
    std::map<std::string, std::string> mInputArgs;
    mutable std::vector<ExceptionRecord> mErrorHistory; //!<Returned number of exception events that match the provided error code.
  };

  /*!
    \class GetVmContextDeltaQuery
    \brief A query to a particular utility function.
   */
  class GetVmContextDeltaQuery : public GenQuery {

  public:
    explicit GetVmContextDeltaQuery(EQueryType queryType) : GenQuery(queryType), mInputArgs(), mDeltaMap() { }
    void GetResults(py::object& rPyObject) const override; //!< Return query results in the passed in rPyObject.
    void AddDetail(const std::string& attrName, const uint64 value) override; //!< Add query detail, with integer value parameter.
    const std::map<std::string, uint64> & GetInputArgs() const { return mInputArgs;} //!< return variable value of argName
    std::map<std::string, uint64> & GetDeltaMap() const {return mDeltaMap;} //!< return reference of mDeltaMap
    bool FindInputArgs(const std::string& argName) const { return mInputArgs.find(argName) != mInputArgs.end();}  //!< find if argName exists
  protected:
    std::map<std::string, uint64> mInputArgs;
    mutable std::map<std::string, uint64> mDeltaMap;
  };

  /*!
    \class VmCurrentContextQuery
    \brief A query to get the current VM context
   */
  class VmCurrentContextQuery : public GenQuery {

  public:
    explicit VmCurrentContextQuery(EQueryType queryType) : GenQuery(queryType), mVmCurrentContext(0) { }
    void GetResults(py::object& rPyObject) const override; //!< Return query results in the passed in rPyObject.
    void SetId(uint32 id) const {mVmCurrentContext = id;} //!< return  mVmCurrentContext
  protected:
    mutable uint32 mVmCurrentContext;
  };

/*!
    \class GenHandlerSetMemoryQuery
    \brief Query handler set memory.
  */
  class GenHandlerSetMemoryQuery : public GenQuery {
  public:
    explicit GenHandlerSetMemoryQuery(EQueryType queryType) : GenQuery(queryType), mMemoryBase(0), mMemorySize(0) { } //!< Default constructor.
    inline void SetMemoryBase(uint64 base) const { mMemoryBase = base; } //!< Set memory base.
    inline void SetMemorySize(uint64 size) const { mMemorySize = size; } //!< Set memory size
    void GetResults(py::object& rPyObject) const override; //!< Return RegisterIndex query results in the passed in rPyObject.
  private:
    mutable uint64 mMemoryBase; //!< Return memory base for Default or Secondary
    mutable uint64 mMemorySize; //!< Return memory size
  };

  /*!
    \class GenHandlerSetMemoryQuery
    \brief Query handler set memory.
  */
  class GenExceptionVectorBaseAddressQuery : public GenQuery {
  public:
    explicit GenExceptionVectorBaseAddressQuery(EQueryType queryType) : GenQuery(queryType), mVectorBaseAddress(0) { } //!< Default constructor.
    inline void SetVectorBaseAddress(uint64 base) const { mVectorBaseAddress = base; } //!< Set memory base.
    void GetResults(py::object& rPyObject) const override; //!< Return RegisterIndex query results in the passed in rPyObject.
  private:
    mutable uint64 mVectorBaseAddress; //!< Return memory base for Default or Secondary
  };

  /*!
    \ class EntropyInfo
    \ brief Keep entropy information for query
   */
  class EntropyInfo {
  public:
    EntropyInfo() : mState(EEntropyStateType(0)), mEntropy(0), mOnThreshold(0), mOffThreshold(0) { }  //!< Default constructor
    EntropyInfo(EEntropyStateType state, uint32 entropy, uint32 on_threshold, uint32 off_threshold) : mState(state), mEntropy(entropy), mOnThreshold(on_threshold), mOffThreshold(off_threshold) { } //!< Constructor
    EntropyInfo& operator=(const EntropyInfo& rOther); //!< assign operator

    EEntropyStateType mState; //!< Entropy state
    uint32 mEntropy;  //!< Entropy
    uint32 mOnThreshold; //!< Threshold entropy to turn on
    uint32 mOffThreshold; //!< Threshold entropy to turn off
  };

  /*!
    \class GenResourceEntropyQuery
    \brief Query resource entropy.
  */
  class GenResourceEntropyQuery : public GenQuery {
  public:
    explicit GenResourceEntropyQuery(EQueryType queryType); //!< Default constructor.
    ~GenResourceEntropyQuery(); //!< Destructor
    void GetResults(py::object& rPyObject) const override; //!< Return RegisterIndex query results in the passed in rPyObject.
    void SetSourceEntropy(const EntropyInfo& info) const; //!< Set source entropy info
    void SetDestEntropy(const EntropyInfo& info) const; //!< Set dest entropy info
  private:
    mutable EntropyInfo mSourceEntropy; //!< Source entropy info
    mutable EntropyInfo mDestEntropy;  //!< Dest entropy info
  };

  /*!
    \class GenRestoreLoopContextQuery
    \brief Query restore loop context info
  */
  class GenRestoreLoopContextQuery : public GenQuery {
  public:
    explicit GenRestoreLoopContextQuery(EQueryType queryType) : GenQuery(queryType), mLoopId(0), mLoopBackAddress(0), mBranchRegIndex(0) { } //!< Default constructor
    void GetResults(py::object& rPyObject) const override; //!< Return restore loop context query results.
    void SetLoopId(cuint32 loopId) const { mLoopId = loopId; } //!< Record ID of current restore loop.
    void SetLoopBackAddress(cuint64 loopBackAddress) const { mLoopBackAddress = loopBackAddress; } //!< Record loop back address of current restore loop.
    void SetBranchRegisterIndex(cuint32 branchRegIndex) const { mBranchRegIndex = branchRegIndex; } //!< Record index of the branch register.
  private:
    mutable uint32 mLoopId; //!< ID of current restore loop
    mutable uint64 mLoopBackAddress; //!< Loop back address of current restore loop
    mutable uint32 mBranchRegIndex; //!< Index of the branch register
  };

}

#endif
