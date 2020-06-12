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
#ifndef Force_Data_H
#define Force_Data_H

#include <Defines.h>
#include <Object.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>
#include <memory>

namespace Force {

  class ConstraintSet;
  class DataPattern;
  class Generator;
  class ChoiceTree;
  class Register;

  /*!
    \class Data
    \brief A data virtual interface
  */
  class Data : public Object {
  public:
    Data() : Object(), mDataType(EDataType::INT32) { } //!< constructor
    Data(const Data& rOther) :Object(rOther), mDataType(rOther.mDataType) { } //!< copy constructor
    ~Data() {} //!< destructor

    EDataType GetDataType() const {return mDataType; } //!< Return data type
    virtual uint32 GetDataWidth() const { return 1u << (uint32(mDataType) & 0xf); } //!< return data width in bytes
    virtual uint64 ChooseData() const = 0; //!< choose a data from the constraint.
    virtual void ChooseLargeData(std::vector<uint64>& rDatas) const = 0; // choose large data from the constraint
    virtual bool HasConstraint() const = 0; //!< whether has data constraint
    virtual void Setup(const Generator& gen ) = 0; //!< set up data pattern
  protected:
    EDataType mDataType; //!< data type
  };

  /*!
    \class IntData
    \brief A data  module to contain int or fix data
  */
  class IntData : public Data {
  public:
    explicit IntData(const std::string& valueStr); //!< constructor
    ~IntData(); //!< destructor

    ASSIGNMENT_OPERATOR_ABSENT(IntData);
    Object* Clone() const override;  //!< Return a cloned Data object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the Data object.
    const char* Type() const override { return "IntData"; } //!< Return type of the Data object.
    uint64 ChooseData() const override; //!< choose a data from the constraint.
    void ChooseLargeData(std::vector<uint64>& rDatas) const override; // choose large data from the constraint
    bool HasConstraint() const override  { return mpDataConstraint != nullptr; } //!< whether has data constraint
    void Setup(const Generator& gen ) override; //!< set up data pattern
  protected:
    IntData(const IntData& rOther); //!< copy constructor
    void ConstructDataConstraint(const std::string& valueStr); //!< construct data constraint
  private:
    const ConstraintSet* mpDataConstraint; //!< data constraint
    DataPattern* mpDataPattern ; //!< Pointer to data pattern.
    void ParseTypeValue(const std::string& valueStr, EDataType& dataType, std::string& value) const; //!< parse the value string and get its data type and evaluate string.
    uint64 SetupBitPatternConstraint(const std::string& pattern); //!< set up bit pattern constraint
  };

   /*!
    \class FPData
    \brief A data  module to contain Floatpoint data
  */
  class FPData : public Data {
  public:
    explicit FPData(const std::string& valueStr); //!< constructor
    ~FPData(); //!< destructor

    ASSIGNMENT_OPERATOR_ABSENT(FPData);
    Object* Clone() const override;  //!< Return a cloned FPData object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the FPData object.
    const char* Type() const override { return "FPData"; } //!< Return type of the FPData object.
    uint64 ChooseData(void) const override; //!< choose a data from the constraint.
    void ChooseLargeData(std::vector<uint64>& rDatas) const override; // choose large data from the constraint
    bool HasConstraint() const override  { return mpExpConstraint != nullptr; } //!< whether has data constraint
    void Setup(const Generator& gen ) override; //!< set up data pattern
 protected:
    FPData(const FPData& rOther); //!< copy constructor
  private:
    void ParseTypeValue(const std::string& valueStr, EDataType& dataType, std::string& exp, std::string& sign, std::string& frac); //!< parse FP value string
    const ConstraintSet* mpExpConstraint; //!< exponent constraint
    const ConstraintSet* mpSignConstraint; //!< sign constraint
    const ConstraintSet* mpFracConstraint; //!< fraction constraint
    DataPattern* mpDataPattern; //!< Pointer to data pattern.
  };

   /*!
    \class SIMDData
    \brief A data  module to contain data on SIMD register
   */
  struct SIMDDataLane;
  class SIMDData : public Data {
  public:
    explicit SIMDData(const std::string& valueStr); //!< constructor
    ~SIMDData(); //!< destructor

    Object* Clone() const override;  //!< Return a cloned FPData object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the SIMDData object.
    const char* Type() const override { return "SIMDData"; } //!< Return type of the SIMDData object.
    uint64 ChooseData() const override; //!< choose a data in little-signment 64-bits.
    void ChooseLargeData(std::vector<uint64>& rDatas) const override; //!< choose all uint64 datas.
    uint32 GetDataWidth() const override; //!< return data width in bytes
    bool HasConstraint() const override; //!< whether has data constraint
    void Setup(const Generator& gen ) override; //!< set up data pattern
  protected:
    SIMDData(const SIMDData& rOther); //copy constructor
  private:
    uint32 mLanes;  //!< number of lanes
    std::vector<std::shared_ptr<const Data *>> mDatas; //!< container for datas
    bool ParseLanes(const std::string& valueStr, std::vector<SIMDDataLane>& simd_lanes, uint32& lanes); //!< parse simd data lanes
  };

  class DataFactory {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static DataFactory* Instance() { return mspDataFactory; } //!< Access data factory instance.
#ifdef UNIT_TEST
    const Data* BuildData(const std::string& valueStr); //!< build data module
#else
    const Data* BuildData(const std::string& valueStr, const Generator& gen); //!< build data module
#endif
#ifndef UNIT_TEST
    const DataPattern* BuildDataPattern(const Register *pReg, const Generator& gen); //!< build data pattern
#endif
  private:
    DataFactory( ) {} //!< constructor, private
    virtual ~DataFactory( ) {} //!< destructor, private
    static DataFactory* mspDataFactory; //!< Pointer to singleton DataFactory object
  };

   /*!
    \class DataPattern
    \brief A data pattern virtual interface
  */
  class DataPattern : public Data {
  public:
   DataPattern() : Data(), mpChoiceTree(nullptr), mChoiceValue(0), mChoiceText()  { } // constructor
    DataPattern(const DataPattern& rOther) : Data(rOther), mpChoiceTree(nullptr), mChoiceValue(0), mChoiceText() {} // copy constructor
    ~DataPattern() {} //!< destructor

    ASSIGNMENT_OPERATOR_ABSENT(DataPattern);
    bool HasConstraint() const override { return false; }; //!< whether has data constraint
  protected:
    uint64 GetDataMask()  const;  //!< Get data mask
    const ChoiceTree* mpChoiceTree; //!< pointer to choices tree
    mutable uint64 mChoiceValue; //!< choice value
    mutable std::string mChoiceText; //!< choice name
  };

  /*!
    \class RandomDataPattern
    \brief class for Random data pattern
  */
  class RandomDataPattern : public DataPattern {
  public:
    explicit RandomDataPattern(EDataType dataType); //!< constructor
    explicit RandomDataPattern(uint32 dataSize); //!< constructor;
    ~RandomDataPattern() { } //!< destructor

    Object* Clone() const override;  //!< Return a cloned DataPattern object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the Data object.
    const char* Type() const override { return "RandomDataPattern"; } //!< Return type of the Data object.
    uint64 ChooseData() const override; //!< choose a data from the pattern tree.
    void ChooseLargeData(std::vector<uint64>& rDatas) const override; // choose large data from the constraint
    void Setup(const Generator& gen ) override { }; //!< set up data pattern
#ifndef UNIT_TEST
  protected:
#endif
    uint64 ChooseRandom() const; //!< choose random
  protected:
    RandomDataPattern(const RandomDataPattern& rOther) : DataPattern(rOther), mBaseDataNumber(rOther.mBaseDataNumber)  { }; //!< copy constructor
  protected:
    uint32 mBaseDataNumber; //!< number of Base data (64-bit)
  };

  /*!
    \class IntDataPattern
    \brief class for integer data pattern
  */
  class IntDataPattern : public RandomDataPattern {
  public:
    explicit IntDataPattern(EDataType dataType); //!< constructor
    explicit IntDataPattern(uint32 dataSize); //!< constructor;
    ~IntDataPattern(); //!< destructor

    Object* Clone() const override;  //!< Return a cloned DataPattern object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the Data object.
    const char* Type() const override { return "IntDataPattern"; } //!< Return type of the Data object.
    uint64 ChooseData() const override; //!< choose a data from the pattern tree.
    void ChooseLargeData(std::vector<uint64>& rDatas) const override; // choose large data from the constraint
    void Setup(const Generator& gen ) override; //!< set up data pattern
    uint64 GetAlmostSatValue() { return mAlmostSatValue; } //!< get AlmostSatValue value
#ifndef UNIT_TEST
  protected:
#endif
    uint64 ChooseAllOne() const;    //!< choose all ones
    uint64 ChooseAllZero() const;   //!< choose all zeroes
    uint64 ChooseLSBZero() const; //!< choose data LSB is zero, and others are random
    uint64 ChooseLSBOne() const;  //!< choose data LSB is one, and others are random
    uint64 ChooseMSBZero() const; //!< choose data MSB is zero, and others are random
    uint64 ChooseMSBOne() const;  //!< choose data MSB is one, and others are random
    uint64 ChooseSignedSaturate() const; //!< choose signed saturated
    uint64 ChooseUnsignedSaturate() const; //!< choose unsigned saturated
    uint64 ChooseSignedAlmostSat() const;  //!< choose signed almost saturated
    uint64 ChooseUnsignedAlmostSat() const; //!< choose unsigned almost saturated
    uint64 ChooseSignedSatPlugOne() const;  //!< choose signed saturated plug one
    uint64 ChooseUnsignedSatPlugOne() const;//!< choose unsigned satruated plug one
  protected:
    IntDataPattern(const IntDataPattern& rOther);
    const uint64 mAlmostSatValue; //!< almost saturate const value
  };

  /*!
    \class FpDataPattern
    \brief class for floating point data pattern
  */
  class FpDataPattern : public RandomDataPattern {
  public:
    explicit FpDataPattern(EDataType dataType); //!< constructor
    explicit FpDataPattern(uint32 dataSize); //!< constructor;
    ~FpDataPattern(); //!< destructor

    Object* Clone() const override;  //!< Return a cloned DataPattern object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the Data object.
    const char* Type() const override { return "FpDataPattern"; } //!< Return type of the Data object.
    uint64 ChooseData() const override; //!< choose a data from the pattern tree.
    void ChooseLargeData(std::vector<uint64>& rDatas) const override; // choose large data from the constraint
    void Setup(const Generator& gen) override; //!< set up data pattern

#ifndef UNIT_TEST
  protected:
#endif
    uint64 ChooseNonZero() const;    //!< choose one zero
    uint64 ChoosePositiveZero() const;   //!< choose positive zero
    uint64 ChooseNegativeZero() const;   //!< choose negative zero
    uint64 ChoosePositiveInfinity() const; //!< choose Positive infinity
    uint64 ChooseNegativeInfinity() const; //!< choose Negative infinity
    uint64 ChooseQuietNan() const;  //!< choose quiet nan
    uint64 ChooseSignalNan() const; //!< choose signal nan
  protected:
    FpDataPattern(const FpDataPattern& rOther);
  };

}

#endif
