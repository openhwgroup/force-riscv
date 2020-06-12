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
#ifndef Force_ResourceAccess_H
#define Force_ResourceAccess_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <Object.h>
#include <vector>

namespace Force {

  class Variable;
  class Generator;
  class ConstraintSet;
  class ResourceTypeEntropy;
  class ResourceAccessQueue;

  /*!
    \class ResourceAccessStage
    \brief Current resource accesses for destination regsiters and source regsiters in an instruction
  */
  class ResourceAccessStage : public Object {
  public:
    ResourceAccessStage(); //!< Constructor.
    ~ResourceAccessStage(); //!< Destructor.

    Object* Clone() const override; //!<Return a cloned object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string for the content of the object.
    const char* Type() const override { return "Resource access stage"; }

    const std::string ToSimpleString() const; //!< Return a simple string representation for debugging purpose.
    void RecordAccess(ERegAttrType access, EResourceType resType, ConstraintSet* pConstr); //!< Record resource access information.
    const ConstraintSet* GetDependenceConstraint(EResourceType resType, EDependencyType depType) const; //!< Get resource dependence constraint.

    inline const ConstraintSet* GetSourceAccess(EResourceType resType) const //!< Get source access of the specified resource type.
    {
      return mSourceAccesses[EResourceTypeBaseType(resType)];
    }

    inline const ConstraintSet* GetDestAccess(EResourceType resType) const //!< Get source access of the specified resource type.
    {
      return mDestAccesses[EResourceTypeBaseType(resType)];
    }

    inline const std::vector<ConstraintSet* >& GetSourceAccesses() const { return mSourceAccesses; } //!< Return source accesses.
    inline const std::vector<ConstraintSet* >& GetDestAccesses() const { return mDestAccesses; } //!< Return dest accesses.

    bool HasSourceAccess(uint32 index, EResourceType resType) const; //!< whether has source access of the specified resoruce type
    bool HasDestAccess(uint32 index, EResourceType resType) const; //!< whether has dest access of the specified resoruce type
    void RemoveSourceAccess(uint32 index, EResourceType resType); //!< Remove source access of the specified resource type, if any.
    void RemoveDestAccess(uint32 index, EResourceType resType); //!< Remove dest access of the specified resource type, if any.
    void Retire(uint32 index, std::vector<ResourceTypeEntropy* >& rTypeEntropy); //!< Retire access stage
  protected:
    ResourceAccessStage(const ResourceAccessStage& rOther); //!< Copy constructor.
  protected:
    std::vector<ConstraintSet* > mSourceAccesses; //!< Source resource accesses.
    std::vector<ConstraintSet* > mDestAccesses; //!< Dest resource accesses.
  };

  /*
    \class AccessAge
    \brief Class recording latest access type and age to a resource.
   */
  class AccessAge {
  public:
    AccessAge(); //!< Default constructor.
    AccessAge(const AccessAge& rOther); //!< Copy Constructor

    void Setup(uint32 index, EResourceType resType); //!< Setup index and resource type for performance gain.
    void UpdateAge(uint32 age, EAccessAgeType ageType, ResourceAccessQueue* pAccessQueue);  //!< Update resource time stamp.
    void CleanUp(); // clean up age
    uint32 Index() const { return mIndex; } //!< Return current index.
    uint32 Age() const { return mAge; } //!< Return current age.
    EAccessAgeType AccessType() const { return mAccessType; } //!< Return current age item access type.
    EResourceType ResourceType() const { return mResourceType; } //!< Return current age item resource type.
  private:

  private:
    uint32 mIndex; //!< Register index.
    uint32 mAge; //!< Time stamp of the access.
    EResourceType mResourceType; //!< Type of the resource.
    EAccessAgeType mAccessType; //!< Type of the resource access time stamp.
  };

  /*
    \class ResourceTypeAges
    \brief Class hosting latest resource access age for the same type of resources.
  */
  class ResourceTypeAges : public Object {
  public:
    explicit ResourceTypeAges(EResourceType type) : mType(type), mAges() { } //!< Constructor with type parameter.
    ~ResourceTypeAges() { } //!< Destructor.

    Object* Clone() const override; //!<Return a cloned object of the same type and same contents of the object.
    const std::string ToString() const override { return ""; } //!< Return a string describing the current state of the object.
    const char* Type() const override { return "resource type ages"; }

    void Setup(uint32 numEntries); //!< Set up resource age entries.
    void UpdateAge(uint32 index, uint32 age, EAccessAgeType ageType, ResourceAccessQueue* pAccessQueue); //!< Update resource access age.
    const AccessAge& GetAccessAge(uint32 index) const; //!< Return access age for an entry.
    EResourceType ResourceType() const { return mType; } //!< Return the type attribute.
  private:
    ResourceTypeAges() : mType(EResourceType(0)), mAges() { } //!< Default constructor.
    ResourceTypeAges(const ResourceTypeAges& rOther); //!< Copy constructor
  private:
    EResourceType mType; //!< Type of resource.
    std::vector<AccessAge> mAges; //!< Resource access ages.
  };

  /*!
    \class AccessEntropy
    \brief Entropy for access type.
  */
  class AccessEntropy {
  public:
    AccessEntropy(); //!< Constructor.
    AccessEntropy(const AccessEntropy& rOther); //!< copy constructor
    ~AccessEntropy() { } //!< Destructor.

    void SetName(const std::string& rName) { mName = rName; } //!< Set object name.
    const std::string& Name() const { return mName; } //!< Return object name.
    void SetThresholds(uint32 on, uint32 off); //!< Set thresholds.
    void SetThresholds(const std::string& rStr); //!< Set thresholds from string parameter.
    inline uint32 OnThreshold() const { return mOnThreshold; } //!< Get on threshold
    inline uint32 OffThreshold() const { return mOffThreshold; } //!< Get off threshold
    const std::string ToString() const; //!< Return string description of the object.
    void UpdateState(); //!< Update entropy state.
    inline void Increase(uint32 num = 1) { mEntropy += num; } //!< Increase entropy.
    inline void Decrease(uint32 num = 1); //!< Decrease entropy.
    inline uint32 Entropy() const { return mEntropy; } //!< Return entropy value.
    inline bool Stable() const { return (mState == EEntropyStateType::Stable); } //!< Return if entropy state is stable.
    inline EEntropyStateType State() const { return mState; } //!< Return state
  private:
    std::string mName; //!< Access entropy object name.
    EEntropyStateType mState; //!< Entropy state.
    uint32 mEntropy; //!< Entropy value.
    uint32 mOnThreshold; //!< Dependence on threshold.
    uint32 mOffThreshold; //!< Dependence off threshold.
  };

   /*
    \class ResourceTypeEntropy
    \brief Class hosting latest resource entropy for the same type of resources.
  */
  class ResourceTypeEntropy : public Object {
  public:
    explicit ResourceTypeEntropy(EResourceType type); //!< Constructor with type parameter.
    ~ResourceTypeEntropy() {} //!< destructor

    Object* Clone() const override; //!< Return a cloned object of the same type and same contents of the object
    const std::string ToString() const override { return ""; }; //!<  Return a string describing the current state of the object.
    const char* Type() const override { return "resource type entropy"; }

    EResourceType ResourceType() const { return mType; } //!< Return the type attribute.
    inline AccessEntropy& SourceEntropy() {return mSourceEntropy; } //!< get source entropy
    inline AccessEntropy& DestEntropy() {return mDestEntropy; } //!< get dest entropy
  private:
    ResourceTypeEntropy(const ResourceTypeEntropy& rOther); //!< copy construtor
  private:
    EResourceType mType; //!< Type of resource.
    AccessEntropy mSourceEntropy; //!< Source entropy object.
    AccessEntropy mDestEntropy; //!< Destination entropy object.
  };

  /*!
    \class WindowLookUp
    \brief Class managing lookup in a slot window.
  */
  class WindowLookUp {
  public:
    WindowLookUp() : mLow(0), mHigh(0) { } //!< Default constructor.
    virtual ~WindowLookUp() { } //!< Destructor.
    virtual void SlideWindow(uint32& value) const = 0; //!< Slide the lookup window.
    void SetRange(uint32 low, uint32 high) //!< Set lookup window boundaries.
    {
      mLow = low; mHigh = high;
    }

    uint32 Low() const { return mLow; } //!< Return low bound.
    uint32 High() const { return mHigh; } //!< Return high bound.
    uint32 Size() const { return (mHigh - mLow + 1); } //!< Return lookup range size.
    virtual const char* Direction() const = 0; //!< Return direction string.
  protected:
    WindowLookUp(const WindowLookUp& rOther) : mLow(rOther.mLow), mHigh(rOther.mHigh) { } //!< copy constructor
  protected:
    uint32 mLow; //!< Lower bound of the lookup window.
    uint32 mHigh; //!< Higher bound of the lookup window.
  };

  /*!
    \class WindowLookUpFar
    \brief Window lookup, slide window from near to far.
  */
  class WindowLookUpFar : public WindowLookUp {
  public:
    WindowLookUpFar() : WindowLookUp() { } //!< Constructor.
    void SlideWindow(uint32& value) const override; //!< Slide the lookup window from near to far.
    const char* Direction() const override { return "Near-to-Far"; } //!< Return direction string.
    WindowLookUpFar(const WindowLookUpFar& rOther) : WindowLookUp(rOther) { } //!< copy constructor
  };

  /*!
    \class WindowLookUpNear
    \brief Window lookup, slide window from far to near.
  */
  class WindowLookUpNear : public WindowLookUp {
  public:
    WindowLookUpNear() : WindowLookUp() { } //!< Constructor.
    void SlideWindow(uint32& value) const override; //!< Slide the lookup window from far to near.
    const char* Direction() const override { return "Far-to-Near"; } //!< Return direction string.
    WindowLookUpNear(const WindowLookUpNear& rOther) : WindowLookUp(rOther) { }  //!< copy constructor
  };

  /*!
    \class ResourceAccessQueue
    \brief Resource access queue.
  */
  class ResourceAccessQueue : public Object {
  public:
    ResourceAccessQueue();  //!< Constructor.
    ~ResourceAccessQueue(); //!< destructor

    ASSIGNMENT_OPERATOR_ABSENT(ResourceAccessQueue);
    Object* Clone() const override; //!< return cloned object
    const std::string ToString() const override;  //!< Return a string describing the current state of the ResourceAccessQueue object.
    const char* Type() const override { return "ResourceAccessQueue"; } //!< Return the type of the ResourceAccessQueue in C string

    void Setup(uint32 historyLimit); //!< Set up the queue with the history limit given.
    ResourceAccessStage* CreateHotResource(); //!< Get hot resource.
    void Commit(ResourceAccessStage* pHotResource); //!< Commit hot resource.
    ResourceAccessStage* GetAccessStage(uint32 dist); //!< Get slot item by distance.
    const ResourceAccessStage* ChosenAccessStage(uint32 dist) const; //!< Return chosen slot item.
    void RemoveSourceAccess(uint32 index, uint32 age, EResourceType resType); //!< Remove a source access with specified parameters.
    void RemoveDestAccess(uint32 index, uint32 age, EResourceType resType); //!< Remove a dest access with specified parameters.
    const ConstraintSet* GetOptimalResourceConstraint(uint32 chosenValue,const WindowLookUp& rLookUp, EResourceType resType, EDependencyType depType) const; //!< Get optimal resource constraint.
    const ConstraintSet* GetRandomResourceConstraint(uint32 low, uint32 high, EResourceType resType, EDependencyType depType) const; //!< Get random resource constraint.
    inline const std::vector<ResourceTypeEntropy* >& GetResourceTypeEntropies() const { return mTypeEntropies; } //!< Get resource Type entropies
  protected:
    ResourceAccessQueue(const ResourceAccessQueue& rOther); //!< Copy constructor.
    ResourceAccessQueue(ResourceAccessQueue& rOther); //!< Copy constructor.
    void RetireReuseStage(); //!< Retire a stage item from the queue and reuse it.
    void UpdateAccessAge(const ResourceAccessStage* pHotResource); //!< Update resource access age.
    void UpdateAccessEntropy(const ResourceAccessStage* pHotResource); //!< update resource access entropy
    void UpdateEntropyState(); //!< update resource entropy state
    inline uint32 GetQueueIndex(uint32 dist) const //!< Return queue index based on the distance parameter given.
    {
      return (mIndex + mHistoryLimit - dist) % mHistoryLimit;
    }

    bool EntropyStable(EResourceType resType, EDependencyType depType) const; //!< whether the current entropy is low.
    void VerifyAccessEntropy() const; //!< Verify access entropy value, a debugging method.
    uint32 CalculateAccessEntropy(EResourceType resType, EDependencyType depType) const; //!< Calculate entroy for a resource type and dependency type, a debugging method.
  protected:
    uint32 mAge; //!< Current resource age.
    uint32 mHistoryLimit; //!< Resource access history limit.
    uint32 mIndex; //!< Index of the current resources slot.
    mutable WindowLookUpFar mLookUpFar; //!< Used in window lookup far to near.
    mutable WindowLookUpNear mLookUpNear; //!< Used in window lookup near to far.
    mutable ConstraintSet* mpReturnConstraint; //!< Pointer to a constraint set to be returned.
    std::vector<ResourceAccessStage* > mQueue; //!< Resource access entries submitted.
    std::vector<ResourceTypeAges* > mTypeAges; //!< Ages contains for all supported resource types.
    std::vector<ResourceTypeEntropy* > mTypeEntropies; //!< Entropies contains for all supported resource types.
  };

}

#endif
