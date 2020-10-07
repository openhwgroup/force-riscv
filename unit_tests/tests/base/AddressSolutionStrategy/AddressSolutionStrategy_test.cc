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
#include <Log.h>
#include <AddressSolutionStrategy.h>
#include <Constraint.h>
#include <Operand.h>
#include <OperandConstraint.h>
#include <OperandSolution.h>
#include <OperandSolutionMap.h>
#include <AddressTagging.h>
#include <InstructionStructure.h>
#include <Random.h>
#include <Defines.h>
#include <Choices.h>

using text = std::string;
using namespace std;
using namespace Force;

namespace Helpers {

bool AreOperandsInTheirConstraints(const OperandSolutionMap& rOperandInfo)
{
  bool all_opr_in_constr = true;

  for(auto const& cr_str_and_opr_sol_pair : rOperandInfo)
  {
    auto const& cr_opr_sol = cr_str_and_opr_sol_pair.second;
    all_opr_in_constr &= cr_opr_sol.GetConstraint()->ContainsValue(cr_opr_sol.GetValue());
  }
  
  return all_opr_in_constr;
}

}

namespace Force {


class AddressTaggingTest : public AddressTagging {
public:
  AddressTaggingTest(cbool enabledForData, cbool enabledForInstruction)
    : mEnabledForData(enabledForData), mEnabledForInstruction(enabledForInstruction)
  {
  }

  bool CanTagAddress(cuint64 address, cbool isInstruction) const override
  {
    if (isInstruction) {
      return mEnabledForInstruction;
    }
    else {
      return mEnabledForData;
    }
  }

private:
  cbool mEnabledForData;
  cbool mEnabledForInstruction;
};

//Stub class methods for ChoicesOperandConstraint
ChoicesOperandConstraint::~ChoicesOperandConstraint(){}
void ChoicesOperandConstraint::Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operand_struct)
{
}

//Stub methods for OperandConstraint
OperandConstraint::~OperandConstraint(){}
void OperandConstraint::Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operand_struct)
{
}

void  OperandConstraint::SubConstraintValue(unsigned long long Val, Force::OperandStructure const& Os) const
{
}

void OperandConstraint::GetAdjustedDifferValues(const Instruction& rInstr, const OperandStructure& rOperandStruct, const OperandStructure& rDifferOperandStruct, cuint64 differVal, ConstraintSet& rAdjDifferValues) const
{
}

class OperandConstraintTest : public OperandConstraint
{
  public:
  OperandConstraintTest()
    : OperandConstraint(), mpTestConstraintSet(nullptr)
  {
    mpTestConstraintSet = new ConstraintSet;
  }

  OperandConstraintTest(uint64 lower_bound, uint64 upper_bound)
    : OperandConstraint(), mpTestConstraintSet(nullptr)
  {
    mpTestConstraintSet = new ConstraintSet(lower_bound, upper_bound);
  }

  OperandConstraintTest(uint64 single_value)
    : OperandConstraint(), mpTestConstraintSet(nullptr)
  {
    mpTestConstraintSet = new ConstraintSet(single_value);
  }

  COPY_CONSTRUCTOR_ABSENT(OperandConstraintTest);
  ASSIGNMENT_OPERATOR_ABSENT(OperandConstraintTest);

  const ConstraintSet* GetConstraint() const
  {
    return mpTestConstraintSet;
  }

  ~OperandConstraintTest()
  {
    delete mpTestConstraintSet;
  }

  ConstraintSet* mpTestConstraintSet;
};

class OperandTest : public Operand {
  public:
  OperandTest() 
  : Operand()
  {
    mpOperandConstraint =  new OperandConstraintTest();
    mpStructure = new OperandStructure();
  }

  OperandTest(uint64 single_value) 
  : Operand()
  {
    mpOperandConstraint =  new OperandConstraintTest(single_value);
    mpStructure = new OperandStructure();
  }

  OperandTest(uint64 lower_bound, uint64 upper_bound) 
  : Operand()
  {
    mpOperandConstraint =  new OperandConstraintTest(lower_bound, upper_bound);
    mpStructure = new OperandStructure();
  }

  const ConstraintSet* GetConstraint() const
  {
    return mpOperandConstraint->GetConstraint();
  }
};

// Stub implementation
Operand::Operand()
: Object(), mpStructure(nullptr), mpOperandConstraint(nullptr), mValue(0)
{
}

// Stub implementation
Operand::~Operand()
{
}

// Stub implementation
const string& Operand::Name() const
{
  return mpStructure->Name();
}

// Stub implementation
EOperandType Operand::OperandType() const
{
  return EOperandType::Constant;
}

// Fake version of virtual function placed here to supress linker errors.
Object* Operand::Clone() const
{ 
  return nullptr;
}

// Fake version of virtual function placed here to supress linker errors.
OperandConstraint* Operand::InstantiateOperandConstraint() const
{ 
  return nullptr;
}

// Fake version of virtual function placed here to supress linker errors.
void Operand::CleanUp()
{
}

// Fake version of virtual function placed here to supress linker errors.
void Operand::Commit(Force::Generator&, Force::Instruction&)
{
}

// Fake version of virtual function placed here to supress linker errors.
void Operand::Generate(Force::Generator&, Force::Instruction&)
{
}

// Fake version of virtual function placed here to supress linker errors.
void Operand::Setup(Force::Generator&, Force::Instruction&)
{
}

// Fake version of virtual function placed here to supress linker errors.
void Operand::Initialize(Force::OperandStructure const*)
{
}

// Fake version of virtual function placed here to supress linker errors.
uint32 Operand::Encoding() const
{
  return 0;
}

// Fake version of virtual function placed here to supress linker errors.
const string Operand::ToString() const
{
  return string("bogus");
}

// Fake version of virtual function placed here to supress linker errors.
const text OperandStructure::ToString() const
{
  return string("bogus");
}

// Fake version of virtual function placed here to supress linker errors.
void OperandStructure::AddShortOperand(std::map<const text, const OperandStructure* >& rShortStructures) const
{
}

}

// This needs to be defined here in order to avoid linking to the iss libraries 
bool execute_uop(uint32_t cpuid, EUop uop, UopParameter* inputParams, uint8_t inputParamCount, UopParameter* outputParams, uint8_t* outputParamCount)
{
  switch(uop){
    case UopMulAdd:
      outputParams[0].value = inputParams[0].value * inputParams[1].value + inputParams[2].value;
      return true;
    case UopDiv:
      outputParams[0].value = inputParams[0].value / inputParams[1].value;
      return true;
    case UopMul:
      outputParams[0].value = inputParams[0].value * inputParams[1].value;
      return true;
    case UopAddWithCarry:
      outputParams[0].value = inputParams[0].value + inputParams[1].value + inputParams[2].value;
      return true;
    case UopSubWithCarry:
      if(inputParams[2].value == 0)
      {
        outputParams[0].value = inputParams[0].value -inputParams[1].value - 1;
      }
      else 
      {
        outputParams[0].value = inputParams[0].value -inputParams[1].value;
      }
      return true;
    default:
      LOG(notice) << "Unimplemented operation" << endl;      
  }

  return false;
}

const lest::test specification[] = {

CASE("Test AddressSolutionStrategy::MulSolutionStrategy unconstrained target addresses") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0, MAX_UINT64-1);
    uint64 align_shift = 0;
    EUop uop = UopMul;
    uint32 cpu_id = 0;
    OperandSolutionMap opr_solutions;
    MulSolutionStrategy mul_solution_strategy(&addr_constr, align_shift, uop, cpu_id);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0x100;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=0, operands unconstrained") {
      OperandTest opr_multiplicand = OperandTest(0x0,MAX_UINT64-1);
      OperandTest opr_multiplier = OperandTest(0x0,MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=0, operands fixed, address tagging enabled for data enabled for instruction") {
      bool enable_for_data = true;
      bool enable_for_instruction = true;

      AddressTaggingTest* addr_tagging = new AddressTaggingTest(enable_for_data, enable_for_instruction);
      OperandTest opr_multiplicand = OperandTest(0x0ff0'0fff'ffff'ffff);
      OperandTest opr_multiplier = OperandTest(0x10);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      solution_success &= mul_solution_strategy.Solve(&opr_solutions, target_address, addr_tagging);
      targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
      operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);

      delete addr_tagging;
    }

    SECTION("Test align_shift=0, multiplicand constrained to single value") {
      OperandTest opr_multiplicand = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_multiplier = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=0, multiplier constrained to single value") {
      OperandTest opr_multiplier = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_multiplicand = OperandTest(0,MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }
  }
},

CASE("Test AddressSolutionStrategy::MulSolutionStrategy target addresses constrained to below 0x0000'0000'ffff'ffff") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0, 0x0000'0000'ffff'ffff);
    uint64 align_shift = 0;
    EUop uop = UopMul;
    uint32 cpu_id = 0;
    OperandSolutionMap opr_solutions;
    MulSolutionStrategy mul_solution_strategy(&addr_constr, align_shift, uop, cpu_id);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=0, multiplicand constrained to single value") {
      OperandTest opr_multiplicand = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_multiplier = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=0, multiplier constrained to single value") {
      OperandTest opr_multiplier = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_multiplicand = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }
  }
},

CASE("Test AddressSolutionStrategy::MulAddSolutionStrategy unconstrained target addresses") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0, MAX_UINT64-1);
    uint64 align_shift = 16;
    EUop uop = UopMulAdd;
    uint32 cpu_id = 0;
    OperandSolutionMap opr_solutions;
    MulAddSolutionStrategy mul_add_solution_strategy(&addr_constr, align_shift, uop, cpu_id);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=16, operands unconstrained") {
      OperandTest opr_multiplicand = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_multiplier = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_addend = OperandTest(0x0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));
      opr_solutions.emplace("addend", OperandSolution(&opr_addend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_add_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, operands fixed, address tagging enabled for data enabled for instruction") {
      bool enable_for_data = true;
      bool enable_for_instruction = true;

      AddressTaggingTest* addr_tagging = new AddressTaggingTest(enable_for_data, enable_for_instruction);
      OperandTest opr_multiplicand = OperandTest(0x1'0000'0000);
      OperandTest opr_multiplier = OperandTest(0xff00'0000);
      OperandTest opr_addend = OperandTest(0xff'0000);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("addend", OperandSolution(&opr_addend));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      solution_success &= mul_add_solution_strategy.Solve(&opr_solutions, target_address, addr_tagging);
      targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
      operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);

      delete addr_tagging;
    }

    SECTION("Test align_shift=16, multiplicand constrained to single value") {
      OperandTest opr_multiplicand = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_multiplier = OperandTest(0, MAX_UINT64-1);
      OperandTest opr_addend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));
      opr_solutions.emplace("addend", OperandSolution(&opr_addend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_add_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, multiplier constrained to single value") {
      OperandTest opr_multiplier = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_multiplicand = OperandTest(0, MAX_UINT64-1);
      OperandTest opr_addend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplier));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));
      opr_solutions.emplace("addend", OperandSolution(&opr_addend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_add_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, addend constrained to any above 0x0000'0000'ffff'ffff") {
      OperandTest opr_addend = OperandTest(0x0000'0000'ffff'ffff,MAX_UINT64-1);
      OperandTest opr_multiplicand = OperandTest(0, MAX_UINT64-1);
      OperandTest opr_multiplier = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplier));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("addend", OperandSolution(&opr_addend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_add_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

  }
},

CASE("Test AddressSolutionStrategy::MulAddSolutionStrategy target addresses constrained to below 0x0000'0000'ffff'ffff") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0, 0x0000'0000'ffff'ffff);
    uint64 align_shift = 16;
    EUop uop = UopMulAdd;
    uint32 cpu_id = 0;
    OperandSolutionMap opr_solutions;
    MulAddSolutionStrategy mul_add_solution_strategy(&addr_constr, align_shift, uop, cpu_id);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=16, operands unconstrained") {
      OperandTest opr_multiplicand = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_multiplier = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_addend = OperandTest(0x0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));
      opr_solutions.emplace("addend", OperandSolution(&opr_addend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_add_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, multiplicand constrained to single value") {
      OperandTest opr_multiplicand = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_multiplier = OperandTest(0, MAX_UINT64-1);
      OperandTest opr_addend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));
      opr_solutions.emplace("addend", OperandSolution(&opr_addend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_add_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, multiplier constrained to single value") {
      OperandTest opr_multiplier = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_multiplicand = OperandTest(0, MAX_UINT64-1);
      OperandTest opr_addend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));
      opr_solutions.emplace("addend", OperandSolution(&opr_addend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_add_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, addend constrained to any above 0x0000'0000'ffff'ffff") {
      OperandTest opr_addend = OperandTest(0x0000'0000'ffff'ffff,MAX_UINT64-1);
      OperandTest opr_multiplicand = OperandTest(0, MAX_UINT64-1);
      OperandTest opr_multiplier = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));
      opr_solutions.emplace("addend", OperandSolution(&opr_addend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= mul_add_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }
  }
},

CASE("Test AddressSolutionStrategy::DivSolutionStrategy unconstrained target addresses") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0, MAX_UINT64-1);
    uint64 align_shift = 0;
    EUop uop = UopDiv;
    uint32 cpu_id = 0;
    bool has_signed_operands = false;
    OperandSolutionMap opr_solutions;
    DivSolutionStrategy div_solution_strategy(&addr_constr, align_shift, uop, cpu_id, has_signed_operands);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=0, operands unconstrained") {
      OperandTest opr_dividend = OperandTest(0x1, UINT64_MAX-1);
      OperandTest opr_divisor = OperandTest(0x1, UINT64_MAX-1);

      //Must manually build the solution map
      opr_solutions.emplace("dividend", OperandSolution(&opr_dividend));
      opr_solutions.emplace("divisor", OperandSolution(&opr_divisor));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= div_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=0, operands fixed, address tagging enabled for data enabled for instruction") {
      bool enable_for_data = true;
      bool enable_for_instruction = true;

      AddressTaggingTest* addr_tagging = new AddressTaggingTest(enable_for_data, enable_for_instruction);
      OperandTest opr_dividend = OperandTest(0xf000'ffff'ffff'ffff);
      OperandTest opr_divisor = OperandTest(0x10);

      //Must manually build the solution map
      opr_solutions.emplace("dividend", OperandSolution(&opr_dividend));
      opr_solutions.emplace("divisor", OperandSolution(&opr_divisor));

      //Test that for a few random runs.
      solution_success &= div_solution_strategy.Solve(&opr_solutions, target_address, addr_tagging);
      targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
      operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);

      delete addr_tagging;
    }


    SECTION("Test align_shift=0, dividend constrained to single value") {
      OperandTest opr_dividend= OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_divisor = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("dividend", OperandSolution(&opr_dividend));
      opr_solutions.emplace("divisor", OperandSolution(&opr_divisor));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= div_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=0, divisor constrained to single value") {
      OperandTest opr_divisor = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_dividend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("dividend", OperandSolution(&opr_dividend));
      opr_solutions.emplace("divisor", OperandSolution(&opr_divisor));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= div_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }
  }
},

CASE("Test AddressSolutionStrategy::DivSolutionStrategy target addresses unconstrained") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0, MAX_UINT64-1);
    uint64 align_shift = 0;
    EUop uop = UopDiv;
    uint32 cpu_id = 0;
    bool has_signed_operands = false;
    OperandSolutionMap opr_solutions;
    DivSolutionStrategy div_solution_strategy(&addr_constr, align_shift, uop, cpu_id, has_signed_operands);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=0, operands unconstrained") {
      OperandTest opr_dividend = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_divisor = OperandTest(0x0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("dividend", OperandSolution(&opr_dividend));
      opr_solutions.emplace("divisor", OperandSolution(&opr_divisor));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= div_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=0, dividend constrained to single value") {
      OperandTest opr_dividend = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_divisor = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("dividend", OperandSolution(&opr_dividend));
      opr_solutions.emplace("divisor", OperandSolution(&opr_divisor));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= div_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=0, divisor constrained to single value") {
      OperandTest opr_divisor = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_dividend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("dividend", OperandSolution(&opr_dividend));
      opr_solutions.emplace("divisor", OperandSolution(&opr_divisor));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= div_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }
  }
},

CASE("Test AddressSolutionStrategy::AddWithCarrySolutionStrategy unconstrained target addresses") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0, MAX_UINT64-1);
    uint64 align_shift = 16;
    EUop uop = UopAddWithCarry;
    uint32 cpu_id = 0;
    OperandSolutionMap opr_solutions;
    ConditionFlags condition_flags;
    //Setting the carry flag to 1 changes causes the Uop to add this value onto the addends.
    condition_flags.mCarryFlag = 1;
    AddWithCarrySolutionStrategy add_with_carry_solution_strategy(&addr_constr, align_shift, uop, cpu_id, condition_flags);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=16, operands fixed, address tagging enabled for data disabled for instruction") {
      bool enable_for_data = true;
      bool enable_for_instruction = false;

      AddressTaggingTest* addr_tagging = new AddressTaggingTest(enable_for_data, enable_for_instruction);
      OperandTest opr_addend1 = OperandTest(0xff00'ffff'0000'ffff);
      OperandTest opr_addend2 = OperandTest(0x0000'0000'ffff'0000);

      //Must manually build the solution map
      opr_solutions.emplace("addend1", OperandSolution(&opr_addend1));
      opr_solutions.emplace("addend2", OperandSolution(&opr_addend2));

      //Test that for a few random runs.
      solution_success &= add_with_carry_solution_strategy.Solve(&opr_solutions, target_address, addr_tagging);
      targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
      operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);

      delete addr_tagging;
    }

    SECTION("Test align_shift=16, operands unconstrained") {
      OperandTest opr_addend1 = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_addend2 = OperandTest(0x0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("addend1", OperandSolution(&opr_addend1));
      opr_solutions.emplace("addend2", OperandSolution(&opr_addend2));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= add_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }


    SECTION("Test align_shift=16, addend1 constrained to single value") {
      OperandTest opr_addend1= OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_addend2 = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("addend1", OperandSolution(&opr_addend1));
      opr_solutions.emplace("addend2", OperandSolution(&opr_addend2));


      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= add_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, addend2 constrained to single value") {
      OperandTest opr_addend2 = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_addend1 = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("addend1", OperandSolution(&opr_addend1));
      opr_solutions.emplace("addend2", OperandSolution(&opr_addend2));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= add_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }
  }
},

CASE("Test AddressSolutionStrategy::AddWithCarrySolutionStrategy target addresses constrained to above 0x0000'0000'ffff'ffff") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0000'0000'ffff'ffff, MAX_UINT64-1);
    uint64 align_shift = 16;
    EUop uop = UopAddWithCarry;
    uint32 cpu_id = 0;
    OperandSolutionMap opr_solutions;
    ConditionFlags condition_flags;
    //Setting the carry flag to 1 causes the Uop to add this value onto the addends.
    condition_flags.mCarryFlag = 1;
    AddWithCarrySolutionStrategy add_with_carry_solution_strategy(&addr_constr, align_shift, uop, cpu_id, condition_flags);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=16, operands unconstrained") {
      OperandTest opr_addend1 = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_addend2 = OperandTest(0x0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("addend1", OperandSolution(&opr_addend1));
      opr_solutions.emplace("addend2", OperandSolution(&opr_addend2));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= add_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, addend1 constrained to single value") {
      OperandTest opr_addend1 = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_addend2 = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("addend1", OperandSolution(&opr_addend1));
      opr_solutions.emplace("addend2", OperandSolution(&opr_addend2));


      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= add_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, addend2 constrained to single value") {
      OperandTest opr_addend2 = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_addend1 = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("addend1", OperandSolution(&opr_addend1));
      opr_solutions.emplace("addend2", OperandSolution(&opr_addend2));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= add_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }
  }
},

CASE("Test AddressSolutionStrategy::SubWithCarrySolutionStrategy unconstrained target addresses") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0, MAX_UINT64-1);
    uint64 align_shift = 16;
    EUop uop = UopSubWithCarry;
    uint32 cpu_id = 0;
    OperandSolutionMap opr_solutions;
    ConditionFlags condition_flags;
    //Setting the carry flag to 1 causes the Uop to not subtract and additional unit from the operands. 
    condition_flags.mCarryFlag = 1;
    SubWithCarrySolutionStrategy sub_with_carry_solution_strategy(&addr_constr, align_shift, uop, cpu_id, condition_flags);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=16, operands unconstrained") {
      OperandTest opr_minuend = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_subtrahend = OperandTest(0x0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("minuend", OperandSolution(&opr_minuend));
      opr_solutions.emplace("subtrahend", OperandSolution(&opr_subtrahend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= sub_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, operands fixed, address tagging enabled for data disabled for instruction") {
      bool enable_for_data = true;
      bool enable_for_instruction = false;

      AddressTaggingTest* addr_tagging = new AddressTaggingTest(enable_for_data, enable_for_instruction);
      OperandTest opr_minuend = OperandTest(0xff00'ffff'abcd'fff1);
      OperandTest opr_subtrahend = OperandTest(0x0000'0000'beef'fff1);

      //Must manually build the solution map
      opr_solutions.emplace("minuend", OperandSolution(&opr_minuend));
      opr_solutions.emplace("subtrahend", OperandSolution(&opr_subtrahend));

      //Test that for a few random runs.
      solution_success &= sub_with_carry_solution_strategy.Solve(&opr_solutions, target_address, addr_tagging);
      targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
      operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);

      delete addr_tagging;
    }

    SECTION("Test align_shift=16, minuend constrained to single value") {
      OperandTest opr_minuend = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_subtrahend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("minuend", OperandSolution(&opr_minuend));
      opr_solutions.emplace("subtrahend", OperandSolution(&opr_subtrahend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= sub_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, subtrahend constrained to single value") {
      OperandTest opr_subtrahend = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_minuend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("minuend", OperandSolution(&opr_minuend));
      opr_solutions.emplace("subtrahend", OperandSolution(&opr_subtrahend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= sub_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }
  }
},

CASE("Test AddressSolutionStrategy::SubWithCarrySolutionStrategy target addresses constrained to above 0x0000'0000'ffff'ffff") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0000'0000'ffff'ffff, MAX_UINT64-1);
    uint64 align_shift = 16;
    EUop uop = UopSubWithCarry;
    uint32 cpu_id = 0;
    OperandSolutionMap opr_solutions;
    ConditionFlags condition_flags;
    //Setting the carry flag to 1 causes the Uop to not subtract and additional unit from the operands. 
    condition_flags.mCarryFlag = 1;
    SubWithCarrySolutionStrategy sub_with_carry_solution_strategy(&addr_constr, align_shift, uop, cpu_id, condition_flags);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=16, operands unconstrained") {
      OperandTest opr_minuend = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_subtrahend = OperandTest(0x0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("minuend", OperandSolution(&opr_minuend));
      opr_solutions.emplace("subtrahend", OperandSolution(&opr_subtrahend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= sub_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, minuend constrained to single value") {
      OperandTest opr_minuend = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_subtrahend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("minuend", OperandSolution(&opr_minuend));
      opr_solutions.emplace("subtrahend", OperandSolution(&opr_subtrahend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= sub_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=16, subtrahend constrained to single value") {
      OperandTest opr_subtrahend = OperandTest(0x0000'0000'ffff'ffff);
      OperandTest opr_minuend = OperandTest(0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("minuend", OperandSolution(&opr_minuend));
      opr_solutions.emplace("subtrahend", OperandSolution(&opr_subtrahend));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= sub_with_carry_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }
  }
},

CASE("Test AddressSolutionStrategy target addresses unconstrained") {

  SETUP("Setup AddressSolutionStrategy")  {
    ConstraintSet addr_constr(0x0, MAX_UINT64-1);
    uint64 align_shift = 0;
    EUop uop = UopMul;
    uint32 cpu_id = 0;
    OperandSolutionMap opr_solutions;
    AddressSolutionStrategy generic_solution_strategy(&addr_constr, align_shift, uop, cpu_id);
    bool solution_success = true;
    bool targets_were_in_constraints = true;
    bool operands_were_in_constraints = true;
    uint64 target_address = 0;
    std::size_t num_runs = 15;

    SECTION("Test align_shift=0, operands unconstrained, address tagging not considered") {
      OperandTest opr_multiplicand = OperandTest(0x0, MAX_UINT64-1);
      OperandTest opr_multiplier = OperandTest(0x0, MAX_UINT64-1);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      for(std::size_t trial=0; trial < num_runs; ++trial)
      {
        solution_success &= generic_solution_strategy.Solve(&opr_solutions, target_address);
        targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
        operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);
      }

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);
    }

    SECTION("Test align_shift=0, operands fixed, address tagging enabled for data enabled for instruction") {
      bool enable_for_data = true;
      bool enable_for_instruction = true;

      AddressTaggingTest* addr_tagging = new AddressTaggingTest(enable_for_data, enable_for_instruction);
      OperandTest opr_multiplicand = OperandTest(0x0ff3'0fff'ffff'ffff);
      OperandTest opr_multiplier = OperandTest(0x10);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      solution_success &= generic_solution_strategy.Solve(&opr_solutions, target_address, addr_tagging);
      targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
      operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);

      delete addr_tagging;
    }

    SECTION("Test align_shift=0, operands fixed, address tagging disabled for data enabled for instruction") {
      bool enable_for_data = false;
      bool enable_for_instruction = true;

      AddressTaggingTest* addr_tagging = new AddressTaggingTest(enable_for_data, enable_for_instruction);
      OperandTest opr_multiplicand = OperandTest(0x00ff'00aa'bbbb'cccc);
      OperandTest opr_multiplier = OperandTest(0x100);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      solution_success &= generic_solution_strategy.Solve(&opr_solutions, target_address, addr_tagging);
      targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
      operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);

      delete addr_tagging;
    }

    SECTION("Test align_shift=0, operands fixed, address tagging enabled for data disabled for instruction") {
      bool enable_for_data = true;
      bool enable_for_instruction = false;

      AddressTaggingTest* addr_tagging = new AddressTaggingTest(enable_for_data, enable_for_instruction);
      OperandTest opr_multiplicand = OperandTest(0x0000'ffff'eeee'dddd);
      OperandTest opr_multiplier = OperandTest(0x1000);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      solution_success &= generic_solution_strategy.Solve(&opr_solutions, target_address, addr_tagging);
      targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
      operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);

      delete addr_tagging;
    }

    SECTION("Test align_shift=0, operands fixed, address tagging disabled for data disabled for instruction") {
      bool enable_for_data = false;
      bool enable_for_instruction = false;

      AddressTaggingTest* addr_tagging = new AddressTaggingTest(enable_for_data, enable_for_instruction);
      OperandTest opr_multiplicand = OperandTest(0xffff);
      OperandTest opr_multiplier = OperandTest(0x1'0000'0000'0000);

      //Must manually build the solution map
      opr_solutions.emplace("multiplicand", OperandSolution(&opr_multiplicand));
      opr_solutions.emplace("multiplier", OperandSolution(&opr_multiplier));

      //Test that for a few random runs.
      solution_success &= generic_solution_strategy.Solve(&opr_solutions, target_address, addr_tagging);
      targets_were_in_constraints &= addr_constr.ContainsValue(target_address);
      operands_were_in_constraints &= Helpers::AreOperandsInTheirConstraints(opr_solutions);

      EXPECT(targets_were_in_constraints);
      EXPECT(solution_success);
      EXPECT(operands_were_in_constraints);

      delete addr_tagging;
    }


  }
},

};

int main(int argc, char * argv[])
{
  Force::Logger::Initialize();
  Random::Initialize();
  int ret = lest::run(specification, argc, argv);
  Random::Destroy();
  Force::Logger::Destroy();
  return ret;
}
