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
#include <Register.h>
#include <Random.h>
#include <Config.h>
#include <XmlTreeWalker.h>
#include <UtilityFunctions.h>
#include <StringUtils.h>
#include <pugixml.h>
#include <ObjectRegistry.h>
#include <Choices.h>
#include <ChoicesModerator.h>
#include <Constraint.h>
#include <GenException.h>
#include <RegisterInitPolicy.h>
#include <RegisterReserver.h>
#include <Log.h>

#include <string.h>
#include <algorithm>
#include <sstream>
#include <memory>
// C++UP accumulate defined in numeric
#include <numeric>

using namespace std;

/*!
  \file Register.cc
*/

namespace Force
{
  /*! \class RegisterParser
      \brief Parser class for register files.

      RegisterParser inherits pugi::xml_tree_walker, used for parsing register
      XML files into Register class structure
  */
  class RegisterParser : public pugi::xml_tree_walker
  {
  public:
    /*!
      Constructor, pass in pointer to Register related objects.
    */
    explicit RegisterParser(RegisterFile* register_file)
      : mpObjectRegistry(nullptr), mpRegisterFile(register_file), mpPhysicalRegister(nullptr), mpRegister(nullptr), mpRegisterField(nullptr), mpBitField(nullptr), mLinkName("")
    {
      mpObjectRegistry = ObjectRegistry::Instance();
    }

    ~RegisterParser() //!< Destructor
    {
      mpObjectRegistry = nullptr;
    }

    ASSIGNMENT_OPERATOR_ABSENT(RegisterParser);
    COPY_CONSTRUCTOR_DEFAULT(RegisterParser);
    /*!
      Handles register file elements.
     */
    virtual bool for_each(pugi::xml_node& node)
    {
      const char * node_name = node.name();
      if ( (strcmp(node_name, "registers") == 0) || (strcmp(node_name, "physical_registers") == 0) ) {
        // known node names.
      }
      else if (strcmp(node_name, "physical_register") == 0) {
        process_physical_register(node);
      }
      else if (strcmp(node_name, "register_file") == 0) {
        process_register_file(node);
      }
      else if (strcmp(node_name, "register") == 0) {
        process_register(node);
      }
      else if (strcmp(node_name, "register_field") == 0) {
        process_register_field(node, "RegisterField");
      }
      else if (strcmp(node_name, "register_field_res0") == 0) {
        process_register_field(node, "RegisterFieldRes0");
      }
      else if (strcmp(node_name, "register_field_res1") == 0) {
        process_register_field(node, "RegisterFieldRes1");
      }
      else if (strcmp(node_name, "bit_field") == 0) {
        process_bit_field(node);
      }
      else {
        LOG(fail) << "Unknown register file node name \'" << node_name << "\'." << endl;
        FAIL("register-unknown-node");
      }

      return true; // continue traversal
    }

    /*!
      Implement end function to add the last Register object.
     */
    virtual bool end(pugi::xml_node& node)
    {
      commit_physical_register();
      commit_register();
      return true;
    }

    /*!
      Process physical_register of \<physical_register\> element.
     */
    void process_physical_register(pugi::xml_node& node)
    {
      commit_physical_register();

      pugi::xml_attribute class_attr = node.attribute("class");
      if (class_attr)
      {
        mpPhysicalRegister = dynamic_cast<PhysicalRegister*>(mpObjectRegistry->ObjectInstance(class_attr.value()));
      }
      else
      {
        mpPhysicalRegister = dynamic_cast<PhysicalRegister*>(mpObjectRegistry->ObjectInstance("PhysicalRegister"));
      }

      bool reset_post_process = false;
      for (pugi::xml_attribute const& attr: node.attributes())
      {
        const char* attr_name = attr.name();
        if (strcmp(attr_name, "name") == 0)
        {
          mpPhysicalRegister->mName = attr.value();
        }
        else if (strcmp(attr_name, "size") == 0)
        {
          uint32 size = parse_uint32(attr.value());
          mpPhysicalRegister->SetSizeAndMask(size);
        }
        else if (strcmp(attr_name, "reset") == 0)
        {
          mpPhysicalRegister->mResetValue = parse_uint64(attr.value());
          reset_post_process = true;
        }
        else if (strcmp(attr_name, "type") == 0)
        {
          mpPhysicalRegister->mRegisterType = string_to_ERegisterType(attr.value());
        }
        else if (strcmp(attr_name, "index") == 0)
        {
           mpPhysicalRegister->mIndex = parse_uint32(attr.value());
        }
        else if (strcmp(attr_name, "sub_index") == 0)
        {
           mpPhysicalRegister->mSubIndex = parse_uint32(attr.value());
        }
        else if (strcmp(attr_name, "link") == 0)
        {
          mLinkName = attr.value();
        }
        else if (strcmp(attr_name, "class") != 0)
        {
          LOG(fail) << "Unknown register attribute \'" << attr_name << endl;
          FAIL("unknown-register-attribute");
        }
      }

      if (reset_post_process)
      {
        mpPhysicalRegister->mResetMask = mpPhysicalRegister->mMask;
      }
    }

    /*!
      Commit last Register related objects if there is any.
    */
    void commit_physical_register()
    {
      if (nullptr != mpPhysicalRegister)
      {
        mpRegisterFile->AddPhyReg(mpPhysicalRegister, mLinkName);
        //TODO need to add to index map too
        mpPhysicalRegister = nullptr;
        mLinkName = "";
      }
    }

    /*!
      Process register_file of \<register_file\> element.
     */
    void process_register_file(pugi::xml_node& node)
    {
      commit_physical_register();

      for (pugi::xml_attribute const& attr: node.attributes()) {
        const char* attr_name = attr.name();
        if (strcmp(attr_name, "name") == 0) mpRegisterFile->mName = attr.value();
        else {
          LOG(fail) << "Unknown register file attribute \'" << attr_name << endl;
          FAIL("unknown-register-file-attribute");
        }
      }
    }

    /*!
      Process register of \<register\> element.
     */
    void process_register(pugi::xml_node& node)
    {
      commit_register();

      pugi::xml_attribute class_attr = node.attribute("class");
      if (class_attr) {
        mpRegister = dynamic_cast<Register*>(mpObjectRegistry->ObjectInstance(class_attr.value()));
      } else {
        mpRegister = dynamic_cast<Register*>(mpObjectRegistry->ObjectInstance("Register"));
      }

      for (pugi::xml_attribute const& attr: node.attributes()) {
        const char* attr_name = attr.name();
        if (strcmp(attr_name, "name") == 0) {
          mpRegister->SetName(attr.value());
        } else if (strcmp(attr_name, "size") == 0) {
          mpRegister->mSize = parse_uint32(attr.value());
        } else if (strcmp(attr_name, "boot") == 0) {
          mpRegister->mBoot = parse_uint32(attr.value());
        } else if (strcmp(attr_name, "index") == 0) {
          mpRegister->mIndex = parse_uint32(attr.value());
        } else if (strcmp(attr_name, "type") == 0) {
          //TODO change after supporting big registers.
          mpRegister->mRegisterType = string_to_ERegisterType(attr.value());
        } else if (strcmp(attr_name, "init_policy") == 0) {
          mpRegister->SetInitPolicy(attr.value());
        } else if (strcmp(attr_name, "class") !=0) {
          LOG(fail) << "Unknown register attribute \'" << attr_name << endl;
          FAIL("unknown-register-attribute");
        }
      }
    }

    /*!
      Commit last Register related objects if there is any.
    */
    void commit_register()
    {
      commit_register_field();
      if (nullptr != mpRegister) {
        mpRegisterFile->AddRegister(mpRegister);
        //TODO need to add to index map too
        mpRegister = nullptr;
      }
    }

    /*!
      Process attributes of \<register_field\> element.
    */

    void process_register_field(pugi::xml_node& node, const string& rFieldClass)
    {
      commit_register_field();

      pugi::xml_attribute class_attr = node.attribute("class");
      if (class_attr) {
        mpRegisterField = dynamic_cast<RegisterField* >(mpObjectRegistry->ObjectInstance(class_attr.value()));
      }
      else {
        mpRegisterField = dynamic_cast<RegisterField* >(mpObjectRegistry->ObjectInstance(rFieldClass));
      }

      for (pugi::xml_attribute const& attr: node.attributes()) {
        const char* attr_name = attr.name();
        if (strcmp(attr_name, "name") == 0) { mpRegisterField->mName = attr.value(); }
        else if (strcmp(attr_name, "size") == 0) { mpRegisterField->mSize = parse_uint32(attr.value()); }
        else if (strcmp(attr_name, "physical_register") == 0) { mpRegisterField->mPhysicalRegisterName = attr.value(); }
        else if (strcmp(attr_name, "shift") == 0) {
          mpRegisterField->mLsbBlock = parse_uint32(attr.value());
          if (mpRegisterField->mLsbBlock > 64) {
            LOG(fail) << "{RegisterParser::process_register_field} the value for the attribute shift is too large: 0x" << hex << mpRegisterField->mLsbBlock << endl;
            FAIL("value-is-too-large");
          }
        }
        else if (strcmp(attr_name, "reset") == 0) {
          mpRegisterField->mHasReset = true;
          mpRegisterField->mReset = parse_uint64(attr.value());
        } else if (strcmp(attr_name, "init_policy") == 0) {
          mpRegisterField->SetInitPolicy(attr.value());
        } else if (strcmp(attr_name, "class") !=0) {
          LOG(fail) << "Unknown register field attribute \'" << attr_name << endl;
          FAIL("unknown-register-field-attribute");
        }
      }
    }

    /*!
      Commit last register field object if there is any.
    */
    void commit_register_field()
    {
      commit_bit_field();
      if (nullptr != mpRegisterField) {
        mpRegister->mRegisterFields.push_back(mpRegisterField);
        mpRegisterField = nullptr;
      }
    }

    /*!
      Process attributes of \<bit_field\> element.
    */
    void process_bit_field(pugi::xml_node& node)
    {
      commit_bit_field();

      mpBitField = new BitField();

      for (pugi::xml_attribute const& attr: node.attributes()) {
        const char* attr_name = attr.name();
        if (strcmp(attr_name, "shift") == 0)
        {
          mpBitField->mShift = parse_uint32(attr.value());
          //mpBitField->mLsb = mpBitField->mShift;
        }
        else if (strcmp(attr_name, "size") == 0)
        {
          mpBitField->SetSize(parse_uint32(attr.value()));
        } else {
          LOG(fail) << "Unknown bit field attribute \'" << attr_name << endl;
          FAIL("unknown-bit-field-attribute");
        }
      }

      mpBitField->SetMask();
    }

    /*!
      Commit last register field object if there is any.
    */
    void commit_bit_field()
    {
      if (nullptr != mpBitField) {
        mpRegisterField->mBitFields.push_back(mpBitField);
        mpBitField = nullptr;
      }
    }


  private:
    ObjectRegistry* mpObjectRegistry; //!< Pointer to ObjectRegistry class instance.
    RegisterFile* mpRegisterFile; //!< Pointer to RegisterFile object.
    PhysicalRegister* mpPhysicalRegister; //!< Pointer to the current PhysicalRegister object being worked on.
    Register* mpRegister; //!< Pointer to the current Register object being worked on.
    RegisterField* mpRegisterField; //!< Pointer to the current RegisterField object being worked on.
    BitField* mpBitField;

    std::string mLinkName;
    friend class RegisterFile;
  };

  /*! \class PhysicalRegister
      \brief Base representation of a Physical Register
   */
  PhysicalRegister::PhysicalRegister()
    : mName(),mRegisterType(ERegisterType::GPR), mValue(0), mMask(0), mInitialValue(0), mInitMask(0),
      mResetValue(0), mResetMask(0), mSize(0), mIndex(0), mSubIndex(0), mAttributes((uint32)ERegAttrType::ReadWrite)
  {
  }

  PhysicalRegister::~PhysicalRegister()
  {
  }

  PhysicalRegister::PhysicalRegister(const PhysicalRegister& rOther)
    : Object(rOther), mName(rOther.mName), mRegisterType(rOther.mRegisterType), mValue(rOther.mValue), mMask(rOther.mMask),
      mInitialValue(rOther.mInitialValue), mInitMask(rOther.mInitMask), mResetValue(rOther.mResetValue), mResetMask(rOther.mResetMask),
      mSize(rOther.mSize), mIndex(rOther.mIndex), mSubIndex(rOther.mSubIndex), mAttributes(rOther.mAttributes)
  {
  }

  bool PhysicalRegister::IsInitialized(uint64 mask, bool* partial) const
  {
    uint64 initMaskOfInterest = mInitMask&mask;
    if (initMaskOfInterest)
    {
      if (initMaskOfInterest == mask)
      {
        if (partial != NULL) *partial = false;
        return true;
      }
      else
      {
        if (partial != NULL) *partial = true;
        return false;
      }
    }
    else
    {
      if (partial != NULL) *partial = false;
      return false;
    }
  }

  bool PhysicalRegister::HasAttribute(ERegAttrType attr) const
  {
    uint32 attr_mask = static_cast<uint32>(attr);
    if ((mAttributes & attr_mask) == attr_mask) return true;
    return false;
  }

  void PhysicalRegister::SetAttribute(ERegAttrType attr)
  {
    mAttributes |= static_cast<uint32>(attr);
  }

  void PhysicalRegister::ClearAttribute(ERegAttrType attr)
  {
    mAttributes &= ~(static_cast<uint32>(attr));
  }

  uint64 PhysicalRegister::Value(uint64 mask) const
  {

    mask &= mMask;

    if (((mInitMask | mResetMask) & mask) != mask) {
      LOG(fail) << "The bit range of physical register: " << mName << " wasn't initialized or reset defined: " << hex << (mask & ~(mInitMask | mResetMask)) << " request mask=0x" << mask << " Init mask=0x" << mInitMask
                << " reset mask 0x" << mResetMask << endl;
      FAIL("physicalregister_not_initialized_nor_reset_fail");
    }

    uint64 initMask = mInitMask & mask;
    uint64 resetMask = mask & (~mInitMask);      // if not initialized, reset mask must be 1 for given bit
    return (initMask & mValue ) | ( resetMask & mResetValue);
  }

  uint64 PhysicalRegister::InitialValue(uint64 mask) const
  {
    mask &= mMask;
    if (((mInitMask | mResetMask) & mask) != mask) {
      LOG(fail) << "The bit range of physical register: " << mName << " wasn't initialized or reset defined: " << hex << (mask & ~(mInitMask | mResetMask)) << endl;
      FAIL("physicalregister-not-initialized-nor-reset-fail");
    }

    uint64 initMask = mInitMask & mask;
    uint64 resetMask = mask & (~mInitMask);      // if not initialized, reset mask must be 1 for given bit
    return (initMask & mInitialValue) | ( resetMask & mResetValue);
  }

  void PhysicalRegister::SetValue(uint64 value, uint64 mask)
  {
    if ((mask & mInitMask) != mask )
    {
      stringstream err_stream;
      err_stream << "[" << EGenExceptionDetatilType_to_string(EGenExceptionDetatilType::RegisterNotInitSetValue) << "] name: " << mName << " set-mask: 0x" << hex << mask << " init-mask: 0x" << mInitMask << " size-mask 0x" << mMask;
      throw RegisterError(err_stream.str());
    }

    mAttributes |= static_cast<uint32>(ERegAttrType::HasValue);
    mValue = (((mValue&(~mask)) + (value&mask))&mMask) + (mValue&(~mMask));
  }

  void PhysicalRegister::SetResetValue(uint64 value, uint64 mask)
  {
    mResetMask  |= mask;
    mResetValue |= (value & mask);
  }

  void PhysicalRegister::Initialize(uint64 value, uint64 mask)
  {
    uint64 overlap_mask = mask & mInitMask;
    if (overlap_mask)
    {
      uint64 new_overlap_val  = value & overlap_mask;
      uint64 init_overlap_val = mInitialValue & overlap_mask;
      //only fail if re-initializing bit to a different value
      if (new_overlap_val != init_overlap_val) {
        LOG(fail) << hex << "{PhysicalRegister::Initialize} " << mName << " - double init of bits (0x" << overlap_mask << ") to different value."
                    << " new_val=0x" << (value&mask) << " old_val=0x" << (mInitialValue&mInitMask) << dec << endl;
        FAIL("physical-register-bit-double-initialization-fail");
      }
    }

    uint64 bitmask_to_init = mask & (mMask^mInitMask);

    mInitMask     = (mInitMask|mask);
    mInitialValue = (((mInitialValue&(~mask)) | (value&mask)) & mMask) | (mInitialValue&(~mMask));
    // << "{PhysicalRegister::Initialize} physical register name:" << Name() << ", init mask: 0x" << hex << mInitMask << ", initial value: 0x" << mInitialValue << endl;
    // << "PhysicalRegister::Initialize register class detected, using SetValue with value=" << hex << value << " bitmask=" << bitmask_to_init << endl;
    if (HasLinkRegister())
      PhysicalRegister::SetValue(value, bitmask_to_init);
    else
      SetValue(value, bitmask_to_init);
  }

  void PhysicalRegister::SetSizeAndMask(uint32 size)
  {
    mSize = size;
    mMask = get_mask64(mSize);
  }

  void PhysicalRegisterRazwi::SetSizeAndMask(uint32 size)
  {
    PhysicalRegister::SetSizeAndMask(size);

    mInitMask = mMask;
    mResetMask = mMask;
    mAttributes |= static_cast<uint32>(ERegAttrType::HasValue);
  }

  /*!
    \class LinkedPhysicalRegister
    \brief physical register which propogates value changes to another physical register
  */
  LinkedPhysicalRegister::LinkedPhysicalRegister() : PhysicalRegister(), mpPhysicalLink(nullptr)
  {

  }

  LinkedPhysicalRegister::~LinkedPhysicalRegister()
  {
  }

  LinkedPhysicalRegister::LinkedPhysicalRegister(const LinkedPhysicalRegister& rOther) : PhysicalRegister(rOther),
    mpPhysicalLink(rOther.mpPhysicalLink)
  {
  }

  void LinkedPhysicalRegister::SetValue(uint64 value, uint64 mask)
  {
    // << "{LinkedPhysicalRegister::SetValue} set value=" << hex << value << " mask=0x" << mask << endl;
    PhysicalRegister::SetValue(value, mask);
    mpPhysicalLink->PhysicalRegister::SetValue(value, mask);
  }

  void LinkedPhysicalRegister::Initialize(uint64 value, uint64 mask)
  {
    // << "{LinkedPhysicalRegister::Initialize} value=" << hex << value << " mask=0x" << mask
               //<< " for " << mName << " linked to " << mpPhysicalLink->mName << endl;
    bool partial = false;
    if (!IsInitialized(mask, &partial))
    {
      // << "{LinkedPhysicalRegister::Initialize} initializing for " << mName << endl;
      PhysicalRegister::Initialize(value, mask);
    }

    if (!(mpPhysicalLink->IsInitialized(mask, &partial)))
    {
      // << "{LinkedPhysicalRegister::Initialize} initializing link of " << mName << " link_regname=" << mpPhysicalLink->mName << endl;
      mpPhysicalLink->PhysicalRegister::Initialize(value, mask);
    }
  }

  /*! \class ConfigureRegister
      \brief Configuration Physical Register which needs to notify others of its updated values
   */
  void ConfigureRegister::SetValue(uint64 value, uint64 mask)
  {
    // TODO this doesn't work when reset value is not 0, but mValue=0 and initializing value to 0.
    //uint64 originalValue = mValue;
    PhysicalRegister::SetValue(value, mask);
    //if (originalValue != mValue)
    //{
    Sender::SendNotification(ENotificationType::RegisterUpdate);
    //}
  }

  /*! \class BitField
      \brief Base represenation of a bitfield inside of a logical register
   */
  BitField::BitField()
    : Object(), mMask(0), mSize(0), mShift(0), mpRegister(nullptr)
  {
  }

  BitField::~BitField()
  {
  }

  BitField::BitField(const BitField& rOther)
    : Object(rOther), mMask(rOther.mMask), mSize(rOther.mSize), mShift(rOther.mShift), mpRegister(rOther.mpRegister)
  {
  }

  bool BitField::IsInitialized(bool *partial) const
  {
    if (nullptr == mpRegister)
    {
      if (partial != NULL) *partial = false;
      return false;
    }
    else
    {
      return mpRegister->IsInitialized(mMask, partial);
    }
  }

  void BitField::GetPhysicalRegisters(std::set<PhysicalRegister*>& phyRegisterSet) const
  {
    phyRegisterSet.insert(mpRegister);
  }

  uint64 BitField::ReloadValue(uint64& reloadValue, uint64 new_value) const
  {
    uint64 full_mask = ~0x0ull;
    reloadValue = (reloadValue & (full_mask & (~mMask))) | (new_value & mMask);
    return new_value & mMask;
  }

  void BitField::Initialize(uint64 value)
  {
    if (nullptr == mpRegister)
    {
      LOG(fail) << "BitField needs Physical Register before initialize value " << endl;
      FAIL("BitField needs Physical Register before initialize value");
    }
    else
    {
      mpRegister->Initialize(value, mMask);
    }
  }

  void BitField::InitializePartial(uint64 value )
  {
    if (nullptr == mpRegister)
    {
      LOG(fail) << "BitField needs Physical Register before initialize value " << endl;
      FAIL("BitField needs Physical Register before initialize value");
    }
    else
    {
      uint64 new_init_mask = ~mpRegister->InitMask() & mMask;
      mpRegister->Initialize(value, new_init_mask);
    }
  }

  void BitField::SetMask()
  {
    mMask = get_mask64(mSize, mShift);
  }

  /*! \class RegisterField
      \brief Base representation of a register field inside a logical register
   */
  RegisterField::RegisterField()
    : Object(), mSize(0), mLsb(0), mLsbBlock(0), mReset(0), mHasReset(false), mName(), mPhysicalRegisterName(), mpInitPolicyInfo(nullptr), mBitFields()
  {
  }

  RegisterField::~RegisterField()
  {
    for (auto bf_struct : mBitFields) {
      delete bf_struct;
    }

    delete mpInitPolicyInfo;
  }

  RegisterField::RegisterField(const RegisterField& rOther)
    : Object(rOther), mSize(rOther.mSize), mLsb(rOther.mLsb), mLsbBlock(rOther.mLsbBlock), mReset(rOther.mReset),
      mHasReset(rOther.mHasReset), mName(rOther.mName), mPhysicalRegisterName(rOther.mPhysicalRegisterName), mpInitPolicyInfo(nullptr), mBitFields()
  {
    std::vector<BitField *>::const_iterator it;
    for (it=rOther.mBitFields.begin(); it!=rOther.mBitFields.end(); ++it) {
      mBitFields.push_back(dynamic_cast<BitField* >((*it)->Clone()));
    }

    if (nullptr != rOther.mpInitPolicyInfo) {
      mpInitPolicyInfo = new InitPolicyTuple(*(rOther.mpInitPolicyInfo));
    }
  }

  void RegisterField::SetInitPolicy(const std::string& rPolicyName)
  {
    delete mpInitPolicyInfo;
    mpInitPolicyInfo = new InitPolicyTuple(rPolicyName);
  }

  bool RegisterField::IsInitialized(bool *partial) const
  {
    std::vector<BitField *>::const_iterator it;
    bool result = true;
    bool or_result = false;
    bool field_partial = false;

    for (it=mBitFields.begin(); it!=mBitFields.end(); ++it) {
      bool bit_result = (*it)->IsInitialized(&field_partial);
      result &= bit_result;
      or_result |= bit_result;

      if (true == field_partial) {
        if (NULL != partial) { *partial = true; }
        return false;
      }

      if (result != or_result ) {
       if (NULL != partial) { *partial = true; }
       return false;
      }
    }

    if (NULL != partial) { *partial = false; }
    return result;
  }

  uint64 RegisterField::Value() const
  {
    std::vector<BitField *>::const_iterator it;
    uint64 result = 0;
    for (it=mBitFields.begin(); it!=mBitFields.end(); ++it) {
      uint64 value = (*it)->Value();
      result = result|value;
    }
    return result << mLsbBlock;
  }

  uint64 RegisterField::InitialValue() const
  {
    std::vector<BitField *>::const_iterator it;
    uint64 result = 0;
    for (it=mBitFields.begin(); it!=mBitFields.end(); ++it) {
      uint64 value = (*it)->InitialValue();
      result = result|value;
    }
    return result << mLsbBlock;
  }

  uint64 RegisterField::FieldValue() const
  {
    uint64 result = 0;
    uint32 shift = 0;
    for (auto bit_it = mBitFields.crbegin(); bit_it != mBitFields.crend(); ++bit_it) {
      uint64 value = (*bit_it)->BitFieldValue() << shift;
      result |= value;
      shift += (*bit_it)->Size();
    }

    return result << mLsbBlock;
  }

  uint64 RegisterField::InitialFieldValue() const
  {
    uint64 result = 0;
    uint32 shift = 0;
    for (auto bit_it = mBitFields.crbegin(); bit_it != mBitFields.crend(); ++bit_it) {
      uint64 value = (*bit_it)->InitialBitFieldValue() << shift;
      result |= value;
      shift += (*bit_it)->Size();
    }

    return result << mLsbBlock;
  }

  uint64 RegisterField::FieldMask() const
  {
    uint64 mask = GetPhysicalRegisterMask();
    return mask << mLsbBlock;
  }

  uint64 RegisterField::GetPhysicalRegisterMask() const
  {
    return accumulate(mBitFields.cbegin(), mBitFields.cend(), uint64(0),
      [](cuint64 partialMask, const BitField* pBitField) { return (partialMask | pBitField->Mask()); });
  }

  uint64 RegisterField::ReloadValue(uint64& reloadValue, const ChoiceTree* pChoiceTree) const
  {
    uint64 return_value = 0;
    LOG(notice) << "RegisterField::ReloadValue begin:0x" << hex << reloadValue << " field name:" << Name() << endl;
    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      if (pChoiceTree)
      {
        uint64 value = pChoiceTree->Choose()->ValueAs64() << mLsb;
        return_value |= (*it)->ReloadValue(reloadValue, value);
        LOG(notice) << "RegisterField::ReloadValue: value:" << value << " return_value:" << return_value << " reload value:" << reloadValue << endl;
      }
      else
      {
        return_value |= (*it)->ReloadValue(reloadValue);
      }
    }
    reloadValue <<= mLsbBlock;
    return return_value;
  }

  uint64 RegisterField::ReloadValue(uint64& reloadValue, const ConstraintSet* pConstraintSet) const
  {
    uint64 return_value = 0;
    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      if (pConstraintSet)
      {
        uint64 value = pConstraintSet->ChooseValue() << mLsb;
        return_value |= (*it)->ReloadValue(reloadValue, value);
      }
      else
      {
        return_value |= (*it)->ReloadValue(reloadValue);
      }
    }
    reloadValue = reloadValue << mLsbBlock;
    return return_value;
  }

  void RegisterField::SetValue(uint64 value)
  {
    uint64 blockValue = value >> mLsbBlock;
    std::vector<BitField *>::iterator it;
    for (it=mBitFields.begin(); it!=mBitFields.end(); ++it) {
      (*it)->SetValue(blockValue);
    }
  }

  void RegisterField::SetFieldValue(uint64 value)
  {
    uint32 shift = 0;
    for (auto bit_it = mBitFields.crbegin(); bit_it != mBitFields.crend(); ++bit_it) {
      (*bit_it)->SetBitFieldValue(value >> shift);
      shift += (*bit_it)->Size();
    }
  }

  void RegisterField::SetFieldResetValue(uint64 value) const
  {
    uint32 shift = 0;
    for (auto bit_it = mBitFields.crbegin(); bit_it != mBitFields.crend(); ++bit_it) {
      (*bit_it)->SetBitResetValue(value >> shift);
      shift += (*bit_it)->mSize;
    }
  }

  bool RegisterField::HasAttribute(ERegAttrType attr) const
  {
    bool all_bit_fields_have_attribute = all_of(mBitFields.cbegin(), mBitFields.cend(),
      [attr](const BitField* pBitField) { return pBitField->HasAttribute(attr); });

    return all_bit_fields_have_attribute;
  }

  void RegisterField::SetAttribute(ERegAttrType attr)
  {
    std::vector<BitField *>::iterator it;
    for (it=mBitFields.begin(); it!=mBitFields.end(); ++it) {
      (*it)->SetAttribute(attr);
    }
  }

  void RegisterField::ClearAttribute(ERegAttrType attr)
  {
    std::vector<BitField *>::iterator it;
    for (it=mBitFields.begin(); it!=mBitFields.end(); ++it) {
      (*it)->ClearAttribute(attr);
    }
  }

  void RegisterField::Block() const
  {
    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      (*it)->Block();
    }
  }

  void RegisterField::Unblock() const
  {
    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      (*it)->Unblock();
    }
  }

  void RegisterField::Initialize(uint64 value)
  {
    // << "{RegisterField::Initialize} field " << Name() << " value 0x" << hex << value << " size " << dec << mSize << " lsb block: " << mLsbBlock << " lsb: " << mLsb << endl;
    auto block_value = value >> mLsbBlock;
    std::vector<BitField *>::iterator it;
    for (it=mBitFields.begin(); it!=mBitFields.end(); ++it) {
      bool bit_partial = false;
      if ((*it)->IsInitialized(&bit_partial)) {
        continue;
      } else if (true == bit_partial) {
        (*it)->InitializePartial(block_value);
      } else {
        (*it)->Initialize(block_value);
      }
    }
  }

  void RegisterField::InitializeField(uint64 value)
  {
    uint32 shift = 0;
    for (auto bit_it = mBitFields.crbegin(); bit_it != mBitFields.crend(); ++bit_it) {
      bool bit_partial = false;

      if ((*bit_it)->IsInitialized(&bit_partial)) {
        continue;
      } else if (bit_partial) {
        (*bit_it)->InitializePartial(value >> shift);
      } else {
        (*bit_it)->InitializeBitField(value >> shift);
      }

      shift += (*bit_it)->Size();
    }
  }

  void RegisterField::InitializeFieldRandomly(const ChoiceTree* pChoiceTree)
  {

    if (IsInitialized())
      return;      // don't need to initialize if already initialized.

    // Check if there is special init policy specified.
    if ((nullptr != mpInitPolicyInfo) and (nullptr != mpInitPolicyInfo->mpInitPolicy)) {
      mpInitPolicyInfo->mpInitPolicy->InitializeRegisterField(this, pChoiceTree);
      return;
    }

    uint64 value = 0;

    if (pChoiceTree != nullptr) {
      value = pChoiceTree->Choose()->ValueAs64(); // 32-bit value chosen
    } else {
      uint64 max_value = get_mask64(mSize);
      value = Random::Instance()->Random64(0, max_value);
    }

    // << "{RegisterField::InitializeFieldRandomly} field " << Name() << " value 0x" << hex << value << " size " << dec << mSize << " lsb " << mLsb << endl;

    InitializeField(value);
  }

  void RegisterField::Setup(const RegisterFile* pRegisterFile)
  {
    std::vector<BitField *>::iterator it;
    PhysicalRegister* my_physical_reg;
    my_physical_reg = pRegisterFile->PhysicalRegisterLookup(mPhysicalRegisterName);
    mLsb= 63;
    for (it=mBitFields.begin(); it!=mBitFields.end(); ++it) {
      (*it)->mpRegister = my_physical_reg;
      mLsb= min(mLsb, (*it)->mShift);
    }

    if (mHasReset) {
      SetFieldResetValue(mReset);
    }

    if (nullptr != mpInitPolicyInfo) {
      mpInitPolicyInfo->Setup(pRegisterFile);
    }
  }

  void RegisterField::GetPhysicalRegisters(std::set<PhysicalRegister*>& phyRegisterSet) const
  {
    for (auto bit_it = mBitFields.begin(); bit_it != mBitFields.end(); ++bit_it)
    {
      (*bit_it)->GetPhysicalRegisters(phyRegisterSet);
    }
  }

  bool RegisterField:: IgnoreUpdate(const std::string& ignoredRegFields) const
  {
    bool ignored = false;
    string reg_field = mPhysicalRegisterName + "." + mName;
    if (ignoredRegFields.find(reg_field) != string::npos)
      ignored = true;

    return ignored;
  }

  /*! \class RegisterFieldRes0
      \brief Register field whose bits are defined as RES0
   */
  Object* RegisterFieldRes0::Clone() const
  {
    return new RegisterFieldRes0(*this);
  }

  RegisterFieldRes0::~RegisterFieldRes0()
  {
  }

  RegisterFieldRes0::RegisterFieldRes0(const RegisterFieldRes0& rOther) : RegisterField(rOther)
  {
  }

  void RegisterFieldRes0::InitializeFieldRandomly(const ChoiceTree* pChoiceTree )
  {
    if (IsInitialized()) return;      // don't need to initialize if already initialized.

    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      bool bit_partial = false;
      if ( (*it)->IsInitialized(&bit_partial)) {
        continue;
      } else if (true == bit_partial) {
        (*it)->InitializePartial(0);
      } else {
        (*it)->Initialize(0);
      }
    }
  }

  uint64 RegisterFieldRes0::ReloadValue(uint64& reloadValue, const ChoiceTree* pChoiceTree) const
  {
    uint64 return_value = 0;
    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      return_value |= (*it)->ReloadValue(reloadValue, 0);
    }
    return return_value;
  }

  uint64 RegisterFieldRes0::ReloadValue(uint64& reloadValue, const ConstraintSet* constraintSets) const
  {
    uint64 return_value = 0;
    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      return_value |= (*it)->ReloadValue(reloadValue, 0);
    }
    return return_value;
  }

  /*! \class RegisterFieldRes1
      \brief Register field whose bits are defined as RES1
   */
  Object* RegisterFieldRes1::Clone() const
  {
    return new RegisterFieldRes1(*this);
  }

  RegisterFieldRes1::~RegisterFieldRes1()
  {
  }

  RegisterFieldRes1::RegisterFieldRes1(const RegisterFieldRes1& rOther) : RegisterField(rOther)
  {
  }

  void RegisterFieldRes1::InitializeFieldRandomly(const ChoiceTree* pChoiceTree)
  {
    if (IsInitialized()) return;      // don't need to initialize if already initialized.

    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      bool bit_partial = false;
      if ( (*it)->IsInitialized(&bit_partial)) {
        continue;
      } else if (true == bit_partial) {
        (*it)->InitializePartial(0xffffffffffffffffull);
      } else {
        (*it)->Initialize(0xffffffffffffffffull);
      }
    }
  }

  uint64 RegisterFieldRes1::ReloadValue(uint64& reloadValue, const ChoiceTree* pChoiceTree) const
  {
    uint64 return_value = 0;
    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      return_value |= (*it)->ReloadValue(reloadValue, 0xffffffffffffffffull);
    }
    return return_value;
  }

  uint64 RegisterFieldRes1::ReloadValue(uint64& reloadValue, const ConstraintSet* constraintSets) const
  {
    uint64 return_value = 0;
    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
    {
      return_value |= (*it)->ReloadValue(reloadValue, 0xffffffffffffffffull);
    }
    return return_value;
  }

  RegisterFieldRazwi::RegisterFieldRazwi(const RegisterFieldRazwi& rOther)
    : RegisterField(rOther)
  {
  }

  RegisterFieldRazwi::~RegisterFieldRazwi()
  {
  }

  Object * RegisterFieldRazwi::Clone() const
  {
    return new RegisterFieldRazwi(*this);
  }

  void RegisterFieldRazwi::InitializeFieldRandomly(const ChoiceTree* pChoiceTree)
  {
    if (IsInitialized()) return;      // don't need to initialize if already initialized.
    InitializeField(0);
  }

  RegisterFieldRaowi::RegisterFieldRaowi(const RegisterFieldRaowi& rOther)
    : RegisterField(rOther)
  {
  }

  RegisterFieldRaowi::~RegisterFieldRaowi()
  {
  }

  Object * RegisterFieldRaowi::Clone() const
  {
    return new RegisterFieldRaowi(*this);
  }

  void RegisterFieldRaowi::InitializeFieldRandomly(const ChoiceTree* pChoiceTree)
  {
    if (IsInitialized()) return;      // don't need to initialize if already initialized.
    InitializeField(MAX_UINT64);
  }

  uint64 ReadOnlyRegisterField::ReloadValue(uint64& reloadValue, const ChoiceTree* pChoiceTree)  const
  {
    uint64 ret_val = 0;
    for (auto it = mBitFields.begin(); it != mBitFields.end(); ++it)
      {
        ret_val |= (*it)->ReloadValue(reloadValue, (*it)->Value());
      }
    return ret_val;
  }

  uint64 ReadOnlyRegisterField::ReloadValue(uint64& reloadValue, const ConstraintSet* constraintSets) const
  {
    ChoiceTree* pChoiceTree = nullptr;
    return ReloadValue(reloadValue, pChoiceTree);
  }

  void ReadOnlyRegisterField::Initialize(uint64 value)
  {
    if (IsInitialized())
      return;
    auto val = Value();
    RegisterField::Initialize(val);
  }

  void ReadOnlyRegisterField::InitializeField(uint64 value)
  {
    if (IsInitialized())
      return;
    auto field_val = FieldValue();
    RegisterField::InitializeField(field_val);
  }

  void ReadOnlyRegisterField::InitializeFieldRandomly(const ChoiceTree* pChoiceTree )
  {
    InitializeField(0); // initialize by reset value
  }

  void ReadOnlyRegisterField::Setup(const RegisterFile* pRegisterFile)
  {
    RegisterField::Setup(pRegisterFile);
    pRegisterFile->RegisterReadOnlyRegisterField(this);
  }

  /*! \class Register
      \brief Base representation of a logical register
   */
  Register::Register()
    : Object(), mSize(0), mBoot(0), mIndex(0), mRegisterType(ERegisterType::GPR), mName(), mpInitPolicyInfo(nullptr), mRegisterFields()
  {
  }

  Register::~Register()
  {
    for (auto rf_struct : mRegisterFields) {
      delete rf_struct;
    }

    delete mpInitPolicyInfo;
  }

  Register::Register(const Register& rOther)
    : Object(rOther), mSize(rOther.mSize), mBoot(rOther.mBoot), mIndex(rOther.mIndex), mRegisterType(rOther.mRegisterType), mName(rOther.mName), mpInitPolicyInfo(nullptr), mRegisterFields()
  {
    std::vector<RegisterField *>::const_iterator it;
    for (it=rOther.mRegisterFields.begin(); it!=rOther.mRegisterFields.end(); ++it) {
      mRegisterFields.push_back(dynamic_cast<RegisterField* >((*it)->Clone()));
    }

    if (nullptr != rOther.mpInitPolicyInfo) {
      mpInitPolicyInfo = new InitPolicyTuple(*(rOther.mpInitPolicyInfo));
    }
  }

  void Register::SetInitPolicy(const std::string& rPolicyName)
  {
    delete mpInitPolicyInfo;
    mpInitPolicyInfo = new InitPolicyTuple(rPolicyName);
  }

  bool Register::IsInitialized(bool *partial) const
  {
    std::vector<RegisterField *>::const_iterator it;

    bool pos_result = true;     // is true when every field is initialized
    bool neg_result = false;    // is false when every field is not initialized
    bool field_partial = false;

    for (it=mRegisterFields.begin(); it!=mRegisterFields.end(); ++it) {
      bool field_result = (*it)->IsInitialized(&field_partial);
      pos_result &= field_result;
      neg_result |= field_result;

      if (true == field_partial) {
        if (NULL != partial) { *partial = true; }
        return false;
      }
      if ( pos_result != neg_result) {   // only possible when pos_result = false, and neg_result = true;
        if (NULL != partial) { *partial = true; }
        return false;
      }
    }
    if (NULL != partial) { *partial = false; }
    return pos_result;
  }

  uint64 Register::Value() const
  {
    std::vector<RegisterField *>::const_iterator it;
    uint64 result = 0;
    for (it=mRegisterFields.begin(); it!=mRegisterFields.end(); ++it) {
      uint64 field_value = (*it)->Value();
      result = result|field_value;
    }
    return result;
  }

  uint64 Register::InitialValue() const
  {
    std::vector<RegisterField *>::const_iterator it;
    uint64 result = 0;
    for (it=mRegisterFields.begin(); it!=mRegisterFields.end(); ++it) {
      uint64 field_value = (*it)->InitialValue();
      result = result|field_value;
    }
    return result;
  }

  uint64 Register::ReloadValue() const
  {
    if ( true == IsReadOnly() )
    {
      LOG(fail) << "The base register reload function is called for readonly registers" << endl;
      FAIL("wrong_reloadValue_function_called_fail");
    }

    uint64 max_value = get_mask64(mSize);
    return Random::Instance()->Random64(0, max_value);
  }

  uint64 Register::ReloadValue(const ChoicesModerator* pChoicesModerator, const map<string, ConstraintSet* >& fieldConstraintMap) const
  {
    if ( true == IsReadOnly() )
    {
      LOG(fail) << "The base register reload function is called for readonly registers" << endl;
      FAIL("wrong_reloadValue_function_called_fail");
    }

    uint64 max_value = get_mask64(mSize);
    uint64 reloadValue = Random::Instance()->Random64(0, max_value);

    for (auto it = mRegisterFields.begin(); it != mRegisterFields.end(); ++it)
    {
      auto field_constraint_it = fieldConstraintMap.find((*it)->Name());
      // custom defined choices is over internally saved choice tree, which is over random selection
      if (field_constraint_it != fieldConstraintMap.end()) {
        (*it)->ReloadValue(reloadValue, field_constraint_it->second);
      } else {
        auto choice_tree = GetChoiceTree((*it)->Name(), pChoicesModerator);
        std::unique_ptr<ChoiceTree> storage_ptr(choice_tree);  // holding the object until the end of scope
        (*it)->ReloadValue(reloadValue, choice_tree);
      }
    }
    return reloadValue;
  }

  uint64 Register::FieldReloadValue(const string& fieldName, const ChoicesModerator* pChoicesModerator) const
  {
    if ( true == IsReadOnly() )
    {
      LOG(fail) << "The base register reload function is called for readonly registers" << endl;
      FAIL("wrong_reloadValue_function_called_fail");
    }

    uint64 max_value = get_mask64(mSize);
    uint64 reloadValue = Random::Instance()->Random64(0, max_value);

    for (auto it = mRegisterFields.begin(); it != mRegisterFields.end(); ++it)
    {
      if (fieldName == (*it)->Name())
      {
        auto choice_tree = GetChoiceTree((*it)->Name(), pChoicesModerator);
        if (nullptr == choice_tree)
        {
          LOG(notice) << "(Register::FieldReloadValue) no choice tree!" << endl;
        }
        else
        {
          reloadValue = 0;
        }
        LOG(notice) << "(Register::FieldReloadValue) field name:" << fieldName << " field mask:0x" << hex << (*it)->FieldMask() << endl;
        std::unique_ptr<ChoiceTree> storage_ptr(choice_tree);  // holding the object until the end of scope
        (*it)->ReloadValue(reloadValue, choice_tree);
        reloadValue &= (*it)->FieldMask();
        reloadValue >>= (*it)->Lsb();
      }
    }
    return reloadValue;
  }

  void Register::SetValue(uint64 value)
  {
    std::vector<RegisterField *>::iterator it;
    for (it=mRegisterFields.begin(); it!=mRegisterFields.end(); ++it) {
      (*it)->SetValue(value);
    }
  }

  void Register::RegisterFieldInfo(const ChoicesModerator* pChoicesModerator, const map<string, ConstraintSet* >& fieldConstraintMap, uint64& mask, uint64& value) const
  {
    mask = 0ull;
    value = 0ull;
    for (auto field : fieldConstraintMap)
    {
      auto reg_field = RegisterFieldLookup(field.first);
      if (reg_field!= nullptr) {
        mask |= reg_field->FieldMask();
        reg_field->ReloadValue(value, field.second);
      }
    }
  }

  RegisterField* Register::RegisterFieldLookup(const std::string& name) const
  {
    std::vector<RegisterField *>::const_iterator it;
    for (it=mRegisterFields.begin(); it!=mRegisterFields.end(); ++it) {
      if (name.compare((*it)->Name()) == 0)
        return (*it);
    }
    LOG(fail) << "Register " << Name() << " can't find register field " << name << endl;
    FAIL("RegisterField can't find");
    return nullptr;
  }

  uint64 Register::GetRegisterFieldMask(const std::vector<std::string>& field_names) const
  {
    uint64 mask = 0x0ull;
    for (auto field_name : field_names)
    {
      auto reg_field = RegisterFieldLookup(field_name);
      mask |= reg_field->FieldMask();
    }
    return mask;
  }

  uint64 Register::GetPhysicalRegisterMask(const PhysicalRegister& rPhyReg) const
  {
    const string& phy_reg_name = rPhyReg.Name();
    uint64 mask = accumulate(mRegisterFields.cbegin(), mRegisterFields.cend(), uint64(0),
      [&phy_reg_name](cuint64 partialMask, const RegisterField* pRegField) {
        if (pRegField->PhysicalRegisterName() == phy_reg_name) {
          return (partialMask | pRegField->GetPhysicalRegisterMask());
        }

        return partialMask;
      });

    return mask;
  }

  const std::map<std::string, RegisterField*> Register::GetRegisterFieldsFromMask(uint64 mask) const
  {
    std::map<std::string, RegisterField*> reg_field_map;
    for (auto reg_field : mRegisterFields)
    {
      if ((reg_field->FieldMask() & mask) != 0)
      {
        reg_field_map.insert(std::make_pair(reg_field->Name(), reg_field));
      }
    }
    return reg_field_map;
  }

  void Register::GetPhysicalRegisters(std::set<PhysicalRegister*>& phyRegisterSet) const
  {
    for (auto field_it = mRegisterFields.begin(); field_it != mRegisterFields.end(); ++field_it)
    {
      (*field_it)->GetPhysicalRegisters(phyRegisterSet);
    }
  }

  bool Register::HasAttribute(ERegAttrType attr) const
  {
    bool all_reg_fields_have_attribute = all_of(mRegisterFields.cbegin(), mRegisterFields.cend(),
      [attr](const RegisterField* pRegField) { return pRegField->HasAttribute(attr); });

    return all_reg_fields_have_attribute;
  }

  void Register::SetAttribute(ERegAttrType attr)
  {
    std::vector<RegisterField *>::iterator it;
    for (it=mRegisterFields.begin(); it!=mRegisterFields.end(); ++it) {
      (*it)->SetAttribute(attr);
    }
  }

  void Register::ClearAttribute(ERegAttrType attr)
  {
    std::vector<RegisterField *>::iterator it;
    for (it=mRegisterFields.begin(); it!=mRegisterFields.end(); ++it) {
      (*it)->ClearAttribute(attr);
    }
  }

  void Register::Block() const
  {
    for (auto it = mRegisterFields.begin(); it != mRegisterFields.end(); ++it)
    {
      (*it)->Block();
    }
  }

  void Register::Unblock() const
  {
    for (auto it = mRegisterFields.begin(); it != mRegisterFields.end(); ++it)
    {
      (*it)->Unblock();
    }
  }

  void Register::Initialize(uint64 value)
  {
    Block(); // only sendout notification events when the whole initialization is done.
    std::vector<RegisterField *>::iterator it;

    for (it=mRegisterFields.begin(); it!=mRegisterFields.end(); ++it) {
      (*it)->Initialize(value);
    }
    Unblock();
  }

  void Register::Initialize(std::vector<uint64> values)
  {
    if (values.size() == 0)
    {
      LOG(fail) << "attempting to initialize " << mName << " with empty vector of values" << endl;
      FAIL("register initialize with empty vector of values");
    }
    else
    {
      Initialize(values[0]);
    }
  }

  void Register::InitializeRandomly(const ChoicesModerator* pChoicesModerator )
  {
    Block(); // block notifications

    // Check if there is special init policy specified.
    if ((nullptr != mpInitPolicyInfo) and (nullptr != mpInitPolicyInfo->mpInitPolicy)) {
      mpInitPolicyInfo->mpInitPolicy->InitializeRegister(this);
    }
    else {
      for (auto reg_fld : mRegisterFields) {
        ChoiceTree* pChoiceTree = nullptr;

        pChoiceTree = GetChoiceTree( reg_fld->Name(), pChoicesModerator);
        std::unique_ptr<ChoiceTree> storage_ptr(pChoiceTree);

        reg_fld->InitializeFieldRandomly(pChoiceTree);
      }
    }

    Unblock(); // unblock notifications
  }

  void Register::Setup(const RegisterFile* pRegisterFile)
  {
    std::vector<RegisterField *>::iterator it;
    for (it=mRegisterFields.begin(); it!=mRegisterFields.end(); ++it) {
      (*it)->Setup(pRegisterFile);
    }

    if (nullptr != mpInitPolicyInfo) {
      mpInitPolicyInfo->Setup(pRegisterFile);
    }

  }

  ChoiceTree* Register::GetChoiceTree(const string& fieldName, const ChoicesModerator* pChoicesModerator) const
  {
    string fieldFullName;
    fieldFullName = RealName() + '.' + fieldName;

    return pChoicesModerator ? pChoicesModerator->TryCloneChoiceTree(fieldFullName) : nullptr;
  }

  void RegisterNoAlias::SetName(const std::string& rName)
  {
    mName = rName;
    string::size_type pos = rName.find("_NoAlias");
    if (pos != string::npos) {
      mRealName = rName.substr(0, pos);
    }
    else {
      mRealName = rName;
    }
  }

  /*! \class LargeRegister
      \brief Logical Register whose size is larger than 64 bits
   */
  LargeRegister::LargeRegister() : Register()
  {
  }

  LargeRegister::LargeRegister(const LargeRegister& rOther)
    : Register(rOther)
  {
  }

  LargeRegister::LargeRegister(const Register& rOther): Register(rOther)
  {
  }

  LargeRegister::~LargeRegister()
  {
  }

  Object* LargeRegister::Clone() const
  {
    return new LargeRegister(*this);
  }

  std::vector<uint64> LargeRegister::Values() const
  {
    std::vector<uint64> values;
    for (auto reg_field_ptr : mRegisterFields)
    {
      if (reg_field_ptr->IsInitialized())
      {
        values.push_back(reg_field_ptr->Value());
      }
    }
    return values;
  }

  void LargeRegister::Initialize(std::vector<uint64> values)
  {
    Block(); // only sendout notification events when the whole initialization is done.

    if (values.size() > mRegisterFields.size())
    {
      LOG(fail) << "Value is too large to fit inside " << mName << "'s fields. value_length="
        << 64*values.size() << " reg_fields_length=" << 64*mRegisterFields.size() << endl;
      FAIL("vector init values mismatch with register size");
    }
    else if (values.size() == 0)
    {
      LOG(fail) << "Attempting to initialize " << mName << " with empty vector of values" << endl;
      FAIL("register initialize with empty vector of values");
    }

    uint32 field_index = 0;
    for (auto value : values)
    {
      mRegisterFields[field_index]->Initialize(value);
      ++field_index;
    }

    Unblock();
  }

  std::vector<uint64> LargeRegister::InitialValues() const
  {
    std::vector<uint64> values;
    for (auto reg_field_ptr : mRegisterFields)
    {
      if (reg_field_ptr->IsInitialized())
      {
        values.push_back(reg_field_ptr->InitialValue());
      }
    }
    return values;
  }

  std::vector<uint64> LargeRegister::ReloadValues() const
  {
    std::vector<uint64> values;
    Random* random = Random::Instance();
    uint64 max_value = get_mask64(mSize);
    for (uint64 reg_field_index = 0; reg_field_index < mRegisterFields.size(); reg_field_index++)
    {
      values.push_back(random->Random64(0, max_value));
    }

    return values;
  }

  /*! \class ReadOnlyRegister
      \brief Logical Register which is specified as Read Only
   */
  void ReadOnlyRegister::SetValue(uint64 value)
  {
    //TODO
    if (mTakeException) {
      //take exception when write to this register
    } else {
      //Write Ignore
    }
  }

  uint64 ReadOnlyRegister::ReloadValue()const
  {
    // reload value stay the same
    return Value();
  }

  uint64 ReadOnlyRegister::ReloadValue(const ChoicesModerator* pChoicesModerator, const map<string, ConstraintSet* >& fieldConstraints) const
  {
    // reload value stay the same
    return Value();
  }

  void ReadOnlyRegister::InitializeRandomly(const ChoicesModerator* pChoicesModerator)
  {
    // ignored.
  }

  void ReadOnlyRegister::Setup(const RegisterFile* pRegisterFile)
  {
     Register::Setup( pRegisterFile);

     pRegisterFile->RegisterReadOnlyRegister(this);
  }

  ReadOnlyRegister::ReadOnlyRegister() : Register(), mTakeException(false)
  {
    initiate_TakeException();
  }

  ReadOnlyRegister::ReadOnlyRegister(const ReadOnlyRegister& rOther)
    : Register(rOther), mTakeException(rOther.mTakeException)
  {
  }

  ReadOnlyRegister::~ReadOnlyRegister()
  {
  }

  Object* ReadOnlyRegister::Clone() const
  {
    return new ReadOnlyRegister(*this);
  }

  void ReadOnlyRegister::initiate_TakeException()
  {
    mTakeException = false;
  }

  /*! \class ReadOnlyZeroRegister
      \brief ReadOnly implementation for Zero Register
   */
  void ReadOnlyZeroRegister::Setup(const RegisterFile* pRegisterFile)
  {
    Register::Setup( pRegisterFile);
    Initialize(0);
  }

  /*! \class BankedRegister
      \brief Combination of different logical register into single logical register object

      Used to create a single logical register with combination of fields from multiple registers.
      Currently an abstract class.
  */
  BankedRegister::BankedRegister(): Register(), Receiver(), mBankIndex(0), mOriginalBankIndex(0), mOriginalSize(0), mRegisterMap(), mOriginalRegisterFields()
  {
  }

  BankedRegister::BankedRegister(const BankedRegister& rOther): Register(rOther), Receiver(rOther),
               mBankIndex(0), mOriginalBankIndex(0), mOriginalSize(0), mRegisterMap(), mOriginalRegisterFields()
  {
  }

  void BankedRegister::Setup(const RegisterFile* pRegisterFile)
  {
    Register::Setup(pRegisterFile);

    mOriginalBankIndex = OriginalBankIndex();
    mBankIndex         = mOriginalBankIndex;
    mOriginalSize      = mSize;
    mOriginalRegisterFields = mRegisterFields;
  }

  void BankedRegister::HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload)
  {
    UpdateBankIndex(sender, pPayload);
    UpdateRegisterFields();
  }

  void BankedRegister::UpdateRegisterFields()
  {
    if (mBankIndex == mOriginalBankIndex) { //use the original values if bank index reset to default
      mRegisterFields = mOriginalRegisterFields;
      mSize           = mOriginalSize;
    }
    else { //lookup new fields/size based on non-default bank index
      auto find_iter  = mRegisterMap.find(mBankIndex);
      if (find_iter == mRegisterMap.end()) {
        BankNotExist(mBankIndex); // report bank not exist.
      }
      else {
        Register* reg   = find_iter->second;
        mRegisterFields = reg->RegisterFields();
        mSize           = reg->Size();
        LOG(info) << "{BankedRegister::UpdateRegisterFields} Orignal register name:" << Name() << "--> new Register:" << reg->Name() << endl;
      }
    }

  }

  void BankedRegister::BankNotExist(uint32 bank)
  {
    LOG(fail) << "{BankedRegister::BankNotExist} bank " << dec << bank << " does not exist." << endl;
    FAIL("register-bank-not-exist");
  }

  SelectorBankedRegister::SelectorBankedRegister(const std::string& rRegName, uint64 fieldMask, uint32 fieldShift)
    : BankedRegister(), mFieldMask(fieldMask), mFieldShift(fieldShift), mSelectorRegister(rRegName), mpSelectorRegister(nullptr)
  {
  }

  SelectorBankedRegister::SelectorBankedRegister (const SelectorBankedRegister& rOther)
    : BankedRegister(rOther), mFieldMask(rOther.mFieldMask), mFieldShift(rOther.mFieldShift), mSelectorRegister(rOther.mSelectorRegister), mpSelectorRegister(nullptr)
  {
  }

  SelectorBankedRegister::SelectorBankedRegister()
    : BankedRegister(), mFieldMask(0), mFieldShift(0), mSelectorRegister(), mpSelectorRegister(nullptr)
  {
  }

  void SelectorBankedRegister::Setup(const RegisterFile* pRegisterFile)
  {
    BankedRegister::Setup(pRegisterFile);

    auto pm_sel_reg = dynamic_cast<ConfigureRegister* >(pRegisterFile->PhysicalRegisterLookup(mSelectorRegister));
    pm_sel_reg->SignUp(this);
  }

  void SelectorBankedRegister::GetRelyUponRegisters(std::vector<std::string>& rRegNames) const
  {
    rRegNames.push_back(mSelectorRegister);
  }

  void SelectorBankedRegister::UpdateBankIndex(const NotificationSender* pSender,  Object* payload)
  {
    auto phys_reg = dynamic_cast<const PhysicalRegister* >(pSender);

    if (nullptr == phys_reg) {
      LOG(fail) << "{SelectorBankedRegister::UpdateBankIndex} expecting sender to be a PhysicalRegister." << endl;
      FAIL("unexpected-sender-type");
    }

    mBankIndex = (phys_reg->Value(phys_reg->Mask()) >> mFieldShift) & mFieldMask;

    // << "reg " << Name() << " gotten update from " << phys_reg->Name() << " bank index now: " << dec << mBankIndex << " reg value: 0x" << phys_reg->Value(phys_reg->Mask()) << " field mask 0x" << mFieldMask << " shift " << mFieldShift << endl;
  }

  SelectorBankedRegisterRAZWI::SelectorBankedRegisterRAZWI(const std::string& rRegName, uint64 fieldMask, uint32 fieldShift)
    : SelectorBankedRegister(rRegName, fieldMask, fieldShift), mpRazwiRegister(nullptr)
  {
  }

  SelectorBankedRegisterRAZWI::SelectorBankedRegisterRAZWI()
    : SelectorBankedRegister(), mpRazwiRegister(nullptr)
  {
  }

  SelectorBankedRegisterRAZWI::SelectorBankedRegisterRAZWI (const SelectorBankedRegisterRAZWI& rOther)
    : SelectorBankedRegister(rOther), mpRazwiRegister(nullptr)
  {
  }

  SelectorBankedRegisterRAZWI::~SelectorBankedRegisterRAZWI()
  {
    mpRazwiRegister = nullptr;
  }

  void SelectorBankedRegisterRAZWI::Setup(const RegisterFile* pRegFile)
  {
    SelectorBankedRegister::Setup(pRegFile);

    mpRazwiRegister = pRegFile->RazwiRegister(Size());
  }

  void SelectorBankedRegisterRAZWI::BankNotExist(uint32 bank)
  {
    mBankIndex = mFieldMask + 1; // use the value that is out of the normal range to represent the invalid bank.
    mRegisterFields = mpRazwiRegister->RegisterFields();
    mSize           = mpRazwiRegister->Size();
  }

  /*!
    \class RegisterFile
    \brief Top Level Register class containing all parsed XML files registers and hierarchy

    Currently when cloning or creating a new RegisterFile object, Setup() needs to be called
    to ensure all maps and pointers inside of subobjects are populated correctly.

    Regarding the RWConstraintSets related members, an architecture dependent mechanism
    is provided to accelerate the constraint lookup when taking register revervation into account.
    Every operand type has one position in the mRwReservationLookUp vector which is populated in Setup().
  */
  RegisterFile::RegisterFile()
    : Object(), Sender(), mName(), mRegisters(), mPhysicalRegisters(), mPhysicalRegisterLinks(), mInitPolicies(), mRegIndex2Name(), mReadOnlyRegisters(), mReadOnlyRegisterFields(), mpConditionSet(nullptr),mpRegisterReserver(nullptr)
  {

  }

  RegisterFile::RegisterFile(const RegisterFile& rOther)
    : Object(rOther), Sender(), mName(rOther.mName), mRegisters(), mPhysicalRegisters(), mPhysicalRegisterLinks(), mInitPolicies(), mRegIndex2Name(), mReadOnlyRegisters(), mReadOnlyRegisterFields(), mpConditionSet(nullptr), mpRegisterReserver(nullptr)
  {
    std::map<std::string, Register*>::const_iterator reg_it;
    std::map<std::string, PhysicalRegister*>::const_iterator phy_it;

    for (phy_it=rOther.mPhysicalRegisters.begin(); phy_it!=rOther.mPhysicalRegisters.end(); ++phy_it)
    {
      mPhysicalRegisters[phy_it->first] = dynamic_cast<PhysicalRegister* >(phy_it->second->Clone());
    }
    for (reg_it=rOther.mRegisters.begin(); reg_it!=rOther.mRegisters.end(); ++reg_it)
    {
      mRegisters[reg_it->first] = dynamic_cast<Register* >(reg_it->second->Clone());
    }

    mRegIndex2Name = rOther.mRegIndex2Name;
    mPhysicalRegisterLinks = rOther.mPhysicalRegisterLinks;

    for (auto map_iter : rOther.mInitPolicies) {
      mInitPolicies[map_iter.first] = dynamic_cast<RegisterInitPolicy* >(map_iter.second->Clone());
    }
  }

  RegisterFile::~RegisterFile()
  {
    mpConditionSet = nullptr;

    std::map<std::string, Register*>::iterator reg_it;
    std::map<std::string, PhysicalRegister*>::iterator phy_it;
    for (phy_it=mPhysicalRegisters.begin(); phy_it!=mPhysicalRegisters.end(); ++phy_it)
    {
      delete phy_it->second;
    }

    for (reg_it=mRegisters.begin(); reg_it!=mRegisters.end(); ++reg_it)
    {
      delete reg_it->second;
    }

    for (auto map_iter : mInitPolicies) {
      delete map_iter.second;
    }

    delete mpRegisterReserver;
  }

  Object* RegisterFile::Clone() const
  {
    return new RegisterFile(*this);
  }

  const string RegisterFile::ToString() const
  {
    return mName;
  }

  void RegisterFile::LoadRegisterFiles(const std::list<std::string>& fileNames)
  {
    auto cfg_ptr = Config::Instance();
    for (auto const& rfile_name : fileNames) {
      RegisterParser reg_parser(this);
      string full_file_path = cfg_ptr->LookUpFile(rfile_name);
      parse_xml_file(full_file_path, "register", reg_parser);
    }
  }

  void RegisterFile::Setup(const GenConditionSet* pCondSet, const string& unpredict_registers)
  {
    mpConditionSet = pCondSet;

    std::map<std::string, Register*>::const_iterator reg_it;
    for (reg_it=mRegisters.begin(); reg_it!=mRegisters.end(); ++reg_it)
    {
      reg_it->second->Setup(this);
    }

    SetupPhysicalRegisterLinks();
    SetupRegisterReserver();
    SetupUnpredictRegister(unpredict_registers);
  }

  void RegisterFile::SetupUnpredictRegister(const std::string& unpredict_registers)
  {
    StringSplitter splitter(unpredict_registers, ',');
    while (not splitter.EndOfString()) {
      auto reg_name = splitter.NextSubString();
      auto reg = RegisterLookup(reg_name);
      reg->SetAttribute(ERegAttrType::Unpredictable);
    }
  }

  void RegisterFile::SetupPhysicalRegisterLinks()
  {
    for (auto& link : mPhysicalRegisterLinks)
    {
      PhysicalRegister* start = PhysicalRegisterLookup(link.first);
      PhysicalRegister* end   = PhysicalRegisterLookup(link.second);
/*    if (strcmp(start->Type(),"LinkedPhysicalRegister") != 0)
      {
        LOG(fail) << "{RegisterFile::SetupPhysicalRegisterLinks} setup on non-linked phy reg reg_name=" << link.first << endl;
        FAIL("setup_link_for_non_linked_phy_reg");
      }*/

      dynamic_cast<LinkedPhysicalRegister* >(start)->mpPhysicalLink = end;
    }
  }

  Register* RegisterFile::RegisterLookup(const std::string& name) const
  {
    auto find_iter = mRegisters.find(name);
    if (find_iter == mRegisters.end()) {
      LOG(fail) << "Register lookup with name \"" << name << "\" not found." << endl;
      FAIL("register-look-up-by-name-fail");
    }
    return find_iter->second;
  }

  PhysicalRegister* RegisterFile::PhysicalRegisterLookup(const std::string& name) const
  {
    auto find_iter = mPhysicalRegisters.find(name);
    if (find_iter == mPhysicalRegisters.end()) {
      LOG(fail) << "Physical Register lookup with name \"" << name << "\" not found." << endl;
      FAIL("physical-register-look-up-by-name-fail");
    }
    return find_iter->second;
  }

  uint64 RegisterFile::GetRegisterFieldMask(const std::string& reg_name, const std::vector<std::string>& field_names) const
  {
    return RegisterLookup(reg_name)->GetRegisterFieldMask(field_names);
  }

  const std::string& RegisterFile::Name() const
  {
    return mName;
  }

  const std::map<std::string, RegisterField*> RegisterFile::GetRegisterFieldsFromMask(const std::string& reg_name, uint64 mask) const
  {
    return RegisterLookup(reg_name)->GetRegisterFieldsFromMask(mask);
  }

  void RegisterFile::AddPhyReg(PhysicalRegister* phy_reg_ptr, const string& rLink)
  {
    string pr_name = phy_reg_ptr->Name();
    LOG(trace) << "Adding new physical register " << pr_name << endl;

    const auto find_iter = mPhysicalRegisters.find(pr_name);
    if (find_iter != mPhysicalRegisters.end())
    {
      LOG(warn) << "Duplicated physical register name \'" << pr_name << "\'." << endl;
      delete phy_reg_ptr;
    }
    else
    {
      mPhysicalRegisters[pr_name] = phy_reg_ptr;
      uint32 index;
      index = ( ((uint32) phy_reg_ptr->RegisterType()) <<28) + phy_reg_ptr->IndexValue();
      mRegIndex2Name[index] = pr_name;
    }

    if (rLink != "")
    {
      auto link_iter = mPhysicalRegisterLinks.find(pr_name);
      if (link_iter != mPhysicalRegisterLinks.end())
      {
        LOG(fail) << "{RegisterFile::AddPhyReg} register has more than one link reg=" << pr_name << endl;
        FAIL("phy_reg_multiple_links_defined");
      }
      else
      {
        mPhysicalRegisterLinks[pr_name] = rLink;
      }
    }
  }

  void RegisterFile::AddRegister(Register* register_ptr)
  {
    string r_name = register_ptr->Name();
    LOG(trace) << "Adding new register " << r_name << endl;

    const auto find_iter = mRegisters.find(r_name);
    if (find_iter != mRegisters.end()) {
      LOG(fail) << "Duplicated register name \'" << r_name << "\'." << endl;
      FAIL("duplicated-register-name");
    } else {
      mRegisters[r_name] = register_ptr;
    }
  }

  void RegisterFile::AddInitPolicy(RegisterInitPolicy* pInitPolicy)
  {
    auto policy_type = pInitPolicy->Type();
    delete (mInitPolicies[policy_type]);
    mInitPolicies[policy_type] = pInitPolicy;
  }

  const RegisterInitPolicy* RegisterFile::GetInitPolicy(const std::string& rPolicyName) const
  {
    auto find_iter = mInitPolicies.find(rPolicyName);
    const RegisterInitPolicy* ret_ptr = nullptr;

    if (find_iter == mInitPolicies.end()) {
      LOG(notice) << "{RegisterFile::GetInitPolicy} policy not found: \"" << rPolicyName << "\"." << endl;
      //LOG(fail) << "{RegisterFile::GetInitPolicy} policy not found: \"" << rPolicyName << "\"." << endl;
      //FAIL("policy-not-found");
    }
    else {
      ret_ptr = find_iter->second;
    }
    return ret_ptr;
  }

  bool RegisterFile::GetRandomRegisters(cuint32 number, const ERegisterType regType, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const
  {
    return GetRandomRegistersForAccess(number, regType, ERegAttrType::ReadWrite, rExcludes, rRegIndices);
  }

  bool RegisterFile::GetRandomRegistersForAccess(cuint32 number, const ERegisterType regType, const ERegAttrType access, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const
  {
    ConstraintSet reg_constr;
    mpRegisterReserver->UsableIndexConstraint(regType, access, &reg_constr);

    if (rExcludes.size() > 0) {
      ConstraintSet exclude_constr(rExcludes);
      reg_constr.SubConstraintSet(exclude_constr);
    }

    vector<uint64> value_list;
    reg_constr.GetValues(value_list);

    if (value_list.size() < number) {
      LOG(warn) << "{Generator::GetRandomRegisters} not enough " << ERegisterType_to_string(regType) << " random registers available, requesting: " << dec << number << " available: " << value_list.size() << endl;
      return false;
    }

    RandomURBG32 urbg32(Random::Instance());
    std::shuffle(value_list.begin(), value_list.end(), urbg32);

    rRegIndices.insert(rRegIndices.begin(), value_list.begin(), value_list.begin() + number);
    return true;
  }

  void RegisterFile::InitializeRelyUponRegisters(const Register* pRegister, const ChoicesModerator* pChoicesModerator) const
  {
    vector<string> reg_names;
    pRegister->GetRelyUponRegisters(reg_names);

    if (reg_names.empty())
      return;

    for (const auto & name_ref : reg_names) {
      Register* rely_upon_reg = RegisterLookup(name_ref);
      if (not rely_upon_reg->IsInitialized()) {
        LOG(notice) << "{RegisterFile::InitializeRelyUponRegisters} initializing rely-upon register: " << name_ref << " for: " << pRegister->Name() << endl;
        InitializeRegisterRandomly(rely_upon_reg, pChoicesModerator);
      }
    }
  }

  void RegisterFile::CheckLinkRegisterInit(const string& rRegName) const
  {
    Register* reg_ptr = RegisterLookup(rRegName);
    std::set<PhysicalRegister* > phy_regs;
    reg_ptr->GetPhysicalRegisters(phy_regs);

    for (auto& phy_reg : phy_regs)
    {
      auto link_reg_it = mPhysicalRegisterLinks.find(phy_reg->Name());
      if (link_reg_it != mPhysicalRegisterLinks.end())
      {
        Register* link_reg_ptr = RegisterLookup(link_reg_it->second);
        Sender::SendNotification(ENotificationType::RegisterInitiation, link_reg_ptr);
      }
    }
  }

  void RegisterFile::InitializeRegister(const string& rRegName, const uint64 value, const ChoicesModerator* pChoicesModerator) const
  {
    Register* reg_ptr = RegisterLookup(rRegName);

    // ensure rely upon registers are initialized.
    InitializeRelyUponRegisters(reg_ptr, pChoicesModerator);

    //If register already initialized, this step will fail, so no need to call IsInitialized here.
    reg_ptr->Initialize(value);
    Sender::SendNotification(ENotificationType::RegisterInitiation, reg_ptr);
    CheckLinkRegisterInit(rRegName);
  }

  void RegisterFile::InitializeRegister(const string& rRegName,
                                        const std::vector<uint64>& values,
                                        const ChoicesModerator* pChoicesModerator) const
  {
    Register* reg_ptr = RegisterLookup(rRegName);

    // ensure rely upon registers are initialized.
    InitializeRelyUponRegisters(reg_ptr, pChoicesModerator);

    //If register already initialized, this step will fail, so no need to call IsInitialized here.
    reg_ptr->Initialize(values);
    Sender::SendNotification(ENotificationType::RegisterInitiation, reg_ptr);
    CheckLinkRegisterInit(rRegName);
  }

  void RegisterFile::InitializeRegisterRandomly(const string& rRegName, const ChoicesModerator* pChoicesModerator) const
  {
    Register* reg_ptr = RegisterLookup(rRegName);

    // ensure rely upon registers are initialized.
    InitializeRelyUponRegisters(reg_ptr, pChoicesModerator);

    reg_ptr->InitializeRandomly(pChoicesModerator);
    Sender::SendNotification(ENotificationType::RegisterInitiation, reg_ptr);
    CheckLinkRegisterInit(rRegName);
  }

  void RegisterFile::InitializeRegisterRandomly(Register* pReg, const ChoicesModerator* pChoicesModerator) const
  {
    // ensure rely upon registers are initialized.
    InitializeRelyUponRegisters(pReg, pChoicesModerator);

    pReg->InitializeRandomly(pChoicesModerator);
    Sender::SendNotification(ENotificationType::RegisterInitiation, pReg);
    CheckLinkRegisterInit(pReg->Name());
  }

  const RegisterField* RegisterFile::InitializeRegisterFieldRandomly(Register* pRegister, const string& rFieldName, const ChoicesModerator* pChoicesModerator) const
  {
    // ensure rely upon registers are initialized.
    InitializeRelyUponRegisters(pRegister, pChoicesModerator);

    RegisterField* reg_field = pRegister->RegisterFieldLookup(rFieldName);

    if ( (not reg_field->IsInitialized())) {

      ChoiceTree* choice_tree = pRegister->GetChoiceTree(rFieldName, pChoicesModerator);
      std::unique_ptr<ChoiceTree> storage_ptr(choice_tree);

      reg_field->InitializeFieldRandomly(choice_tree);
      Sender::SendNotification(ENotificationType::RegisterInitiation, pRegister);
      CheckLinkRegisterInit(pRegister->Name());
    }

    return reg_field;
  }

  const RegisterField* RegisterFile::InitializeRegisterField(Register* pRegister, const string& rFieldName, const uint64 value, const ChoicesModerator* pChoicesModerator) const
  {
    // ensure rely upon registers are initialized.
    InitializeRelyUponRegisters(pRegister, pChoicesModerator);

    // if register field already initialized, it will fail, so no need to check IsInitialized here.
    RegisterField* reg_field = pRegister->RegisterFieldLookup(rFieldName);
    reg_field->InitializeField(value);
    Sender::SendNotification(ENotificationType::RegisterInitiation, pRegister);
    CheckLinkRegisterInit(pRegister->Name());
    return reg_field;
  }

  const RegisterField* RegisterFile::InitializeRegisterFieldFullValue(Register* pRegister, const string& rFieldName, const uint64 value, const ChoicesModerator* pChoicesModerator) const
  {
    // ensure rely upon registers are initialized.
    InitializeRelyUponRegisters(pRegister, pChoicesModerator);

    // if register field already initialized, it will fail, so no need to check IsInitialized here.
    RegisterField* reg_field = pRegister->RegisterFieldLookup(rFieldName);
    reg_field->Initialize(value);
    Sender::SendNotification(ENotificationType::RegisterInitiation, pRegister);
    CheckLinkRegisterInit(pRegister->Name());
    return reg_field;
  }

  void RegisterFile::SetPhysicalRegisterValueAndInit(PhysicalRegister* pPhysRegister, uint64 valueToSet, uint64 valueMask, uint64 initValue, bool notifyInit) const
  {
    if (notifyInit) {
      // TODO currently not expecting use case with notifyInit=true.
      LOG(fail) << "{RegisterFile::SetPhysicalRegisterValueAndInit} notifyInit=true use case not expected currently." << endl;
      FAIL("unexpected-use-case");
    }

    pPhysRegister->Initialize(initValue, valueMask);
    pPhysRegister->SetValue(valueToSet, valueMask);
  }

  void RegisterFile::RegisterReadOnlyRegister(ReadOnlyRegister* pReadOnlyReg) const
  {
    mReadOnlyRegisters.push_back(pReadOnlyReg);
  }

  void RegisterFile::InitializeReadOnlyRegistersNoISS()
  {
    for (auto readonly_reg : mReadOnlyRegisters) {
      // read only register should hold a reset value, if that is the case, then we don't need the above code handing reset-value in PhysicalRegister.
      uint64 reset_value = readonly_reg->Value();
      readonly_reg->Initialize(reset_value);
    }
  }

  void RegisterFile::InitializeReadOnlyRegisterFromIss(ReadOnlyRegister* pRoReg, uint64 value) const
  {
    pRoReg->Initialize(value);
  }

  const Register* RegisterFile::RazwiRegister(uint32 size) const
  {
    string razwi_reg_name;
    switch (size) {
    case 32:
    case 64:
      break;
    default:
      LOG(fail) << "{RegisterFile::RazwiRegister} unsupported RAZ/WI register size: " << dec << size << endl;
      FAIL("unsupported-razwi-register-size");
    }
    razwi_reg_name = string("FORCE_RAZWI_REGISTER") + std::to_string(size);
    return RegisterLookup(razwi_reg_name);
  }

  void RegisterFile::RegisterReadOnlyRegisterField(ReadOnlyRegisterField* pReadOnlyRegField) const
  {
    mReadOnlyRegisterFields.push_back(pReadOnlyRegField);
  }

  void RegisterFile::ResetReadOnlyRegisterFieldFromIss(ReadOnlyRegisterField* pRoRegField, uint64 value)
 const
  {
    pRoRegField->SetFieldResetValue(value);
  }

}
