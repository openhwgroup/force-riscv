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
#ifndef Force_Choices_H
#define Force_Choices_H

#include <Object.h>

#include <Defines.h>
#include <vector>
#include <string>
#include <map>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  class ChoicesFilter;
  class ConstraintSet;

  /*!
    \class Choice
    \brief Base class of Choice/Choices
  */
  class Choice : public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the Choice object.
    const std::string ToString() const override; //!< Return a string describing the current state of the Choice object.
    const char* Type() const override { return "Choice"; } //!< Return a string describing the actual type of the Choice Object

    Choice(const std::string& name, uint32 value, uint32 weight) //!< Constructor with necessary parameters.
      : Object(), mName(name), mValue(value), mWeight(weight)
    {

    }

    Choice() : Object(), mName(), mValue(0), mWeight(0) { } //!< Constructor.
    ~Choice(); //!< Destructor.

    const std::string& Name() const { return mName; } //!< Return the name of the Choice object.
    virtual uint32 Value() const { return mValue; } //!< Return value contained in the Choice object.
    inline uint64 ValueAs64() const { return (uint64)mValue; } //!< Return value as an unsigned 64 bit integer.
    uint32 Weight() const { return mWeight; } //!< Return relative weight for the Choice object.
    void SetWeight(uint32 weight) { mWeight = weight; } //!< Set weight to a new value.
    virtual const Choice* Choose() const { return this; } //!< Return a const pointer to self.
    virtual Choice* ChooseMutable() { return this; } //!< Return a mutable pointer to self.
    virtual uint32 ApplyFilter(const ChoicesFilter& filter); //!< Apply choices filter.

    virtual const Choice* CyclicChoose() //!< Method used by hierarchical cyclic choosing.
    {
      mWeight = 0; // not choosing again until weight restored.
      return this;
    }

    virtual void RestoreWeight(const Choice* pRefChoice) //!< Restore weight from reference Choice object.
    {
      mWeight = pRefChoice->Weight();
    }

    virtual bool HasChoice() const { return (mWeight > 0); } //!< Return if the choice is available.
    virtual uint32 AvailableChoices() const { return (mWeight > 0) ? 1 : 0; } //!< Return number of available choices, 1 or 0.
    virtual void GetAvailableChoices(const ChoicesFilter& filter, std::vector<const Choice*>& rChoicesList) const; //!< Get available choices after filtering.

    virtual void GetAvailableChoices(std::vector<const Choice*>& rChoicesList) const //!< Get available choices.
    {
      if (mWeight) rChoicesList.push_back(this);
    }

  protected:
    Choice(const Choice& rOther); //!< Copy constructor.
  protected:
    std::string mName; //!< Name of choice.
    uint32 mValue; //!< Associated value of the choice.
    uint32 mWeight; //!< Relative weight of the choice.
  };

  /*!
    \class RangeChoice
    \brief range choice
  */
class RangeChoice : public Choice {
  public:
    Object* Clone() const override;  //!< Return a cloned object
    const std::string ToString() const override; //!< Return a string describing the current state of the Choice object.
    const char* Type() const override { return "RangeChoice"; } //!< Return a string describing the actual type of the RangeChoice Object
    RangeChoice() : Choice(), mLower(0), mHigh(0) {} //!< constructor
    RangeChoice(const std::string& name, const std::string& range, uint32 weight); //!< constructor
    ~RangeChoice() {} //!< Destructor.
    uint32 Value() const override; //!< return value contained in range choice
    inline void GetRange(uint32& lower, uint32& high) const { lower = mLower; high = mHigh; } //!< return range
    void LimitRange(uint32 limitValue); //!< Adjust range boundaries if outside of the limit.
  protected:
    RangeChoice(const RangeChoice& rOther); //!< Copy constructor.
  protected:
    uint32 mLower; //!< lower value
    uint32 mHigh;  // higher value
  };

  /*!
    \class ChoiceTree
    \brief A non leaf or a root node in a choice tree.
  */
  class ChoiceTree : public Choice {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the ChoiceTree object.
    const std::string ToString() const override; //!< Return a string describing the current state of the ChoiceTree object.
    const char* Type() const override { return "ChoiceTree"; } //!< Return a string describing the actual type of the ChoiceTree Object

    ChoiceTree(const std::string& name, uint32 value, uint32 weight) //!< Constructor with necessary parameters.
      : Choice(name, value, weight), mChoices()
      {

      }

    ChoiceTree() : Choice(), mChoices() { } //!< Constructor.
    ~ChoiceTree(); //!< Destructor.

    const Choice* Choose() const override; //!< Choose a random choice from the children.
    Choice* ChooseMutable() override; //!< Return a mutable pointer to chosen choice.
    uint32 ApplyFilter(const ChoicesFilter& filter) override; //!< Apply choices filter to the ChoiceTree.
    const Choice* CyclicChoose() override; //!< Method used by hierarchical cyclic choosing.
    void RestoreWeight(const Choice* pRefChoice) override; //!< Restore weight from reference ChoiceTree object.
    uint64 ChooseValueWithHardConstraint(const ConstraintSet& rHardConstr); //!< Choose value with a hard constraint, if no choice available then randomly choose from the hard constraint.  Constraint is applied to the ChoiceTree.
    uint64 ChooseValueWithConstraint(const ConstraintSet& rConstr, bool& rHasChoice, uint64& rFallBackValue); //!< Choose value with a constraint, if no choice available, then select without the constraint and return the value in rFallBackValue.  Constraint is applied to the ChoiceTree.

    void AddChoice(Choice* choice); //!< Add choice item.
    const std::vector<Choice* >& GetChoices() const { return mChoices; } //!< Return a constant reference to the choices.
    std::vector<Choice* >& GetChoicesMutable() { return mChoices; } //!< Return a modifiable reference to the choices.
    const Choice* FindChoiceByValue(uint32 value) const; //!< Find a Choice child that has the specified value.
    Choice* FindChoiceByValue(uint32 value); //!< Find a Choice child that has the specified value.
    const Choice* FindChoiceByName(const std::string& rChoiceName) const; //!< Find a Choice child that has the specified name.

    inline Choice* Chosen(uint64 pickedValue) const { //!< Return a Choice object based on the pickedValue given.
      for (auto const choice_item: mChoices) {
        if (pickedValue < choice_item->Weight()) {
          return choice_item;
        }
        pickedValue -= choice_item->Weight();
      }
      return nullptr;
    }

    bool HasChoice() const override; //!< Return if any choice is available.
    bool OnlyChoice() const; //!< Return if only one choice is available.
    uint32 AvailableChoices() const override; //!< Return number of available choices.
    void GetAvailableChoices(std::vector<const Choice*>& rChoicesList) const override; //!< Obtain available choices.
    void GetAvailableChoices(const ChoicesFilter& filter, std::vector<const Choice*>& rChoicesList) const override; //!< Obtain available choices after filtering.
  protected:
    ChoiceTree(const ChoiceTree& rOther); //!< Copy constructor.
  protected:
    std::vector<Choice* > mChoices; //!< Container of all children choices.
  private:
    uint32 SumChoiceWeights() const; //!< Get the sum of the weights of all choices.
  };

  /*!
    \class CyclicChoiceTree
    \brief A ChoiceTree sub class that support cyclic picking.
  */
  class CyclicChoiceTree : public ChoiceTree {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the CyclicChoiceTree object.
    const char* Type() const override { return "CyclicChoiceTree"; } //!< Return a string describing the actual type of the CyclicChoiceTree Object

    explicit CyclicChoiceTree(ChoiceTree* pBaseChoiceTree); //!< Constructor with base ChoiceTree given.
    ~CyclicChoiceTree(); //!< Destructor.
    const Choice* CyclicChoose() override; //!< Method used by hierarchical cyclic choosing.
  protected:
    CyclicChoiceTree() : ChoiceTree(), mpBaseChoiceTree(nullptr) { } //!< Constructor.
    CyclicChoiceTree(const CyclicChoiceTree& rOther); //!< Copy constructor.
    ASSIGNMENT_OPERATOR_ABSENT(CyclicChoiceTree);
  protected:
    ChoiceTree* mpBaseChoiceTree; //!< Pointer to base ChoiceTree.
  };

  /*!
    \class ChoicesSet
    \brief A collection of choices tree of the same type
  */
  class ChoicesSet {
  public:
    explicit ChoicesSet(const EChoicesType type) : mChoiceType(type), mChoiceTrees() { } //!< Constructor.
    ~ChoicesSet(); //!< Destructor.
    COPY_CONSTRUCTOR_ABSENT(ChoicesSet); //!< No copy constructor defined.

    const ChoiceTree* FindChoiceTree(const std::string& treeName) const; //!< Find a choices tree given the name.
    const ChoiceTree* TryFindChoiceTree(const std::string& treeName) const; //!< Find a choices tree given the name, called when not sure if choice tree exists or not
    void AddChoiceTree(ChoiceTree* choiceTree); //!< Add an ChoiceTree to the container.
    const std::string ToString() const; //!< Return a string describing the current state of the ChoicesSet object.
    std::string Name() const { return EChoicesType_to_string(mChoiceType); }
    const EChoicesType Type() const { return mChoiceType;}
  private:
    EChoicesType mChoiceType;
    std::map<std::string, ChoiceTree* > mChoiceTrees; //!< Container of all the ChoiceTrees of the same type.
  };

}

#endif
