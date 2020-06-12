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
#ifndef Force_PteStructure_H
#define Force_PteStructure_H

#include <Defines.h>

#include <vector>
#include <string>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <FieldEncoding.h>

namespace Force {

  class PteAttributeStructure;

  /*!
    \class PteStructure
    \brief Record static structural information about an pte.
  */
  class PteStructure {
  public:
    explicit PteStructure(const std::string & pteClass) : mClass(pteClass), mType(EPteType(0)), mCategory(EPteCategoryType(0)), mGranule(EPageGranuleType(0)), mLevel(0), mStage(0), mAttributeStructures(), mSize(0) { } //!< Constructor with PTE class given.
    ~PteStructure(); //!< Destructor.

    const std::string FullId() const; //!< Return an unique string ID for the PTE type.
    void AddAttribute(PteAttributeStructure* pAttrStruct); //!< Add a PteAttributeStructure object.
    uint32 Size() const { return mSize; } //!< Return the size of the PTE.
    const std::vector<PteAttributeStructure* >& AttributeStructures() const { return  mAttributeStructures; } //!< Return a const reference to the attribute structures.
  public:
    std::string mClass; //!< Associated PTE class.
    EPteType mType; //!< Type of the PTE.
    EPteCategoryType mCategory; //!< Category of the PTE.
    EPageGranuleType mGranule; //!< Granule of the PTE.
    uint32 mLevel; //!< Level of the PTE.
    uint32 mStage; //!< Stage of the PTE.
  private:
    PteStructure() : mClass(), mType(EPteType(0)), mCategory(EPteCategoryType(0)), mGranule(EPageGranuleType(0)), mLevel(0), mStage(0), mAttributeStructures(), mSize(0) { } //!< Default constructor, empty, private.
    PteStructure(const PteStructure& rOther); //!< Copy constructor, private.
  private:
    std::vector<PteAttributeStructure* > mAttributeStructures; //!< Static structures of the PTE attributes.
    uint32 mSize; //!< Size of the PTE in number of bits.

    friend class PagingParser;
  };

  /*!
    \class PteAttributeStructure
    \brief Record static structural information about a PTE attribute.
  */
  class PteAttributeStructure {
  public:
    explicit PteAttributeStructure(const std::string& rClass) : mClass(rClass), mTypeText(), mType(EPteAttributeType(0)), mSize(0), mValue(0), mLsb(0), mMask(0), mFactory(false), mEncodingBits() { } //!< Constructor
    ~PteAttributeStructure() { } //!< Destructor.

    void SetBits(const std::string& bitsStr); //!< Assign the PTE bits occupied by the PTE attribute.
    inline const std::string& Class() const { return mClass; } //!< Return PteAttribute class name if any.
    uint64 Encoding(uint64 pteValue) const; //!< Encode the PTE attribute value into its occupied bits.
    void SetValue(uint32 value) { mValue = value; } //!< Set PTE attribute constant value.
    inline uint32 Value() const { return mValue; } //!< Return PTE attribute constant value.
    inline bool Factory() const { return mFactory; } //!< Return whether the PteAttribute should be created from factory.
    inline uint32 Lsb() const { return mLsb; } //!< Return Lsb value.
    inline uint64 Mask() const { return mMask; } //!< Return mask value.
    inline EPteAttributeType Type() const { return mType; } //!< Return PteAttribute type.
    inline const std::string& TypeText() const { return mTypeText; } //!< Return PteAttribute type in text.
  private:
    PteAttributeStructure(const PteAttributeStructure& rOther) = delete; //!< Copy constructor not implemented.
    PteAttributeStructure() = delete; //!< Default constructor not implemented.
  private:
    std::string mClass; //!< Associated PTE attribute class.
    std::string mTypeText; //!< String representation of the type of the PTE attribute.
    EPteAttributeType mType; //!< Type of the PTE attribute.
    uint32 mSize; //!< Size of operand field.
    uint32 mValue; //!< Constant value if applicable.
    uint32 mLsb; //!< LSB bit if applicable.
    uint64 mMask; //!< Pte attribute size-mask used in randomization.
    bool mFactory; //!< Use factory to construct.
    std::vector<EncodingBits> mEncodingBits; //!< PTE bits of the the PTE attribute object.

    friend class PagingParser;
    friend class PteStructure;
    friend class PteAttribute;
  };

}

#endif
