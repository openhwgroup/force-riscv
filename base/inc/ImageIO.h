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
#ifndef Force_ImageIO_H
#define Force_ImageIO_H

#include <map>
#include <Defines.h>

namespace Force {

  class Memory;
  class RegisterFile;
  class ImagePrinter;
  class ImageLoader;
  /*!
    \class ImageIO
    \brief print/load memory and register in text format.
   */
  class ImageIO {
  public:
    ImageIO(); //!< Default constructor
    ~ImageIO(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(ImageIO);
    COPY_CONSTRUCTOR_ABSENT(ImageIO);
    void PrintMemoryImage(const std::string& imageFile, const Memory* memory);  //!< write memory initial data to an image file.
    void PrintRegistersImage(const std::string& imageFile, const std::map<std::string, uint64>& threadInfo, const RegisterFile* regFile); //!< write registers initial value to an Text file.

    void LoadMemoryImage(const std::string& imageFile, Memory* memory); //!< load memory initial data from an image file.
    void LoadRegistersImage(const std::string& imageFile, std::map<std::string, uint64>& threadInfo, RegisterFile* registerFile); //!< load registers initial value from an Text file.
  private:
    ImagePrinter* mpImagePrinter; //!< pointer of Image Printer.
    ImageLoader* mpImageLoader; //!< pointer of Image Loader.
  };

}
#endif
