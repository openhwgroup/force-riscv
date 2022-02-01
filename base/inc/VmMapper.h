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
#ifndef Force_VmMapper_H
#define Force_VmMapper_H

#include <Defines.h>
#include <Object.h>
#include <Notify.h>
#include <NotifyDefines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>
#include <map>
#include <fstream>

namespace Force {

  class VmFactory;
  class Generator;
  class ConstraintSet;
  class Page;
  class GenPageRequest;
  class Register;
  class TranslationRange;
  class PageInformation;
  class VmInfo;
  class RegisterReload;
  class AddressTagging;
  class PhysicalRegion;
  class AddressFilteringRegulator;
  class AddressReuseMode;

  /*!
    \class VmMapper
    \brief Virtual memory mapper class
  */

  class VmMapper {
  public:

    explicit VmMapper(const VmFactory* pFactory) : mpGenerator(nullptr), mpVmFactory(pFactory), mState(EVmStateType::Uninitialized) { } //!< Constructor with VmFactory parameter.
    virtual ~VmMapper() = default; //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(VmMapper);

    //State and Setup Methods
    virtual void Setup(Generator* gen) { mpGenerator = gen; } //!< Setup the virtual memory mapper object.
    virtual void Activate()   { mState = EVmStateType::Active;        } //!< Activate the VmMapper object.
    virtual void Initialize() { mState = EVmStateType::Initialized;   } //!< Initialize the VmMapper object.
    virtual void Deactivate() { mState = EVmStateType::Uninitialized; } //!< Deactivate the VmMapper object.
    bool IsActive()      const { return (mState == EVmStateType::Active); } //!< Return if the VmMapper is active.
    bool IsInitialized() const { return (mState == EVmStateType::Active || mState == EVmStateType::Initialized); } //!< Return if the VmMapper is active.

    GenPageRequest* GenPageRequestInstance(bool isInstr=true, EMemAccessType memAccessType=EMemAccessType::Unknown) const; //!< Return a GenPageRequest instance.
    GenPageRequest* GenPageRequestRegulated(bool isInstr=true, EMemAccessType memAccessType=EMemAccessType::Unknown, bool noFault = false) const; //!< Retuan a regulated GenPageRequest instance.
    bool VerifyStreamingVa(uint64 va, uint64 size, bool isInstr) const; //!< Verify if the streaming virtual address range is usable.
    bool VerifyVirtualAddress(uint64 va, uint64 size, bool isInstr, const GenPageRequest* pPageReq) const; //!< Verify if the virtual address range is usable.

    //Accessors
    const Generator* GetGenerator() const { return mpGenerator; } //!< Return associated generator object.
    const VmFactory* GetVmFactory() const { return mpVmFactory; } //!< Return const pointer to VmFactory object.
    const AddressFilteringRegulator* GetAddressFilteringRegulator() const; //!< Return a pointer to address filtering regulator object.

    //Mapper Interface Definition
    virtual void   AddPhysicalRegion(PhysicalRegion* pRegion, bool map) = 0; //!< Add Physical Region to be mapped
    virtual void   ApplyVirtualUsableConstraint(const EMemDataType memDataType, const EMemAccessType memAccessType, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const = 0; //!< Apply virtual usable constraint to specified constraint.
    virtual void   DumpPage(const EDumpFormat dumpFormat, std::ofstream& os) const = 0; //!< Dump pages.
    virtual void   GetVmContextDelta(std::map<std::string, uint64> & rDeltaMap) const = 0; //!< Find the delta map between the VmMapper and currect machine state.
    virtual bool   GetPageInfo(uint64 addr, const std::string& type, uint32 bank, PageInformation& page_info) const = 0; //!< Return the page information record according to the given address/address type
    virtual bool   GetTranslationRange(uint64 VA, TranslationRange& rTransRange) const = 0; //!< Get translation range that covers the VA, if exists.
    virtual bool   MapAddressRange(uint64 VA, uint64 size, bool isInstr, const GenPageRequest* pPageReq=nullptr) = 0; //!< Map virtual address range.
    virtual bool   VaInAddressErrorRange(const uint64 VA) const = 0; //!< Check if VA is in address error range.
    virtual bool   ValidateContext(std::string& rErrMsg) const = 0; //!< Validate a VmMapper's context is valid before switching into it.
    virtual uint32 GenContextId() const = 0; //!< By default this is not implemented.
    virtual uint64 MapAddressRangeForPA(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, const GenPageRequest* pPageReq=nullptr) = 0; //!< Map virtual address range for a given PA.
    virtual uint64 MaxVirtualAddress() const = 0; //!< Get max virtual address of current mapper
    virtual uint64 MaxPhysicalAddress() const = 0; //!< Get max physical address of current mapper
    virtual ConstraintSet*        VirtualUsableConstraintSetClone(bool isInstr) = 0; //!< Return cloned pointer to applicable virtual constraint object.
    virtual RegisterReload*       GetRegisterReload() const = 0; //!< Get register reload pointer
    virtual const AddressTagging* GetAddressTagging() const = 0; //!< Get address tagging object.
    virtual const ConstraintSet*  VirtualUsableConstraintSet(bool isInstr) const = 0; //!< Return const pointer to applicable virtual constraint object.
    virtual const ConstraintSet*  VirtualSharedConstraintSet() const = 0; //!< Return const pointer to virtual shared constraint object.
    virtual const ConstraintSet*  GetVmConstraint(EVmConstraintType constrType) const = 0; //!< Obtaisn certain type of VM constraint.
    virtual const Page*           GetPage(uint64 VA) const = 0; //!< Return the page that contains the virtual address, if exists.
    virtual ETranslationResultType   TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const = 0; //!< Translate VA to PA, return true if address is mapped.
    virtual ETranslationResultType   TranslatePaToVa(uint64 PA, EMemBankType bank, uint64& VA) const = 0; //!< Translate VA to PA, return true if address is mapped.
    virtual EExceptionConstraintType GetExceptionConstraintType(const std::string& rExceptName) const = 0; //!< Return exception constraint type for the specified exception name.
    virtual EMemBankType             DefaultMemoryBank() const = 0; //!< Return default memory bank type.

    virtual bool VerifyStreamingPageCrossing(uint64 start, uint64 end) const = 0; //!< Check instruction stream page crossing.
    virtual void ApplyVmConstraints(const GenPageRequest* pPageReq, ConstraintSet& rConstr) const = 0; //!< Apply VmConstraints.

  protected:
    VmMapper(const VmMapper& rOther) : mpGenerator(nullptr), mpVmFactory(rOther.mpVmFactory), mState(EVmStateType::Uninitialized) { } //!< Copy constructor.
    VmMapper() : mpGenerator(nullptr), mpVmFactory(nullptr), mState(EVmStateType::Uninitialized) { } //!< Default constructor.
  protected:
    Generator* mpGenerator; //!< Pointer to Generator object.
    const VmFactory* mpVmFactory; //!< Factory for creating VM objects.
    EVmStateType mState; //!< Indicate the mapper's initialized and active state
  };

  class VmAddressSpace;
  class VmContext;

  /*!
    \class VmRegime
    \brief Represent a virtual memory regime in a virtual memory system architecture.
  */

  class VmRegime : public Object {
  public:
    Object * Clone() const override { return new VmRegime(*this); } //!< Clone VmRegime object.
    const std::string ToString() const override { return Type(); } //!< Return a string describing the VmRegime object.
    const char* Type() const override { return "VmRegime"; }

    explicit VmRegime(const VmFactory* pVmFactory, EVmRegimeType regimeType, EMemBankType bankType) : Object(), mRegimeType(regimeType), mDefaultBankType(bankType), mState(EVmStateType::Uninitialized), mpGenerator(nullptr), mpVmFactory(pVmFactory), mpCurrentMapper(nullptr) { } //!< Constructor with regime type parameter given.
    ~VmRegime() override {} //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(VmRegime);

    EVmRegimeType VmRegimeType()      const { return mRegimeType; } //!< Return regime type of this VmRegime object.
    EMemBankType  DefaultMemoryBank() const { return mDefaultBankType; } //!< Return default memory bank type.
    VmMapper*     CurrentVmMapper()   const { return mpCurrentMapper; } //!< Return a pointer to the curernt VmMapper object in the virtual memory translation regime.
    const VmFactory* GetVmFactory()   const { return mpVmFactory; } //!< Return const pointer to VmFactory object.

    virtual void Setup(Generator* gen) { mpGenerator = gen; } //!< Setup the virtual memory regime object.
    virtual void Activate()   { mState = EVmStateType::Active;        } //!< Activate the VmRegime object.
    virtual void Initialize() { mState = EVmStateType::Initialized;   } //!< Initialize the VmRegime object.
    virtual void Deactivate() { mState = EVmStateType::Uninitialized; } //!< Deactivate the VmRegime object.

    virtual void DumpPage(const EDumpFormat dumpFormat, std::ofstream& os) const { } //!< Dump pages.

    virtual const std::vector<Register* > RegisterContext() const; //!<Return vector of context register pointers.
    virtual void FinalizeRegisterContext() const; //!< Initialize necessary context registers
    virtual bool PagingEnabled() const; //!< Return whether paging is enabled.

    virtual VmMapper* GetVmMapper   (const VmInfo& rVmInfo)       { return nullptr; } //!< Return VmMapper based on the VmInfo parameter.
    virtual VmMapper* FindVmMapper  (const VmContext& rVmContext) { return nullptr; } //!< Find VmMapper if exist based on VmContext.
    virtual VmMapper* CreateVmMapper(const VmContext* pVmContext) { return nullptr; } //!< Create VmMapper based on VmContext.
  protected:
    VmRegime(const VmRegime& rOther) : Object(rOther), mRegimeType(rOther.mRegimeType), mDefaultBankType(rOther.mDefaultBankType), mState(rOther.mState), mpGenerator(rOther.mpGenerator), mpVmFactory(rOther.mpVmFactory), mpCurrentMapper(nullptr) { } //!< Copy constructor.
    VmRegime() : Object(), mRegimeType(EVmRegimeType(0)), mDefaultBankType(EMemBankType(0)), mState(EVmStateType::Uninitialized), mpGenerator(nullptr), mpVmFactory(nullptr), mpCurrentMapper(nullptr) { } //!< Default constructor.
  protected:
    EVmRegimeType mRegimeType; //!< Regime type of this VmRegime object.
    EMemBankType mDefaultBankType; //!< Default bank type for the paging regime.
    EVmStateType mState; //!< Indicate the regime's state
    Generator* mpGenerator; //!< Pointer to Generator object.
    const VmFactory* mpVmFactory; //!< Factory for creating VM objects.
    VmMapper* mpCurrentMapper; //!< Pointer the current VmMapper in the virtual memory translation regime.
  };

  class VmPagingRegime;
  class VmDirectMapControlBlock;

  /*!
    \class VmDirectMapper
    \brief Direct one-to-one mapping base class when paging is not on.
  */

  class VmDirectMapper : public VmMapper, public Object {
  public:
    Object * Clone() const override { return new VmDirectMapper(*this); } //!< Clone VmDirectMapper object.
    const std::string ToString() const override { return Type(); } //!< Return a string describing the VmDirectMapper object.
    const char* Type() const override { return "VmDirectMapper"; }

    VmDirectMapper(const VmFactory* pVmFactory, EMemBankType bankType) : VmMapper(pVmFactory), Object(), mMemoryBankType(bankType), mpPagingRegime(nullptr), mpVmDirectMapControlBlock(nullptr), mpAddressTagging(nullptr), mpAddressErrorConstraint(nullptr){ } //!< Constructor with memory bank type given.
    virtual ~VmDirectMapper() override; //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(VmDirectMapper);

    //VmMapper Overrides
    virtual void Activate() override; //!< Activate the VmMapper object.
    virtual void Initialize() override; //!< Initialize the VmMapper object.
    virtual void AddPhysicalRegion(PhysicalRegion* pRegion, bool map) override { } //!< Add Physical Region to be mapped
    virtual void ApplyVirtualUsableConstraint(const EMemDataType memDataType, const EMemAccessType memAccessType, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const override; //!< Apply virtual usable constraint to specified constraint.
    virtual void GetVmContextDelta(std::map<std::string, uint64> & rDeltaMap) const override; //!< Find the delta map between the VmMapper and currect machine state.
    virtual void DumpPage(const EDumpFormat dumpFormat, std::ofstream& os) const override { } //!< Dump pages.
    virtual bool GetPageInfo(uint64 addr, const std::string& type, uint32 bank, PageInformation& page_info) const override { return false; } //!< Return the page information record according to the given address/address type
    virtual bool GetTranslationRange(uint64 VA, TranslationRange& rTransRange) const override; //!< Get translation range that covers the VA, if exists.
    virtual bool MapAddressRange(uint64 VA, uint64 size, bool isInstr, const GenPageRequest* pPageReq=nullptr) override { return false; } //!< Map virtual address range.
    virtual bool VaInAddressErrorRange(const uint64 VA) const override; //!< Check if VA is in address error range.
    virtual bool ValidateContext(std::string& rErrMsg) const override; //!< Validate a VmMapper's context is valid before switching into it.
    virtual uint32 GenContextId() const override; //!< Return context ID.
    virtual uint64 MapAddressRangeForPA(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, const GenPageRequest* pPageReq=nullptr) override { return PA; } //!< Map virtual address range for a given PA.
    virtual uint64 MaxVirtualAddress() const override; //!< Get Max Address from current address space
    virtual uint64 MaxPhysicalAddress() const override; //!< Get Max Address from current address space
    virtual ConstraintSet* VirtualUsableConstraintSetClone(bool isInstr) override; //!< Return cloned pointer to applicable virtual constraint object.
    virtual RegisterReload* GetRegisterReload() const override; //!< Get register reload pointer.
    virtual const AddressTagging* GetAddressTagging() const override { return mpAddressTagging; } //!< Get address tagging object.
    virtual const ConstraintSet* VirtualUsableConstraintSet(bool isInstr) const override; //!< Return const pointer to applicable virtual constraint object.
    virtual const ConstraintSet* VirtualSharedConstraintSet() const override; //!< Return const pointer to virtual shared constraint object.
    virtual const ConstraintSet* GetVmConstraint(EVmConstraintType constrType) const override; //!< Obtain VM constraint of the specified type.
    virtual const Page* GetPage(uint64 VA) const override { return nullptr; } //!< Return the page that contains the virtual address, if exists.
    virtual ETranslationResultType TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const override; //!< Translate VA to PA, return true if address is mapped.
    virtual ETranslationResultType TranslatePaToVa(uint64 PA, EMemBankType bank, uint64& VA) const override; //!< Translate PA to VA, return true if address is mapped.
    virtual EExceptionConstraintType GetExceptionConstraintType(const std::string& rExceptName) const override { return EExceptionConstraintType::Invalid; }; //!< Return exception constraint type for the specified exception name.
    virtual EMemBankType DefaultMemoryBank() const override; //!< Return default memory bvank.
    virtual inline bool VerifyStreamingPageCrossing(uint64 start, uint64 end) const override { return true; } //!< Check instruction stream page crossing.
    virtual inline void ApplyVmConstraints(const GenPageRequest* pPageReq, ConstraintSet& rConstr) const override { } //!< Apply VmConstraints.

    void SetPagingRegime(const VmPagingRegime* pPagingRegime) { mpPagingRegime = pPagingRegime; } //!< Set pointer to parent paging regime.

  protected:
    VmDirectMapper(const VmDirectMapper& rOther) : VmMapper(rOther), Object(rOther), mMemoryBankType(rOther.mMemoryBankType), mpPagingRegime(nullptr), mpVmDirectMapControlBlock(nullptr), mpAddressTagging(nullptr), mpAddressErrorConstraint(nullptr) { } //!< Copy constructor.
    VmDirectMapper() : VmMapper(), Object(), mMemoryBankType(EMemBankType(0)), mpPagingRegime(nullptr), mpVmDirectMapControlBlock(nullptr), mpAddressTagging(nullptr), mpAddressErrorConstraint(nullptr) { } //!< Default constructor.
  protected:
    EMemBankType mMemoryBankType; //!< The memory bank type this VmDirectMapper uses.
    const VmPagingRegime* mpPagingRegime; //!< Pointer to parent paging regime object.
    VmDirectMapControlBlock* mpVmDirectMapControlBlock; //!< Direct map control block.
    AddressTagging* mpAddressTagging; //!< Pointer to address tagging object.
    ConstraintSet *mpAddressErrorConstraint; //!< The pointer to address error constraint
 protected:
   void InitializeConstraint(); //!< Initialize constraints

  };

  class VmasControlBlock;

  /*!
    \class VmPagingMapper
    \brief Page mapping base class.
  */
  class VmPagingMapper : public VmMapper, public Object, public NotificationReceiver {
  public:
    Object * Clone() const override { return new VmPagingMapper(*this); } //!< Clone VmPagingMapper object.
    const std::string ToString() const override { return Type(); } //!< Return a string describing the VmPagingMapper object.
    const char* Type() const override { return "VmPagingMapper"; }

    explicit VmPagingMapper(const VmFactory* pFactory) : VmMapper(pFactory), Object(), mpCurrentAddressSpace(nullptr), mpPagingRegime(nullptr), mpAddressTagging(nullptr), mAddressSpaces() { } //!< Constructor with factory parameter.
    virtual ~VmPagingMapper() override; //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(VmPagingMapper);
    DEFAULT_CONSTRUCTOR_ABSENT(VmPagingMapper);

    virtual void Setup(Generator* gen) override; //!< Setup this VmPagingMapper object.
    virtual void Activate()   override; //!< Activate the VmPagingMapper.
    virtual void Initialize() override; //!< Activate the VmPagingMapper.
    virtual void Deactivate() override; //!< Deactivate the VmPagingMapper.

    //VmMapper Overrides
    virtual void AddPhysicalRegion(PhysicalRegion* pRegion, bool map) override; //!< Add Physical Region to be mapped
    virtual void ApplyVirtualUsableConstraint(const EMemDataType memDataType, const EMemAccessType memAccessType, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const override; //!< Apply virtual usable constraint to specified constraint.
    virtual void ApplyVmConstraints(const GenPageRequest* pPageReq, ConstraintSet& rConstr) const override; //!< Apply VmConstraints.
    virtual void DumpPage(const EDumpFormat dumpFormat, std::ofstream& os) const override; //!< dump pages
    virtual void GetVmContextDelta(std::map<std::string, uint64> & rDeltaMap) const override { }; //!< Find the delta map between the VmMapper and currect machine state.
    virtual bool GetPageInfo(uint64 addr, const std::string& type, uint32 bank, PageInformation& page_info) const override; //!< Return the page information record according to the given address/address type
    virtual bool GetTranslationRange(uint64 VA, TranslationRange& rTransRange) const override; //!< Get translation range that covers the VA from a Page object, if exists.
    virtual bool MapAddressRange(uint64 VA, uint64 size, bool isInstr, const GenPageRequest* pPageReq=nullptr) override; //!< Map virtual address range.
    virtual bool VaInAddressErrorRange(const uint64 VA) const override; //!< Check if VA is in address space error range.
    virtual bool ValidateContext(std::string& rErrMsg) const override; //!< Validate VmPagingMapper context.
    virtual bool VerifyStreamingPageCrossing(uint64 start, uint64 end) const override; //!< Check instruction stream page crossing.
    virtual uint32 GenContextId() const override; //!< Return context ID.
    virtual uint64 MapAddressRangeForPA(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, const GenPageRequest* pPageReq=nullptr) override; //!< Map virtual address range for a given PA
    virtual uint64 MaxVirtualAddress() const override; //!< Get Max Address from current address space
    virtual uint64 MaxPhysicalAddress() const override; //!< Get Max Address from current address space
    virtual ConstraintSet* VirtualUsableConstraintSetClone(bool isInstr) override; //!< Return cloned pointer to applicable virtual constraint object.
    virtual RegisterReload* GetRegisterReload() const override; //!< Get register reload pointer
    virtual const AddressTagging* GetAddressTagging() const override { return mpAddressTagging; } //!< Get address tagging object.
    virtual const ConstraintSet* GetVmConstraint(EVmConstraintType constrType) const override; //!< Obtain VM constraint of the specified type.
    virtual const ConstraintSet* VirtualUsableConstraintSet(bool isInstr) const override; //!< Return const pointer to applicable virtual constraint object.
    virtual const ConstraintSet* VirtualSharedConstraintSet() const override; //!< Return const pointer to virtual shared constraint object.
    virtual const Page* GetPage(uint64 VA) const override; //!< Return the page that contains the virtual address, if exists.
    virtual ETranslationResultType TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const override; //!< Translate VA to PA, return true if address is mapped.
    virtual ETranslationResultType TranslatePaToVa(uint64 PA, EMemBankType bank, uint64& VA) const override; //!< Translate VA to PA, return true if address is mapped.
    virtual EExceptionConstraintType GetExceptionConstraintType(const std::string& rExceptName) const override; //!< Return exception constraint type for the specified paging exception name.
    virtual EMemBankType DefaultMemoryBank() const override; //!< Return default memory bank.

    //NotificationReceiver Overrides
    virtual void HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload) override; //!< Handle a notification.

    void SetPagingRegime(const VmPagingRegime* pPagingRegime) { mpPagingRegime = pPagingRegime; } //!< Set pointer to parent paging regime.
    VmAddressSpace* CreateAddressSpace(const VmContext* pVmContext = nullptr); //!< Create an VmAddressSpace object based on vmContextParameters.
    VmAddressSpace* FindAddressSpace(const VmContext& rVmContext); //!< Find VmAddressSpace that matches the VmContext object if exist.
    const VmAddressSpace* CurrentAddressSpace() { return mpCurrentAddressSpace; } //!< get CurrentAddressSpace.
  protected:
    VmPagingMapper(const VmPagingMapper& rOther) : VmMapper(rOther), Object(rOther), mpCurrentAddressSpace(nullptr), mpPagingRegime(nullptr), mpAddressTagging(nullptr), mAddressSpaces() { } //!< Copy constructor.
    virtual VmAddressSpace* AddressSpaceInstance(VmasControlBlock* pVmasCtlrBlock) const; //!< Return a bare-bone VmAddressSpace object.
    virtual inline uint64 PageCrossMask(void) const { return 0xfffffffffffff000ull; } //!< Default page crossing check using 4K page size.
    void AddAddressSpace(VmAddressSpace* pAddressSpace); //!< Add VmAddressSpace object to the vector.
    void UpdateCurrentAddressSpace(); //!< Update current VmAddressSpace.
  protected:
    VmAddressSpace* mpCurrentAddressSpace; //!< Pointer to the current address space object.
    const VmPagingRegime* mpPagingRegime; //!< Pointer to parent paging regime object.
    AddressTagging* mpAddressTagging; //!< Pointer to address tagging object.
    std::vector<VmAddressSpace* > mAddressSpaces; //!< Holds all VmAddressSpace object ever used in the VmPagingMapper.
  };

  /*!
    \class VmPagingRegime
    \brief Represent a virtual memory regime in a virtual memory system architecture.
  */

  class VmPagingRegime : public VmRegime {
  public:
    Object * Clone() const override { return new VmPagingRegime(*this); } //!< Clone VmPagingRegime object.
    const std::string ToString() const override { return Type(); } //!< Return a string describing the VmPagingRegime object.
    const char* Type() const override { return "VmPagingRegime"; }

    VmPagingRegime(const VmFactory* pVmFactory, EVmRegimeType regimeType, EPrivilegeLevelType privType, EMemBankType bankType); //!< Constructor with regime type parameter given.
    VmPagingRegime() : VmRegime(), mPrivilegeLevelType(EPrivilegeLevelType(0)), mpDirectMapper(nullptr), mpPagingMapper(nullptr) { } //!< Default constructor.
    ~VmPagingRegime() override; //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(VmPagingRegime);

    void Setup(Generator* gen) override; //!< Setup this VmPagingRegime object.
    void Activate()   override; //!< Activate the VmPagingRegime.
    void Initialize() override; //!< Initialize the VmPagingRegime.
    void Deactivate() override; //!< Deactivate the VmPagingRegime.

    void DumpPage(const EDumpFormat dumpFormat, std::ofstream& os) const override; //!< dump pages

    VmMapper* GetVmMapper(const VmInfo& rVmInfo) override; //!< Return VmMapper based on the VmInfo object specified.
    VmMapper* FindVmMapper(const VmContext& rVmContext) override; //!< Find VmMapper if exist based on VmContext.
    VmMapper* CreateVmMapper(const VmContext* pVmContext) override; //!< Create VmAddressSpace based on VmContext.

    EPrivilegeLevelType PrivilegeLevel() const { return mPrivilegeLevelType; } //!< Return exception level.
  protected:
    VmPagingRegime(const VmPagingRegime& rOther) : VmRegime(rOther), mPrivilegeLevelType(rOther.mPrivilegeLevelType), mpDirectMapper(nullptr), mpPagingMapper(nullptr) { } //!< Copy constructor.
    virtual VmDirectMapper* DirectMapperInstance() const; //!< Return an instance of VmDirectMapper
    virtual VmPagingMapper* PagingMapperInstance() const; //!< Return an instance of VmPagingMapper
    void UpdateCurrentMapper(); //!< Update to use correct current mapper.
  protected:
    EPrivilegeLevelType mPrivilegeLevelType; //!< Exception level type for the paging regime.
    VmDirectMapper* mpDirectMapper; //!< Pointer to the direct mapper object.
    VmPagingMapper* mpPagingMapper; //!< Pointer to the paging mapper object.
  };

}

#endif
