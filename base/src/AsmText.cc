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
#include "AsmText.h"

#include "Instruction.h"
#include "Log.h"
#include "ObjectRegistry.h"
#include "Operand.h"

using namespace std;

static const int ASM_BUFFER_SIZE = 64;

namespace Force {

  AsmText::AsmText(const AsmText& rOther) : mFormat(), mResolved(false), mOperandText()
  {
    LOG(fail) << "AsmText type object is not to be copied." << endl;
    FAIL("object-no-copy");
  }

  AsmText::~AsmText()
  {
    for (auto opr_text : mOperandText) {
      delete opr_text;
    }
  }

  void AsmText::ResolveOperandText(const Instruction& instr) const
  {
    mResolved = true;

    const ObjectRegistry* obj_registry = ObjectRegistry::Instance();
    int vec_size = mOperandText.size();
    for (int i = 0; i < vec_size; ++ i) {
      const string& orig_text = mOperandText[i]->OriginalText();
      if (instr.FindOperand(orig_text)) {
        auto resolved_opr_text = new LookUpOperandText(orig_text);
        delete mOperandText[i];
        mOperandText[i] = resolved_opr_text;
        continue;
      }

      OperandText* lookup_opr_text = dynamic_cast<OperandText* >(obj_registry->ObjectInstanceTry(orig_text));
      if (nullptr != lookup_opr_text) {
        lookup_opr_text->SetText(orig_text);
        delete mOperandText[i];
        mOperandText[i] = lookup_opr_text;
        continue;
      }
    }
  }

  const string AsmText::Text(const Instruction& instr) const
  {
    if (not mResolved) {
      ResolveOperandText(instr);
    }

    if (mOperandText.size() == 0) return mFormat;

    char print_buffer[ASM_BUFFER_SIZE];
    int buffer_loc = 0;
    string::size_type format_loc = 0;
    const char* format_ptr = mFormat.c_str();
    for (auto opr_text : mOperandText) {
      string::size_type format_insert_loc = mFormat.find("%s", format_loc);
      string op_text_str = opr_text->Text(instr);
      if (format_insert_loc == string::npos) {
        LOG(fail) << "Found no insertion location for operand text \"" << op_text_str << "\" in instruction \"" << instr.FullName() << "\"." << endl;
        FAIL("no-insertion-location-for-operand-text");
        break;
      }
      int format_copy_size = (format_insert_loc - format_loc);
      if (buffer_loc + format_copy_size >= ASM_BUFFER_SIZE) {
        LOG(fail) << "Out of range copying format to ASM buffer: " << dec << (buffer_loc + format_copy_size) << " > " << ASM_BUFFER_SIZE << endl;
        FAIL("out-of-rarnge-copying-format");
      }
      // << "copying format " << dec << format_loc << "-" << format_insert_loc << " to " << buffer_loc << endl;
      std::copy(format_ptr + format_loc, format_ptr + format_insert_loc, print_buffer + buffer_loc);
      format_loc = format_insert_loc + 2;
      buffer_loc += format_copy_size;
      int op_text_size = op_text_str.size();
      if (buffer_loc + op_text_size >= ASM_BUFFER_SIZE) {
        LOG(fail) << "Out of range copying op-text to ASM buffer: " << dec << (buffer_loc + op_text_size) << " > " << ASM_BUFFER_SIZE << endl;
        FAIL("out-of-rarnge-copying-op-text");
      }
      // << "copying op-text " << op_text_str << " to " << buffer_loc << endl;
      std::copy(op_text_str.begin(), op_text_str.end(), print_buffer + buffer_loc);
      buffer_loc += op_text_size;
    }

    // check if there is remainder at the end of the format string, if so, copy it.
    if (format_loc < mFormat.size()) {
      string::size_type append_size = mFormat.size() - format_loc;
      const char* copy_loc = format_ptr + format_loc;
      std::copy(copy_loc, copy_loc + append_size, print_buffer + buffer_loc);
      buffer_loc += append_size;
    }

    print_buffer[buffer_loc] = '\0'; // mark end of string.
    return print_buffer;
  }

  void AsmText::AddOperandText(const std::string& op_text)
  {
    mOperandText.push_back(new UnresolvedOperandText(op_text));
  }

  OperandText::OperandText(const OperandText& rOther)
    : mText(rOther.mText)
  {
  }

  const string UnresolvedOperandText::Text(const Instruction& instr) const
  {
    LOG(fail) << "Operand text \"" << mText << "\" not yet resolved for instruction \"" << instr.FullName() << "\"." << endl;
    FAIL("unresolved-operand-text");
    return "";
  }

  const string LookUpOperandText::Text(const Instruction& instr) const
  {
    auto opr_ptr = instr.FindOperand(mText, true);

    return opr_ptr->AssemblyText();
  }

  const string AddressingOperandText::Text(const Instruction& instr) const
  {
    auto addr_opr = instr.FindOperandType<AddressingOperand>();
    if (nullptr == addr_opr) {
      LOG(fail) << "{AddressingOperand::Text} \"AddressingOperand\" type operand not found." << endl;
      FAIL("addressing-operand-not-found");
    }
    auto target_addr = addr_opr->TargetAddress();
    char print_buffer[32];

    snprintf(print_buffer, 32, "0x%llx", target_addr);
    return print_buffer;
  }

}
