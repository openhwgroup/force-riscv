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
#ifndef Force_AsmText_H
#define Force_AsmText_H

#include <vector>
#include <string>
#include <Object.h>

namespace Force {

  class OperandText;
  class Instruction;

  /*!
    \class AsmText
    \brief Class to output instruction assembly text.
  */
  class AsmText {
  public:
    AsmText() : mFormat(), mResolved(false), mOperandText() {} //!< Constructor, empty.
    virtual ~AsmText();  //!< Destructor, should delete OperandText children.

    const std::string Text(const Instruction& instr) const; //!< Return assembly code string.
    virtual void AddOperandText(const std::string& op_text); //!< Add an unresolved operand text piece.
  private:
    AsmText(const AsmText& rOther); //!< Copy constructor, private
    void ResolveOperandText(const Instruction& instr) const; //!< Resolve operand text.
  public:
    std::string mFormat; //!< Assembly output format string.
    mutable bool mResolved; //!< Indicate whether the operand text components has been resolved.

  private:
    mutable std::vector<OperandText* > mOperandText;
  };

  /*!
    \class OperandText
    \brief Class to output operand assembly text.
  */
  class OperandText {
  public:
    explicit OperandText(const std::string& text) : mText(text) { } //!< Constructor with original text string parameter given.
    OperandText() : mText() {} //!< Constructor,
    virtual ~OperandText() {} //!< Destructor

    virtual const std::string Text(const Instruction& instr) const = 0; //!< Return text associated with the assembly code piece.
    const std::string& OriginalText() const { return mText; } //!< Return the original text string associated with the OperandText object.
    void SetText(const std::string& text) { mText = text; } //!< Set the mText attribute.
  protected:
    OperandText(const OperandText& rOther); //!< Copy constructor, protected
  protected:
    std::string mText; //!< The text string associated with the operand assembly piece.
  };

  /*!
    \class UnresolvedOperandText
    \brief Represent operand text object that has not be resolved to a class that can output valid assembly operand text.
  */
  class UnresolvedOperandText : public OperandText {
  public:
    explicit UnresolvedOperandText(const std::string& oprText) : OperandText(oprText) { } //!< Constructor with unresolved text parameter specified.
    ~UnresolvedOperandText() { } //!< Destructor.

    const std::string Text(const Instruction& instr) const override; //!< When called will fail out, since this is an unresolved assembly code piece.
  private:
    UnresolvedOperandText(const UnresolvedOperandText& rOther) : OperandText(rOther) { } //!< Private copy constructor, not meant to be called.
  };

  /*!
    \class LookUpOperandText
    \brief Represent operand text object that can be resolved by simply look up associated operand from instruction object.
  */
  class LookUpOperandText : public OperandText {
  public:
    explicit LookUpOperandText(const std::string& oprText) : OperandText(oprText) { } //!< Constructor with look-up text parameter specified.
    ~LookUpOperandText() { } //!< Destructor.

    const std::string Text(const Instruction& instr) const override; //!< Look up associated operand and return its assembly text.
  private:
    LookUpOperandText(const LookUpOperandText& rOther) : OperandText(rOther) { } //!< Private copy constructor, not meant to be called.
  };

  /*!
    \class OperandTextObject
    \brief Base class of various OperandText type classes that are based also on Object class.
  */
  class OperandTextObject : public OperandText, public Object {
  public:
    OperandTextObject() : OperandText(), Object() { } //!< Constructor with look-up text parameter specified.
    ~OperandTextObject() { } //!< Destructor.
    const std::string ToString() const override { return mText; } //!< Return a string describing the current state of the OperandTextObject object.
    const char* Type() const override { return "OperandTextObject"; } //!< Return a string describing the actual type of the OperandTextObject Object
  protected:
    OperandTextObject(const OperandTextObject& rOther) : OperandText(rOther), Object(rOther) { } //!< Private copy constructor, not meant to be called.
  };

  /*!
    \class AddressingOperandText
    \brief Operand text class that shows target address for AddressingOperand object.
  */
  class AddressingOperandText : public OperandTextObject {
  public:
    AddressingOperandText() : OperandTextObject() { } //!< Constructor with  Complement imm text parameter specified.
    ~AddressingOperandText() { } //!< Destructor.

    const std::string Text(const Instruction& instr) const override; //!< Look up associated operands and return assembly text.

    Object* Clone() const override { return new AddressingOperandText(*this); } //!< Return a cloned object of the same type and same contents of the object.
    const char* Type() const override { return "AddressingOperandText"; } //!< Return a string describing the actual type of the Object
  private:
    AddressingOperandText(const AddressingOperandText& rOther) : OperandTextObject(rOther) { } //!< Private copy constructor, not meant to be called.
  };

}

#endif
