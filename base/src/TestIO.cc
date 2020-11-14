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
#include <vector>
#include <elfio/elfio.h>
#include <fstream>
#include <numeric>

#include <Memory.h>
#include <fmt.h>
#include <Generator.h>
#include <Instruction.h>
#include <InstructionResults.h>
#include <TestIO.h>
#include <SymbolManager.h>
#include <Log.h>

using namespace std;
using namespace ELFIO;
/*!
  \file ElfIO.cc
  \brief Code for ELF IO module
*/

namespace Force {

#define MAX_SEGMENTS_NUM   ((1u << 16) - 1)  //<! maximal segments number

 /*!
    \class TestSection
    \brief class for test section.
  */
  struct TestSection {
    string  mName;
    uint64  mAddress;
    uint64  mSize;   //<! size in bytes
    uint32  mType;  //<!  section type like SHT_PROGBITS
    uint64  mFlag;   //<! section flag like  SHF_ALLOC | SHF_WRITE|SHF_EXECINSTR
    char*   mpData;  //<! Contends in a section

    TestSection(const std::string& name, uint64 address,
                 uint64 size, uint32 type, uint64 flag)
              : mName(name), mAddress(address), mSize(size), mType(type), mFlag(flag), mpData(nullptr)
    {
      mpData = new char[mSize];
    }
    ~TestSection()
    {
      delete[] mpData;
    }
    ASSIGNMENT_OPERATOR_ABSENT(TestSection);
    COPY_CONSTRUCTOR_ABSENT(TestSection);

  };

  /*!
    \class TestSegment
    \brief class for test segment
  */
  struct TestSegment {
    uint64 mVirtAddress;  //<! start address
    uint64 mSize; //<! size in bytes
    uint64 mFlag; //<!  PF_R,  PF_W or  PF_X
    std::vector<const TestSection* > mSections;

    TestSegment() : mVirtAddress(-1ull), mSize(0), mFlag(PF_R | PF_W), mSections() { }
    TestSegment(const TestSection *pSection, uint64 flag) : mVirtAddress(pSection->mAddress) , mSize(0), mFlag(flag), mSections()
    {
      mSections.push_back(pSection);
    }
    ~TestSegment()
    {
      for (auto s : mSections) {
        delete s;
      }
    }
  };

  /*!
    \class TestImage
    \brief class for in-memory Test image.
  */
  class TestImage {
  public:

    /*!
      build a Test image from memory object
    */
    void CreateFromMem(const Memory& memory, const SymbolManager* pSymManager)
    {
      mpSymbolManager = pSymManager;
      
      int d_no = 0;
      int t_no = 0;
      std::vector<Section> sections;
      memory.GetSections(sections);
      for (auto& rSection : sections){
        // << "{TestIO} section address: 0x" << hex << rSection.mAddress << " size: 0x" << rSection.mSize << " type: " << EMemDataType_to_string(rSection.mType) << endl;
        switch (rSection.mType) {
        case EMemDataType::Data:
        {
          char data[16];
          std::sprintf(data, "data%d", d_no++);
          auto *pDataSection = new TestSection(string(data), rSection.mAddress, rSection.mSize, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
          memory.ReadInitialWithPattern(pDataSection->mAddress, pDataSection->mSize, (uint8 *)pDataSection->mpData);
          PushSectionToSegment(pDataSection, mDataSegments);
          break;
        }
        case EMemDataType::Both:
        case EMemDataType::Instruction:
        {
          char text[16];
          std::sprintf(text, "text%d", t_no++);
          auto *pTextSection = new TestSection(string(text), rSection.mAddress, rSection.mSize, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR);
          memory.ReadInitialWithPattern(pTextSection->mAddress, pTextSection->mSize, (uint8 *)pTextSection->mpData);
          PushSectionToSegment(pTextSection, mTextSegments);
          break;
        }
        case EMemDataType::Init:
          break;
        default:
          FAIL("unsupport data type");
        }
      }
    }

    /*!
      build a test image from the specified ELF file
    */
    void CreateFromElf(const std::string& elfFilePath, uint32 machineType)
    {
      elfio reader;

      if (!reader.load(elfFilePath)) {
        LOG(fail) << "Can't find or open test case " << elfFilePath << endl;
        FAIL("Can't find or open test case");
      }
      if (reader.get_class() != ELFCLASS64 || reader.get_machine() != machineType)
        FAIL("invalid test case format");

      mEntry = reader.get_entry();
      mBigEndian = (reader.get_encoding() == ELFDATA2MSB) ? true : false;
      Elf_Half sectNum = reader.sections.size();
      for (auto i = 0; i < sectNum; i ++) {
        const section *pSection = reader.sections[i];
        Elf_Word type = pSection->get_type();
        Elf_Xword flags = pSection->get_flags();
        const char *pData = pSection->get_data();

        if (type != SHT_PROGBITS)
          continue;

        auto *pImageSection = new TestSection(pSection->get_name(), pSection->get_address(), pSection->get_size(), type, flags);
        if (memcpy(pImageSection->mpData, pData, pImageSection->mSize) == nullptr)
          FAIL("memory copy failed");

        if (flags & SHF_EXECINSTR)
          PushSectionToSegment(pImageSection, mTextSegments);
        else if (flags & SHF_WRITE)
          PushSectionToSegment(pImageSection, mDataSegments);
        else
          FAIL("unknown section type");
      }
    }

    /*!
      dump a test image content to the memory object
    */
    void DumpToMem(const Memory& memory)
    {
      DumpSegmentsToMem(mTextSegments, memory);
      DumpSegmentsToMem(mDataSegments, memory);
    }

    /*!
      dump a test image content to the specifed ELF file
    */
    void DumpToElf(const std::string& elfFilePath, uint32 machineType)
    {
      elfio writer;

      writer.create(ELFCLASS64, (mBigEndian) ? ELFDATA2MSB : ELFDATA2LSB);
      writer.set_os_abi(ELFOSABI_LINUX);
      writer.set_type(ET_EXEC);
      writer.set_machine(machineType);

      DumpSegmentsToElf(mTextSegments, writer);
      DumpSegmentsToElf(mDataSegments, writer);
      DumpSymbolsToElf(writer);

      writer.set_entry(mEntry);
      if (!writer.save(elfFilePath)) {
        LOG(fail) << "Can't generate the test case " << elfFilePath << std::endl;
        FAIL("Can't generate test case");
      }
    }

#ifndef UNIT_TEST
    /*!
      dump test image assembly into the file
    */
    void DumpToAssembly(uint32 memBank, const Generator* generator, ofstream& asmFile)
    {
      const ThreadInstructionResults* inst_results = generator->GetInstructionResults();
      const std::map<uint64, Instruction* >& instructions = inst_results->GetInstructions(memBank);

      for (auto inst : instructions)
          asmFile << fmtx0(inst.first, 16) << ":" << fmtx0(inst.second->Opcode()) << " " << inst.second->AssemblyText() << endl;
    }
#endif
    /*!
      set endian for ELF header
    */
    void SetEndian(bool bigEndian)
    {
      mBigEndian = bigEndian;
    }

    /*!
      get endian for ELF header
    */
    bool GetEndian(void)
    {
      return mBigEndian;
    }

    /*!
      set Entry point
    */
    void SetEntry(uint64 entry)
    {
      mEntry = entry;
    }

    /*!
      get Entry point
    */
    uint64 GetEntry(void)
    {
      return mEntry;
    }

#ifdef UNIT_TEST
    unsigned CountDataSections(void)
    {
      unsigned num = accumulate(mDataSegments.cbegin(), mDataSegments.cend(), unsigned(0),
        [](const unsigned partialSum, const TestSegment* pDataSegment) { return (partialSum + pDataSegment->mSections.size()); });

      return num;
    }

    unsigned CountTextSections(void)
    {
      unsigned num = accumulate(mTextSegments.cbegin(), mTextSegments.cend(), unsigned(0),
        [](const unsigned partialSum, const TestSegment* pTextSegment) { return (partialSum + pTextSegment->mSections.size()); });

      return num;
    }
#endif

    TestImage() : mEntry(0ull), mBigEndian(true), mDataSegments(), mTextSegments(), mTotalSegments(0), mpSymbolManager(nullptr) {}

    ~TestImage()  {
      for (auto pDataSegment : mDataSegments)
        delete pDataSegment;
      for (auto pTextSegment : mTextSegments)
        delete pTextSegment;
    }
  private:
    uint64 SegmentFlagForSection(uint64 sectionFlag) const
    {
      uint64 flag = 0;
      if (sectionFlag & SHF_EXECINSTR)
        flag = PF_R | PF_X;
      else if (sectionFlag & SHF_WRITE)
        flag = PF_R | PF_W;
      else {
        LOG(fail) << "Unknown section flags: 0x" << hex << sectionFlag << endl;
        FAIL("unknown section flags");
      }
      return flag;
    }

    void PushSectionToSegment(const TestSection* section, vector<TestSegment* >& segments) //<! push a section into a segment
    {
      if (segments.empty()) {
        auto *first_segment = CreateTestSegment(section);
        segments.push_back(first_segment);
        return;
      }

      auto last_segment = segments.back();
      auto last_section = last_segment->mSections.back();
      if (section->mAddress == last_section->mAddress + last_section->mSize ) {
        last_segment->mSections.push_back(section);
        if (last_segment->mVirtAddress > section->mAddress)
          last_segment->mVirtAddress = section->mAddress;
      }
      else {
        auto *new_segment = CreateTestSegment(section);
        segments.push_back(new_segment);
      }
    }

    //!< dump segments to memory object
    void DumpSegmentsToMem(const vector<TestSegment*>& segments, const Memory& memory)
    {
      for (auto seg : segments)
        for (auto sect: seg->mSections) {
          if (sect->mAddress % 8)
            FAIL("memory address not 8-byte aligned");
          Memory *pMemory = const_cast<Memory *>(&memory);
          vector<uint8> mem_attrs(sect->mSize);
          pMemory->Initialize(sect->mAddress, (uint8 *)sect->mpData, mem_attrs.data(), sect->mSize,
                            (seg->mFlag & PF_X) ?  EMemDataType::Instruction :  EMemDataType::Data);
        }
    }

    void DumpSegmentsToElf(const vector<TestSegment*>& segments, elfio& elf_writer)
    {
      for (auto seg : segments) {
        segment* pSegment = elf_writer.segments.add();
        pSegment->set_type(PT_LOAD);
        pSegment->set_flags(seg->mFlag);
	
        for (auto sect : seg->mSections) {
          section* pSect = elf_writer.sections.add(sect->mName);
          pSect->set_address(sect->mAddress);
          pSect->set_type(sect->mType);
          pSect->set_flags(sect->mFlag);
          TestSection* testSect = const_cast<TestSection* >(sect);
          pSect->swap_data(testSect->mpData, testSect->mSize);
          pSegment->add_section_index(pSect->get_index(), pSect->get_addr_align());
        }
        pSegment->set_virtual_address(seg->mVirtAddress);
        pSegment->set_physical_address(seg->mVirtAddress);
        pSegment->set_align(4);
      }      
    }
    
    void DumpSymbolsToElf(elfio& elf_writer)
    {
      if (not mpSymbolManager->HasSymbols())
	return;

      // Add .symtab
      section* pSymtabSection = elf_writer.sections.add(".symtab");
      pSymtabSection->set_address(0);
      pSymtabSection->set_type(SHT_SYMTAB);
      //pSymtabSection->set_flags();
      section* pStrtabSection = elf_writer.sections.add(".strtab");
      pStrtabSection->set_address(0);
      pStrtabSection->set_type(SHT_STRTAB);
	  
      symbol_section_accessor symbols_accessor(elf_writer, pSymtabSection);
      string_section_accessor strings_accessor(pStrtabSection);

      const map<string, Symbol*>& symbol_map = mpSymbolManager->GetSymbols();
      for (auto map_item : symbol_map) {
	const Symbol* symbol_ptr = map_item.second;
	symbols_accessor.add_symbol(strings_accessor, symbol_ptr->Name().c_str(), symbol_ptr->Address(), 0, (STT_NOTYPE | (STB_GLOBAL << 4)), STV_DEFAULT, SHT_SYMTAB);
      }

      pSymtabSection->set_link(pStrtabSection->get_index());
      pSymtabSection->set_info(3); // number of entries + 1
      pSymtabSection->set_addr_align(8); // 64-bit ELF align.
      pSymtabSection->set_entry_size(24); // 64-bit ELF Symbol table entry size.

      pStrtabSection->set_addr_align(1);
    }

    TestSegment* CreateTestSegment(const TestSection* section)
    {
      if (mTotalSegments++ >= MAX_SEGMENTS_NUM) {
        LOG(fail) << "segments number " << mTotalSegments << "is overflow";
        FAIL("overflow-segments-number");
        return nullptr;
      }
      auto *new_segment = new TestSegment(section, SegmentFlagForSection(section->mFlag));
      return new_segment;
    }
    
    ASSIGNMENT_OPERATOR_ABSENT(TestImage);
    COPY_CONSTRUCTOR_ABSENT(TestImage);
    
    uint64 mEntry;  //!< elf entries
    bool   mBigEndian;  //!< big endian or not
    vector<TestSegment* > mDataSegments; //!< more data segments to store sparse data
    vector<TestSegment* > mTextSegments; //!< more instruction segments to store sparse data
    unsigned mTotalSegments; //!< total segments
    const SymbolManager* mpSymbolManager; //!< Pointer to symbol manager.
  };

  TestIO::TestIO(uint32 memBank, const Memory* pMem, const SymbolManager* pSymManager, bool createImage)
    : mMemoryBank(memBank), mpMemory(pMem), mpSymbolManager(pSymManager), mpTestImage(nullptr)
  {
    mpTestImage = new TestImage();
    if (createImage)
      mpTestImage->CreateFromMem(*mpMemory, mpSymbolManager);

  }

  TestIO::~TestIO(void)
  {
    delete mpTestImage;
  }

  void TestIO::WriteTestElf(const std::string& elfFilePath, bool bigEndian, uint64 entry, uint32 machineType)
  {
    mpTestImage->SetEndian(bigEndian);
    mpTestImage->SetEntry(entry);
    mpTestImage->DumpToElf(elfFilePath, machineType);
  }

  void TestIO::ReadTestElf(const std::string& elfFilePath, bool& bigEndian, uint64& entry, uint32 machineType)
  {
    mpTestImage->CreateFromElf(elfFilePath, machineType);
    bigEndian = mpTestImage->GetEndian();
    entry = mpTestImage->GetEntry();
    mpTestImage->DumpToMem(*mpMemory);
  }

#ifndef UNIT_TEST
  void TestIO::WriteTestAssembly(const std::map<uint32, Generator *>& generators, const std::string& disasmFilePath)
  {
    if (mpTestImage == nullptr)
      FAIL("can't create test image");

    ofstream asmFile;
    asmFile.open(disasmFilePath);
    if (asmFile.bad()) {
      LOG(fail) << "Can't open file " << disasmFilePath << std::endl;
      FAIL("Can't open file");
    }

    for (auto genItem : generators)
      mpTestImage->DumpToAssembly(mMemoryBank, genItem.second, asmFile);

    asmFile.close();

  }
#endif

#ifdef UNIT_TEST
  unsigned TestIO::CountSections(void)
  {
    return mpTestImage->CountDataSections() + mpTestImage->CountTextSections();
  }
#endif

#undef  MAX_SEGMENT_NUM

}
