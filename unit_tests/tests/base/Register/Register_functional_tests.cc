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

#include <memory>               //unique_ptr used for choice tree interaction
#include <string.h>

//TODO - use constant or count # regs programatically. or just remove
#define TOTAL_REGISTER_NUMBER 20
//TODO - use constant or count # regs programatically. or just remove
#define TOTAL_PHYSICAL_REGISTER_NUMBER 21

#define private public
#define protected public

#include <Architectures.h>
#include <Register.h>
#include <RegisterRISCV.h>
#include <Choices.h>
#include <ChoicesModerator.h>
#include <ChoicesParser.h>
#include <Constraint.h>
#include <Enums.h>
#include <UnitTestUtilities.h>

using namespace std;
using namespace Force;
using text = std::string;

extern RegisterFile* register_file_top;
extern lest::tests& specification();

CASE("Value init propogation")
{
  RegisterFile* test_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
  test_register_file->Setup();

  Register*       pS10_register = test_register_file->RegisterLookup("S10");
  RegisterField*  pS10_field    = pS10_register->RegisterFieldLookup("S10");

  EXPECT( nullptr != pS10_register );
  EXPECT( nullptr != pS10_field );

  pS10_register->Initialize(0x923c2a1d72e3923e);   // pS10 is 32 bit reigster, so only keeps last 32-bit value
  pS10_field->ToString();
  EXPECT (0x72e3923eu == pS10_field->Value() );

  Register* stvec = test_register_file->RegisterLookup("stvec");
  PhysicalRegister* stvec_phy = test_register_file->PhysicalRegisterLookup("stvec");
  stvec->Initialize(0xffffffffffffffff);

  EXPECT( stvec_phy->mInitMask == 0xffffffffffffffff);
  stvec_phy->Value(0xffffffffffffffff);

  // For D0 and Q0_0
  Register* pD0_register = test_register_file->RegisterLookup("D0");
  pD0_register->Initialize(0x123456789a);

  RegisterField* pQ0_0_field = test_register_file->RegisterLookup("Q0")->RegisterFieldLookup("Q0_0");
  pQ0_0_field->ToString();
  EXPECT (0x123456789au == pQ0_0_field->Value());

  // For Q1 and D1
  LargeRegister* pQ1_register = dynamic_cast<LargeRegister* >(test_register_file->RegisterLookup("Q1"));
  std::vector<uint64> q_values = {0x1234, 0x5678};
  pQ1_register->Initialize(q_values);
  
  Register* pD1_register = test_register_file->RegisterLookup("D1");
  EXPECT(0x1234u == pD1_register->Value());
  
  auto init_values = pQ1_register->InitialValues();
  EXPECT(0x1234u == init_values[0]);
  EXPECT(0x5678u == init_values[1]);

  // For x1 register
  Register* pX1_register = test_register_file->RegisterLookup("x1");
  pX1_register->Initialize(0x5a5b5c5d);
  EXPECT(0x5a5b5c5du == pX1_register->Value());

  // For x2 register
  Register* pX2_register = test_register_file->RegisterLookup("x2");
  pX2_register->Initialize(0x5a5b5c5d);
  EXPECT(0x5a5b5c5du == pX2_register->InitialValue());
}
CASE("Register class")
{
  RegisterFile* test_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
  test_register_file->Setup();
  
  auto pPmpcfg0_reg = test_register_file->RegisterLookup("pmpcfg0");
  pPmpcfg0_reg->Initialize(0x5a5b5c5du);
  pPmpcfg0_reg->SetValue(0x12345678u);
  EXPECT(pPmpcfg0_reg->Value() == 0x12345678u);
  
  auto pPmp1cfg_field = pPmpcfg0_reg->RegisterFieldLookup("pmp1cfg");
  EXPECT(pPmp1cfg_field->InitialFieldValue() == 0x5cu);
  EXPECT(pPmp1cfg_field->FieldValue() == 0x56u);
  
  auto field_reload_value = pPmpcfg0_reg->FieldReloadValue("pmp2cfg", nullptr);
  EXPECT((field_reload_value & 0x00u) == 0x0u);

  const std::vector<std::string> pmpcfg0_fields = {"pmp1cfg", "pmp3cfg"};
  auto pmpcfg0_mask = pPmpcfg0_reg->GetRegisterFieldMask(pmpcfg0_fields);
  EXPECT(pmpcfg0_mask == 0xff00ff00u);

  PhysicalRegister* pPmpcfg0_phys_reg = test_register_file->PhysicalRegisterLookup("pmpcfg0");
  EXPECT(pPmpcfg0_reg->GetPhysicalRegisterMask(*pPmpcfg0_phys_reg) == 0xffffffffffffffffu);

  auto reg_fields = pPmpcfg0_reg->GetRegisterFieldsFromMask(0x00ffff00u);
  EXPECT(reg_fields.find("pmp0cfg") == reg_fields.end());
  EXPECT(reg_fields.find("pmp1cfg") != reg_fields.end());
  EXPECT(reg_fields.find("pmp2cfg") != reg_fields.end());
  EXPECT(reg_fields.find("pmp3cfg") == reg_fields.end());
}
CASE("RegisterFile class - RISCV regs")
{
  RegisterFile* test_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
  test_register_file->Setup();

  EXPECT( TOTAL_PHYSICAL_REGISTER_NUMBER == (int)test_register_file->PhysicalRegisters().size());
  EXPECT( TOTAL_REGISTER_NUMBER == (int)test_register_file->Registers().size());

  // test of physical register attributes
  const PhysicalRegister* phys1 = test_register_file->PhysicalRegisterLookup("scounteren");
  EXPECT( 32 == (int)phys1->Size() );
  EXPECT( ERegisterType::SysReg == phys1->RegisterType() );
  EXPECT( 0 == phys1->Name().compare("scounteren") );
  // for physical register whose index is not defined from register file, the default value is 0.
  EXPECT( 0x106 == (int)phys1->IndexValue() );
  EXPECT( 0 == (int)phys1->SubIndexValue() );
  EXPECT( 0 == string(phys1->Type()).compare("PhysicalRegister") );

  // physical register from test_app_reigsters.xml
  const PhysicalRegister* phys3 = test_register_file->PhysicalRegisterLookup("v27_0");
  EXPECT( 64 == (int)phys3->Size() );
  EXPECT( ERegisterType::VECREG == phys3->RegisterType() );
  EXPECT( 0 == phys3->Name().compare("v27_0") );
  EXPECT( 27 == (int)phys3->IndexValue() );
  EXPECT( 0 == (int)phys3->SubIndexValue() );
  EXPECT( 0 == string(phys3->Type()).compare("PhysicalRegister") );

  const PhysicalRegister* phys4 = test_register_file->PhysicalRegisterLookup("v27_1");
  EXPECT( 64 == (int)phys4->Size() );
  // default PhysicalRegister type is GPR
  EXPECT( ERegisterType::VECREG == phys4->RegisterType() );
  EXPECT( 0 == phys4->Name().compare("v27_1") );
  // for physical register whose index is not defined from register file, the default value is 0.
  EXPECT( 27 == (int)phys4->IndexValue() );
  EXPECT( 1 == (int)phys4->SubIndexValue() );
  EXPECT( 0 == string(phys4->Type()).compare("PhysicalRegister") );

  //TODO - change test to non x0 reset val register (x0 reset is different)
  const PhysicalRegister* phys5 = test_register_file->PhysicalRegisterLookup("x0");
  EXPECT( 64 == (int)phys5->Size() );
  EXPECT( ERegisterType::GPR == phys5->RegisterType() );
  EXPECT( 0 == phys5->Name().compare("x0") );
  EXPECT( 0 == (int)phys5->IndexValue() );
  EXPECT( 0 == (int)phys5->SubIndexValue() );
  EXPECT( 0 == string(phys5->Type()).compare("PhysicalRegisterRazwi") );
  uint32 index = (((uint32) ERegisterType::GPR) << 28 );
  EXPECT( 0 == test_register_file->mRegIndex2Name.find(index)->second.compare("x0") );

  // test loading register from test_system_registers.xml
  const Register* reg1 = test_register_file->RegisterLookup("scounteren");
  EXPECT( 32 == (int)reg1->Size() );
  EXPECT( 0 == reg1->Name().compare("scounteren") );
  EXPECT( ERegisterType::SysReg == reg1->RegisterType() );
  EXPECT( 0x106 == (int)reg1->IndexValue() );
  EXPECT( 1 == (int)reg1->Boot() );
  EXPECT( 0 == string(reg1->Type()).compare("Register") );
  EXPECT( false == reg1->IsReadOnly() );

  // test loading register from test_app_registers.xml
  const Register* reg3 = test_register_file->RegisterLookup("S9");
  EXPECT( 32 == (int)reg3->Size() );
  EXPECT( 0 == reg3->Name().compare("S9") );
  EXPECT( ERegisterType::FPR == reg3->RegisterType() );
  EXPECT( 9 == (int)reg3->IndexValue() );
  EXPECT( 0 == (int)reg3->Boot() );
  EXPECT( 0 == string(reg3->Type()).compare("Register") );

  const Register* reg4 = test_register_file->RegisterLookup("Q0");
  EXPECT( 128 == (int)reg4->Size() );
  EXPECT( 0 == reg4->Name().compare("Q0") );
  EXPECT( ERegisterType::FPR == reg4->RegisterType() );
  EXPECT( 0 == (int)reg4->IndexValue() );
  EXPECT( 0 == (int)reg4->Boot() );
  EXPECT( 0 == string(reg4->Type()).compare("LargeRegister") );

  // test loading large register with type VECREG from test_app_registers.xml
  const Register* largeReg = test_register_file->RegisterLookup("v0");
  EXPECT( 128 == (int)largeReg->Size() );
  EXPECT( 0 == largeReg->Name().compare("v0") );
  EXPECT( ERegisterType::VECREG == largeReg->RegisterType() );
  EXPECT( 0 == (int)largeReg->IndexValue() );
  EXPECT( 0x3000 == (int)largeReg->Boot() );
  EXPECT( 0 == string(largeReg->Type()).compare("LargeRegister") );

  // test RegisterFields within registers.
  RegisterField* reg4field = reg4->RegisterFieldLookup("Q0_0");
  EXPECT( 0 == reg4field->Name().compare("Q0_0") );
  EXPECT( 64 == (int)reg4field->Size() );
  EXPECT( 0 == (int)reg4field->mLsb );
  EXPECT( 0 == reg4field->mPhysicalRegisterName.compare("f0_0") );

  // test BitFields within RegisterFields;
  vector<BitField*> bits = reg4field->mBitFields;
  EXPECT( 1 == (int)bits.size() );
  BitField* bit = *bits.rbegin();
  EXPECT( 64 == (int)bit->Size() );
  EXPECT( 0 == (int)bit->mShift );

  //1. Test clone function
  RegisterFile* clone = dynamic_cast<RegisterFile*>(test_register_file->Clone());
  EXPECT( clone != test_register_file);
  EXPECT( TOTAL_PHYSICAL_REGISTER_NUMBER == (int)clone->PhysicalRegisters().size());
  EXPECT( TOTAL_REGISTER_NUMBER == (int)clone->Registers().size());

  // test of physical register attributes
  const PhysicalRegister* phys1_clone = clone->PhysicalRegisterLookup("scounteren");
  EXPECT( phys1_clone != phys1 );
  EXPECT( 32 == (int)phys1_clone->Size() );
  EXPECT( ERegisterType::SysReg == phys1_clone->RegisterType() );
  EXPECT( 0 == phys1_clone->Name().compare("scounteren") );
  // for physical register whose index is not defined from register file, the default value is 0.
  EXPECT( 0x106 == (int)phys1_clone->IndexValue() );
  EXPECT( 0 == (int)phys1_clone->SubIndexValue() );
  EXPECT( 0 == string(phys1_clone->Type()).compare("PhysicalRegister") );

  const Register* reg4_clone = clone->RegisterLookup("Q0");
  EXPECT( reg4_clone != reg4 );
  EXPECT( 128 == (int)reg4_clone->Size() );
  EXPECT( 0 == reg4_clone->Name().compare("Q0") );
  EXPECT( ERegisterType::FPR == reg4_clone->RegisterType() );
  EXPECT( 0 == (int)reg4_clone->IndexValue() );
  EXPECT( 0 == (int)reg4_clone->Boot() );
  EXPECT( 0 == string(reg4_clone->Type()).compare("LargeRegister") );

  // test RegisterFields within registers.
  RegisterField* reg4field_clone = reg4_clone->RegisterFieldLookup("Q0_0");
  EXPECT( reg4field_clone != reg4field );
  EXPECT( 0 == reg4field_clone->Name().compare("Q0_0") );
  EXPECT( 64 == (int)reg4field_clone->Size() );
  EXPECT( 0 == (int)reg4field_clone->mLsb );
  EXPECT( 0 == reg4field_clone->mPhysicalRegisterName.compare("f0_0") );

  // test BitFields within RegisterFields;
  vector<BitField*> bits_clone = reg4field_clone->mBitFields;
  EXPECT( 1 == (int)bits_clone.size() );
  BitField* bit_clone = *bits_clone.rbegin();
  EXPECT( 64 == (int)bit_clone->Size() );

  //2. Test copy constructor
  RegisterFile* copy = new RegisterFile(*test_register_file);
  EXPECT( copy != test_register_file);
  EXPECT( TOTAL_PHYSICAL_REGISTER_NUMBER == (int)copy->PhysicalRegisters().size());
  EXPECT( TOTAL_REGISTER_NUMBER == (int)copy->Registers().size());

  // test of physical register attributes
  const PhysicalRegister* phys1_copy = copy->PhysicalRegisterLookup("scounteren");
  EXPECT( phys1_copy != phys1 );
  EXPECT( 32 == (int)phys1_copy->Size() );
  EXPECT( ERegisterType::SysReg == phys1_copy->RegisterType() );
  EXPECT( 0 == phys1_copy->Name().compare("scounteren") );
  // for physical register whose index is not defined from register file, the default value is 0.
  EXPECT( 0x106 == (int)phys1_copy->IndexValue() );
  EXPECT( 0 == (int)phys1_copy->SubIndexValue() );
  EXPECT( 0 == string(phys1_copy->Type()).compare("PhysicalRegister") );

  const Register* reg4_copy = copy->RegisterLookup("Q0");
  EXPECT( reg4_copy != reg4 );
  EXPECT( 128 == (int)reg4_copy->Size() );
  EXPECT( 0 == reg4_copy->Name().compare("Q0") );
  EXPECT( ERegisterType::FPR == reg4_copy->RegisterType() );
  EXPECT( 0 == (int)reg4_copy->IndexValue() );
  EXPECT( 0 == (int)reg4_copy->Boot() );
  EXPECT( 0 == string(reg4_copy->Type()).compare("LargeRegister") );

  // test RegisterFields within registers.
  RegisterField* reg4field_copy = reg4_copy->RegisterFieldLookup("Q0_0");
  EXPECT( reg4field_copy != reg4field );
  EXPECT( 0 == reg4field_copy->Name().compare("Q0_0") );
  EXPECT( 64 == (int)reg4field_copy->Size() );
  EXPECT( 0 == (int)reg4field_copy->mLsb );
  EXPECT( 0 == reg4field_copy->mPhysicalRegisterName.compare("f0_0") );

  // test BitFields within RegisterFields;
  vector<BitField*> bits_copy = reg4field_copy->mBitFields;
  EXPECT( 1 == (int)bits_copy.size() );
  BitField* bit_copy = *bits_copy.rbegin();
  EXPECT( 64 == (int)bit_copy->Size() );

  // TODO - evaluate if this is valid idea to test -> Test RegisterFields with shift > 0

  // Register fcsr has field RES0
  const Register* reg_fcsr = copy->RegisterLookup("fcsr");
  RegisterField* res0_fcsr = reg_fcsr->RegisterFieldLookup("RES0");
  EXPECT( 1 == (int)res0_fcsr->mBitFields.size() );
  vector<BitField*>::iterator res0_bit_it = res0_fcsr->mBitFields.begin();
  (*res0_bit_it)->ToString();

  //8. Test Name() Type() function
  EXPECT( 0 == test_register_file->Name().compare("RISC-V Registers") );
  EXPECT( 0 == string(test_register_file->Type()).compare("RegisterFile") );
}
CASE("Paritial register init") 
{
  RegisterFile* test_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
  test_register_file->Setup();

  Register* S9 = test_register_file->RegisterLookup("S9");
  Register* S10 = test_register_file->RegisterLookup("S10");

  //TODO - figure out possible expects for random init regs
  S9->InitializeRandomly(nullptr);
  S10->InitializeRandomly(nullptr);
}
CASE("ReadOnly Register tests")
{
  RegisterFile* test_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
  test_register_file->Setup();

  ReadOnlyRegister* readonly_reg = dynamic_cast<ReadOnlyRegister*>(test_register_file->RegisterLookup("x0"));
  EXPECT( true == readonly_reg->IsReadOnly() );
  EXPECT( 0ull == readonly_reg->ReloadValue() );
  readonly_reg->InitializeRandomly(nullptr);
  readonly_reg->SetValue(0x123);
  EXPECT( 0x0 == (int)readonly_reg->Value() );
}
CASE("Test Register class lookups, interaction, and propogation")
{
  RegisterFile* test_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
  test_register_file->Setup();

  Register* mstatus_reg = test_register_file->RegisterLookup("mstatus");
  PhysicalRegister* physicalR = test_register_file->PhysicalRegisterLookup("mstatus");
  RegisterField* fieldMPP = mstatus_reg->RegisterFieldLookup("MPP");
  BitField* bitMPP = fieldMPP->mBitFields.at(0);
  EXPECT( 11 == (int) bitMPP->mShift);
  EXPECT( 11 == (int) fieldMPP->mLsb);
  EXPECT( 0 == (int) fieldMPP->mLsbBlock);

  fieldMPP->Initialize((0x3 << 11));
  EXPECT( 0x1800 == (int) physicalR->Value(0x1800));

  RegisterField* fieldFS = mstatus_reg->RegisterFieldLookup("FS");
  BitField* bitFS = fieldFS->mBitFields.at(0);
  EXPECT( 13 == (int) bitFS->mShift);
  EXPECT( 13 == (int) fieldFS->mLsb);
  EXPECT( 0 == (int) fieldFS->mLsbBlock);

  fieldFS->Initialize((0x1 << 13));
  EXPECT( 0x2000 == (int) physicalR->Value(0x2000));

  RegisterField* fieldSUM = mstatus_reg->RegisterFieldLookup("SUM");
  BitField* bitSUM = fieldSUM->mBitFields.at(0);
  EXPECT( 18 == (int) bitSUM->mShift);
  EXPECT( 18 == (int) fieldSUM->mLsb);
  EXPECT( 0 == (int) fieldSUM->mLsbBlock);

  fieldSUM->Initialize(0);
  EXPECT( 0 == (int) physicalR->Value(0x40000));

  Register* mscratch_reg = test_register_file->RegisterLookup("mscratch");
  mscratch_reg->Initialize(0x12345678);
  EXPECT(0x12345678u == (uint64) mscratch_reg->Value());

  mscratch_reg->SetValue(0x107);
  EXPECT(0x107u == (uint64) mscratch_reg->Value());

  Register* satp_reg = test_register_file->RegisterLookup("satp");
  PhysicalRegister* phy_satp_reg = test_register_file->PhysicalRegisterLookup("satp");
  RegisterField* satp_mode_fld = satp_reg->RegisterFieldLookup("MODE");
  BitField* bit = satp_mode_fld->mBitFields.at(0);
  satp_mode_fld->InitializeField(3);
  EXPECT( 60u == satp_mode_fld->mLsb);
  EXPECT((0x3ull << 60) == phy_satp_reg->Value(0xfull<<60));
  EXPECT((0x3ull << 60) == bit->Value());
  EXPECT(0x3u == bit->BitFieldValue());
  EXPECT(0x3u == satp_mode_fld->FieldValue());
}
CASE("Reset value logic")
{
  //TODO x0 is a read-only register, some of the testing assumption here are not correct, need revisit very soon.
  RegisterFile* test_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
  test_register_file->Setup();

  PhysicalRegister* x0_phyR = test_register_file->PhysicalRegisterLookup("x0");
  EXPECT( 0x0u == x0_phyR->mResetValue);
  EXPECT( 64u == x0_phyR->mSize);
  EXPECT( 0xffffffffffffffff == x0_phyR->mMask);
  EXPECT( 0xffffffffffffffff == x0_phyR->mResetMask);
  EXPECT( 0x0u == x0_phyR->Value(0xffffffffffffffff));
  EXPECT( 0x0u == x0_phyR->InitialValue(0xffffffffffffffff));

  x0_phyR->mResetValue = 0x2u;
  EXPECT( 0x0u == x0_phyR->mValue );
}
CASE("Choice Tree logic")
{
  SETUP("Choice Tree and Register Init")
  {
    RegisterFile* test_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
    test_register_file->Setup();

    Register* sstatus_reg = test_register_file->RegisterLookup("sstatus");
    RegisterField* sum_field = sstatus_reg->RegisterFieldLookup("SUM");

    vector<ChoicesSet*> choicesSets;
    ChoicesParser choices_parser(choicesSets);

    ArchInfo* archInfo = new ArchInfoTest("TEST");
    archInfo->AddChoicesFile("test_register_field_choices.xml");
    choices_parser.Setup(*archInfo);

    ChoicesSet* theSet {nullptr};
    for (auto it = choicesSets.begin(); it!= choicesSets.end(); ++it)
    {
      if ((*it)->Type() == EChoicesType::RegisterFieldValueChoices)
      {
        theSet = *it;
      }
    }

    ChoicesModerator* pModerator = new ChoicesModerator(theSet);

    SECTION("Reload values")
    {
      ChoiceTree* sum_tree = sstatus_reg->GetChoiceTree("SUM", pModerator);
      std::unique_ptr<ChoiceTree> storage_ptr(sum_tree);

      uint64 inputRandom = 0ul;

      int index = 0;
      for (auto it = pModerator->mpChoicesSet->mChoiceTrees.begin(); it != pModerator->mpChoicesSet->mChoiceTrees.end(); ++it)
      {
        EXPECT ((0x1u << 18) == sum_field->ReloadValue(inputRandom, sum_tree));
        ++index;
        if (index == 3) break;
      }
    }
    SECTION("Repeated random init 1")
    {
      //TODO - figure out possible expects for random register init
      test_register_file->InitializeRegisterFieldRandomly(sstatus_reg, "SUM", pModerator);
      sstatus_reg->InitializeRandomly(pModerator);
      sstatus_reg->InitializeRandomly(pModerator);
    }

    SECTION("Randomly initialize RegisetrField")
    {
      // Only one choice value has non-zero weight to make the test predictable
      Register* vtype_reg = test_register_file->RegisterLookup("vtype");
      test_register_file->InitializeRegisterFieldRandomly(vtype_reg, "VSEW", pModerator);
      RegisterField* vsew_field = vtype_reg->RegisterFieldLookup("VSEW");
      EXPECT(vsew_field->Value() == 0x8ull);
      EXPECT(vsew_field->FieldValue() == 0x2ull);
    }

    SECTION("Randomly initialize RegisetrField with disjoint BitFields")
    {
      // Only one choice value has non-zero weight to make the test predictable
      Register* vtype_reg = test_register_file->RegisterLookup("vtype");
      test_register_file->InitializeRegisterFieldRandomly(vtype_reg, "VLMUL", pModerator);
      RegisterField* vlmul_field = vtype_reg->RegisterFieldLookup("VLMUL");
      EXPECT(vlmul_field->Value() == 0x21ull);
      EXPECT(vlmul_field->FieldValue() == 0x5ull);
    }
  }
}
CASE("RES0 logic")
{
  SETUP("Registers and Randomization")
  {
    RegisterFile* test_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
    test_register_file->Setup();

    Register* fcsr = test_register_file->RegisterLookup("fcsr");
    PhysicalRegister* phy_fcsr = test_register_file->PhysicalRegisterLookup("fcsr");
    EXPECT( fcsr != nullptr);
    RegisterField* res0_fcsr = fcsr->RegisterFieldLookup("RES0");

    SECTION("Random Init")
    {
      res0_fcsr->InitializeFieldRandomly();
      for (auto it = res0_fcsr->mBitFields.begin(); it != res0_fcsr->mBitFields.end(); ++it)
      {
        EXPECT( 0u == (*it)->BitFieldValue());
      }
    }
    SECTION("Init with Value")
    {
      uint32 init_val  = 0xA5A5A5A5;
      uint32 size_mask = 0xFFFFFFFF;
      res0_fcsr->Initialize(init_val);
      for (auto it = res0_fcsr->mBitFields.begin(); it != res0_fcsr->mBitFields.end(); ++it)
      {
        uint32 bit_field_value =  (init_val >> (*it)->mShift) & (size_mask >> (32-(*it)->mSize));
        EXPECT(bit_field_value == (*it)->BitFieldValue());
      }
    }
    SECTION("Partial init of bit field")
    {
      uint32 init_val  = 0x32345678;
      uint32 init_mask = 0xA5A5A5A5;
      uint32 size_mask = 0xFFFFFFFF;

      phy_fcsr->Initialize(init_val, init_mask);
      EXPECT_FAIL(phy_fcsr->Value(size_mask), "physicalregister_not_initialized_nor_reset_fail");
      res0_fcsr->InitializeFieldRandomly();

      //random init of field should set any uninit bits to 0
      for (auto it = res0_fcsr->mBitFields.begin(); it != res0_fcsr->mBitFields.end(); ++it)
      {
        uint32 bit_field_value =  ((init_val & init_mask) >> (*it)->mShift) & (size_mask >> (32-(*it)->mSize));
        EXPECT(bit_field_value == (*it)->BitFieldValue());
      }
    }
    SECTION("Multiple Random Init")
    {
      res0_fcsr->InitializeFieldRandomly();
      res0_fcsr->InitializeFieldRandomly();
      for (auto it = res0_fcsr->mBitFields.begin(); it != res0_fcsr->mBitFields.end(); ++it)
      {
        EXPECT( 0u == (*it)->BitFieldValue());
      }
    }
    SECTION("Reload Value")
    {
      ChoiceTree*   choice_tree        = nullptr;
      ConstraintSet constraint_set_0   = ConstraintSet("0xffffffffffffffff");
      ConstraintSet constraint_set_1   = ConstraintSet("0x0");
      uint32        reg_field_mask     = 0;
      uint64        initial_reload_val = 0xA5A5A5A5ull;
      uint64        reload_val         = initial_reload_val;

      for (auto it = res0_fcsr->mBitFields.begin(); it != res0_fcsr->mBitFields.end(); ++it)
      {
        reg_field_mask |= (*it)->mMask;
      }

      res0_fcsr->ReloadValue(reload_val, choice_tree);
      //reload_val should == init reload val with res0 fields 0'd out
      EXPECT((initial_reload_val & ~reg_field_mask) == reload_val);

      reload_val = initial_reload_val; 
      res0_fcsr->ReloadValue(reload_val, &constraint_set_0);
      EXPECT((initial_reload_val & ~reg_field_mask) == reload_val);
    }
  }
}
