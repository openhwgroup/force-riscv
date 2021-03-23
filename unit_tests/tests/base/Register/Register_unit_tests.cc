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
#include <lest/lest.hpp>
#define CASE( name ) lest_CASE( specification(), name )

#include <string.h>

#define private public
#define protected public

#include <Register.h>
#include <Choices.h>
#include <Constraint.h>
#include <Enums.h>
#include <RegisterReserver.h>
#include <ReservationConstraint.h>
#include <Log.h>

using namespace std;
using namespace Force;
using text = std::string;

extern lest::tests& specification();

class TestRegisterReserver : public RegisterReserver{
public:
  TestRegisterReserver()
    : RegisterReserver(), mReserveGroups({ERegReserveGroup::GPR})
  {
  }

  TestRegisterReserver(const TestRegisterReserver& rOther)
    : RegisterReserver(), mReserveGroups(rOther.mReserveGroups)
  {
  }

  ~TestRegisterReserver() { }
  Object* Clone() const override { return new TestRegisterReserver(*this); }
  const char* Type() const override { return "TestRegisterReserver"; }

protected:
  const vector<ERegReserveGroup>& GetReserveGroupsForRegisterType(const ERegisterType regType) const override
  {
    if (regType != ERegisterType::GPR) {
      LOG(fail) << "{TestRegisterReserver::IsPhysicalRegisterReserved} unsupported register type: " << ERegisterType_to_string(regType) << endl;
      FAIL("unsupported-register-type");
    }

    return mReserveGroups;
  }

  ERegReserveGroup GetReserveGroupForOperandType(const EOperandType oprType) const override
  {
    LOG(fail) << "{TestRegisterReserver::GetReserveGroupForOperandType} unexpected method call";
    FAIL("unexpected-method-call");

    return ERegReserveGroup(0);
  }

  void GetRegisterIndexRange(const ERegisterType regType, ConstraintSet* pIndexConstr) const override
  {
    if (regType != ERegisterType::GPR) {
      LOG(fail) << "{TestRegisterReserver::GetRegisterIndexRange} unsupported register type: " << ERegisterType_to_string(regType) << endl;
      FAIL("unsupported-register-type");
    }

    pIndexConstr->AddRange(0, 30);
  }

  uint32 GetPhysicalRegisterIndex(const PhysicalRegister* pPhysicalRegister, const ERegisterType regType) const override
  {
    return pPhysicalRegister->IndexValue();
  }

private:
  const vector<ERegReserveGroup> mReserveGroups;
};

class TestReserverRegisterFile : public RegisterFile {

  void SetupRegisterReserver() override
  {
    mpRegisterReserver = new TestRegisterReserver();
  }

};

//Utility functions for initializing register objects conveniently
void init_phy_reg(PhysicalRegister* phy_reg, string name, uint32 size, ERegisterType reg_type, uint32 index, uint32 reset_val, uint64 reset_mask)
{
  phy_reg->SetSizeAndMask(size);
  phy_reg->mName         = name;
  phy_reg->mRegisterType = reg_type;
  phy_reg->mIndex        = index;
  phy_reg->mResetValue   = reset_val;
  phy_reg->mResetMask    = reset_mask;
}

void init_bit_field(BitField* bfield, PhysicalRegister* phy_reg, uint32 size, uint32 shift)
{
  bfield->mpRegister       = phy_reg;
  bfield->SetSize           (size);
  bfield->mShift            = shift;
  bfield->SetMask();
}

void init_reg_field(RegisterField* reg_field, string name, uint32 size, string phy_reg_name, uint32 reset_val, bool has_reset, uint32 lsb, uint32 lsb_block)
{
  reg_field->mName                 = name;
  reg_field->mSize                 = size;
  reg_field->mPhysicalRegisterName = phy_reg_name;
  reg_field->mReset                = reset_val;
  reg_field->mHasReset             = has_reset;
  reg_field->mLsb                  = lsb;
  reg_field->mLsbBlock             = lsb_block;
}

void init_register(Register* reg, string name, ERegisterType reg_type, uint32 size, uint32 boot, uint32 index)
{
  reg->mName                     = name;
  reg->mRegisterType             = reg_type;
  reg->mSize                     = size;
  reg->mBoot                     = boot;
  reg->mIndex                    = index;
}

CASE("PhysicalRegister class") {
  bool partial            = false; //flag for partial masks
  PhysicalRegister        phy_reg_0;
  phy_reg_0.SetSizeAndMask(32);
  phy_reg_0.mName         = "phy_reg_0";
  phy_reg_0.mRegisterType = ERegisterType::GPR;
  phy_reg_0.mIndex        = 0x1u;
  phy_reg_0.mSubIndex     = 0x0u;
  phy_reg_0.Initialize    (0x123u, 0xFFFu);

  //Test Type (from Object)
  EXPECT("PhysicalRegister" == phy_reg_0.Type());
  //Test IsInit
  EXPECT(true   == phy_reg_0.IsInitialized(0xFFFu,    &partial));
  EXPECT(false  == partial);
  EXPECT(false  == phy_reg_0.IsInitialized(0xFFFFu,   &partial));
  EXPECT(true   == partial);
  EXPECT(true   == phy_reg_0.IsInitialized(0xFFu,     &partial));
  EXPECT(false  == partial);
  EXPECT(false  == phy_reg_0.IsInitialized(0xFFF000u, &partial));
  EXPECT(false  == partial);
  //Test Value Accessors
  EXPECT(0x123u == phy_reg_0.Value(0xFFFu));
  EXPECT(0x23u  == phy_reg_0.Value(0xFFu ));
  phy_reg_0.SetValue(0x456u, 0xFFFu);
  EXPECT(0x456u == phy_reg_0.Value(0xFFFu));
  //Test Mask
  EXPECT(0xFFFFFFFFu == phy_reg_0.Mask()); //mask is just ~0x0>>64-size
  //Test InitialValue
  EXPECT(0x123u == phy_reg_0.InitialValue(0xFFFu));
  EXPECT(0x23u  == phy_reg_0.InitialValue(0xFFu ));
  //Test SetResetValue
  phy_reg_0.SetResetValue(0x12345678u, 0xFFFF0000u);
  EXPECT(0x12340000u == phy_reg_0.mResetValue);
  EXPECT(0xFFFF0000u == phy_reg_0.mResetMask);
  phy_reg_0.SetResetValue(0xFEDCBA98u, 0x0000FFFFu);
  EXPECT(0x1234BA98u == phy_reg_0.mResetValue);
  EXPECT(0xFFFFFFFFu == phy_reg_0.mResetMask);
  //Test Attribute Functions
  EXPECT(true   == phy_reg_0.HasAttribute(ERegAttrType::HasValue));
  phy_reg_0.ClearAttribute(ERegAttrType::HasValue);
  EXPECT(false  == phy_reg_0.HasAttribute(ERegAttrType::HasValue));
  phy_reg_0.SetAttribute(ERegAttrType::HasValue);
  EXPECT(true   == phy_reg_0.HasAttribute(ERegAttrType::HasValue));
  //Test Name and ToString (from Object)
  EXPECT("phy_reg_0" == phy_reg_0.Name());
  EXPECT("phy_reg_0" == phy_reg_0.ToString());
  //Test IndexValue
  EXPECT(0x1u   == phy_reg_0.IndexValue());
  //Test SubIndexValue
  EXPECT(0x0u == phy_reg_0.SubIndexValue());
  //Test RegisterType
  EXPECT(ERegisterType::GPR == phy_reg_0.RegisterType());
  //Test InitMask
  EXPECT(0xFFFu == phy_reg_0.InitMask());
  //Test Size
  EXPECT(32u    == phy_reg_0.Size());

  //Clone and test copy constructed values
  PhysicalRegister* phy_reg_0_clone = dynamic_cast<PhysicalRegister*>(phy_reg_0.Clone());
  EXPECT(true        == phy_reg_0_clone->IsInitialized(0xFFFu, &partial));
  EXPECT(false       == partial);
  EXPECT(0x456u      == phy_reg_0_clone->Value(0xFFFu));
  EXPECT(0xFFFFFFFFu == phy_reg_0_clone->Mask()); //mask is just ~0x0>>64-size
  EXPECT(0x123u      == phy_reg_0_clone->InitialValue(0xFFFu));
  EXPECT(true        == phy_reg_0_clone->HasAttribute(ERegAttrType::HasValue));
  EXPECT("phy_reg_0" == phy_reg_0_clone->Name());
  EXPECT(0x1u        == phy_reg_0_clone->IndexValue());
  EXPECT(0x0u        == phy_reg_0_clone->SubIndexValue());
  EXPECT(0xFFFu      == phy_reg_0_clone->InitMask());
  EXPECT(32u         == phy_reg_0_clone->Size());
  EXPECT(0x1234BA98u == phy_reg_0_clone->mResetValue);
  EXPECT(0xFFFFFFFFu == phy_reg_0_clone->mResetMask);

  delete phy_reg_0_clone;
}

CASE("BitField class") {
  PhysicalRegister* phy_reg_0 = new PhysicalRegister();
  PhysicalRegister* phy_reg_1 = new PhysicalRegister();
  PhysicalRegister* phy_reg_2 = new PhysicalRegister();

  init_phy_reg(phy_reg_0, "phy_reg_0", 64, ERegisterType::GPR, 1u, 0u, 0u);
  init_phy_reg(phy_reg_1, "phy_reg_1", 64, ERegisterType::GPR, 2u, 0u, 0u);
  init_phy_reg(phy_reg_2, "phy_reg_2", 64, ERegisterType::GPR, 3u, 0u, 0u);

  BitField                    bfield_0;
  bfield_0.SetSize            (12u);
  bfield_0.mShift             = 4u;
  bfield_0.SetMask();
  bfield_0.mpRegister        = phy_reg_0;
  bfield_0.Initialize         (0x1234u);


  bool partial                = false;  //flag for partial masks
  uint64 reload_val           = 0x0ull; //reload val for intf test

  //Test Type (from Object)
  EXPECT("BitField" == bfield_0.Type());
  //Test ToString (from Object)
  EXPECT("phy_reg_0" == bfield_0.ToString());
  //Test IsInit
  EXPECT(true    == bfield_0.IsInitialized(&partial));
  EXPECT(false   == partial);
  //Test Value Accessors
  EXPECT(0x1230u == bfield_0.Value());
  EXPECT(0x123u  == bfield_0.BitFieldValue());
  bfield_0.SetValue(0x4567u);
  EXPECT(0x4560u == bfield_0.Value());
  EXPECT(0x456u  == bfield_0.BitFieldValue());
  bfield_0.SetBitFieldValue(0x89Au);
  EXPECT(0x89A0u == bfield_0.Value());
  EXPECT(0x89Au  == bfield_0.BitFieldValue());
  //Test InitialValue Accessors
  EXPECT(0x1230u == bfield_0.InitialValue());
  EXPECT(0x123u  == bfield_0.InitialBitFieldValue());
  //Test ResetValue
  bfield_0.SetBitResetValue(0x123u);
  EXPECT(0x1230u == phy_reg_0->mResetValue);
  EXPECT(0xFFF0u == phy_reg_0->mResetMask);
  //Test ReloadValue Accessors
  reload_val = 0x5555ull;
  EXPECT(0x5550u  == bfield_0.ReloadValue(reload_val));
  EXPECT(0x5555u == reload_val);
  reload_val = 0x55555555ull;
  EXPECT(0xAAA0u == bfield_0.ReloadValue(reload_val, 0xAAAAAAAAu));
  EXPECT(0x5555AAA5u == reload_val);
  //Test Mask Accessor
  EXPECT(0xFFF0u == bfield_0.Mask());
  //Test Attribute Functions
  EXPECT(true    == bfield_0.HasAttribute(ERegAttrType::HasValue));
  bfield_0.ClearAttribute(ERegAttrType::HasValue);
  EXPECT(false   == bfield_0.HasAttribute(ERegAttrType::HasValue));
  bfield_0.SetAttribute(ERegAttrType::HasValue);
  EXPECT(true    == bfield_0.HasAttribute(ERegAttrType::HasValue));
  //Test Size
  EXPECT(12u     == bfield_0.Size());

  //Clone and test copy constructed values
  BitField* bfield_0_clone = dynamic_cast<BitField*>(bfield_0.Clone());
  EXPECT(true    == bfield_0.IsInitialized(&partial));
  EXPECT(false   == partial);
  EXPECT(0x89A0u == bfield_0.Value());
  EXPECT(0x89Au  == bfield_0.BitFieldValue());
  EXPECT(0x1230u == bfield_0.InitialValue());
  EXPECT(0x123u  == bfield_0.InitialBitFieldValue());
  EXPECT(true    == bfield_0.HasAttribute(ERegAttrType::HasValue));
  EXPECT(12u     == bfield_0.Size());

  delete bfield_0_clone;

  //Test InitializePartial and InitializeBitField
  bfield_0.mpRegister = phy_reg_1;
  bfield_0.InitializeBitField(0xABCu);
  EXPECT(true    == bfield_0.IsInitialized(&partial));
  EXPECT(false   == partial);
  EXPECT(0xABC0u == bfield_0.Value());
  EXPECT(0xABCu  == bfield_0.BitFieldValue());

  bfield_0.mpRegister = phy_reg_2;
  phy_reg_2->Initialize(0xD000u, 0xF000u);
  EXPECT(0xD000u == phy_reg_2->Value(0xF000u));
  EXPECT(false   == bfield_0.IsInitialized(&partial));
  EXPECT(true    == partial);
  bfield_0.InitializePartial(0xFED0u);
  EXPECT(true    == bfield_0.IsInitialized(&partial));
  EXPECT(false   == partial);
  EXPECT(0xDED0u == bfield_0.Value());
  EXPECT(0xDEDu  == bfield_0.BitFieldValue());

  delete phy_reg_0;
  delete phy_reg_1;
  delete phy_reg_2;
}

CASE("RegisterField class") {
  PhysicalRegister* phy_reg_0 = new PhysicalRegister();
  PhysicalRegister* phy_reg_1 = new PhysicalRegister();
  BitField* bfield_0          = new BitField();
  BitField* bfield_1          = new BitField();

  init_phy_reg(phy_reg_0, "phy_reg_0", 64, ERegisterType::GPR, 1u, 0u, 0u);
  init_phy_reg(phy_reg_1, "phy_reg_1", 64, ERegisterType::GPR, 2u, 0u, 0u);
  init_bit_field(bfield_0, phy_reg_0, 12u, 4u);
  init_bit_field(bfield_1, phy_reg_0, 8u , 16u);

  RegisterField reg_field_0;
  reg_field_0.mName                 = "reg_field_0";
  reg_field_0.mSize                 = 20u;
  reg_field_0.mPhysicalRegisterName = "phy_reg_0";
  reg_field_0.mReset                = 0xFEDCBu;
  reg_field_0.mHasReset             = true;
  reg_field_0.mLsb                  = 4u;
  reg_field_0.mLsbBlock             = 0u;

  // BitFields must be added from high order to low order
  reg_field_0.mBitFields.push_back (bfield_1);
  reg_field_0.mBitFields.push_back (bfield_0);

  reg_field_0.Initialize           (0x1234567u);

  bool           partial        = false; //flag for partial masks
  uint64         reload_val     = 0xA5A5A5A5ull;
  ChoiceTree*    choice_tree    = nullptr;
  ConstraintSet* constraint_set = nullptr;

  //Test Type (from Object)
  EXPECT("RegisterField" == reg_field_0.Type());
  //Test IsInit
  EXPECT(true      == reg_field_0.IsInitialized(&partial));
  EXPECT(false     == partial);
  //Test Value Accessors
  EXPECT(0x234560u == reg_field_0.Value());
  EXPECT(0x23456u  == reg_field_0.FieldValue());
  reg_field_0.SetValue(0x89ABCDEu);
  EXPECT(0x9ABCD0u == reg_field_0.Value());
  EXPECT(0x9ABCDu  == reg_field_0.FieldValue());
  reg_field_0.SetFieldValue(0xFEDCB);
  EXPECT(0xFEDCB0u == reg_field_0.Value());
  EXPECT(0xFEDCBu  == reg_field_0.FieldValue());
  //Test InitialValue Accessors
  EXPECT(0x234560u == reg_field_0.InitialValue());
  EXPECT(0x23456u  == reg_field_0.InitialFieldValue());
  //Test ResetValue
  reg_field_0.SetFieldResetValue(0xFEDCB);
  EXPECT(0xFEDCB0u == phy_reg_0->mResetValue);
  EXPECT(0xFFFFF0u == phy_reg_0->mResetMask);
  //Test ReloadValue Accessors (test w/ valid choice/constraint in functional)
  EXPECT(0xA5A5A0u   == reg_field_0.ReloadValue(reload_val, choice_tree));
  EXPECT(0xA5A5A5A5u == reload_val);
  reload_val = 0x5A5A5A5Au;
  EXPECT(0x5A5A50u   == reg_field_0.ReloadValue(reload_val, constraint_set));
  EXPECT(0x5A5A5A5Au == reload_val);
  //Test FieldMask Accessor
  EXPECT(0xFFFFF0u == reg_field_0.FieldMask());
  //Test GetPhysicalRegisterMask
  EXPECT(0xFFFFF0u == reg_field_0.GetPhysicalRegisterMask());
  //Test Attribute Functions
  EXPECT(true    == reg_field_0.HasAttribute(ERegAttrType::HasValue));
  reg_field_0.ClearAttribute(ERegAttrType::HasValue);
  EXPECT(false   == reg_field_0.HasAttribute(ERegAttrType::HasValue));
  reg_field_0.SetAttribute(ERegAttrType::HasValue);
  EXPECT(true    == reg_field_0.HasAttribute(ERegAttrType::HasValue));
  //Test Name and ToString (from Object)
  EXPECT("reg_field_0" == reg_field_0.Name());
  EXPECT("reg_field_0" == reg_field_0.ToString());
  //Test Size
  EXPECT(20u == reg_field_0.Size());

  //Clone and test copy constructed values
  RegisterField* reg_field_0_clone = dynamic_cast<RegisterField*>(reg_field_0.Clone());
  EXPECT("phy_reg_0"   == reg_field_0.mPhysicalRegisterName);
  EXPECT("reg_field_0" == reg_field_0.Name());
  EXPECT(20u           == reg_field_0.Size());
  EXPECT(4u            == reg_field_0.mLsb);
  EXPECT(true          == reg_field_0.IsInitialized(&partial));
  EXPECT(false         == partial);
  EXPECT(0xFEDCB0u     == reg_field_0.Value());
  EXPECT(0xFEDCBu      == reg_field_0.FieldValue());
  EXPECT(0x234560u     == reg_field_0.InitialValue());
  EXPECT(0x23456u      == reg_field_0.InitialFieldValue());
  EXPECT(true          == reg_field_0.HasAttribute(ERegAttrType::HasValue));
  EXPECT(0xFEDCBu      == reg_field_0.mReset);
  EXPECT(true          == reg_field_0.mHasReset);

  delete reg_field_0_clone;

  //Test InitializeField
  bfield_0->mpRegister = phy_reg_1;
  bfield_1->mpRegister = phy_reg_1;
  reg_field_0.InitializeField(0x1234567u);
  EXPECT(true      == reg_field_0.IsInitialized(&partial));
  EXPECT(false     == partial);
  EXPECT(0x345670u == reg_field_0.Value());
  EXPECT(0x34567u  == reg_field_0.FieldValue());

  delete phy_reg_0;
  delete phy_reg_1;
}

CASE("RegisterField with disjoint BitFields") {
  PhysicalRegister phy_reg;
  BitField* bfield_0 = new BitField();
  BitField* bfield_1 = new BitField();

  init_phy_reg(&phy_reg, "phy_reg", 64, ERegisterType::SysReg, 2, 0, 0);
  init_bit_field(bfield_0, &phy_reg, 4, 20);
  init_bit_field(bfield_1, &phy_reg, 3, 6);

  RegisterField reg_field;
  reg_field.mName = "reg_field";
  reg_field.mSize = 7;
  reg_field.mPhysicalRegisterName = "phy_reg";
  reg_field.mReset = 0x38;
  reg_field.mHasReset = true;
  reg_field.mLsb = 6;
  reg_field.mLsbBlock = 0;

  // BitFields must be added from high order to low order
  reg_field.mBitFields.push_back(bfield_0);
  reg_field.mBitFields.push_back(bfield_1);

  reg_field.InitializeField(0x65);
  bool partial = false;
  EXPECT(reg_field.IsInitialized(&partial));
  EXPECT_NOT(partial);
  EXPECT(reg_field.Value() == 0xC00140ull);
  EXPECT(reg_field.FieldValue() == 0x65ull);

  reg_field.SetFieldValue(0x7C);
  EXPECT(reg_field.Value() == 0xF00100ull);
  EXPECT(reg_field.FieldValue() == 0x7Cull);
  EXPECT(reg_field.InitialValue() == 0xC00140ull);
  EXPECT(reg_field.InitialFieldValue() == 0x65ull);

  reg_field.SetFieldResetValue(0x3E);
  EXPECT(phy_reg.mResetValue == 0x700180ull);
  EXPECT(phy_reg.mResetMask == 0xF001C0ull);
}

CASE("Register class") {
  PhysicalRegister* phy_reg_0 = new PhysicalRegister();
  BitField* bfield_0          = new BitField();
  BitField* bfield_1          = new BitField();
  BitField* bfield_2          = new BitField();
  RegisterField* reg_field_0  = new RegisterField();
  RegisterField* reg_field_1  = new RegisterField();

  init_phy_reg(phy_reg_0, "phy_reg_0", 64, ERegisterType::GPR, 1u, 0u, 0u);
  init_bit_field(bfield_0, phy_reg_0, 12u, 4u );
  init_bit_field(bfield_1, phy_reg_0, 8u , 16u);
  init_bit_field(bfield_2, phy_reg_0, 4u , 0u );
  init_reg_field(reg_field_0, "reg_field_0", 20u, "phy_reg_0", 0x1u, true, 4u, 0u);
  init_reg_field(reg_field_1, "reg_field_1", 4u , "phy_reg_0", 0x1u, true, 0u, 0u);

  // BitFields must be added from high order to low order
  reg_field_0->mBitFields.push_back(bfield_1);
  reg_field_0->mBitFields.push_back(bfield_0);
  reg_field_1->mBitFields.push_back(bfield_2);

  Register reg_0;
  reg_0.mName                     = "reg_0";
  reg_0.mRegisterType             = ERegisterType::GPR;
  reg_0.mSize                     = 64u;
  reg_0.mBoot                     = 1u;
  reg_0.mIndex                    = 2u;
  reg_0.mRegisterFields.push_back (reg_field_0);
  reg_0.mRegisterFields.push_back (reg_field_1);
  reg_0.Initialize                (0x1234567u);

  bool partial                    = false; //flag for partial masks

  //Test Type (from Object)
  EXPECT("Register" == reg_0.Type());
  //Test IsInit (all regfields, not entire size initialized)
  EXPECT(true  == reg_0.IsInitialized(&partial));
  EXPECT(false == partial);
  //Test Value Accessors
  EXPECT(0x234567u == reg_0.Value());
  reg_0.SetValue(0xFEDCBA9u);
  EXPECT(0xEDCBA9u == reg_0.Value());
  //Test Initial Value Accessors
  EXPECT(0x234567u == reg_0.InitialValue());
  //Test RegisterType
  EXPECT(ERegisterType::GPR == reg_0.RegisterType());
  //Test Boot
  EXPECT(1u == reg_0.Boot());
  //Test IndexValue
  EXPECT(2u == reg_0.IndexValue());
  //Test Name and ToString (from Object)
  EXPECT("reg_0" == reg_0.Name());
  EXPECT("reg_0" == reg_0.ToString());
  //Test Size
  EXPECT(64u == reg_0.Size());
  //Test RegisterFieldLookup
  EXPECT(reg_field_0 == reg_0.RegisterFieldLookup("reg_field_0"));
  EXPECT(reg_field_1 == reg_0.RegisterFieldLookup("reg_field_1"));
  //Test GetRegisterFieldMask
  vector<std::string> field_names;
  field_names.push_back("reg_field_0");
  EXPECT(0xFFFFF0u == reg_0.GetRegisterFieldMask(field_names));
  field_names.push_back("reg_field_1");
  EXPECT(0xFFFFFFu == reg_0.GetRegisterFieldMask(field_names));
  field_names.clear();
  field_names.push_back("reg_field_1");
  EXPECT(0xFu      == reg_0.GetRegisterFieldMask(field_names));
  //Test GetPhysicalRegisterMask
  EXPECT(0xFFFFFFu == reg_0.GetPhysicalRegisterMask(*phy_reg_0));
  //Test GetRegisterFieldsFromMask
  std::map<std::string, RegisterField*> reg_fields;
  auto it = reg_fields.end();
  reg_fields = reg_0.GetRegisterFieldsFromMask(0x800010u);
  it         = reg_fields.find("reg_field_0");
  EXPECT(it         != reg_fields.end());
  EXPECT(it->second == reg_field_0);
  reg_fields.clear();
  reg_fields = reg_0.GetRegisterFieldsFromMask(0x800019u);
  it         = reg_fields.find("reg_field_0");
  EXPECT(it         != reg_fields.end());
  EXPECT(it->second == reg_field_0);
  it         = reg_fields.find("reg_field_1");
  EXPECT(it         != reg_fields.end());
  EXPECT(it->second == reg_field_1);
  reg_fields.clear();
  reg_fields = reg_0.GetRegisterFieldsFromMask(0x9u);
  it         = reg_fields.find("reg_field_1");
  EXPECT(it         != reg_fields.end());
  EXPECT(it->second == reg_field_1);
  //Test Attribute Functions
  EXPECT(true    == reg_0.HasAttribute(ERegAttrType::HasValue));
  reg_0.ClearAttribute(ERegAttrType::HasValue);
  EXPECT(false   == reg_0.HasAttribute(ERegAttrType::HasValue));
  reg_0.SetAttribute(ERegAttrType::HasValue);
  EXPECT(true    == reg_0.HasAttribute(ERegAttrType::HasValue));
  //Test IsReadOnly (always false for base)
  EXPECT(false   == reg_0.IsReadOnly());

  //Test Clone and copy constructed values
  Register* reg_0_clone = dynamic_cast<Register*>(reg_0.Clone());
  EXPECT("reg_0"            == reg_0.Name());
  EXPECT(ERegisterType::GPR == reg_0.RegisterType());
  EXPECT(64u                == reg_0.Size());
  EXPECT(1u                 == reg_0.Boot());
  EXPECT(2u                 == reg_0.IndexValue());
  EXPECT(true               == reg_0.IsInitialized(&partial));
  EXPECT(false              == partial);
  EXPECT(0xEDCBA9u          == reg_0.Value());
  EXPECT(0x234567u          == reg_0.InitialValue());
  EXPECT(true               == reg_0.HasAttribute(ERegAttrType::HasValue));
  //reg fields should be cloned, verify not pointing to original reg fields
  EXPECT(reg_field_0 != reg_0_clone->RegisterFieldLookup("reg_field_0"));
  EXPECT(reg_field_1 != reg_0_clone->RegisterFieldLookup("reg_field_1"));

  delete reg_0_clone;
  delete phy_reg_0;
}

CASE("RegisterFile class") {

  SETUP("Setup RegisterFile") {
    PhysicalRegister* phy_reg_0 = new PhysicalRegister();
    PhysicalRegister* phy_reg_1 = new PhysicalRegister();
    BitField* bfield_0          = new BitField();
    BitField* bfield_1          = new BitField();
    BitField* bfield_2          = new BitField();
    BitField* bfield_3          = new BitField();
    BitField* bfield_4          = new BitField();
    BitField* bfield_5          = new BitField();
    RegisterField* reg_field_0  = new RegisterField();
    RegisterField* reg_field_1  = new RegisterField();
    RegisterField* reg_field_2  = new RegisterField();
    RegisterField* reg_field_3  = new RegisterField();
    Register* reg_0             = new Register();
    Register* reg_1             = new Register();

    //allowing setup to set correct mResetValue
    init_phy_reg(phy_reg_0, "phy_reg_0", 64, ERegisterType::GPR, 0u, 0u, 0u);
    init_phy_reg(phy_reg_1, "phy_reg_1", 32, ERegisterType::GPR, 1u, 0u, 0u);
    //allowing setup to set mpRegister
    init_bit_field(bfield_0, nullptr, 12u, 4u );
    init_bit_field(bfield_1, nullptr, 8u , 16u);
    init_bit_field(bfield_2, nullptr, 4u , 0u );
    init_bit_field(bfield_3, nullptr, 10u, 16u);
    init_bit_field(bfield_4, nullptr, 2u , 14u);
    init_bit_field(bfield_5, nullptr, 14u, 0u );
    //allowing setup to set correct mLsb
    init_reg_field(reg_field_0, "reg_field_0", 20u, "phy_reg_0", 0xAAAAAu, true, 63u, 0u);
    init_reg_field(reg_field_1, "reg_field_1", 4u , "phy_reg_0", 0xBu    , true, 63u, 0u);
    init_reg_field(reg_field_2, "reg_field_2", 10u, "phy_reg_1", 0xFCCu  , true, 63u, 0u);
    init_reg_field(reg_field_3, "reg_field_3", 16u, "phy_reg_1", 0xDDDDu , true, 63u, 0u);
    init_register(reg_0, "reg_0", ERegisterType::GPR, 64u, 1u, 2u);
    init_register(reg_1, "reg_1", ERegisterType::GPR, 32u, 0u, 1u);

    // BitFields must be added from high order to low order
    reg_field_0->mBitFields.push_back(bfield_1);
    reg_field_0->mBitFields.push_back(bfield_0);
    reg_field_1->mBitFields.push_back(bfield_2);
    reg_field_2->mBitFields.push_back(bfield_3);
    reg_field_3->mBitFields.push_back(bfield_4);
    reg_field_3->mBitFields.push_back(bfield_5);

    reg_0->mRegisterFields.push_back(reg_field_0);
    reg_0->mRegisterFields.push_back(reg_field_1);
    reg_1->mRegisterFields.push_back(reg_field_2);
    reg_1->mRegisterFields.push_back(reg_field_3);

    TestReserverRegisterFile reg_file_0;
    reg_file_0.mName = "reg_file_0";
    reg_file_0.AddPhyReg(phy_reg_0, "");
    reg_file_0.AddPhyReg(phy_reg_1, "");
    reg_file_0.AddRegister(reg_0);
    reg_file_0.AddRegister(reg_1);

    string converted_name = "";
    bool partial          = false; //flag for partial masks

    reg_file_0.Setup();

    SECTION("Verify Reg File Setup sets bfield's phy_reg ptr, reg_field's lsb, phy_reg's reset val") {
      EXPECT(phy_reg_0  == bfield_0->mpRegister);
      EXPECT(phy_reg_0  == bfield_1->mpRegister);
      EXPECT(phy_reg_0  == bfield_2->mpRegister);
      EXPECT(phy_reg_1  == bfield_3->mpRegister);
      EXPECT(phy_reg_1  == bfield_4->mpRegister);
      EXPECT(phy_reg_1  == bfield_5->mpRegister);
      EXPECT(4u         == reg_field_0->mLsb);
      EXPECT(0u         == reg_field_1->mLsb);
      EXPECT(16u        == reg_field_2->mLsb);
      EXPECT(0u         == reg_field_3->mLsb);
      EXPECT(0xAAAAABu  == phy_reg_0->mResetValue);
      EXPECT(0x3CCDDDDu == phy_reg_1->mResetValue);
    }

    SECTION("Test Type (from Object)") {
      EXPECT("RegisterFile" == reg_file_0.Type());
    }

    SECTION("Test RegisterLookup") {
      EXPECT(reg_0     == reg_file_0.RegisterLookup("reg_0"));
      EXPECT(reg_1     == reg_file_0.RegisterLookup("reg_1"));
      EXPECT(phy_reg_0 == reg_file_0.PhysicalRegisterLookup("phy_reg_0"));
      EXPECT(phy_reg_1 == reg_file_0.PhysicalRegisterLookup("phy_reg_1"));
    }

    SECTION("Test GetRegisterFieldMask") {
      vector<std::string> field_names_0;
      vector<std::string> field_names_1;
      field_names_0.push_back("reg_field_0");
      field_names_1.push_back("reg_field_3");
      EXPECT(0xFFFFF0u  == reg_file_0.GetRegisterFieldMask("reg_0", field_names_0));
      EXPECT(0xFFFFu    == reg_file_0.GetRegisterFieldMask("reg_1", field_names_1));
      field_names_0.push_back("reg_field_1");
      field_names_1.push_back("reg_field_2");
      EXPECT(0xFFFFFFu  == reg_file_0.GetRegisterFieldMask("reg_0", field_names_0));
      EXPECT(0x3FFFFFFu == reg_file_0.GetRegisterFieldMask("reg_1", field_names_1));
      field_names_0.clear();
      field_names_1.clear();
      field_names_0.push_back("reg_field_1");
      field_names_1.push_back("reg_field_2");
      EXPECT(0xFu       == reg_file_0.GetRegisterFieldMask("reg_0", field_names_0));
      EXPECT(0x3FF0000u == reg_file_0.GetRegisterFieldMask("reg_1", field_names_1));
    }

    SECTION("Test GetRegisterFieldsFromMask") {
      std::map<std::string, RegisterField*> reg_fields;
      auto it = reg_fields.end();
      reg_fields = reg_file_0.GetRegisterFieldsFromMask("reg_0", 0x001001u);
      it = reg_fields.find("reg_field_0");
      EXPECT(it         != reg_fields.end());
      EXPECT(it->second == reg_field_0);
      it = reg_fields.find("reg_field_1");
      EXPECT(it         != reg_fields.end());
      EXPECT(it->second == reg_field_1);
      reg_fields.clear();
      reg_fields = reg_file_0.GetRegisterFieldsFromMask("reg_0", 0x001000u);
      it = reg_fields.find("reg_field_0");
      EXPECT(it         != reg_fields.end());
      EXPECT(it->second == reg_field_0);
      reg_fields.clear();
      reg_fields = reg_file_0.GetRegisterFieldsFromMask("reg_0", 0x1u);
      it = reg_fields.find("reg_field_1");
      EXPECT(it         != reg_fields.end());
      EXPECT(it->second == reg_field_1);
      reg_fields.clear();
      reg_fields = reg_file_0.GetRegisterFieldsFromMask("reg_1", 0x0100010);
      it = reg_fields.find("reg_field_2");
      EXPECT(it         != reg_fields.end());
      EXPECT(it->second == reg_field_2);
      it = reg_fields.find("reg_field_3");
      EXPECT(it         != reg_fields.end());
      EXPECT(it->second == reg_field_3);
      reg_fields.clear();
      reg_fields = reg_file_0.GetRegisterFieldsFromMask("reg_1", 0x0100000);
      it = reg_fields.find("reg_field_2");
      EXPECT(it         != reg_fields.end());
      EXPECT(it->second == reg_field_2);
      reg_fields.clear();
      reg_fields = reg_file_0.GetRegisterFieldsFromMask("reg_1", 0x10);
      it = reg_fields.find("reg_field_3");
      EXPECT(it         != reg_fields.end());
      EXPECT(it->second == reg_field_3);
    }

    SECTION("Test Name and ToString (from Object)") {
      EXPECT("reg_file_0"   == reg_file_0.Name());
      EXPECT("reg_file_0"   == reg_file_0.ToString());
    }

    SECTION("Test ConvertRegisterName") {
      reg_file_0.ConvertRegisterName("reg_0", 0u, converted_name);
      EXPECT("reg_0" == converted_name);
    }

    SECTION("Test Initialize functions (random init covered in func test)") {
      reg_file_0.InitializeRegister("reg_0", 0x12345678u, nullptr);
      EXPECT(true       == reg_0->IsInitialized(&partial));
      EXPECT(false      == partial);
      EXPECT(0x345678u  == reg_0->Value());
      reg_file_0.InitializeRegisterField(reg_1, "reg_field_2", 0xFEDCBA98u);
      EXPECT(false      == reg_1->IsInitialized(&partial));
      EXPECT(true       == partial);
      EXPECT(true       == reg_field_2->IsInitialized(&partial));
      EXPECT(false      == partial);
      EXPECT(0x2980000u == reg_field_2->Value());
      reg_file_0.InitializeRegisterField(reg_1, "reg_field_3", 0xFEDCBA98u);
      EXPECT(true       == reg_1->IsInitialized(&partial));
      EXPECT(false      == partial);
      EXPECT(true       == reg_field_3->IsInitialized(&partial));
      EXPECT(false      == partial);
      EXPECT(0xBA98u    == reg_field_3->Value());
      EXPECT(0x298BA98u == reg_1->Value());
    }

    SECTION("Test Clone and test copy constructed values") {
      RegisterFile* reg_file_0_clone = dynamic_cast<RegisterFile*>(reg_file_0.Clone());
      reg_file_0_clone->Setup(); //setup after clone fixes bitfield phy_reg ptrs
      Register*     reg_0_clone      = reg_file_0_clone->RegisterLookup("reg_0");
      Register*     reg_1_clone      = reg_file_0_clone->RegisterLookup("reg_1");
      EXPECT("reg_file_0" == reg_file_0_clone->Name());
      EXPECT(phy_reg_0    != reg_file_0_clone->PhysicalRegisterLookup("phy_reg_0"));
      EXPECT(phy_reg_1    != reg_file_0_clone->PhysicalRegisterLookup("phy_reg_1"));
      EXPECT(reg_0        != reg_0_clone);
      EXPECT(reg_1        != reg_1_clone);
      EXPECT(reg_field_0  != reg_0_clone->RegisterFieldLookup("reg_field_0"));
      EXPECT(reg_field_1  != reg_0_clone->RegisterFieldLookup("reg_field_1"));
      EXPECT(reg_field_2  != reg_1_clone->RegisterFieldLookup("reg_field_2"));
      EXPECT(reg_field_3  != reg_1_clone->RegisterFieldLookup("reg_field_3"));
      EXPECT(reg_file_0.mRegIndex2Name == reg_file_0_clone->mRegIndex2Name);

      delete reg_file_0_clone;
    }

    SECTION("Test getting random registers") {
      vector<uint64> reg_indices;
      EXPECT(reg_file_0.GetRandomRegisters(1, ERegisterType::GPR, "", reg_indices));
      EXPECT(reg_indices.size() == 1ull);
    }

    SECTION("Test getting random registers with exclusions") {
      vector<uint64> reg_indices;
      EXPECT(reg_file_0.GetRandomRegisters(1, ERegisterType::GPR, "2", reg_indices));
      EXPECT(reg_indices.size() == 1ull);
      EXPECT(reg_indices[0] != 2u);
    }

    SECTION("Test getting too many random registers") {
      vector<uint64> reg_indices;
      EXPECT_NOT(reg_file_0.GetRandomRegisters(50, ERegisterType::GPR, "", reg_indices));
    }

    SECTION("Test getting random registers for a particular access type") {
      RegisterReserver* reg_reserver = reg_file_0.GetRegisterReserver();
      reg_reserver->ReserveRegister(reg_0, ERegAttrType::Read);

      vector<uint64> reg_indices;
      EXPECT(reg_file_0.GetRandomRegistersForAccess(1, ERegisterType::GPR, ERegAttrType::Read, "2-30", reg_indices));
      EXPECT(reg_indices.size() == 1ull);
      EXPECT(reg_indices[0] == 1u);
    }

    SECTION("Test getting too many random registers for a particular access type") {
      RegisterReserver* reg_reserver = reg_file_0.GetRegisterReserver();
      reg_reserver->ReserveRegister(reg_1, ERegAttrType::Write);

      vector<uint64> reg_indices;
      EXPECT_NOT(reg_file_0.GetRandomRegistersForAccess(2, ERegisterType::GPR, ERegAttrType::Write, "2-30", reg_indices));
    }
  }
}

CASE("LargeRegister class") {
  PhysicalRegister* phy_reg_0 = new PhysicalRegister();
  PhysicalRegister* phy_reg_1 = new PhysicalRegister();
  PhysicalRegister* phy_reg_2 = new PhysicalRegister();
  PhysicalRegister* phy_reg_3 = new PhysicalRegister();
  BitField* bfield_0          = new BitField();
  BitField* bfield_1          = new BitField();
  BitField* bfield_2          = new BitField();
  BitField* bfield_3          = new BitField();
  RegisterField* reg_field_0  = new RegisterField();
  RegisterField* reg_field_1  = new RegisterField();
  RegisterField* reg_field_2  = new RegisterField();
  RegisterField* reg_field_3  = new RegisterField();
  LargeRegister* reg_0        = new LargeRegister();
  LargeRegister* reg_1        = new LargeRegister();

  init_phy_reg(phy_reg_0, "phy_reg_0", 64, ERegisterType::FPR, 0u, 0u, 0u);
  init_phy_reg(phy_reg_1, "phy_reg_1", 64, ERegisterType::FPR, 0u, 0u, 0u);
  init_bit_field(bfield_0, nullptr, 64u , 0u);
  init_bit_field(bfield_1, nullptr, 64u , 0u);
  init_reg_field(reg_field_0, "reg_field_0", 64u, "phy_reg_0", 0x0u, false, 63u, 0u);
  init_reg_field(reg_field_1, "reg_field_1", 64u, "phy_reg_1", 0x0u, false, 63u, 0u);
  init_register(reg_0, "reg_0", ERegisterType::FPR, 128u, 0u, 0u);

  init_phy_reg(phy_reg_2, "phy_reg_2", 64, ERegisterType::FPR, 0u, 0u, 0u);
  init_phy_reg(phy_reg_3, "phy_reg_3", 64, ERegisterType::FPR, 0u, 0u, 0u);
  init_bit_field(bfield_2, nullptr, 64u , 0u);
  init_bit_field(bfield_3, nullptr, 64u , 0u);
  init_reg_field(reg_field_2, "reg_field_2", 64u, "phy_reg_2", 0x0u, false, 63u, 0u);
  init_reg_field(reg_field_3, "reg_field_3", 64u, "phy_reg_3", 0x0u, false, 63u, 0u);
  init_register(reg_1, "reg_1", ERegisterType::FPR, 128u, 0u, 0u);

  reg_field_0->mBitFields.push_back(bfield_0);
  reg_field_1->mBitFields.push_back(bfield_1);

  reg_0->mRegisterFields.push_back(reg_field_0);
  reg_0->mRegisterFields.push_back(reg_field_1);

  reg_field_2->mBitFields.push_back(bfield_2);
  reg_field_3->mBitFields.push_back(bfield_3);

  reg_1->mRegisterFields.push_back(reg_field_2);
  reg_1->mRegisterFields.push_back(reg_field_3);

  RegisterFile reg_file_0;
  reg_file_0.mName = "reg_file_0";
  reg_file_0.AddPhyReg(phy_reg_0, "");
  reg_file_0.AddPhyReg(phy_reg_1, "");
  reg_file_0.AddPhyReg(phy_reg_2, "");
  reg_file_0.AddPhyReg(phy_reg_3, "");
  reg_file_0.AddRegister(reg_0);
  reg_file_0.AddRegister(reg_1);
  reg_file_0.Setup();

  EXPECT(0u        == reg_field_0->mLsb);
  EXPECT(0u        == reg_field_1->mLsb);
  EXPECT(0u        == reg_field_2->mLsb);
  EXPECT(0u        == reg_field_3->mLsb);
  EXPECT(phy_reg_0 == bfield_0->mpRegister);
  EXPECT(phy_reg_1 == bfield_1->mpRegister);
  EXPECT(phy_reg_2 == bfield_2->mpRegister);
  EXPECT(phy_reg_3 == bfield_3->mpRegister);

  EXPECT(true == reg_0->IsLargeRegister());
  Register* reg_ptr_1 = reg_1;
  EXPECT(true == reg_ptr_1->IsLargeRegister());

  std::vector<uint64> write_values;
  EXPECT_FAIL(reg_file_0.InitializeRegister("reg_0", write_values, nullptr), "register initialize with empty vector of values");

  std::vector<uint64> read_values;
  write_values.push_back(0x0123456789ABCDEFull);
  write_values.push_back(0xFEDCBA9876543210ull);
  write_values.push_back(0xDEADDEADDEADDEADull);

  EXPECT_FAIL(reg_file_0.InitializeRegister("reg_0", write_values, nullptr), "vector init values mismatch with register size");
  write_values.pop_back();

  reg_file_0.InitializeRegister("reg_0", write_values, nullptr);
  read_values = reg_0->Values();

  EXPECT(write_values == read_values);

  write_values.clear();
  read_values.clear();
  write_values.push_back(0xA5A5A5A5A5A5A5A5ull);

  reg_file_0.InitializeRegister("reg_1", write_values, nullptr);
  read_values = reg_1->Values();

  EXPECT(write_values == read_values);

  reg_field_3->Initialize(0x5A5A5A5A5A5A5A5Aull);
  write_values.push_back(0x5A5A5A5A5A5A5A5Aull);

  read_values.clear();
  read_values = reg_1->Values();
  EXPECT(write_values == read_values);
}

CASE("Reserve/Unreserve register") {
  SETUP( "Setup register" )  {
    PhysicalRegister* phy_reg_0 = new PhysicalRegister();
    PhysicalRegister* phy_reg_1 = new PhysicalRegister();
    BitField* bfield_0          = new BitField();
    BitField* bfield_1          = new BitField();
    BitField* bfield_2          = new BitField();
    BitField* bfield_3          = new BitField();
    BitField* bfield_4          = new BitField();
    BitField* bfield_5          = new BitField();
    RegisterField* reg_field_0  = new RegisterField();
    RegisterField* reg_field_1  = new RegisterField();
    RegisterField* reg_field_2  = new RegisterField();
    RegisterField* reg_field_3  = new RegisterField();
    Register* reg_0             = new Register();
    Register* reg_1             = new Register();

    //allowing setup to set correct mResetValue
    init_phy_reg(phy_reg_0, "phy_reg_0", 64, ERegisterType::GPR, 0u, 0u, 0u);
    init_phy_reg(phy_reg_1, "phy_reg_1", 32, ERegisterType::GPR, 1u, 0u, 0u);
    //allowing setup to set mpRegister
    init_bit_field(bfield_0, nullptr, 12u, 4u );
    init_bit_field(bfield_1, nullptr, 8u , 16u);
    init_bit_field(bfield_2, nullptr, 4u , 0u );
    init_bit_field(bfield_3, nullptr, 10u, 16u);
    init_bit_field(bfield_4, nullptr, 2u , 14u);
    init_bit_field(bfield_5, nullptr, 14u, 0u );
    //allowing setup to set correct mLsb
    init_reg_field(reg_field_0, "reg_field_0", 20u, "phy_reg_0", 0xAAAAAu, true, 63u, 0u);
    init_reg_field(reg_field_1, "reg_field_1", 4u , "phy_reg_0", 0xBu    , true, 63u, 0u);
    init_reg_field(reg_field_2, "reg_field_2", 10u, "phy_reg_1", 0xFCCu  , true, 63u, 0u);
    init_reg_field(reg_field_3, "reg_field_3", 16u, "phy_reg_1", 0xDDDDu , true, 63u, 0u);
    init_register(reg_0, "reg_0", ERegisterType::GPR, 64u, 1u, 2u);
    init_register(reg_1, "reg_1", ERegisterType::GPR, 32u, 0u, 1u);

    // BitFields must be added from high order to low order
    reg_field_0->mBitFields.push_back(bfield_1);
    reg_field_0->mBitFields.push_back(bfield_0);
    reg_field_1->mBitFields.push_back(bfield_2);
    reg_field_2->mBitFields.push_back(bfield_3);
    reg_field_3->mBitFields.push_back(bfield_4);
    reg_field_3->mBitFields.push_back(bfield_5);

    reg_0->mRegisterFields.push_back(reg_field_0);
    reg_0->mRegisterFields.push_back(reg_field_1);
    reg_1->mRegisterFields.push_back(reg_field_2);
    reg_1->mRegisterFields.push_back(reg_field_3);

    RegisterFile reg_file_0;
    reg_file_0.mName = "reg_file_0";
    reg_file_0.AddPhyReg(phy_reg_0, "");
    reg_file_0.AddPhyReg(phy_reg_1, "");
    reg_file_0.AddRegister(reg_0);
    reg_file_0.AddRegister(reg_1);
    reg_file_0.Setup();

    TestRegisterReserver reg_reserver;

    SECTION("Basic Reserve/Unreserve register") {
      reg_reserver.ReserveRegister(reg_0, ERegAttrType::Read);
      EXPECT(reg_reserver.IsRegisterReserved(reg_0, ERegAttrType::Read));
      EXPECT_NOT(reg_reserver.IsRegisterReserved(reg_0, ERegAttrType::Write));
      reg_reserver.UnreserveRegister(reg_0, ERegAttrType::Read);
      EXPECT_NOT(reg_reserver.IsRegisterReserved(reg_0, ERegAttrType::Read));
    }

    SECTION("Reserve/Unreserve register from different type") {
      reg_reserver.ReserveRegister(reg_0, ERegAttrType::Read, ERegReserveType::User);
      reg_reserver.ReserveRegister(reg_0, ERegAttrType::Read, ERegReserveType::Unpredictable);
      reg_reserver.UnreserveRegister(reg_0, ERegAttrType::Read, ERegReserveType::Unpredictable);
      EXPECT(reg_reserver.IsRegisterReserved(reg_0, ERegAttrType::Read));
      reg_reserver.UnreserveRegister(reg_0, ERegAttrType::Read, ERegReserveType::User);
      EXPECT_NOT(reg_reserver.IsRegisterReserved(reg_0, ERegAttrType::Read));
    }

    SECTION("Reserve/Unreserve register from different type with different access") {
      reg_reserver.ReserveRegister(reg_0, ERegAttrType::Read, ERegReserveType::User);
      reg_reserver.ReserveRegister(reg_0, ERegAttrType::Write, ERegReserveType::Unpredictable);
      reg_reserver.UnreserveRegister(reg_0, ERegAttrType::Write, ERegReserveType::Unpredictable);
      EXPECT(reg_reserver.IsRegisterReserved(reg_0, ERegAttrType::Read, ERegReserveType::User));
      EXPECT_NOT(reg_reserver.IsRegisterReserved(reg_0, ERegAttrType::Write, ERegReserveType::Unpredictable));

      EXPECT_FAIL(reg_reserver.UnreserveRegister(reg_1, ERegAttrType::Read, ERegReserveType::User), "register-not-reserved");
    }
  }
}
