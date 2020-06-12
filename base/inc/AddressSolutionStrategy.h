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
#ifndef Force_AddressSolutionStrategy_H
#define Force_AddressSolutionStrategy_H
#include <UopInterface.h>
#include <ConditionFlags.h>
#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <map>

namespace Force {

  class AddressTagging;
  class ConstraintSet;
  class OperandSolution;
  class OperandSolutionMap;

  /*!
    \class AddressSolutionStrategy
    \brief Class to support solving for operands in order to yield a valid target address.
  */
  class AddressSolutionStrategy {
  public:
    AddressSolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(AddressSolutionStrategy);
    SUPERCLASS_DESTRUCTOR_DEFAULT(AddressSolutionStrategy); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(AddressSolutionStrategy);

    virtual bool Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging=nullptr) const; //!< Solve for operand values and target address; return true if successful.
  protected:
    bool HasRepeatedRegisters(const OperandSolutionMap& rOprSolutions) const; //!< Return true if two or more operands use the same register.
    bool IsTargetAddressValidAndAligned(uint64 targetAddress) const; //!< Return true if the target address parameter is valid and aligned.
    OperandSolution* GetOperandSolution(const std::string& rOprRole, OperandSolutionMap* pOprSolutions) const; //!< Get the operand solution corresponding to the specified role.
    const ConstraintSet* GetAddressConstraint() const; //!< Return the address constraint.
    uint32 GetAlignShift() const; //!< Return the alignment shift.
    uint32 GetCpuId() const; //!< Return the CPU ID.
  private:
    const ConstraintSet* mpAddressConstr; //!< Constraint specifying permissible target address values.
    cuint32 mAlignShift; //!< Address alignment shift amount.
    const EUop mUop; //!< Micro-op type.
    cuint32 mCpuId; //!< ID of CPU on which instruction will be executed.
  };

  /*!
    \class MulAddSolutionStrategy
    \brief Class to solve for operands in order to yield a valid target address for multiply-add operations.
  */
  class MulAddSolutionStrategy : public AddressSolutionStrategy {
  public:
    MulAddSolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(MulAddSolutionStrategy);
    SUBCLASS_DESTRUCTOR_DEFAULT(MulAddSolutionStrategy); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(MulAddSolutionStrategy);

    bool Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging=nullptr) const override; //!< Solve for operand values and target address; return true if successful.
  protected:
    bool SolveForMultiplicationOperand(cuint64 indOperandValue, cuint64 addend, const ConstraintSet& rDepOperandConstr, uint64& rDepOperandValue) const; //!< Solve for one of the multiplication operands when the other multiplication operand value and addend value are specified; return true if succesful.
    uint64 ComputeTargetAddress(cuint64 multiplicand, cuint64 multiplier, cuint64 addend) const; //!< Compute the target address given the operand values.
    void DivideConstraintElements(cuint64 divisor, ConstraintSet* pConstr) const; //!< Modify constraint to contain elements E such that E * divisor is in the input constraint. This method only computes of subset of such elements in order to save time.
  private:
    bool SolveForAddend(cuint64 multiplicand, cuint64 multiplier, const ConstraintSet& rAddendConstr, uint64& rAddendValue) const; //!< Solve for the addend when the multiplication operand values are sepcified; return true if succesful.
  };

  /*!
    \class DivSolutionStrategy
    \brief Class to solve for operands in order to yield a valid target address for division operations.
  */
  class DivSolutionStrategy : public AddressSolutionStrategy {
  public:
    DivSolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId, cbool signedOperands); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(DivSolutionStrategy);
    SUBCLASS_DESTRUCTOR_DEFAULT(DivSolutionStrategy); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(DivSolutionStrategy);

    bool Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging=nullptr) const override; //!< Solve for operand values and target address; return true if successful.
  private:
    uint64 ComputeTargetAddress(cuint64 dividend, cuint64 divisor) const; //!< Compute the target address given the operand values.
  private:
    cbool mSignedOperands; //!< True if operands are signed.
  };

  /*!
    \class AddWithCarrySolutionStrategy
    \brief Class to solve for operands in order to yield a valid target address for add with carry operations.
  */
  class AddWithCarrySolutionStrategy : public AddressSolutionStrategy {
  public:
    AddWithCarrySolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId, const ConditionFlags condFlags); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(AddWithCarrySolutionStrategy);
    SUBCLASS_DESTRUCTOR_DEFAULT(AddWithCarrySolutionStrategy); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(AddWithCarrySolutionStrategy);

    bool Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging=nullptr) const override; //!< Solve for operand values and target address; return true if successful.
  private:
    bool SolveForAdditionOperand(cuint64 indOperandValue, const ConstraintSet& rDepOperandConstr, uint64& rDepOperandValue) const; //!< Solve for one of the addition operands when the other addition operand value is specified; return true if succesful.
    uint64 ComputeTargetAddress(cuint64 addend1, cuint64 addend2) const; //!< Compute the target address given the operand values.
  private:
    ConditionFlags mCondFlags; //!< Condition flags.
  };

  /*!
    \class SubWithCarrySolutionStrategy
    \brief Class to solve for operands in order to yield a valid target address for subtract with carry operations.
  */
  class SubWithCarrySolutionStrategy : public AddressSolutionStrategy {
  public:
    SubWithCarrySolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId, const ConditionFlags condFlags); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(SubWithCarrySolutionStrategy);
    SUBCLASS_DESTRUCTOR_DEFAULT(SubWithCarrySolutionStrategy); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(SubWithCarrySolutionStrategy);

    bool Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging=nullptr) const override; //!< Solve for operand values and target address; return true if successful.
  private:
    bool SolveForMinuend(cuint64 subtrahendValue, const ConstraintSet& rMinuendConstr, uint64& rMinuendValue) const; //!< Solve for the minuend when the subtrahend value is specified; return true if succesful.
    bool SolveForSubtrahend(cuint64 minuendValue, const ConstraintSet& rSubtrahendConstr, uint64& rSubtrahendValue) const; //!< Solve for the subtrahend when the minuend value is specified; return true if succesful.
    uint64 ComputeTargetAddress(cuint64 minuend, cuint64 subtrahend) const; //!< Compute the target address given the operand values.
  private:
    ConditionFlags mCondFlags; //!< Condition flags.
  };

  /*!
    \class MulSolutionStrategy
    \brief Class to solve for operands in order to yield a valid target address for multiply operations.
  */
  class MulSolutionStrategy : public MulAddSolutionStrategy {
  public:
    MulSolutionStrategy(const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(MulSolutionStrategy);
    SUBCLASS_DESTRUCTOR_DEFAULT(MulSolutionStrategy); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(MulSolutionStrategy);

    bool Solve(OperandSolutionMap* pOprSolutions, uint64& rTargetAddress, const AddressTagging* pAddrTagging=nullptr) const override; //!< Solve for operand values and target address; return true if successful.
  };

  /*!
    \class AddressSolutionStrategyFactory
    \brief Class to generate different types of solution strategies.
  */
  class AddressSolutionStrategyFactory {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(AddressSolutionStrategyFactory); //!< Default constructor
    COPY_CONSTRUCTOR_ABSENT(AddressSolutionStrategyFactory);
    DESTRUCTOR_DEFAULT(AddressSolutionStrategyFactory); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(AddressSolutionStrategyFactory);

    AddressSolutionStrategy* CreateSolutionStrategy(const EDataProcessingOperationType operationType, const ConstraintSet* pAddressConstr, cuint32 alignShift, const EUop uop, cuint32 cpuId, const ConditionFlags condFlags) const; //!< Create an AddressSolutionStrategy of the appropriate type in order to solve for the specified operation type.
  };

}

#endif  // Force_AddressSolutionStrategy_H
