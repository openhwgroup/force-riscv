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
#ifndef Force_Register_H
#define Force_Register_H

#include <vector>
#include <Defines.h>
#include <Object.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <set>
#include <map>
#include <list>
#include <Notify.h>
#include <NotifyDefines.h>

namespace Force {

  class Register;
  class PhysicalRegister;
  class BitField;
  class RegisterFile;
  class ChoicesModerator;
  class ConstraintSet;
  class ChoiceTree;
  class ConstraintSet;

  /*! \class PhysicalRegister
   */
  class PhysicalRegister : public Object
  {
  public:
    PhysicalRegister();                              //!< Default constructor - zero init, GPR, ReadWrite Attrs.
    virtual ~PhysicalRegister();                     //!< Destructor
    PhysicalRegister(const PhysicalRegister& rOther); //!< Copy constructor - Calls Object copy, copies all other members

    Object*           Clone()    const override { return new PhysicalRegister(*this); } //!< Clone function for Object
    const std::string ToString() const override { return mName;                       } //!< ToString function for Object class
    const char*       Type()     const override { return "PhysicalRegister";          } //!< Type function for Object class

    bool         IsInitialized(uint64 mask, bool* partial=NULL) const;   //!< Checks if bits in mask are initialized
    bool         HasAttribute(ERegAttrType attr) const;                        //!< True if mAttributes contains attr
    void         SetAttribute(ERegAttrType attr);                //!< Set attr bits in mAttributes
    void         ClearAttribute(ERegAttrType attr);                      //!< Clear attr bits in mAttributes
    virtual void Block()                                        const {} //!< Placeholder for derived object w/ Sender
    virtual void Unblock()                                      const {} //!< Placeholder for derived object w/ Sender

    const std::string&  Name()                    const { return mName;         } //!< Get mName
    const ERegisterType RegisterType()            const { return mRegisterType; } //!< Get mRegisterType
    uint64              Value(uint64 mask)        const;                          //!< Get value for mask's bits
    uint64              Mask()                    const { return mMask;         } //!< Get mMask
    uint64              InitialValue(uint64 mask) const;                          //!< Get mInitValue for mask's bits
    uint64              InitMask()                const { return mInitMask;     } //!< Get mInitMask
    uint64              ResetValue()              const { return mResetValue;   } //!< Get mResetValue.
    uint64              ResetMask()               const { return mResetMask;    } //!< Get mResetMask.
    uint32              Size()                    const { return mSize;         } //!< Get mSize
    uint32              IndexValue()              const { return mIndex;        } //!< Get mIndex
    uint32              SubIndexValue()           const { return mSubIndex;     } //!< Get mSubIndex

    virtual void SetValue(uint64 value, uint64 mask);      //!< Set mValue using value for mask's bits
    void         SetResetValue(uint64 value, uint64 mask); //!< Set mResetValue/mResetMask using value for mask's bits
  protected:
    virtual void Initialize(uint64 value, uint64 mask); //!< Initialize mask's bits to value
    virtual void SetSizeAndMask(uint32);  //!< Set mSize, mMask
    virtual inline bool HasLinkRegister() { return false; } //!< whether has link register
  protected:
    std::string mName;           //!< Name
    ERegisterType mRegisterType; //!< Register Type from Enums
    uint64 mValue;               //!< Current Value
    uint64 mMask;                //!< Mask (~0x0 >> 64-size). For example, size = 8, mask = 0xff
    uint64 mInitialValue;        //!< Initialized Register Value
    uint64 mInitMask;            //!< Initialized Bits Mask. For example, size = 8, mInitMask may be 0xff00, determined by its bit-field shift
    uint64 mResetValue;          //!< Current Reset Value
    uint64 mResetMask;           //!< Current Reset Mask
    uint32 mSize;                //!< Size (0 <= size <= 64)
    uint32 mIndex;               //!< Index value
    uint32 mSubIndex;            //!< SubIndex value
    uint32 mAttributes;          //!< Bitmap of ERegAttrType (3 bits - HasValue | Write | Read)

    friend class RegisterParser;
    friend class RegisterFile;
    friend class BitField;
    friend class LinkedPhysicalRegister;
    friend class ImageLoader;
  };

  /*!
    \class LinkedPhysicalRegister
    \brief physical register which propogates value changes to another physical register
  */
  class LinkedPhysicalRegister : public PhysicalRegister
  {
  public:
    LinkedPhysicalRegister();  //!< Default constructor - zero init, GPR, ReadWrite Attrs.
    ~LinkedPhysicalRegister(); //!< Destructor
    LinkedPhysicalRegister(const LinkedPhysicalRegister& rOther); //!< Copy constructor - Calls Object copy, copies all other members
    ASSIGNMENT_OPERATOR_ABSENT(LinkedPhysicalRegister);

    Object*     Clone() const override { return new LinkedPhysicalRegister(*this); } //!< Clone function for Object
    const char* Type()  const override { return "LinkedPhysicalRegister";          } //!< Type function for Object class

    void SetValue(uint64 value, uint64 mask) override;
  protected:
    void Initialize(uint64 value, uint64 mask) override;
    inline bool HasLinkRegister() override { return true; } //!< whether has link register
    PhysicalRegister* mpPhysicalLink;

    friend class RegisterFile;
  };

  /*!
    \class PhysicalRegisterRazwi
    \brief model to behavior of physical registers that are RAZ/WI
  */
  class PhysicalRegisterRazwi : public PhysicalRegister
  {
  public:
    PhysicalRegisterRazwi(): PhysicalRegister() {}                                     //!< Default constructor.
    PhysicalRegisterRazwi(const PhysicalRegisterRazwi& rOther) : PhysicalRegister(rOther) {} //!< Copy constructor.

    Object*     Clone() const override { return new PhysicalRegisterRazwi(*this); } //!< Clone function for PhysicalRegisterRazwi class.
    const char* Type()  const override { return "PhysicalRegisterRazwi";          } //!< Type Function for PhysicalRegisterRazwi class.

    void SetValue(uint64 value, uint64 mask) override { } //!< Overriding SetValue to ignore.
  protected:
    void SetSizeAndMask(uint32 size) override;  //!< Set mSize, mMask
  };

  /*! \class ConfigureRegister
  */
  class ConfigureRegister: public NotificationSender, public PhysicalRegister
  {
  public:
    ConfigureRegister(): Sender(), PhysicalRegister() {}                                     //!< Default constructor
    ConfigureRegister(const ConfigureRegister& rOther) : Sender(), PhysicalRegister(rOther) {} //!< Copy constructor - copies PhysicalRegister

    Object*     Clone() const override { return new ConfigureRegister(*this); } //!< Clone function for Object class.
    const char* Type()  const override { return "ConfigureRegister";          } //!< Type Function for Object class.

    void SetValue(uint64 value, uint64 mask) override; //!< Overriding SetValue to send notification when updating

    void Block()   const override { const_cast<ConfigureRegister*>(this)->Sender::Block();   } //!< Called to block the Sender object from sending out notification.
    void Unblock() const override { const_cast<ConfigureRegister*>(this)->Sender::Unblock(); } //!< Called to unblock the Sender object, and send out cached notifications.
  };

   /*! \class BitField
   */
  class BitField : public Object
  {
  public:
    BitField();                       //!< Default constructor, zero/nullptr init
    ~BitField();                      //!< Destructor
    BitField(const BitField& rOther); //!< Copy constructor, private

    Object*           Clone()    const override { return new BitField(*this);     } //!< Clone function in Object class
    const std::string ToString() const override { return mpRegister->ToString(); } //!< ToString function in Object class
    const char*       Type()     const override { return "BitField";              } //!< Type function in Object class

    bool IsInitialized(bool *partial=NULL) const; //!< Test if the register is initialized.
    bool HasAttribute(ERegAttrType attr) const { return mpRegister->HasAttribute(attr);   } //!< Wrapper for phy reg HasAttribute
    void SetAttribute(ERegAttrType attr)   { return mpRegister->SetAttribute(attr);   } //!< Wrapper for phy reg SetAttribute
    void ClearAttribute(ERegAttrType attr) { return mpRegister->ClearAttribute(attr); } //!< Wrapper for phy reg ClearAttribute
    void Block()                     const { mpRegister->Block();   } //!< Block Sender class from sending notifications
    void Unblock()                   const { mpRegister->Unblock(); } //!< Unblock Sender class from sending notifications
    void GetPhysicalRegisters(std::set<PhysicalRegister*>& phyRegisterSet) const; //!< Add new physical registers in this vector;

    uint32 Size()                 const { return mSize; }                               //!< Get mSize
    uint64 Value()                const { return mpRegister->Value(mMask);           } //!< Get masked phy reg value
    uint64 BitFieldValue()        const { return mpRegister->Value(mMask) >> mShift; } //!< Get shifted field value
    uint64 InitialValue()         const { return mpRegister->InitialValue(mMask);           } //!< Get initial masked phy reg value
    uint64 InitialBitFieldValue() const { return mpRegister->InitialValue(mMask) >> mShift; } //!< Get initial shifted field value
    uint64 ReloadValue(cuint64 reloadValue) const { return (reloadValue & mMask); } //!< Get ReloadValue w/ no choice value
    uint64 ReloadValue(uint64& reloadValue, uint64 new_value) const;                //!< Get ReloadValue w/ choice value
    uint64 Mask()                 const { return mMask; }                           //!< Get mMask of bitfield, not shifted

    void SetValue(uint64 value)         { mpRegister->SetValue(value, mMask);                } //!< Set masked phy reg value
    void SetBitFieldValue(uint64 value) { mpRegister->SetValue(value << mShift, mMask);      } //!< Set shifted field value.
    void SetBitResetValue(uint64 value) { mpRegister->SetResetValue(value << mShift, mMask); } //!< Set Reset Members for phy reg
    ASSIGNMENT_OPERATOR_ABSENT(BitField) ;
  private:
    void Initialize(uint64 value);        //!< Init phy reg with (value & bitfield mask)
    void InitializePartial(uint64 value); //!< Init phy reg using only uninit'd bits from bitfield mask
    void InitializeBitField(uint64 value) { Initialize(value << mShift); } //!< Init using unshifted field value

    uint64            mMask;       //!< Mask bit map determined from mSize and mShift
    uint32            mSize;       //!< Size (in bits)
    uint32            mShift;      //!< Shift from phy_reg LSB (in bits)
    PhysicalRegister* mpRegister; //!< Associated physical reigster.

    void SetSize(const uint32 size) { mSize = size; } //!< Set mSize
    void SetMask();                                   //!< Set mMask once mSize and mShift are valid

    friend class RegisterParser;
    friend class Register;
    friend class RegisterFile;
    friend class RegisterField;
    friend class RegisterFieldRes0;
    friend class RegisterFieldRes1;
  };

  class InitPolicyTuple;

  /*!
    \class RegisterField
    \brief Main class modeling register fields.
  */
  class RegisterField : public Object
  {
  public:
    RegisterField();    //!< Default constructor, empty, private.
    ~RegisterField();   //!< Destructor
    RegisterField(const RegisterField& rOther); //!< Copy constructor
    ASSIGNMENT_OPERATOR_ABSENT(RegisterField);

    Object*           Clone()    const override { return new RegisterField(*this); } //!< Clone function as declared in Object.
    const std::string ToString() const override { return mName;                    } //!< ToString function as declared in Object.
    const char*       Type()     const override { return "RegisterField";          } //!< Type function as declared in Object.

    bool               IsInitialized(bool *partial=NULL) const; //!< Test if field is initialized, and whether it is fully or partially intialized.
    virtual uint64     Value()             const; //!< Getter for logical register value.
    virtual uint64     InitialValue()      const; //!< Getter for initial logical register value.
    virtual uint64     FieldValue()        const; //!< Getter for phyical register value.
    virtual uint64     InitialFieldValue() const; //!< Getter for initial phyicial register value.
    uint64             FieldMask()         const; //!< Getter for bitmask of register field.
    uint64             GetPhysicalRegisterMask() const; //!< Return bitmask representing the mapping of this register field unto its underlying physical register.
    uint32             Lsb()               const { return mLsb;  } //!< Return the lowest mLsb of bits
    uint32             Size()              const { return mSize; } //!< Return physical register size.
    const std::string& Name()              const { return mName; } //!< Getter for current RegisterField name.
    const std::string& PhysicalRegisterName() const { return mPhysicalRegisterName; } //!< Getter for current RegisterField name.

    virtual uint64 ReloadValue(uint64& reloadValue, const ChoiceTree* pChoiceTree = 0)      const; //!< Get Reload Value without setting it.
    virtual uint64 ReloadValue(uint64& reloadValue, const ConstraintSet* constraintSets= 0) const; //!< Get Reload Value without setting it.

    void         SetPhysicalRegisterName(const std::string& phyName) { mPhysicalRegisterName = phyName; } //!< Setter for Physical Register Name.
    virtual void SetValue(uint64 value);                                 //!< Setter for logical register value.
    virtual void SetFieldValue(uint64 value);                            //!< Setter for physical register value.
    virtual void SetFieldResetValue(uint64 value) const;                 //!< Setup resetvalue and resetMask for underlying physical register.

    virtual bool IgnoreUpdate() const { return false; } //! Virtual ignore updates, used for ISS Write Update checking
    bool IgnoreUpdate(const std::string& ignoredRegFields) const; //!< ignore update if falls in ignored register fields variable.
    bool HasAttribute(ERegAttrType) const;                        //!< Test if any of the bit fields contain specified attribute
    void SetAttribute(ERegAttrType);                            //!< Set all bit fields to specified attribute.
    void ClearAttribute(ERegAttrType);                          //!< Clear all bit fields at specified bits.

    void Block()   const; //!< Called to block the Sender object from sending out notification.
    void Unblock() const; //!< Called to unblock the Sender object, and send out cached notifications.
    void SetInitPolicy(const std::string& rPolicyName); //!< Set init policy.
  protected:
    virtual void Initialize(uint64 value);       //!< Initialization from logical register value.
    virtual void InitializeField(uint64 value);                               //!< Initialize from physical register value.
    virtual void InitializeRandomly(const ChoiceTree* pChoiceTree = nullptr); //!< Initialize this field randomly.

    virtual void Setup(const RegisterFile* pRegisterFile);                         //!< Setup bitfields phy reg pointers and phy reg reset.
    void GetPhysicalRegisters(std::set<PhysicalRegister*>& phyRegisterSet) const; //!< Add this registerfields phy registers to input vector.

    uint32      mSize;                  //!< Size of this register field.
    uint32      mLsb;                   //!< mLsb relative to the block, the lowest mLsb of its bitfields.
    uint32      mLsbBlock;              //!< mLsb of entire block eg. 128 bit register, upper block = 64/lower block = 0
    uint64      mReset;                 //!< reset value if there is any, default to 0
    bool        mHasReset;              //!< Flag to indicate if resetValue is set
    std::string mName;                  //!< Name of this register field.
    std::string mPhysicalRegisterName;  //!< Associated Physical Register.
    InitPolicyTuple* mpInitPolicyInfo;  //!< Pointer to initialization policy info.
    std::vector<BitField *> mBitFields; //!< child BitFields.

    friend class RegisterParser;
    friend class Register;
    friend class LargeRegister;
    friend class RegisterFile;
    friend class RegisterInitPolicy;
  };

  /*! \class RegisterFielRes0
      \brief register class for RES0 field
  */
  class RegisterFieldRes0 : public RegisterField
  {
  public:
    //\section Object_functions functions to implement basic Object methods.
    Object * Clone() const override;                                      //!< Clone function as declared in Object.
    //const std::string ToString() const override;
    const char* Type() const override { return "RegisterFieldRes0"; }         //!< Type function as declared in Object.

    RegisterFieldRes0() : RegisterField() { } //!< Default constructor
    ~RegisterFieldRes0();   //!< Destructor

  protected:
    RegisterFieldRes0(const RegisterFieldRes0& rOther); //!< Copy constructor, protected.

    void InitializeRandomly(const ChoiceTree* pChoiceTree = nullptr) override;  //!< Initialize this field randomly.
    uint64 ReloadValue(uint64& reloadValue, const ChoiceTree* pChoiceTree = 0) const override;  //!< Get Reload Value without setting it.
    uint64 ReloadValue(uint64& reloadValue, const ConstraintSet* constraintSets= 0) const override;  //!< Get Reload Value without setting it.
  };

  /*! \class RegisterFieldRes1
      \brief register class for RES1 field
  */
  class RegisterFieldRes1 : public RegisterField
  {
  public:
    //\section Object_functions functions to implement basic Object methods.
    Object * Clone() const override;                                      //!< Clone function as declared in Object.
    //const std::string ToString() const override;
    const char* Type() const override { return "RegisterFieldRes1"; }         //!< Type function as declared in Object.

    RegisterFieldRes1() : RegisterField() { } //!< Default constructor
    ~RegisterFieldRes1();   //!< Destructor

  protected:
    RegisterFieldRes1(const RegisterFieldRes1& rOther); //!< Copy constructor

    void InitializeRandomly(const ChoiceTree* pChoiceTree = nullptr) override;  //!< Initialize this field randomly.
    uint64 ReloadValue(uint64& reloadValue, const ChoiceTree* pChoiceTree = 0) const override;  //!< Get Reload Value without setting it.
    uint64 ReloadValue(uint64& reloadValue, const ConstraintSet* constraintSets= 0) const override;  //!< Get Reload Value without setting it.
  };

  /*! \class RegisterFielRazwi
      \brief register class for RAZ/WI field, reading this register field will return 0, writing to this register field will be ignored (write-inhibit).
  */
  class RegisterFieldRazwi : public RegisterField
  {
  public:
    //\section Object_functions functions to implement basic Object methods.
    Object * Clone() const override;                                   //!< Clone function as declared in Object.
    const char* Type() const override { return "RegisterFieldRazwi"; } //!< Type function as declared in Object.

    RegisterFieldRazwi() : RegisterField() { } //!< Default constructor
    ~RegisterFieldRazwi();                     //!< Destructor

    uint64 Value()             const override { return 0u; } //!< Getter for logical register value.
    uint64 InitialValue()      const override { return 0u; } //!< Getter for initial logical register value.
    uint64 FieldValue()        const override { return 0u; } //!< Getter for phyical register value.
    uint64 InitialFieldValue() const override { return 0u; } //!< Getter for initial phyicial register value.

    void SetFieldValue(uint64 value)      const { }  //!< Setter for physical register value.
    void SetValue(uint64 value)           const { }  //!< Setter for logical register value.
    void SetFieldResetValue(uint64 value) const override { RegisterField::SetFieldResetValue(0u); }  //!< Setup resetvalue and resetMask for underlying physical register.

    bool IgnoreUpdate() const override { return true; } //! ignore updates for ISS Write Update checking
  protected:
    RegisterFieldRazwi(const RegisterFieldRazwi& rOther); //!< Copy constructor, protected.

    void InitializeField(uint64 value) override { RegisterField::InitializeField(0u); } //!< Initialize from physical register value.
    void Initialize(uint64 value) override { RegisterField::Initialize(0u);  } //!< Initialization from logical register value .
    void InitializeRandomly(const ChoiceTree* pChoiceTree = nullptr) override; //!< Initialize this field randomly.
  };

  /*! \class RegisterFielRaowi
      \brief register class for RAO/WI field, reading this register field will return 1, writing to this register field will be ignored (write-inhibit).
  */
  class RegisterFieldRaowi : public RegisterField
  {
  public:
    //\section Object_functions functions to implement basic Object methods.
    Object * Clone() const override;                                      //!< Clone function as declared in Object.
    const char* Type() const override { return "RegisterFieldRaowi"; }         //!< Type function as declared in Object.

    RegisterFieldRaowi() : RegisterField() { } //!< Default constructor
    ~RegisterFieldRaowi();   //!< Destructor

    uint64 Value()             const override { return MAX_UINT64 & FieldMask(); } //!< Getter for logical register value.
    uint64 InitialValue()      const override { return MAX_UINT64 & FieldMask(); } //!< Getter for initial logical register value.
    uint64 FieldValue()        const override { return MAX_UINT64 & FieldMask(); } //!< Getter for phyical register value.
    uint64 InitialFieldValue() const override { return MAX_UINT64 & FieldMask(); } //!< Getter for initial phyicial register value.

    void SetFieldValue(uint64 value)      const { }  //!< Setter for physical register value.
    void SetValue(uint64 value)           const { }  //!< Setter for logical register value.
    void SetFieldResetValue(uint64 value) const override { RegisterField::SetFieldResetValue(MAX_UINT64); }  //!< Setup resetvalue and resetMask for underlying physical register.

    bool IgnoreUpdate() const override { return true; } //! ignore updates for ISS Write Update checking
  protected:
    RegisterFieldRaowi(const RegisterFieldRaowi& rOther); //!< Copy constructor, protected.

    void InitializeField(uint64 value) override { RegisterField::InitializeField(MAX_UINT64); } //!< Initialize from physical register value.
    void Initialize(uint64 value) override { RegisterField::Initialize(MAX_UINT64); } //!< Initialization from logical register value .
    void InitializeRandomly(const ChoiceTree* pChoiceTree = nullptr) override; //!< Initialize this field randomly.
  };

   /*!
    \class ReadOnlyRegisterField
    \brief Main class modeling register read-only fields.
  */
  class ReadOnlyRegisterField : public RegisterField
  {
  public:
    ReadOnlyRegisterField() : RegisterField() { };    //!< Default constructor
    ~ReadOnlyRegisterField() { }   //!< Destructor
    ReadOnlyRegisterField(const ReadOnlyRegisterField& rOther): RegisterField(rOther) { } //!< Copy constructor

    Object* Clone()    const override { return new ReadOnlyRegisterField(*this); } //!< Clone function as declared in Object.
    const char* Type()     const override { return "ReadOnlyRegisterField"; } //!< Type function as declared in Object.

    uint64 ReloadValue(uint64& reloadValue, const ChoiceTree* pChoiceTree = 0)  const override; //!< Get Reload Value without setting it.
    uint64 ReloadValue(uint64& reloadValue, const ConstraintSet* constraintSets= 0) const override; //!< Get Reload Value without setting it.

    void SetValue(uint64 value) override { }       //!< write ignore
    void SetFieldValue(uint64 value) override { }   //!< write ignore

    bool IgnoreUpdate() const override { return true; } //! Virtual ignore updates, used for ISS Write Update checking
    void Initialize(uint64 value) override;  //!< Initialization from logical register value
    void InitializeField(uint64 value) override; //!< Initialize from physical register value.
    void InitializeRandomly(const ChoiceTree* pChoiceTree = nullptr) override; //!< Initialize this field randomly.
  protected:
    void Setup(const RegisterFile* pRegisterFile) override;  //!< Setup bitfields phy reg pointers and phy reg reset.
  };

  /*!
    \class Register
    \brief The main class modeling logical registers.
  */
  class Register : public Object
  {
  public:
    Register();  //!< Default Constructor
    ~Register(); //!< Destructor

    Object*           Clone()    const override { return new Register(*this); } //!< Clone function as declared in Object.
    const std::string ToString() const override { return mName;               } //!< ToString function as declared in Object.
    const char*       Type()     const override { return "Register";          } //!< Type function as declared in Object.

    uint32              Size()         const { return mSize;         } //!< Return size.
    uint32              Boot()         const { return mBoot;         } //!< Getter for the mBoot value.
    uint32              IndexValue()   const { return mIndex;        } //!< Getter for mIndex.
    const ERegisterType RegisterType() const { return mRegisterType; } //!< Getter for mRegisterType.
    const std::string&  Name()         const { return mName;         } //!< Getter for the mName.
    virtual const std::string RealName() const { return mName;       } //!< Return the real name of the register.

    virtual bool   IsReadOnly()                      const { return false; } //!< Base register class is not readonly.
    virtual bool   IsLargeRegister()                 const { return false; } //!< Base register class is not a large register (>64b).
    bool           IsInitialized(bool *partial=NULL) const; //!< Test if the register is initialized.
    uint64         Value()                           const; //!< Getter for value.
    uint64         InitialValue()                    const; //!< Getter for initial value.
    virtual uint64 ReloadValue()                     const; //!< Reload value.
    virtual uint64 ReloadValue(const ChoicesModerator* pChoicesModerator, const std::map<std::string, ConstraintSet* >& fieldConstraints) const; //!< Reload value.
    virtual uint64 FieldReloadValue(const std::string& fieldName, const ChoicesModerator* pChoicesModerator) const;  //!< reload field value

    virtual void   SetValue(uint64 value); //!< Setter for value.

    virtual void   RegisterFieldInfo(const ChoicesModerator* pChoicesModerator, const std::map<std::string, ConstraintSet*>& fieldConstraints, uint64& mask, uint64& value) const; //!< generate mask/value with given field contraints
    RegisterField* RegisterFieldLookup(const std::string& name) const; //!< Get the register field from its name.
    const std::vector<RegisterField* >& RegisterFields() const { return mRegisterFields; } //!< Return constant reference of vector of children RegisterField

    uint64 GetRegisterFieldMask(const std::vector<std::string>& field_names) const; //!< Return bitmask of register fields provided
    uint64 GetPhysicalRegisterMask(const PhysicalRegister& rPhyReg) const; //!< Return bitmask representing the mapping of this register unto the specified physical register
    const std::map<std::string, RegisterField*> GetRegisterFieldsFromMask(uint64 mask) const; // Return map of field names to RegisterField* of fields with bits inside give mask
    void GetPhysicalRegisters(std::set<PhysicalRegister*>& phyRegisterSet) const; //!< Add this register's physical registers to input vector

    bool HasAttribute(ERegAttrType) const; //!< Test if the register has the specified attribute.
    void SetAttribute(ERegAttrType);   //!< Set the attribute from given type.
    void ClearAttribute(ERegAttrType); //!< Clear the attribut from given type.

    void Block()   const; //!< Called to block the Sender object from sending out notification.
    void Unblock() const; //!< Called to unblock the Sender object, and send out cached notifications.
    void SetInitPolicy(const std::string& rPolicyName); //!< Set init policy.
    virtual void GetRelyUponRegisters(std::vector<std::string>& rRegNames) const { } //!< Return rely upon register names, if any.
    ASSIGNMENT_OPERATOR_ABSENT(Register) ;
 protected:
    Register(const Register& rOther); //!< Copy Constructor
    void                 Initialize(uint64 value);                   //!< Init value of register w/ value, or if choices are defined select value from choice tree
    virtual void         Initialize(std::vector<uint64> values );     //!< Init value of register w/ value, or if choices are defined select value from choice tree
    virtual void         InitializeRandomly(const ChoicesModerator* pChoicesModerator);                          //!< Initialize randomly
    virtual void Setup(const RegisterFile* registerFile);  //!< Setup the associated physical register.
    virtual void SetName(const std::string& rName) { mName = rName; } //!< Set register name.
    ChoiceTree* GetChoiceTree(const std::string& fieldName, const ChoicesModerator* pChoicesModerator) const; //<! search for the Choice tree for <RegisterName>.<FieldName>
  protected:
    uint32 mSize;                                 //!< Size, number of bits.
    uint32 mBoot;                                 //!< Boot priority value.
    uint32 mIndex;                                //!< Index value, unique for each ERegisterType.
    ERegisterType mRegisterType;                  //!< RegistType: GPR, SIMDR etc.
    std::string mName;                            //!< Name.
    InitPolicyTuple* mpInitPolicyInfo;            //!< Pointer to initialization policy info.
    std::vector<RegisterField *> mRegisterFields; //!< Vector of RegisterFields.
    // TODO may change from vector to dictionary to increase search performance.

    friend class RegisterParser;
    friend class RegisterFile;
    friend class RegisterInitPolicy;
  };

  /*!
    \class RegisterNoAlias
    \brief A register class to allow no-aliased access.
  */
  class RegisterNoAlias : public Register
  {
  public:
    RegisterNoAlias() : Register(), mRealName() { }  //!< Default Constructor
    ~RegisterNoAlias() { } //!< Destructor

    Object*           Clone()    const override { return new RegisterNoAlias(*this); } //!< Clone function as declared in Object.
    const char*       Type()     const override { return "RegisterNoAlias";   } //!< Type function as declared in Object.
    const std::string RealName() const override { return mRealName;       } //!< Return the real name of the register.
  protected:
    RegisterNoAlias(const RegisterNoAlias& rOther) : Register(rOther), mRealName(rOther.mRealName) { } //!< Copy Constructor
    void SetName(const std::string& rName) override; //!< Set register name.
  protected:
    std::string mRealName; //!< Real name of the register.
  };

  /*! \class LargeRegister
   */
  class LargeRegister : public Register
  {
  public:

    ///\section override_functions
    const char* Type() const override { return "LargeRegister";}

    ///\section constructor_destructor constructor, destructor & copy
    Object * Clone() const override;
    LargeRegister();      //!< Default constructor, empty, private.
    LargeRegister(const LargeRegister& rOther); //!< Copy constructor from same class
    explicit LargeRegister(const Register& rOther); //!< Copy constructor from parent class
    ~LargeRegister();   //!< Destructor

    std::vector<uint64> Values() const; //!< return vector of values for entire large register
    std::vector<uint64> InitialValues() const; //!< return vector of initial values for entire large register
    std::vector<uint64> ReloadValues() const; //!< return vector of reload values for entire large register

    bool IsLargeRegister() const override { return true; } //!< Return true for being a LargeRegister

  private:
    void Initialize(std::vector<uint64> values) override;
  };

  /*! \class ReadOnlyRegister
   */
  class ReadOnlyRegister : public Register
  {
  public:

    //\section Write_ignore_functions functions to do nothing when trying to rewrite to this register
    void SetValue(uint64 value) override;       // Ignore to write
    uint64 ReloadValue(const ChoicesModerator* pChoicesModerator, const std::map<std::string, ConstraintSet* >& fieldConstraintMap ) const override;                // Ignore to write
    uint64 ReloadValue() const override;            //!< Reload value.
    void InitializeRandomly(const ChoicesModerator* pChoicesModerator) override;      // Ignore to write.
    void Setup(const RegisterFile* pRegisterFile) override;  //!< Setup the read-only-register.

    ///\section overide_functions
    const char* Type() const override { return "ReadOnlyRegister"; }
    bool IsReadOnly() const override { return true; }  //!< Return true for being readonly.

    ///\section constructors constructor, copy constructor & destructor
    ReadOnlyRegister(); //!< Default constructor, empty.
    ~ReadOnlyRegister();   //!< Destructor
    ReadOnlyRegister(const ReadOnlyRegister& rOther); //!< Copy constructor from same class
    Object * Clone() const override;

  protected:
    bool mTakeException;

  private:
    void initiate_TakeException();

  };

  /*! \class ReadOnlyZeroRegister
   */
  class ReadOnlyZeroRegister : public ReadOnlyRegister
  {
  public:
    void Setup(const RegisterFile* pRegisterFile) override;  //!< Setup the read-only-register.

    ///\section overide_functions
    const char* Type() const override { return "ReadOnlyZeroRegister"; }

    ///\section constructors constructor, copy constructor & destructor
    ReadOnlyZeroRegister() : ReadOnlyRegister() { }; //!< Default constructor, empty.
    ~ReadOnlyZeroRegister() { } //!< Destructor
    Object * Clone() const override { return new ReadOnlyZeroRegister(*this); } //!< Return cloned object.
  protected:
    ReadOnlyZeroRegister(const ReadOnlyZeroRegister& rOther) : ReadOnlyRegister(rOther) { } //!< Copy constructor.
  };

  /*! \class BankedRegister

      Still an abstract class for not overriding receiver virtual functions, so not having Clone function.
  */
  class BankedRegister : public Register, public NotificationReceiver
  {
  public:
    ///\section constructor/destructor
    BankedRegister ();                                               //!< Default constructor
    BankedRegister (const BankedRegister& rOther);                    //!< Copy constructor
    ~BankedRegister() { mRegisterFields = mOriginalRegisterFields; } //!< Default destructor

    //\section overrides
    const char* Type() const override { return "BankedRegister"; }   //!< Object Type override
    Object * Clone() const override = 0; //!< Return cloned BankedRegister object.
    void Setup(const RegisterFile* registerFile) override;           //!< Setup banked register object - TODO should be pure virtual
    void HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload) override; //!< Receive Notification override

    //\section banked register functions
    virtual void UpdateBankIndex(const NotificationSender* sender, Object* payload = nullptr) = 0; //!< Overriden in derived to update bank index
  protected:
    virtual void BankNotExist(uint32 bank); //!< Report a bank doesn't exist.
    virtual uint32 OriginalBankIndex() const { return 0; } //!< Return the original bank index.
    void UpdateRegisterFields(); //!< update register fields
  protected:
    uint32 mBankIndex;                                    //!< Current bank index
    uint32 mOriginalBankIndex;                            //!< Original/Default bank index
    uint32 mOriginalSize;                                 //!< Original/Default register size
    std::map<uint64, Register*> mRegisterMap;             //!< Map of bank index to Register
    std::vector<RegisterField*> mOriginalRegisterFields;  //!< Default register fields determined by original bank index
  };

  /*! \class SelectorBankedRegister

      A type of banked register that uses a RegisterField as selector of register bank.
  */
  class SelectorBankedRegister : public BankedRegister
  {
  public:
    SelectorBankedRegister(const std::string& rRegName, uint64 fieldMask, uint32 fieldShift); //!< Constructor with parameters provided.
    ~SelectorBankedRegister() { } //!< Default destructor
    ASSIGNMENT_OPERATOR_ABSENT(SelectorBankedRegister);

    const char* Type() const override { return "SelectorBankedRegister"; }   //!< Object Type override
    void Setup(const RegisterFile* pRegisterFile) override; //!< Common setup code for SelectorBankedRegister sub classes.
    void UpdateBankIndex(const NotificationSender* pSender, Object* payload = nullptr) override; //!< Read selector for updated bank index.
  protected:
    SelectorBankedRegister (const SelectorBankedRegister& rOther);                    //!< Copy constructor
    SelectorBankedRegister(); //!< Default constructor
    void GetRelyUponRegisters(std::vector<std::string>& rRegNames) const override; //!< Return rely upon register names, here mainly the selector register.
    void BankNotExist(uint32 bank) override = 0; //!< Report a bank doesn't exist.
  protected:
    uint32 mFieldMask; //!< Mask to isolate the selector field.
    uint32 mFieldShift; //!< Number of shifts to get the value value to lsb=0.
    const std::string mSelectorRegister; //!< Name of the selector register.
    Register* mpSelectorRegister; //!< Pointer to the selector register.
  };

  /*! \class SelectorBankedRegisterRAZWI

      Sub class of SelectorBankedRegister, when the index points to invalid bank, the behavior of the register is RAZ/WI.
  */
  class SelectorBankedRegisterRAZWI : public SelectorBankedRegister
  {
  public:
    SelectorBankedRegisterRAZWI(const std::string& rRegName, uint64 fieldMask, uint32 fieldShift); //!< Constructor with parameters provided.
    ~SelectorBankedRegisterRAZWI(); //!< Default destructor
    ASSIGNMENT_OPERATOR_ABSENT(SelectorBankedRegisterRAZWI);

    const char* Type() const override { return "SelectorBankedRegisterRAZWI"; }   //!< Object Type override
    void Setup(const RegisterFile* pRegFile) override; //!< Setup SelectorBankedRegisterRAZWI.
  protected:
    SelectorBankedRegisterRAZWI(); //!< Default constructor
    SelectorBankedRegisterRAZWI (const SelectorBankedRegisterRAZWI& rOther); //!< Copy constructor
    void BankNotExist(uint32 bank) override; //!< Report a bank doesn't exist.
  protected:
    const Register* mpRazwiRegister; //!< Pointer to RAZ/WI register object.
  };

  class RegisterInitPolicy;
  class GenConditionSet;
  class RegisterReserver;

  /*! \class  RegisterFile
      \brief Container of all Register and PhysicalRegister objects and various register related lookup and accelerator mechanisms.
   */
  class RegisterFile : public Object, public NotificationSender
  {
  public:

    //\section Object_functions function implementations for Object declared methods.
    Object * Clone() const override;            //!< Clone() function as declared in Object.
    const std::string ToString() const override;   //!< ToString() function as declared in Object.
    const char* Type() const override { return "RegisterFile"; }    //!< Type() function as declared in Object.

    RegisterFile();  //!< Constructor.
    ~RegisterFile(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(RegisterFile);

    //\section Setup_functions functions in initialization/setup step.
    void LoadRegisterFiles(const std::list<std::string>& fileNames); //!< Load register files to setup RegisterFile object.
    void Setup(const GenConditionSet* pCondSet = nullptr, const std::string& unpredict_registers = ""); //!< Setup RegisterFile internals

    //\section Getter/Search_functions
    Register*         RegisterLookup               (const std::string& name) const; //!< Return Register from its name.
    virtual Register* RegisterLookupByIndex        (uint32 index, const ERegisterType reg_type, uint32 size) const { return nullptr; } //!< Return Register from its type and index.
    PhysicalRegister* PhysicalRegisterLookup       (const std::string& name) const; //!< Return Physical register from its name.
    uint64            GetRegisterFieldMask         (const std::string& reg_name, const std::vector<std::string>& field_names) const; //!< Return a bit mask of all fields from given register.

    const std::map<std::string, Register*> Registers() const { return mRegisters; }                        //!< Getter for registers map.
    const std::map<std::string, PhysicalRegister*> PhysicalRegisters() const { return mPhysicalRegisters; }    //!< Getter for physical registers map.
    const std::map<std::string, RegisterField*> GetRegisterFieldsFromMask(const std::string& reg_name, uint64 mask) const; //!< Return map of field names to RegisterField* for fields with bits inside given mask
    const std::string& Name() const;                                                          //!< Getter for mName.
    virtual void ConvertRegisterName (const std::string& name, uint32 index, std::string& new_name) const    //!< return actual register name with given register name and index
    {
      new_name = name;  //!< for base class simply return the given name only
    }

    bool GetRandomRegisters(cuint32 number, const ERegisterType regType, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const; //!< Get random registers of the specified type.
    bool GetRandomRegistersForAccess(cuint32 number, const ERegisterType regType, const ERegAttrType access, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const; //!< Get random registers of the specified type for the specified access.

    //\section value_functions functions to intialize/set value for registers/fields.
    void InitializeRegister(const std::string& rRegName, const uint64 value, const ChoicesModerator* pChoicesModerator) const;  //!< Initialization interface.  If there is no pChoiceTree passed in, using the value. otherwise ignore the value
    void InitializeRegister(const std::string& rRegName, const std::vector<uint64>& values, const ChoicesModerator* pChoicesModerator) const;  //!< Initialization interface.  If there is no pChoiceTree passed in, using the values. otherwise ignore the values and use choice tree
    void InitializeRegisterRandomly(const std::string& rRegName, const ChoicesModerator* pChoicesModerator = nullptr) const;              //!< Initialize register randomly.
    void InitializeRegisterRandomly(Register* pRegister, const ChoicesModerator* pChoicesModerator = nullptr) const;              //!< Initialize register randomly.
    virtual Register* GetContainingRegister(const Register* pReg) const { return nullptr; } //!< Return a register that contains the passed register object, if applicable.
    const RegisterField* InitializeRegisterFieldRandomly(Register* pRegister, const std::string& fieldName, const ChoicesModerator* pChoicesModerator) const; //!< Initialze a register field if not yet initialized.
    const RegisterField* InitializeRegisterField(Register* pRegister, const std::string& rFieldName, const uint64 value, const ChoicesModerator* pChoicesModerator = nullptr) const; //!< Initialize register field with value meant for the field.
    const RegisterField* InitializeRegisterFieldFullValue(Register* pRegister, const std::string& rFieldName, const uint64 value, const ChoicesModerator* pChoicesModerator = nullptr) const;  //!< Initialization register field from full logical register value .
    void SetPhysicalRegisterValueAndInit(PhysicalRegister* pPhysRegister, uint64 valueToSet, uint64 valueMask, uint64 initValue, bool notifyInit) const; //!< In certain special cases, allow this method to initialize PhysicalRegister then set value.
    void RegisterReadOnlyRegister(ReadOnlyRegister* pReadOnlyReg) const; //!< Register ReadOnlyRegisters to be batch processed.
    std::vector<ReadOnlyRegister* >& GetReadOnlyRegisters() const { return mReadOnlyRegisters; } //< Return read-only registers.
    void InitializeReadOnlyRegistersNoISS(); //!< Initialize readonly registers in the mode without ISS.
    void InitializeReadOnlyRegisterFromIss(ReadOnlyRegister* pRoReg, uint64 value) const; //!< Initialize register value from ISS.
    void RegisterReadOnlyRegisterField(ReadOnlyRegisterField* pReadOnlyRegField) const; //!< Register ReadOnlyRegister field to be batch processed.
    std::vector<ReadOnlyRegisterField* >& GetReadOnlyRegisterFields() const { return mReadOnlyRegisterFields; } //< Return read-only register fields.
    void ResetReadOnlyRegisterFieldFromIss(ReadOnlyRegisterField* pRoRegField, uint64 value) const; //!< Initialize register value from ISS.
    virtual bool AllowReExecutionInit(const std::string& rRegName) const { return false; } //!< Check if a register is allowed to be initialized in re-execution.
    const Register* RazwiRegister(uint32 size) const; //!< Return pointer to the RAZWI register with the specified size.
    const RegisterInitPolicy* GetInitPolicy(const std::string& rPolicyName) const; //!< Look up a init policy object.
    const std::map<std::string, RegisterInitPolicy* >& GetInitPolicies() const { return mInitPolicies; } //!< Return all init policies.
    const GenConditionSet* GetConditionSet() const { return mpConditionSet; } //!< Return pointer to GenConditionSete object.
    inline RegisterReserver* GetRegisterReserver() const { return mpRegisterReserver; } //!< return pointer to register reserver
  protected:
    RegisterFile(const RegisterFile& rOther); //!< Copy constructor, protected
    void AddRegister(Register* reg_ptr); //!< Add a new instance of RegisterStructure.
    void AddPhyReg(PhysicalRegister* phy_reg, const std::string& rLink); //!< Add a new instance of PhyRegStructure.
    void SetupPhysicalRegisterLinks(); //!< Set the physical register ptrs inside of the linked registers
    void CheckLinkRegisterInit(const std::string& rRegName) const; //!< Check if register being initialized has a link reg, if so send iss register init notification
    void AddInitPolicy(RegisterInitPolicy* pInitPolicy); //!< Add init policy type.
    void InitializeRelyUponRegisters(const Register* pRegister, const ChoicesModerator* pChoicesModerator) const; //!< initliaze registers relied on
    virtual void SetupRegisterReserver() { } //!< set up register reserver
    void SetupUnpredictRegister(const std::string& unpredict_registers); //!< set up unpredicatble registers
  protected:
    std::string mName;                                                    //!< Name.
    std::map<std::string, Register* > mRegisters;                         //!< Registers map.
    std::map<std::string, PhysicalRegister* > mPhysicalRegisters;         //!< Physical registers map.
    std::map<std::string, std::string > mPhysicalRegisterLinks;           //!< Physical registers map.
    std::map<std::string, RegisterInitPolicy* > mInitPolicies;            //!< Reggister init policies map.
    std::map<uint32, std::string > mRegIndex2Name; //!< RegisterType[31:28] + index[27:0] for Physical Registers, but not Registers, because registers will have clashes for the same index
    mutable std::vector<ReadOnlyRegister *> mReadOnlyRegisters;           //!< A vector of all ReadOnlyRegisters that need to be handled differently in modes with ISS and without ISS.
    mutable std::vector<ReadOnlyRegisterField *>mReadOnlyRegisterFields; //!< A vector of all read-only register fields that need to be handled differently in modes with ISS and without ISS.
    const GenConditionSet* mpConditionSet; //!< Const pointer to GenConditionSet object.
    RegisterReserver* mpRegisterReserver; //!< the pointer to register reserver
    friend class RegisterParser;
  };
}

#endif
