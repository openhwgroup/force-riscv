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
#ifndef Force_RegisterInitData_H
#define Force_RegisterInitData_H

namespace Force {

  class Generator;
  enum EOperandType;
  class OperandDataRequest;
  class Data;
  class DataPattern;
  class Register;

  /*!
    \class RegisterInitData
    \brief A class to help managing register data
  */

  class RegisterInitData {
  public:
    RegisterInitData(): mpDataRequest(nullptr), mpData(nullptr) { }  //!< Constructor
    ~RegisterInitData() { }

    ASSIGNMENT_OPERATOR_ABSENT(RegisterInitData);
    COPY_CONSTRUCTOR_DEFAULT(RegisterInitData);
    void Setup(const Generator& gen, const Register *pRegister, const OperandDataRequest *pDataRequest); //!< set up constraint.
    void Commit(const Generator& gen, Register* pRegister); //!< Commit data to generator.
    void Cleanup(); //!< do some clean up
  private:
    const OperandDataRequest* mpDataRequest; //!< pointer to data request
    const Data *mpData; //!< pointer to data
  };

}

#endif
