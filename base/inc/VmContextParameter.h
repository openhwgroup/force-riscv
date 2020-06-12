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
#ifndef Force_VmContextParameter_H
#define Force_VmContextParameter_H

#include <Object.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <Defines.h>
#include <string>
#include <map>
#include <vector>

namespace Force
{

  class Generator;

  class VmContextParameter {
  public:
    VmContextParameter(EVmContextParamType paramType, const std::string& rRegName, const std::string& rFieldName); //!< Constructor with necessary setup info.
    virtual ~VmContextParameter(); //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(VmContextParameter);
    DEFAULT_CONSTRUCTOR_ABSENT(VmContextParameter);

    EVmContextParamType ParamType() const { return mParamType; } //!< Return parameter type.
    virtual void   Initialize(Generator* pGen); //!< Store the context value in mValue, initializing the field if uninitialized
    virtual uint64 Reload(Generator* pGen);     //!< Return a reload value for the context parameter's field
    virtual bool   Validate(Generator* pGen, std::string& rErrMsg) const; //!< Return whether mValue matches the current register field value
    virtual void   SetValue(uint64 value){ mValue = value;} //!<Manually set ContextParameter value, used by MultiAddressSpace context switching
    virtual uint64 Value() const { return mValue; } //!< Return value of parameter
    virtual const std::string& RegisterName() const { return mRegisterName; } //!< Return the parameter's register name
    virtual const std::string& FieldName() const    { return mFieldName; } //!< Return the parameter's field name
    virtual const std::string ToString() const; //!< Return a string description of the object.
    virtual VmContextParameter* Clone() const { return new VmContextParameter(*this); } //!< Clone the VmContextParameter object.
    virtual void Update(const VmContextParameter& rOther); //!< Update VmContextParameter content.
    virtual bool LessThan(const VmContextParameter& rOther) const; //!< Return whether this object compares less than the passed in parameter.
    virtual bool Matches(const VmContextParameter& rOther) const; //!< Return whether this object matches the passed in parameter.
    virtual bool GetDelta(uint64& rValue, const Generator* pGen) const; //!< Return different from current machine state.
    virtual const std::string ParameterName() const; //!< Return parameter name.
  protected:
    VmContextParameter(const VmContextParameter& rOther); //!< Copy Constructor.
    void VerifyCompatibility(const VmContextParameter& rOther) const; //!< Check if the passed in VmContextParameter is compatible with this object.
  protected:
    EVmContextParamType mParamType; //!< Context parameter type.
    std::string mRegisterName; //!< Register that contains the parameter.
    std::string mFieldName; //!< Register field corresponding to the context parameter.
    uint64 mValue; //!< Value of the context parameter.
    bool mInitialized; //!< Is the parameter initialized.
  };

  /*!
    \class VmContext
    \brief Container of VmContextParameters and base class for VmControlBlock class.
  */
  class VmContext : public Object {
  public:
    Object *          Clone()    const override { return new VmContext(*this); } //!< Clone VmContext object.
    const std::string ToString() const override;  //!< Return a string describing the VmContext object.
    const char*       Type()     const override { return "VmContext"; }

    VmContext(); //!< Default constructor.
    ~VmContext() override;//!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(VmContext);

    inline bool ContextParamBoolean(EVmContextParamType cpType) const               //!< Return boolean value for a context parameter.
    {
      return (mContextParams.at(EVmContextParamTypeBaseType(cpType))->Value() != 0);
    }

    inline uint64 ContextParamValue(EVmContextParamType cpType) const               //!< Return integer value for a context parameter.
    {
      return mContextParams.at(EVmContextParamTypeBaseType(cpType))->Value();
    }

    inline bool HasContextParam(EVmContextParamType cpType) const                   //!< Return whether a context parameter exists.
    {
      return (EVmContextParamTypeBaseType(cpType) < mContextParams.size()) and (nullptr != mContextParams[EVmContextParamTypeBaseType(cpType)]);
    }

    uint32 GenContextId() const { return mGenContextId; } //!< Return VmContext object's generator context ID.
    bool UpdateContextParams(const std::map<std::string, uint64>& rRawContextParams); //!< user to input rRawContextParams map to update mContextParams
    bool UpdateContext(const VmContext* pVmContext); //!< user to input VmContext to update mContextParams.
    void InitializeContext(Generator* pGen); //!< Initialize context parameters.
    void GetContextDelta(std::map<std::string, uint64> & rDeltaMap, const Generator* pGen) const; //!< Find the delta map between the VmContext and currect machine state.
    bool operator < (const VmContext& rOther) const; //!< Compare operator.
    bool Matches(const VmContext& rOther) const; //!< Check if this object matches with passed in VmContext object.
    void AddParameter(VmContextParameter* pContextParam); //!< Add context parameter object.
  protected:
    VmContext(const VmContext& rOther); //!< Copy constructor.
    void ObtainGenContextId(); //!< Obtain a generator context ID for the VmContext object.
    VmContextParameter* GetContextParameter(EVmContextParamType paramType) const; //!< Get VmContextParamter object.
    bool NullParameterPair(const VmContextParameter* pParam1, const VmContextParameter* pParam2) const; //!< Check if the parameters are both nullptr.
  protected:
    static uint32 msSequentialGenContextId; //!< Current sequential generator context ID.
    uint32 mGenContextId; //!< Generator context ID for the VmContext object.
    std::vector<VmContextParameter *> mContextParams;  //!< map of Context
  };
}

#endif //Force_VmContextParameter_H
