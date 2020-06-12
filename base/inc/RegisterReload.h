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
#ifndef Force_RegisterReload_H
#define Force_RegisterReload_H

#include <Defines.h>
#include <Object.h>
#include <map>

namespace Force {

  class RegisterFile;
  class Register;
  class ConstraintSet;
  class ChoicesModerator;

  /*!
    \class RegisterUpdate
    \brief Register to be updated.
  */

  class RegisterUpdate {
  public:
    explicit RegisterUpdate(const Register* pReg); //!< Constructor with register pointer given.
    virtual ~RegisterUpdate();

    void Finalize(const ChoicesModerator* pChoicesModerator);  //!< Finalize RegisterUpdate states
    bool Validate() const; //!< Validate register update against current register value
    void Apply(RegisterFile* pRegFile);  //!< Apply the update to the register file
    void Update(const std::string& fieldName, uint64 value, uint64 dont_care_bits);  //!< register field value to be updated
    uint64 Value() const { return mUpdateValue; }
  private:
    DEFAULT_CONSTRUCTOR_ABSENT(RegisterUpdate);  //!< Hidden default constructor.
    ASSIGNMENT_OPERATOR_ABSENT(RegisterUpdate);
    COPY_CONSTRUCTOR_ABSENT(RegisterUpdate);
  protected:
    std::string mName;  //!< register name
    const Register* mpRegister;  //!< Read-only pointer to Register object
    uint64 mUpdateValue;  //!< Value to be updated
    uint64 mValidateMask;  //!< The fields that we must ensure matching
    std::map<std::string, ConstraintSet*> mFieldContraints;  //!< Constraints on fields
    uint64 mDontCareBits;  //!< don't care bits in field masks
  };

  /*!
    \class RegisterReload
    \brief register context
   */

  class RegisterReload : public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned PeStateUpdate object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the PeStateUpdate object.
    const char* Type() const override { return "RegisterContext"; } //!< Return the type of the object

    RegisterReload(const RegisterFile *pRegFile, const ChoicesModerator* pChoicesMod); //!< Constructor.
    ~RegisterReload(); //!< Destructor.

    void AddRegisterFieldUpdate(const Register* pReg, const std::string& fieldName, uint64 value, uint64 dont_care_bits = 0ull);  //!< Request to register a register field update
    void Finalize();  //!< Finalize RegisterUpdate data objects.
    uint64 GetRegisterValue(const std::string& regName) const;
    bool Validate() const;  //!< Validate the register context.
    void Apply(RegisterFile* pRegFile);  //!< Apply the register context.
  private:
    RegisterReload(); //!< Default constructor.
    RegisterReload(const RegisterReload& rOther); //!< Copy constructor hidden.
    ASSIGNMENT_OPERATOR_ABSENT(RegisterReload);
  protected:
    const RegisterFile* mpRegisterFile;  //!< Read-only pointer to register file object
    const ChoicesModerator* mpChoicesModerator;  //!< Read-only pointer to choices moderator
    std::map<std::string, RegisterUpdate*> mReloadMap;  //!< a map of reloads to be processed
  };

}

#endif
