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
#include <set>
#include <sstream>
#include <fstream>
#include <ImageIO.h>
#include <Memory.h>
#include <Config.h>
#include <fmt.h>
#include <Log.h>
#include <Register.h>
#include <StringUtils.h>

/*!
  \file ImageIO.cc
  \brief Code for Image IO module
*/

using namespace std;

namespace Force {

  /*!
    \class SegmentDataUnit
    \brief class record data units
   */
  struct SegmentDataUnit {
  public:
    uint64 mData;  //!< data value.
    uint32 mSize;   //!< data size.
  public:
    SegmentDataUnit(uint64 data, uint64 size) : mData(data), mSize(size) { } //!< Constructor
  };

  /*!
    \class ImageSegment
    \brief class base Segment of image file.
  */
  class ImageSegment {
  public:
    ImageSegment() { }  //!< Default constructor
    virtual const string Type() const = 0; //!< Return a string describing the actual type of the ImageSegment
    virtual ~ImageSegment() { } //!< Destructor
  };

  /*!
    \class MemoryImageSegment
    \brief class memory Segment of image file.
  */
  class MemoryImageSegment : public ImageSegment {
  public:
    MemoryImageSegment() : mFlag(), mAddress(0x0), mValues() { } //!< constructor
    ~MemoryImageSegment() { } //!< Destructor
    const string Type() const override { return "Memory"; } //!< Return a string describing the actual type of the ImageSegment
    uint64 Address() const { return mAddress; }  //!< return the address of memory Segment.
    const string& Flag() const { return mFlag; }  //!< return the flag of Image Segment.
    const vector<SegmentDataUnit>& Values() const { return mValues; }  //!< return the value of the memory section.
    void AddValueUnit(uint64 value, uint32 size) { mValues.push_back(SegmentDataUnit(value, size)); } //!< add value unit
  private:
    string mFlag;    //!< the flag of Image Segment.
    uint64 mAddress;      //!< the address of memory Segment.
    vector<SegmentDataUnit> mValues; //!< the data of the memory section.
    friend class ImageLoader;
    friend class ImagePrinter;
  };

  /*!
    \class ThreadImageSegment
    \brief class thread information of image file.
  */
  class ThreadImageSegment : public ImageSegment {
  public:
    ThreadImageSegment() : mFlag(), mName(), mValues() { } //!< constructor
    ~ThreadImageSegment() { } //!< Destructor
    const string Type() const override { return "Thread"; } //!< Return a string describing the actual type of the ImageSegment
    const string& Flag() const { return mFlag; }  //!< return the flag of Image Segment.
    const string& Name() const { return mName; } //!< return the name of the thread Segment.
    const vector<SegmentDataUnit>& Values() const { return mValues; } //!< return thre value of thre thread Segment.
    void AddValueUnit(uint64 value, uint32 size) { mValues.push_back(SegmentDataUnit(value, size)); } //!< add data unit
  protected:
    string mFlag; //!< the flag of Image Segment.
    string mName; //!< the name of thread segment.
    vector<SegmentDataUnit> mValues; //!< the value of thread Segment.
    friend class ImageLoader;
    friend class ImagePrinter;
  };

  /*!
    \class ImageLoader
    \brief class loader of image file.
  */
  class ImageLoader {
  public:
    ImageLoader() : mThreadsSegments(), mMemoryImageSegments(), mImageFile(), mThreadId(0) { } //!< Constructor
    ~ImageLoader(); //!< Destructor
    bool IsOpen() { return mImageFile.is_open(); } //!< return thre open state of image file.
    void Load(const string& imageFile); //!< load image file.
    void WriteToMemory(Memory* memory) const; //!< write memory.
    void WriteToThread(map<string, uint64>& threadInfo, RegisterFile* registerFile) const; //!< write thread info.
  private:
    ImageSegment* GetOneSegment(); //!< return the next one image Segment.
    MemoryImageSegment* BuiltMemoryImageSegment(const string& line_str); //!< built the MemoryImageSegment.
    ThreadImageSegment* BuiltThreadImageSegment(const string& line_str); //!< built the ThreadImageSegment.
  private:
    map<uint32, vector<ThreadImageSegment* > > mThreadsSegments;  //!< the threads initial values.
    vector<MemoryImageSegment* > mMemoryImageSegments;                 //!< the memory intital values.
    ifstream mImageFile;                             //!< the pointer of the image file.
    uint32 mThreadId;                                //!< the current thread id.
  };

  class ImagePrinter {
  public:
    ImagePrinter() : mImageFile(), mPrinted() { } //!< Constructor
    virtual ~ImagePrinter(); //!< Destructor
    void PrintMemoryImage(const string& imageFile, const Memory* memory);  //!< write memory initial data to an image file.
    void PrintRegistersImage(const string& imageFile, const map<string, uint64>& threadInfo, const RegisterFile* regFile); //!< write registers initial value to an Text file.
  private:
    void PrintInitialMemorySection(EMemDataType type, uint64 address, uint32 size, const uint8* data); //!< print initial memory section.
    void SetupThreadImageSegment(const Register* pReg, const RegisterFile* regFile, ThreadImageSegment* imageSegment); //!< built thread image segment.
    void PrintThreadImageSegment(const ThreadImageSegment* pSegment); //!< print thread image segment.
    bool OpenImageFile(const string& imageFile); //!< Open image file.
    void PrintThreadInfo(const map<string, uint64>& threadInfo); //!< write thread info to the Text file.
  private:
    ofstream mImageFile; //!< image file output stream.
    set<string> mPrinted; //!< already printed name.
  };

  ImageLoader::~ImageLoader()
  {
    if (mImageFile.is_open()) {
      mImageFile.close();
    }
    for (auto it : mMemoryImageSegments) {
      delete it;
    }
    for (auto thread : mThreadsSegments) {
      for (auto it : thread.second) {
        delete it;
      }
    }
  }

  void ImageLoader::Load(const string& imageFile)
  {
    mImageFile.open(imageFile);
    if (mImageFile.bad())
    {
      LOG(fail) << "Can't open file " << imageFile << endl;
      FAIL("can-not-open-file");
    }

    while (not mImageFile.eof()) {
      ImageSegment* segment_ptr = GetOneSegment();
      if (segment_ptr != nullptr)
      {
        if (segment_ptr->Type() == "Thread")
        {
          mThreadsSegments[mThreadId].push_back(dynamic_cast<ThreadImageSegment* >(segment_ptr));
        }
        else if (segment_ptr->Type() == "Memory")
        {
          mMemoryImageSegments.push_back(dynamic_cast<MemoryImageSegment* >(segment_ptr));
        }
      }
    }
    mImageFile.close();
  }

  void ImageLoader::WriteToMemory(Memory* memory) const
  {
    for (auto segment_ptr : mMemoryImageSegments)
    {
      uint64 address = segment_ptr->Address();
      auto values = segment_ptr->Values();
      EMemDataType type = segment_ptr->Flag() == "I" ? EMemDataType::Instruction : EMemDataType::Data;
      for (auto value : values) {
        memory->Initialize(address, value.mData, value.mSize, type);
        address += value.mSize;
      }
    }
  }

  void ImageLoader::WriteToThread(map<string, uint64>& threadInfo, RegisterFile* registerFile) const
  {
    auto it = threadInfo.find("ThreadID");
    if (it == threadInfo.end())
    {
      LOG(fail) << "Can't find the thread ID in threadInfo" << endl;
      FAIL("can-not-find-thread-id");
    }

    auto iter = mThreadsSegments.find(it->second);
    if (iter == mThreadsSegments.end())
    {
      LOG(fail) << "Can't find threadId " << it->second << endl;
      FAIL("can-not-find-thread");
    }

    for (auto segment_ptr : iter->second)
    {
      if (segment_ptr->Flag() == "V")
      {
        auto values = segment_ptr->Values();
        threadInfo[segment_ptr->Name()] = values[0].mData;
      }
      else
      {
        auto reg_name = segment_ptr->Name();
        auto values = segment_ptr->Values();
        uint32 size = values.size();
        if (size > 1)  // handle Z and V registers.
        {
          string index = reg_name.substr(1, 1);
          for (uint32 i = 0; i < 2; ++i) {
            auto phys_reg_ptr = registerFile->PhysicalRegisterLookup("V" + index + "_" + to_string(i));
            phys_reg_ptr->Initialize(values[size - i - 1].mData, phys_reg_ptr->Mask());
          }
          for (uint32 i = 2; i < size; ++i) {
            auto phys_reg_ptr = registerFile->PhysicalRegisterLookup("Z" + index + "_" + to_string(i));
            phys_reg_ptr->Initialize(values[size - i - 1].mData, phys_reg_ptr->Mask());
          }
        }
        else if (reg_name[0] == 'P' and isdigit(reg_name[1])) // Handle P registers
        {
          registerFile->InitializeRegister(reg_name, values[0].mData, nullptr);
        }
        else
        {
          auto phys_register = registerFile->PhysicalRegisterLookup(reg_name);
          phys_register->Initialize(values[0].mData, phys_register->Mask());
        }
      }
    }
  }

  MemoryImageSegment* ImageLoader::BuiltMemoryImageSegment(const string& line_str)
  {
    MemoryImageSegment* segment_ptr = new MemoryImageSegment();
    StringSplitter ss(line_str, ' ');
    segment_ptr->mFlag = ss.NextSubString();
    segment_ptr->mAddress = parse_uint64("0x" + ss.NextSubString());
    uint32 size = parse_uint32(ss.NextSubString());
    cuint32 piece_size = 32;

    uint32 lines = (size + piece_size - 1) / piece_size;
    string value_str;
    for (uint32 l = 0; l < lines; ++l) {
      if (size <= piece_size)
      {
        value_str = ss.NextSubString();
      }
      else if (not mImageFile.eof())
      {
        getline(mImageFile, value_str);
      }

      StringSplitter spliter(value_str, '_');
      while (not spliter.EndOfString()) {
        segment_ptr->AddValueUnit(parse_uint64("0x" + spliter.NextSubString()), 8);
      }
    }
    return segment_ptr;
  }

  ThreadImageSegment* ImageLoader::BuiltThreadImageSegment(const string& line_str)
  {
    StringSplitter ss(line_str, ' ');
    string flag = ss.NextSubString();
    if (flag == "T")
    {
      mThreadId = parse_uint64(ss.NextSubString());
      return nullptr;
    }

    ThreadImageSegment* segment_ptr = new ThreadImageSegment();
    segment_ptr->mFlag = flag;
    segment_ptr->mName = ss.NextSubString();

    string val_str = ss.NextSubString();
    uint32 str_len = val_str.length();
    uint32 resolved_len = 0;
    cuint32 each_len = 16;

    while (resolved_len < str_len) {
      string sub_str = val_str.substr(resolved_len, each_len);
      segment_ptr->AddValueUnit(parse_uint64("0x" + sub_str), 8);
      resolved_len += each_len ;
    }

    return segment_ptr;
  }

  ImageSegment* ImageLoader::GetOneSegment()
  {
    while (not mImageFile.eof()) {
      string line_str;
      getline(mImageFile, line_str);
      if (line_str.size()) {
        switch (line_str.at(0)) {
        case 'T':
        case 'V':
        case 'R':
          return BuiltThreadImageSegment(line_str);
        case 'I':
        case 'D':
          return BuiltMemoryImageSegment(line_str);
        default: break;
        }
      }
    }
    return nullptr;
  }

  ImagePrinter::~ImagePrinter()
  {
    if (mImageFile.is_open()) {
      mImageFile.close();
    }
  }

  bool ImagePrinter::OpenImageFile(const string& imageFile)
  {
    if (not mImageFile.is_open())
    {
      mImageFile.open(imageFile);
      if (mImageFile.bad())
      {
        LOG(fail) << "Can't open file " << imageFile << endl;
        FAIL("can-not-open-file");
      }
      return true;
    }
    else // already open one file.
    {
      return false;
    }
  }

  void ImagePrinter::PrintInitialMemorySection(EMemDataType type, uint64 address, uint32 size, const uint8* data)
  {
    string data_type_str = (type == EMemDataType::Instruction) ? "I" : "D";
    mImageFile << data_type_str << " " << fmtx0(address, 16) << " " << fmtd(size, 4) << " " ;
    cuint32 memory_piece_size = 32;
    if (size > memory_piece_size)
    {
      mImageFile << endl << "  ";
    }
    uint64 value = 0;
    uint64 delimiter_count = 0;
    for (uint32 i = 0; i < size - 1; ++i) {
      value = data[i];
      mImageFile << fmtx0(value, 2);
      if ((++delimiter_count % memory_piece_size) == 0)
      {
        mImageFile << endl << "  ";
      }
      else if ((delimiter_count % 8) == 0)
      {
        mImageFile << "_";
      }
    }
    value = data[size - 1];
    mImageFile << fmtx0(value, 2);
    mImageFile << endl;
  }

  void ImagePrinter::PrintMemoryImage(const string& imageFile, const Memory* memory)
  {
    OpenImageFile(imageFile);
    mImageFile << Config::Instance()->HeadOfImage() << endl;
    mImageFile << "# Initializations " << " Memory" << endl;

    vector<Section> sections;
    memory->GetSections(sections);
    for (auto const& rSection : sections) {
      uint8 data[rSection.mSize];
      memory->ReadPartiallyInitialized(rSection.mAddress, rSection.mSize, (uint8*)data);
      PrintInitialMemorySection(rSection.mType, rSection.mAddress, rSection.mSize, data);
    }
    mImageFile.close();
  }

  void ImagePrinter::PrintThreadImageSegment(const ThreadImageSegment* pSegment)
  {
    mImageFile << pSegment->Flag() << " " << fmt(pSegment->Name(), 14).left() << " ";
    auto values = pSegment->Values();
    for (auto value : values) {
      mImageFile << fmtx0(value.mData, value.mSize << 1);
    }
    mImageFile << endl;
  }

  void ImagePrinter::SetupThreadImageSegment(const Register* pReg, const RegisterFile* regFile, ThreadImageSegment* segmentPtr)
  {
    set<PhysicalRegister* > phys_regs_set;
    pReg->GetPhysicalRegisters(phys_regs_set);
    segmentPtr->mFlag = "R";

    auto reg_type = pReg->RegisterType();
    switch (reg_type) {
    case ERegisterType::SIMDVR:
    case ERegisterType::VECREG:
    case ERegisterType::PREDREG:
      segmentPtr->mName = pReg->Name();
      for (auto riter = phys_regs_set.rbegin(); riter != phys_regs_set.rend(); ++riter) {
        auto phys_reg_ptr = *riter;
        segmentPtr->AddValueUnit(phys_reg_ptr->InitialValue(phys_reg_ptr->Mask()), phys_reg_ptr->Size()/8);
      }
      break;
    default:
      if (phys_regs_set.size() > 1) {
        LOG(fail) << "have multi physical registers" << endl;;
        FAIL("multi-physical-registers");
      }
      for (auto phys_reg_ptr : phys_regs_set) {
        segmentPtr->mName = phys_reg_ptr->Name();
        segmentPtr->AddValueUnit(phys_reg_ptr->InitialValue(phys_reg_ptr->Mask()), phys_reg_ptr->Size()/8);
      }
      break;
    }
  }

  void ImagePrinter::PrintThreadInfo(const map<string, uint64>& threadInfo)
  {
    auto thread_id_iter = threadInfo.find("ThreadID");
    if (thread_id_iter == threadInfo.end())
    {
      LOG(fail) << "Can't find the thread ID in threadInfo" << endl;;
      FAIL("can-not-find-thread-id");
    }

    mImageFile << "# Thread " << thread_id_iter->second << " Registers Initializations" << endl;
    mImageFile << "T " << thread_id_iter->second << endl;
    for (auto info_iter = threadInfo.begin(); info_iter != threadInfo.end(); ++info_iter) {
      if (info_iter->first != "ThreadID")
      {
        mImageFile << "V " << fmt(info_iter->first, 14).left() << " " << fmtx0(info_iter->second, 16) << endl;
      }
    }
  }

  void ImagePrinter::PrintRegistersImage(const string& imageFile, const map<string, uint64>& threadInfo, const RegisterFile* regFile)
  {
    if (OpenImageFile(imageFile))
    {
      mImageFile << Config::Instance()->HeadOfImage() << endl;
    }

    PrintThreadInfo(threadInfo);

    auto registers = regFile->Registers();
    list<Register*> initialized_regs;
    for (auto map_iter = registers.begin(); map_iter != registers.end(); ++map_iter)
    {
      if (map_iter->second->Boot() and map_iter->second->IsInitialized())
      {
        initialized_regs.push_back(map_iter->second);
      }
    }

    for (auto reg_ptr : initialized_regs) {
      ThreadImageSegment image_segment;
      SetupThreadImageSegment(reg_ptr, regFile, &image_segment);
      if (mPrinted.find(image_segment.Name()) == mPrinted.end())
      {
        PrintThreadImageSegment(&image_segment);
        mPrinted.insert(image_segment.Name());
      }
    }
  }

  ImageIO::ImageIO(): mpImagePrinter(nullptr), mpImageLoader(nullptr)
  {
    mpImagePrinter = new ImagePrinter;
    mpImageLoader = new ImageLoader;
  }

  ImageIO::~ImageIO()
  {
    delete mpImagePrinter;
    delete mpImageLoader;
  }

  void ImageIO::LoadMemoryImage(const string& imageFile, Memory* memory)
  {
    ImageLoader image_loader;
    image_loader.Load(imageFile);
    image_loader.WriteToMemory(memory);
  }

  void ImageIO::LoadRegistersImage(const string& imageFile, map<string, uint64>& threadInfo, RegisterFile* registerFile)
  {
    if (!mpImageLoader->IsOpen()) {
      mpImageLoader->Load(imageFile);
    }
    mpImageLoader->WriteToThread(threadInfo, registerFile);
  }

  void ImageIO::PrintMemoryImage(const string& imageFile, const Memory* memory)
  {
    mpImagePrinter->PrintMemoryImage(imageFile, memory);
  }

  void ImageIO::PrintRegistersImage(const string& imageFile, const map<string, uint64>& threadInfo, const RegisterFile* regFile)
  {
    mpImagePrinter->PrintRegistersImage(imageFile, threadInfo, regFile);
  }

}
