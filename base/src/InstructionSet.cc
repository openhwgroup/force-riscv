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
#include <InstructionSet.h>
#include <InstructionStructure.h>
#include <Architectures.h>
#include <Config.h>
#include <XmlTreeWalker.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <UtilityFunctions.h>
#include <StringUtils.h>
#include <AsmText.h>
#include <UopUtils.h>
#include <GenException.h>
#include <Log.h>

#include <pugixml.h>

#include <string.h>

using namespace std;

/*!
  \file InstructionSet.cc
  \brief Code for InstructionSet container.
 */

namespace Force {

  /*!
    \class InstructionParser
    \brief Parser class for instruction files.

    InstructionParser inherits pugi::xml_tree_walker.  This parser parse various instruction files.
  */
  class InstructionParser : public pugi::xml_tree_walker {
  public:
    /*!
      Constructor, pass in pointer to InstructionSet object.
    */
    explicit InstructionParser(InstructionSet* instr_set)
      : mpInstructionSet(instr_set), mpInstructionStructure(nullptr), mOperandDepth(0), mpOperandStructure(nullptr), mpGroupOperandStructure(nullptr), mpAsmText(nullptr)
    {
    }
    ~InstructionParser() //!< Destructor, check if there is dangling pointers.
    {
      if ((nullptr != mpInstructionStructure) || (nullptr != mpOperandStructure) || (nullptr != mpGroupOperandStructure) || (nullptr != mpAsmText)) {
        LOG(fail) << "Unexpected dangling pointer: mpInstructionStructure? " << (nullptr != mpInstructionStructure) << ", mpOperandStructure? " << (nullptr != mpOperandStructure) << ", mpGroupOperandStructure? " << (nullptr != mpGroupOperandStructure) << ", mpAsmText? " << (nullptr != mpAsmText) << endl;
        FAIL("unexpected-dangling-pointer");
      }
    }

    ASSIGNMENT_OPERATOR_ABSENT(InstructionParser);
    COPY_CONSTRUCTOR_DEFAULT(InstructionParser);
    /*!
      Handles instruction file elements.
     */
    virtual bool for_each(pugi::xml_node& node)
    {
      const char * node_name = node.name();
      if (strcmp(node_name, "instruction_file") == 0) {
        // known node names.
      }
      else if (strcmp(node_name, "I") == 0) {
        process_instruction_attributes(node);
      }
      else if (strcmp(node_name, "O") == 0) {
        process_operand_attributes(node);
      }
      else if (strcmp(node_name, "asm") == 0) {
        process_asm_attributes(node);
      }
      else {
        LOG(fail) << "Unknown instruction file node name \'" << node_name << "\'." << endl;
        FAIL("instruction-unknown-node");
      }

      return true; // continue traversal
    }

    /*!
      Implement end function to add the last InstructionStructure object.
    */
    virtual bool end(pugi::xml_node& node)
    {
      commit_instruction_structure();
      return true;
    }

    /*!
      Process attributes of \<I\> element.
     */
    void process_instruction_attributes(pugi::xml_node& node)
    {
      commit_instruction_structure();

      mpInstructionStructure = new InstructionStructure(msInstructionClass);

      for (pugi::xml_attribute const& attr: node.attributes()) {
        const char* attr_name = attr.name();
        try {
          if (strcmp(attr_name, "name") == 0) mpInstructionStructure->mName = attr.value();
          else if (strcmp(attr_name, "form") == 0) mpInstructionStructure->mForm = attr.value();
          else if (strcmp(attr_name, "isa") == 0) mpInstructionStructure->mIsa = attr.value();
          else if (strcmp(attr_name, "group") == 0) {
            mpInstructionStructure->mGroup = string_to_EInstructionGroupType(attr.value());
          }
          else if (strcmp(attr_name, "class") == 0) mpInstructionStructure->mClass = attr.value();
          else if (strcmp(attr_name, "aliasing") == 0) mpInstructionStructure->mAliasing = attr.value();
          else if (strcmp(attr_name, "extension") == 0) {
            mpInstructionStructure->mExtension = string_to_EInstructionExtensionType(attr.value());
          }
          else {
            LOG(fail) << "Unknown instruction attribute \'" << attr_name << endl;
            FAIL("unknown-instruction-attribute");
          }
        }
        catch (const EnumTypeError& rError) {
          LOG(fail) << "{process_instruction_attributes} unexpected value \"" << attr.value() << "\" for attribute \"" << attr_name << "\" for instruction: " << mpInstructionStructure->FullName() << endl;
          FAIL("unhandled-instruction-attribute-value-type");
        }
      }
    }

    /*!
      Commit last InstructionStructure object if there is any.
    */
    void commit_instruction_structure()
    {
      if (nullptr != mpInstructionStructure) {
        if (nullptr != mpAsmText) {
          if (nullptr != mpInstructionStructure->mpAsmText) {
            LOG(fail) << "InstructionStructure object already has AsmText assigned." << endl;
            FAIL("double-assign-AsmText");
          }
          mpInstructionStructure->mpAsmText = mpAsmText;
          mpAsmText = nullptr;
        }
        if (mpInstructionStructure->mElementSize == 0) {
          mpInstructionStructure->mElementSize = mpInstructionStructure->mSize / 8;
        }
        mpInstructionSet->AddInstruction(mpInstructionStructure);
        mpInstructionStructure = nullptr;
      }
    }

    /*!
      Set instruction const value
    */
    bool set_const_value(OperandStructure* op_struct, const char* value_str)
    {
      const char first_char = value_str[0];
      if (first_char == '!' || first_char == 'x') return false; //!< TODO skip the not-equal type values

      uint32 const_value = parse_bin32(value_str);
      uint32 exp_const_value = op_struct->Encoding(const_value);
      mpInstructionStructure->SetConstantValue(exp_const_value, op_struct->mSize);
      delete op_struct;
      if (op_struct == mpOperandStructure) {
        mpOperandStructure = nullptr;
      } else {
        LOG(fail) << "{set_const_value} expecting const to be non-group operand." << endl;
        FAIL("operand-structure-pointer-issue");
      }
      return true;
    }

    /*!
      Process an attribute of a DataProcessing operand.
    */
    void process_data_processing_operand_attribute(const char* attr_name, const char* attr_value, DataProcessingOperandStructure* data_proc_op_struct)
    {
      if (strcmp(attr_name, "data-proc-op") == 0) {
        data_proc_op_struct->SetOperationType(string_to_EDataProcessingOperationType(attr_value));
      }
      if (strcmp(attr_name, "uop") == 0) {
        data_proc_op_struct->SetUop(string_to_EUop(attr_value));
      }
      else {
        data_proc_op_struct->AddOperandRole(attr_value, attr_name);
      }
    }

    /*!
      Process the attribute of a SystemOp operand.
    */

    void process_systemop_operand_attribute(const string& attr_name, const string& attr_value, SystemOpOperandStructure* system_op_struct)
    {
      if (attr_value.find("AT") != string::npos) {
        system_op_struct->SetAlignment(1);
        system_op_struct->SetDataSize(8);
        system_op_struct->SetElementSize(8);
        system_op_struct->SetMemAccessType((attr_value.find("W") != string::npos) ? EMemAccessType::Write : EMemAccessType::Read);
      }
    }

    /*!
      Process the differ attribute of an operand.
    */
    void process_differ_attribute(const string& attr_value, OperandStructure* op_struct)
    {
      StringSplitter string_splitter(attr_value, ',');
      while (not string_splitter.EndOfString()) {
        op_struct->AddDiffer(string_splitter.NextSubString());
      }
    }

    /*!
      Process attributes of \<O\> element.  Make sure not to use the mpOperandStructure pointer here, since get_operand_structure could return either mpOperandStructure or mpGroupOperandStructure.
    */
    void process_operand_attributes(pugi::xml_node& node)
    {
      commit_operand_structure();

      OperandStructure* op_struct = get_operand_structure(node);
      try {
        for (pugi::xml_attribute const& attr: node.attributes()) {
          const char* attr_name = attr.name();
          if (strcmp(attr_name, "name") == 0) {
            op_struct->mName = attr.value();
            op_struct->mShortName =mpInstructionSet->mpArchInfo->FindOperandShortName(op_struct->mName);
          }
          else if (strcmp(attr_name, "type") == 0) {
            op_struct->mType = string_to_EOperandType(attr.value());
          }
          else if (strcmp(attr_name, "bits") == 0) op_struct->SetBits(attr.value());
          else if (strcmp(attr_name, "class") == 0) op_struct->mClass = attr.value();
          else if (strcmp(attr_name, "value") == 0) {
            if (set_const_value(op_struct, attr.value())) return;
          }
          else if (strcmp(attr_name, "access") == 0) {
            op_struct->mAccess = string_to_ERegAttrType(attr.value());
          }
          else if (strcmp(attr_name, "slave") == 0) {
            op_struct->mSlave = parse_bool(attr.value());
          }
          else if (strcmp(attr_name, "uop-param-type") == 0) {
            op_struct->mUopParamType = string_to_EUopParameterType(attr.value());
          }
          else if ((strcmp(attr_name, "choices") == 0) || (strcmp(attr_name, "choices2") == 0)) {
            auto cast_struct = dynamic_cast<ChoicesOperandStructure* > (op_struct);
            cast_struct->mChoices.push_back(attr.value());
          }
          else if (strcmp(attr_name, "differ") == 0) {
            process_differ_attribute(attr.value(), op_struct);
          }
          else if (strcmp(attr_name, "exclude") == 0) {
            auto cast_struct = dynamic_cast<ExcludeOperandStructure* > (op_struct);
            cast_struct->mExclude = attr.value();
          }
          else if (strcmp(attr_name, "base") == 0) {
            auto cast_struct = dynamic_cast<AddressingOperandStructure*> (op_struct);
            cast_struct->SetBase(attr.value());
          }
          else if (strcmp(attr_name, "element-size") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetElementSize(parse_uint32(attr.value()));
          }
          else if (strcmp(attr_name, "offset") == 0) {
            auto cast_struct = dynamic_cast<AddressingOperandStructure*> (op_struct);
            cast_struct->SetOffset(attr.value());
          }
          else if (strcmp(attr_name, "index") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetIndex(attr.value());
          }
          else if (strcmp(attr_name, "pre-index") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetPreIndex(attr.value());
          }
          else if (strcmp(attr_name, "post-index") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetPostIndex(attr.value());
          }
          else if (strcmp(attr_name, "data-size") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetDataSize(parse_uint32(attr.value()));
          }
          else if (strcmp(attr_name, "alignment") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetAlignment(parse_uint32(attr.value()));
          }
          else if (strcmp(attr_name, "sign-extension") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetSignExtension(parse_uint32(attr.value()));
          }
          else if (strcmp(attr_name, "offset-scale") == 0) {
            auto cast_struct = dynamic_cast<AddressingOperandStructure*> (op_struct);
            cast_struct->SetOffsetScale(parse_uint32(attr.value()));
          }
          else if (strcmp(attr_name, "unprivileged") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetUnprivileged(parse_uint32(attr.value()));
          }
          else if (strcmp(attr_name, "mem-access") == 0) {
            auto cast_struct = dynamic_cast<AddressingOperandStructure*> (op_struct);
            cast_struct->SetMemAccessType(string_to_EMemAccessType(attr.value()));
          }
          else if (strcmp(attr_name, "lorder") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetLoadType(string_to_EMemOrderingType(attr.value()));
          }
          else if (strcmp(attr_name, "sorder") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetStoreType(string_to_EMemOrderingType(attr.value()));
          }
          else if (strcmp(attr_name, "extend-amount") == 0) {
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetExtendAmount(attr.value());
          }
          else if (strcmp(attr_name, "offs-size") == 0){
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetOffsSize(parse_uint32(attr.value()));
          }
          else if (strcmp(attr_name, "predicate-reg") == 0){
            auto cast_struct = dynamic_cast<LoadStoreOperandStructure*> (op_struct);
            cast_struct->SetPredicateReg(attr.value());
          }
          else if (strcmp(attr_name, "condition") == 0) {
            auto cast_struct = dynamic_cast<BranchOperandStructure*> (op_struct);
            cast_struct->mCondition = string_to_EBranchConditionType(attr.value());
          }
          else if (strcmp(attr_name, "pointer") == 0) {
            auto cast_struct = dynamic_cast<AuthOperandStructure*> (op_struct);
            cast_struct->SetPointer(attr.value());
          }
          else if (strcmp(attr_name, "modifier") == 0) {
            auto cast_struct = dynamic_cast<AuthOperandStructure*> (op_struct);
            cast_struct->SetModifier(attr.value());
          }
          else if (strcmp(attr_name, "key") == 0) {
            auto cast_struct = dynamic_cast<AuthOperandStructure*> (op_struct);
            cast_struct->SetKey(attr.value());
          }
          else if (strcmp(attr_name, "address-based") == 0) {
            auto cast_struct = dynamic_cast<SystemOpOperandStructure*> (op_struct);
            cast_struct->SetAddressBased(parse_bool(attr.value()));
          }
          else if (strcmp(attr_name, "sysop-type") == 0) {
            auto cast_struct = dynamic_cast<SystemOpOperandStructure*> (op_struct);
            cast_struct->SetSystemOpType(string_to_ESystemOpType(attr.value()));
            process_systemop_operand_attribute(attr_name, attr.value(), cast_struct);
          }
          else if (strcmp(attr_name, "offset-shift") == 0) {
            auto cast_struct = dynamic_cast<AluOperandStructure*> (op_struct);
            cast_struct->SetOffsetShift(attr.value());
          }
          else if (strcmp(attr_name, "imm") == 0) {
            auto cast_struct = dynamic_cast<AluOperandStructure*> (op_struct);
            cast_struct->SetImmediate(attr.value());
          }
          else if (strcmp(attr_name, "alu-op") == 0) {
            auto cast_struct = dynamic_cast<AluOperandStructure*> (op_struct);
            cast_struct->SetOperationType(string_to_EAluOperationType(attr.value()));
          }
          else if (strcmp(attr_name, "layout-multiple") == 0) {
            auto cast_struct = dynamic_cast<VectorRegisterOperandStructure*> (op_struct);
            cast_struct->SetLayoutMultiple(parse_uint32(attr.value()));
          }
          else if (strcmp(attr_name, "reg-count") == 0) {
            auto cast_struct = dynamic_cast<VectorLayoutOperandStructure*> (op_struct);
            cast_struct->SetRegisterCount(parse_uint32(attr.value()));
          }

          // TODO(Noah): Devise and implement a better way of processing attributes generically when there is time to
          // do so. For now, keep the DataProcessing attribute processing at the end.
          else if (op_struct->Type() == EOperandType::DataProcessing) {
            auto cast_struct = dynamic_cast<DataProcessingOperandStructure*> (op_struct);
            process_data_processing_operand_attribute(attr_name, attr.value(), cast_struct);
          }
          else {
            LOG(fail) << "Unknown operand attribute \'" << attr_name << endl;
            FAIL("unknown-operand-attribute");
          }
        }
      }
      catch (const EnumTypeError& enum_error) {
        LOG(fail) << "{process operand attributes} failed to handle attribute. Error is \"" << enum_error.what() <<"\"" << endl;
        FAIL("failed-process-operand-attributes");
      }

      if (op_struct->mClass.size() == 0) {
        set_default_operand_class(op_struct);
      }
    }

    /*!
      Commit last OperandStructure object if there is any.
    */
    void commit_operand_structure()
    {
      if (nullptr != mpOperandStructure) {
        if (mOperandDepth > 2) {
          mpGroupOperandStructure->AddOperand(mpOperandStructure);
        }
        else {
          commit_group_operand_structure();
          mpInstructionStructure->AddOperand(mpOperandStructure);
        }
        mpOperandStructure = nullptr;
      }
    }

    /*!
      Commit last GroupOperandStructure object if there is any.
    */
    void commit_group_operand_structure()
    {
      if (nullptr != mpGroupOperandStructure) {
        mpInstructionStructure->AddOperand(mpGroupOperandStructure);
        mpGroupOperandStructure = nullptr;
      }
    }

    /*!
      Return an OperandStructure sub class type depending on the operand type given.
    */
    OperandStructure* get_operand_structure(pugi::xml_node& node)
    {
      mOperandDepth = depth();
      const char* opr_type = node.attribute("type").value();
      if (strcmp(opr_type, "Branch") == 0) {
        mpGroupOperandStructure = new BranchOperandStructure();
        return mpGroupOperandStructure;
      }
      else if (strcmp(opr_type, "LoadStore") == 0) {
        mpGroupOperandStructure = new LoadStoreOperandStructure();
        return mpGroupOperandStructure;
      }
      else if (strcmp(opr_type, "AuthBranch") == 0) {
        mpGroupOperandStructure = new AuthBranchOperandStructure();
        return mpGroupOperandStructure;
      }
      else if (strcmp(opr_type, "AuthLoadStore") == 0) {
        mpGroupOperandStructure = new AuthLoadStoreOperandStructure();
        return mpGroupOperandStructure;
      }
      else if (strcmp(opr_type, "SystemOp") == 0) {
        mpGroupOperandStructure = new SystemOpOperandStructure();
        return mpGroupOperandStructure;
      }
      else if (strcmp(opr_type, "ALU") == 0) {
        mpGroupOperandStructure = new AluOperandStructure();
        return mpGroupOperandStructure;
      }
      else if (strcmp(opr_type, "DataProcessing") == 0) {
        mpGroupOperandStructure = new DataProcessingOperandStructure();
        return mpGroupOperandStructure;
      }
      else if (strcmp(opr_type, "VECREG") == 0) {
        mpOperandStructure = new VectorRegisterOperandStructure();
        return mpOperandStructure;
      }
      else if (strcmp(opr_type, "VectorLayout") == 0) {
        mpOperandStructure = new VectorLayoutOperandStructure();
        return mpOperandStructure;
      }

      auto const choices_attr = node.attribute("choices");
      if (not choices_attr.empty()) {
        mpOperandStructure = new ChoicesOperandStructure();
        return mpOperandStructure;
      }
      auto const exclude_attr = node.attribute("exclude");
      if (not exclude_attr.empty()) {
        mpOperandStructure = new ExcludeOperandStructure();
        return mpOperandStructure;
      }
      mpOperandStructure = new OperandStructure();
      return mpOperandStructure;
    }

    void set_default_operand_class(OperandStructure* oprStructure)
    {
      EOperandType ot = oprStructure->mType;
      oprStructure->mClass = msOperandClasses[int(ot)];
    }

    /*!
      Process asm portion in instructions file.
     */
    void process_asm_attributes(pugi::xml_node& node)
    {
      commit_operand_structure();
      commit_group_operand_structure();

      mpAsmText = mpInstructionSet->AsmTextInstance();

      uint32 op_index = 0;
      for (pugi::xml_attribute const& attr: node.attributes()) {
        const char* attr_name = attr.name();
        if (strcmp(attr_name, "format") == 0) mpAsmText->mFormat = attr.value();
        else if (strncmp(attr_name, "op", 2) == 0) {
          const char* op_index_char = attr_name + 2;
          uint32 new_op_index = parse_uint32(op_index_char);
          if (new_op_index != (op_index + 1)) {
            LOG(fail) << "Asm ops in incorrect order." << endl;
            FAIL("asm-ops-wrong-order");
          }
          mpAsmText->AddOperandText(attr.value());
          ++ op_index;
        }
        else {
          LOG(fail) << "Unknown asm attribute \'" << attr_name << endl;
          FAIL("unknown-asm-attribute");
        }
      }
    }

  private:
    InstructionSet* mpInstructionSet; //!< Pointer to InstructionSet object.
    InstructionStructure* mpInstructionStructure; //!< Pointer to the current InstructionStructure object being worked on.
    uint32 mOperandDepth; //!< Depth of the current operand.
    OperandStructure* mpOperandStructure; //!< Pointer to the current OperandStructure object being worked on.
    GroupOperandStructure* mpGroupOperandStructure; //!< Pointer to the current GroupOperandStructure object being worked on.
    AsmText* mpAsmText; //!< Pointer to the current AsmText object being worked on.

    static std::string msInstructionClass;     //!< Default instruction class
    static std::vector<std::string> msOperandClasses; //!< Default operand classes

    friend class InstructionSet;
  };

  string InstructionParser::msInstructionClass;
  vector<string> InstructionParser::msOperandClasses;

  /*!
    \class InstructionSet

    Contains all InstructionStructure object representing instructions in the ISA.
    Provide ways to look up InstructionStructure and create Instruction object out of the InstructionStructure object.
   */
  InstructionSet::InstructionSet() : mInstructionSet(), mpArchInfo(nullptr)
  {

  }

  InstructionSet::~InstructionSet()
  {
    for (auto & map_item : mInstructionSet) {
      delete map_item.second;
    }
  }

  void InstructionSet::Setup(const ArchInfo& archInfo)
  {
    mpArchInfo = &archInfo;
    InstructionParser::msInstructionClass = archInfo.DefaultInstructionClass();
    check_enum_size(EOperandTypeSize);
    InstructionParser::msOperandClasses = vector<string> (EOperandTypeSize);
    for (EOperandTypeBaseType i = 0; i < EOperandTypeSize; ++ i) {
      EOperandType ot = EOperandType(i);
      auto ot_name = EOperandType_to_string(ot);
      auto oc_name = archInfo.DefaultOperandClass(ot_name);
      InstructionParser::msOperandClasses[int(i)] = oc_name;
    }

    auto cfg_ptr = Config::Instance();
    const list<string>& instr_files = archInfo.InstructionFiles();
    for (auto const& ifile_name : instr_files) {
      InstructionParser instr_parser(this);
      string full_file_path = cfg_ptr->LookUpFile(ifile_name);
      parse_xml_file(full_file_path, "instruction", instr_parser);
    }
  }

  void InstructionSet::AddInstruction(InstructionStructure* instr_struct)
  {
    string ifull_name = instr_struct->FullName();
    LOG(trace) << "Adding new instruction " << ifull_name << endl;

    const auto find_iter = mInstructionSet.find(ifull_name);
    if (find_iter != mInstructionSet.end()) {
      LOG(fail) << "Duplicated instruction full name \'" << ifull_name << "\'." << endl;
      FAIL("duplicated-instruction-full-name");
    } else {
      mInstructionSet[ifull_name] = instr_struct;
    }
  }

  const InstructionStructure* InstructionSet::LookUpById(const std::string& instrName) const
  {
    auto find_iter = mInstructionSet.find(instrName);
    if (find_iter == mInstructionSet.end()) {
      LOG(fail) << "Instruction with ID \"" << instrName << "\" not found." << endl;
      FAIL("instruction-look-up-by-id-fail");
    }

    return find_iter->second;
  }

  void InstructionSet::Dump() const
  {
      for (auto & map_item : mInstructionSet) {
          LOG(notice) << map_item.second->ToString() << endl;
      }
  }

  AsmText* InstructionSet::AsmTextInstance() const
  {
    return new AsmText();
  }

}
