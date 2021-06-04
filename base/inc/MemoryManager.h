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
#ifndef Force_MemoryManager_H
#define Force_MemoryManager_H

#include <Notify.h>
#include <NotifyDefines.h>
#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>
#include <map>

namespace Force {

  class Memory;
  class MemoryInitRecord;
  class Generator;
  class ConstraintSet;
  class PhysicalPageManager;
  class PageTableManager;
  class GenPageRequest;
  class PageSizeInfo;
  class MemoryReservation;
  class PhysicalRegion;
  class PaTuple;
  class PagingChoicesAdapter;
  class MemoryConstraint;
  class AddressReuseMode;
  class SymbolManager;
  class MemoryTraitsManager;

  /*!
    \class MemoryBank
    \breif Class holding physical memory constraints to one memory bank.
   */
  class MemoryBank {
  public:
    explicit MemoryBank(EMemBankType bankType); //!< Constructor with memory bank type given.
    COPY_CONSTRUCTOR_ABSENT(MemoryBank);
    ~MemoryBank(); //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(MemoryBank);
    inline Memory* MemoryInstance() { return mpMemory; } //!< Return pointer to memory object
    EMemBankType MemoryBankType() const; //!< Return name of memory bank.
    void AddMemoryRange(uint64 start, uint64 end); //!< Add usable physical memory range.
    void SubMemoryRange(uint64 start, uint64 end); //!< Subtract usable physical memory range.
    void Configure(); //!< Configure base line memory constraints.
    void InitializeMemory(const MemoryInitRecord& memInitRecord);  //!< Initialize memory.
    void ReadMemoryPartiallyInitialized(cuint64 address, cuint32 nBytes, uint8* data) const; //!< Read data from memory that may not be fully initialized.
    void WriteMemory(uint64 address, cuint8* data, uint32 nBytes);  //!< write data in the buffer to memory
    void MarkShared(cuint64 address, cuint32 nBytes); //!< Mark an address range as shared.
    void GetMemoryAttributes(cuint64 address, cuint32 nBytes, uint8* memAttrs) const; //!< Get memory attributes for the specified number of bytes starting at the specified address.
    uint8 GetByteMemoryAttributes(cuint64 address) const; //!< Get memory attributes of the byte at the specified address.
    const ConstraintSet* Free() const { return mpFree; } //!< Return const pointer to free ConstraintSet.
    const ConstraintSet* Usable() const; //!< Return const pointer to usable ConstraintSet.
    const ConstraintSet* Shared() const; //!< Return const pointer to shared ConstraintSet.
    const ConstraintSet* Unmapped() const; //!< Return const pointer to ConstraintSet of unmapped addresses.
    void ApplyUsableConstraint(const EMemDataType memDataType, const EMemAccessType memAccessType, cuint32 threadId, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const; //!< Apply usable constraint to specified constraint.
    void SetupPageTableRegion(); //!< Setup page table region in the memory bank.
    void ReserveMemory(const ConstraintSet& memConstr); //!< Reserve memory ranges.
    void UnreserveMemory(const ConstraintSet& memConstr); //!< Unreserve memory ranges
    bool AllocatePageTableBlock(uint64 align, uint64 size, const ConstraintSet* range, uint64& start); //!< Allocate page table block in a certain memory bank.
    inline PhysicalPageManager* GetPhysicalPageManager() const { return mpPhysicalPageManager; } //!< Return the physical page manager for the specified memory bank.
    inline PageTableManager* GetPageTableManager() const { return mpPageTableManager; } //!< Return page table manager for the specified memory bank.
    inline SymbolManager* GetSymbolManager() const { return mpSymbolManager; } //!< Return symbol manager for the specified memory bank.
  private:
    Memory* mpMemory; //!< Pointer to memory object.
    ConstraintSet* mpBaseConstraint; //!< Pointer to base memory constraint.
    ConstraintSet* mpFree; //!< Free memory constraint.
    MemoryConstraint* mpUsable; //!< Usable memory constraint.
    PhysicalPageManager* mpPhysicalPageManager; //!< Pointer to physical page manager object
    PageTableManager* mpPageTableManager; //!< Pointer to the page table manager object
    SymbolManager* mpSymbolManager; //!< Pointer to the symbol manager object.
    MemoryTraitsManager* mpMemTraitsManager; //!< Pointer to the memory traits manager object.
  };

  /*!
    \class MemoryManager
    \brief Manager of all memory banks and memory constraints.
  */

  class MemoryManager : public NotificationSender {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static MemoryManager* Instance() { return mspMemoryManager; } //!< Access MemoryManager instance.

    void InitializeMemory(const MemoryInitRecord* memInitRecord); //!< Initialize memory.
    void ReadMemoryPartiallyInitialized(const PaTuple& rPaTuple, cuint32 size, uint8* memData) const; //!< Read data from memory that may not be fully initialized.
    void MarkShared(const PaTuple& rPaTuple, cuint32 size); //!< Mark an address range as shared.
    void GetMemoryAttributes(const PaTuple& rPaTuple, cuint32 size, uint8* memAttrs) const; //!< Get attributes for the specified memory location and size.
    const std::vector<MemoryBank* >& MemoryBanks() const { return mMemoryBanks; } //!< Return constant reference to memory banks.
    void OutputTest(const std::map<uint32, Generator *>& generators, uint64 resetPC, uint32 machineType); //!< Output test.
    void AddMemoryRange(uint32 bank, uint64 start, uint64 end); //!< Add usable physical memory range.
    void SubMemoryRange(uint32 bank, uint64 start, uint64 end); //!< Subtract usable physical memory range.
    void ConfigureMemoryBanks(); //!< Configure base line memory constraints.
    uint32 NumberBanks() const { return mMemoryBanks.size(); } //!< Return number of memory banks.

    inline MemoryBank* GetMemoryBank(uint32 bank) //!< Return memory bank pointer with check.
    {
      if (bank >= mMemoryBanks.size()) {
        BankOutOfBound(bank);
      }
      return mMemoryBanks[bank];
    }

    inline const MemoryBank* GetMemoryBank(uint32 bank) const //!< Return memory bank pointer with check, const version.
    {
      if (bank >= mMemoryBanks.size()) {
        BankOutOfBound(bank);
      }
      return mMemoryBanks[bank];
    }

    void SetupPageTableRegions(); //!< Setup page table regions.
    PhysicalPageManager* GetPhysicalPageManager(EMemBankType memType); //!< Return the physical page manager for the specified memory bank
    bool AllocatePageTableBlock(EMemBankType memType, uint64 align, uint64 size, const ConstraintSet* usable, uint64& start); //!< Allocate page table block in a certain memory
    PageTableManager* GetPageTableManager(EMemBankType memType) const; //!< Return page table manager for the specified memory bank.
    void ReserveMemory(MemoryReservation* pMemReserv); //!< Called to reserve memory.
    void UnreserveMemory(MemoryReservation* pMemReserv); //!< Called to unreserve memory.
    void AddPhysicalRegion(PhysicalRegion* pPhysRegion); //!< Add physical region.
    const std::vector<PhysicalRegion* > & GetPhysicalRegions() const { return mPhysicalRegions; } //!< Return physical regions.
    bool PaInitialized(const PaTuple& rPaTuple, cuint32 size) const; //!< Check if the memory from target PA through target PA + size - 1 is initialized.
    bool InstructionPaInitialized(const PaTuple& rPaTuple) const; //!< Check if the instruction-sized block of memory starting at the target PA is initialized.
    void OutputImage() const; //!< Output image in text format.
  private:
    MemoryManager();  //!< Constructor, private.
    ~MemoryManager(); //!< Destructor, private.
    void Setup(); //!< Setup memory banks.
    void BankOutOfBound(uint32 bank) const; //!< Report memory bank index out of bank error.
  private:
    static MemoryManager* mspMemoryManager; //!< Pointer to singleton MemoryManager object.
    std::vector<MemoryBank* > mMemoryBanks; //!< Container for all memory banks.
    bool mConstraintConfigured; //!< Indicates whether memory constraints are configured.
    std::map<std::string, MemoryReservation* > mReservations; //!< Container of all memory reservations.
    std::vector<PhysicalRegion* > mPhysicalRegions; //!< Physical memory regions to be mapped by virtual memory system.
  };

}

#endif
