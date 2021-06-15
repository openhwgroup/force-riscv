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
#include <PyInterface.h>
#include <Config.h>
#include <PathUtils.h>
#include <StringUtils.h>
#include <Scheduler.h>
#include <GenRequest.h>
#include <GenQuery.h>
#include <InstructionStructure.h>
#include <Constraint.h>
#include <ThreadGroup.h>
#include <ThreadGroupPartitioner.h>
#include <Log.h>
#include <Constraint.h>

#include <pybind11/eval.h>
#include <pybind11/stl.h>

#include <algorithm>

using namespace std;

namespace Force {

  py::object PyInterface::LoadTestTemplate(const std::string& templatePath, py::object& globals)
  {
    string module_name = get_file_stem(templatePath);
    py::dict locals;
    locals["module_name"] = py::cast(module_name); // have to cast the std::string first

    py::eval<py::eval_statements>(            // tell eval we're passing multiple statements
        "import importlib\n"
        "template_module = importlib.import_module(module_name)\n",
        globals,
        locals);

    return locals["template_module"];
  }

  /*!
    Add necessary module import searching paths, that include the current directory of the test template, and the Force/py path.
   */
  void PyInterface::SetupModulePaths(const std::string& templatePath)
  {
    string parent_path = get_parent_path(templatePath);
    PyObject* sys_path = PySys_GetObject("path");
    PyObject* template_path_py = PyUnicode_FromString(parent_path.c_str());
    PyList_Insert(sys_path, 0, template_path_py);
    mLibPath = Config::Instance()->MainPath() + "/py";
    PyObject* lib_path_py = PyUnicode_FromString(mLibPath.c_str());
    PyList_Insert(sys_path, 1, lib_path_py);

    // decrement reference to temporary objects created here
    Py_DECREF(template_path_py);
    Py_DECREF(lib_path_py);
  }

  string PyInterface::GetLibModuleName(const std::string& inFilePath)
  {
    if (inFilePath.compare(0, mLibPath.size(), mLibPath) != 0) {
      LOG(fail) << "{PyInterface::GetLibModuleName} expect file \"" << inFilePath << "\" to be located in Py lib path: \"" << mLibPath << "\"." << endl;
      FAIL("file-not-in-lib-path");
    }
    string module_name = inFilePath.substr(mLibPath.size() + 1);
    for (string::iterator s_iter = module_name.begin(); s_iter != module_name.end(); ++ s_iter) {
      if ((*s_iter) == '/') {
        (*s_iter) = '.';
      }
    }
    string::size_type str_len = module_name.size();
    if ((str_len > 3) && (module_name.substr(str_len - 3) == ".py")) {
      module_name = module_name.substr(0, str_len - 3);
    }
    return module_name;
  }

  void PyInterface::GenerateTemplate(py::object& templateObj)
  {
    Config* cfg_ptr = Config::Instance();
    string memfile_module = GetLibModuleName(cfg_ptr->LookUpFile(cfg_ptr->MemoryFile()));
    auto mod_file = cfg_ptr->ChoicesModificationFile();
    string modfile_module;
    if (mod_file != "")
      modfile_module = GetLibModuleName(cfg_ptr->LookUpFile(mod_file));

    py::object env_class_obj = templateObj.attr("EnvClass");
    mEnvObject = env_class_obj(*this);
    mTemplateObject = templateObj;
    mEnvObject.attr("defaultGenClass") = templateObj.attr("GenThreadClass");
    mEnvObject.attr("defaultSeqClass") = templateObj.attr("MainSequenceClass");

    py::object global_init_seq_class = py::none();
    if (py::hasattr(templateObj, "GlobalInitialSequenceClass")) {
      global_init_seq_class = templateObj.attr("GlobalInitialSequenceClass");
    }
    mEnvObject.attr("addInitialSequence")(global_init_seq_class);

    // check if there is GenThreadInitialization variable defined.
    if (py::hasattr(templateObj, "GenThreadInitialization")) {
      py::object gen_thread_init = templateObj.attr("GenThreadInitialization");
      mEnvObject.attr("genThreadInitFunc") = gen_thread_init;
    }
    mEnvObject.attr("configureMemory")(memfile_module);
    mpScheduler->ConfigureMemoryBanks();
    mEnvObject.attr("setup")();

    if (modfile_module != "")
      mEnvObject.attr("configureChoicesModifier")(modfile_module);

    mEnvObject.attr("generate")();

    mTemplateObject = py::none();
    mEnvObject = py::none();
  }

  void PyInterface::RunTest()
  {
    try {
      std::string template_path = Config::Instance()->TestTemplate();
      SetupModulePaths(template_path);
      py::module main = py::module::import("__main__");
      py::object globals = main.attr("__dict__");
      py::object template_obj = LoadTestTemplate(template_path, globals);
      GenerateTemplate(template_obj);
    }
    catch (const py::error_already_set& pye) {
      LOG(fail) << "Error running template: " << pye.what() << endl;
      FAIL("template-error");
    }
  }

  uint32 PyInterface::CallBackTemplate(uint32 threadId, ECallBackTemplateType callBackType, const std::string& primaryValue, const std::map<std::string, uint64>& callBackValues)
  {
    LOG(notice) << "Call back entering [" << ECallBackTemplateType_to_string(callBackType) << "] gen(" << hex << threadId << ")." << endl;
    switch (callBackType) {
    case ECallBackTemplateType::SetBntSeq:
      {
        if (py::hasattr(mTemplateObject, primaryValue.c_str()))
          mEnvObject.attr("setSequenceClass")(threadId, py::int_(0), mTemplateObject.attr(py::cast(primaryValue)));
        else {
          Config* cfg_ptr = Config::Instance();
          string bnt_name = GetLibModuleName(cfg_ptr->LookUpFile(cfg_ptr->BntFile()));
          py::module bnt_module = py::module::import(bnt_name.c_str());
          mEnvObject.attr("setSequenceClass")(threadId, py::int_(0), bnt_module.attr(py::cast("DefaultBntSequence")));
        }
        break;
      }
    case ECallBackTemplateType::RunBntSeq:
      {
        py::dict param_dict;
        for (auto &item : callBackValues) {
          param_dict[py::cast(item.first)] = py::int_(item.second);
        }
        mEnvObject.attr("runSequence")(threadId, py::int_(0), primaryValue, param_dict);
        break;
      }
    case ECallBackTemplateType::SetEretPreambleSeq:
      {
        if (py::hasattr(mTemplateObject, primaryValue.c_str()))
          mEnvObject.attr("setSequenceClass")(threadId, py::int_(1), mTemplateObject.attr(py::cast(primaryValue)));
        else
          mEnvObject.attr("setSequenceClass")(threadId, py::int_(1), py::none()); // default sequence
        break;
      }
    case ECallBackTemplateType::RunEretPreambleSeq:
      {
        py::dict param_dict;
        for (auto &item : callBackValues) {
          param_dict[py::cast(item.first)] = py::int_(item.second);
        }
        mEnvObject.attr("runSequence")(threadId, py::int_(1), primaryValue, param_dict);
        break;
      }
    default:
      LOG(fail) << "Unknown call back template type" << ECallBackTemplateType_to_string(callBackType) << endl;
        FAIL("unknown call back tempate type");
    }
    LOG(notice) << "Call back exiting [" << ECallBackTemplateType_to_string(callBackType) << "] gen(" << hex << threadId << ")." << endl;
    return 0;
  }

  //======================================================================================
  // API code
  //======================================================================================

  uint32 PyInterface::NumberOfChips() const
  {
    return mpScheduler->NumberOfChips();
  }

  uint32 PyInterface::NumberOfCores() const
  {
    return mpScheduler->NumberOfCores();
  }

  uint32 PyInterface::NumberOfThreads() const
  {
    return mpScheduler->NumberOfThreads();
  }

  uint32 PyInterface::CreateGeneratorThread(uint32 iThread, uint32 iCore, uint32 iChip)
  {
    return mpScheduler->CreateGeneratorThread(iThread, iCore, iChip);
  }

  static inline uint64 cast_py_int(const py::handle& rIntObj)
  {
    uint64 cast_value = 0;
    try {
      cast_value = rIntObj.cast<uint64>();
    }
    catch(const py::cast_error& rCastErr) {
      cast_value = uint64(rIntObj.cast<int64>());
    }

    return cast_value;
  }

  /*!
    Template function to process Transaction details, primarily for GenRequest and GenQuery based objects.
  */
  template <class Transaction>
  static void process_transaction_parameters(const py::dict& parms, Transaction* trans)
  {
    for (const auto & dict_pair : parms) {
      string key = dict_pair.first.cast<string>();
      const py::handle& value_obj = dict_pair.second;
      if (py::isinstance<py::str>(value_obj)) {
        string value_str = value_obj.cast<string>();
        trans->AddDetail(key, value_str);
      } else if (py::isinstance<py::int_>(value_obj)) {
        //uint64 value = value_obj.cast<uint64>();
        uint64 value = cast_py_int(value_obj);
        trans->AddDetail(key, value);
      } else {
        LOG(fail) << "not handled key " << key << " value " << value_obj << endl;
        FAIL("not-handled key");
      }

    }

  }

  /*!
    Template function to process Transaction detail, primarily for GenRequest and GenQuery based objects.
  */
  template <class Transaction>
  static void process_primary_parameter(const py::object& paramValue, Transaction* trans)
  {
    if (py::isinstance<py::str>(paramValue)) {
      string value_str = paramValue.cast<string>();
      trans->SetPrimaryString(value_str);
    } else if (py::isinstance<py::int_>(paramValue)) {
      uint64 value = paramValue.cast<uint64>();
      trans->SetPrimaryValue(value);
    } else {
      // << "not handled param value type" << paramValue << endl;
    }
  }

  py::object PyInterface::GenInstruction(uint32 threadId, const std::string& instrName, const py::dict& parms)
  {
    GenInstructionRequest * new_instr_req = new GenInstructionRequest(instrName);
    process_transaction_parameters<GenRequest>(parms, new_instr_req);
    std::string rec_id;
    mpScheduler->GenInstruction(threadId, new_instr_req, rec_id);
    py::str ret_str(rec_id);
    return ret_str;
  }

  static void process_meta_requests(const py::dict& metaParams,  const std::map<const std::string, const OperandStructure* >& operandStructs, GenInstructionRequest* request)
  {
    py::dict parms;
    for (const auto & meta_pair : metaParams) {
      string meta_key = meta_pair.first.cast<string>();
      bool convert_okay = false;
      try_string_to_EInstrBoolAttrType(meta_key, convert_okay);
      if (!convert_okay)
        try_string_to_EInstrConstraintAttrType(meta_key, convert_okay);
      if (convert_okay) {
        parms[py::str(meta_key)] = meta_pair.second;
        continue;
      }

      // handle operand and operand data request
      string key;
      size_t data_req = meta_key.find(".Data");
      if (data_req != string::npos)
        meta_key = meta_key.substr(0, data_req);
      auto it = operandStructs.find(meta_key);
      if (it == operandStructs.end()) {
        LOG(info) << "Not find meta key: \"" << meta_key << "\" when generating meta instruction: \"" << request->InstructionId() << "\", skipped" << endl;
        continue;
      }
      else
        key = it->second->Name();

      if (data_req != string::npos)
        key =+ ".Data";

      // cut some overflow immediate value
      if (meta_key == "imm" || meta_key == "hw" || meta_key == "sh"
          || meta_key == "lsb"  || meta_key == "fbits" || meta_key == "index"
          || meta_key == "amount" || meta_key == "T") {
        uint32 imm_val = 0;
        const py::handle& value_obj = meta_pair.second;
        if (py::isinstance<py::str>(value_obj)) {
          string value_str = value_obj.cast<string>();
          imm_val = parse_uint32(value_str);
        }
        else if (py::isinstance<py::int_>(value_obj)) {
          imm_val = value_obj.cast<int32>();
        }
        else {
          LOG(fail) << "not handled key " << meta_key << " value " << value_obj << endl;
          FAIL("not-handled key");
        }
        imm_val &= ((1u << it->second->Size()) - 1);
        parms[py::str(key)] = py::int_(imm_val);
        //<< "imm request: 0x" << hex << imm_val << endl;
      }
      else
        parms[py::str(key)] = meta_pair.second;
    }

    process_transaction_parameters<GenRequest>(parms, request);
  }

  py::object PyInterface::GenMetaInstruction(uint32 threadId, const std::string& instrName, const py::dict& metaParms)
  {
    GenInstructionRequest * new_instr_req = new GenInstructionRequest(instrName);
    auto instr_struct = mpScheduler->GetInstructionStructure(threadId, instrName);
    auto opr_structs = instr_struct->GetShortOperandStructures();
    process_meta_requests(metaParms, opr_structs, new_instr_req);
    std::string rec_id;
    mpScheduler->GenInstruction(threadId, new_instr_req, rec_id);
    py::str ret_str(rec_id);
    return ret_str;
  }

  void PyInterface::InitializeMemory(uint32 threadId, uint64 addr, uint32 bank, uint32 size, uint64 data, bool isInstr, bool isVirtual)
  {
    mpScheduler->InitializeMemory(threadId, addr, bank, size, data, isInstr, isVirtual);
  }

  static void process_AddChoicesModification_parameters(const py::dict& params, std::map<std::string, uint32>& modifications)
  {
     for (const auto & dict_pair : params) {
      string key = dict_pair.first.cast<string>();
      uint32 value = dict_pair.second.cast<uint32>();
      if (modifications.find(key) != modifications.end()) {
        LOG(fail) << "Add duplicated choice modification with name " << key << endl;
        FAIL("Add duplicated choice modification");
      }
      modifications[key] = value;
    }
  }

  py::object PyInterface::AddChoicesModification(uint32 threadId, const py::object& choicesType, const std::string& treeName, const py::dict& params, bool globalModification)
  {
    uint64 choices_type  = choicesType.cast<uint64>();
    std::map<std::string, uint32> modifications;
    process_AddChoicesModification_parameters(params, modifications);
    uint32 setId;
    if (globalModification)
      setId = mpScheduler->DoChoicesModification(threadId, (EChoicesType)choices_type, treeName, modifications);
    else
      setId = mpScheduler->AddChoicesModification(threadId, (EChoicesType)choices_type, treeName, modifications);

    py::int_ ret_int(setId);
    return ret_int;
  }

  void PyInterface::CommitModificationSet(uint32 threadId, const py::object& choicesType, const py::object& setId)
  {
    uint64 choices_type  = choicesType.cast<uint64>();
    uint32 set_id = setId.cast<uint32>();
    mpScheduler->CommitModificationSet(threadId, (EChoicesType)choices_type, set_id);
  }

  void PyInterface::RevertModificationSet(uint32 threadId, const py::object& choicesType, const py::object& setId)
  {
    uint64 choices_type  = choicesType.cast<uint64>();
    uint32 set_id = setId.cast<uint32>();
    mpScheduler->RevertModificationSet(threadId, (EChoicesType)choices_type, set_id);
  }

  py::object PyInterface::GenPA(uint32 threadId, const py::dict& parms) const
  {
    GenPaRequest pa_req;
    process_transaction_parameters<GenRequest>(parms, &pa_req);
    mpScheduler->GenVmRequest(threadId, &pa_req);
    // << "size: " << pa_req.Size() << " align: " << pa_req.Align() << endl;
    py::int_ ret_int(pa_req.PA());
    return ret_int;
  }

  py::object PyInterface::GenVA(uint32 threadId, const py::dict& parms) const
  {
    GenVaRequest va_req;
    process_transaction_parameters<GenRequest>(parms, &va_req);
    mpScheduler->GenVmRequest(threadId, &va_req);
    py::int_ ret_int(va_req.VA());
    return ret_int;
  }

  py::object PyInterface::GenVMVA(uint32 threadId, const py::dict& parms) const
  {
    GenVmVaRequest vm_va_req;
    process_transaction_parameters<GenRequest>(parms, &vm_va_req);
    mpScheduler->GenVmRequest(threadId, &vm_va_req);
    py::int_ ret_int(vm_va_req.VA());
    return ret_int;
  }

  py::object PyInterface::GenVAforPA(uint32 threadId, const py::dict& parms) const
  {
    GenVaForPaRequest vapa_req;
    process_transaction_parameters<GenRequest>(parms, &vapa_req);
    mpScheduler->GenVmRequest(threadId, &vapa_req);
    py::int_ ret_int(vapa_req.VA());
    return ret_int;
  }

  // Random module API
  py::object PyInterface::Sample(uint64 totals, uint64 samples) const
  {
    std::vector<uint64> list;

    mpScheduler->Sample(totals, samples, list);

    py::list pyList;

    for (int i=0; i<(int)list.size(); ++i)
    {
        pyList.append(py::int_(list.at(i)));
    }

    return pyList;
  }

  // Register module API
  py::object PyInterface::GetRandomRegisters(cuint32 threadId, cuint32 number, const string& rRegType, const string& rExcludes) const
  {
    std::vector<uint64> reg_indices;
    if (mpScheduler->GetRandomRegisters(threadId, number, rRegType, rExcludes, reg_indices)) {
      py::tuple reg_tuple(reg_indices.size());

      for (size_t i = 0; i < reg_indices.size(); i++) {
        reg_tuple[i] = reg_indices[i];
      }

      return reg_tuple;
    }

    return py::none();
  }

  py::object PyInterface::GetRandomRegistersForAccess(cuint32 threadId, cuint32 number, const string& rRegType, const string& rAccess, const string& rExcludes) const
  {
    std::vector<uint64> reg_indices;
    if (mpScheduler->GetRandomRegistersForAccess(threadId, number, rRegType, rAccess, rExcludes, reg_indices)) {
      py::tuple reg_tuple(reg_indices.size());

      for (size_t i = 0; i < reg_indices.size(); i++) {
        reg_tuple[i] = reg_indices[i];
      }

      return reg_tuple;
    }

    return py::none();
  }

  py::object PyInterface::IsRegisterReserved(uint32 threadId, const std::string& name, const std::string& access, const std::string& type) const
  {
    bool ret = mpScheduler->IsRegisterReserved(threadId, name, access, type);
    LOG(debug) << "register: " << name << " access: " << access << " type: " << type << " isres: " << ret << endl;
    py::bool_ ret_bool(ret);
    return ret_bool;
  }

  void PyInterface::ReserveRegisterByIndex(uint32 threadId, uint32 size, uint32 index, const string& regType, const std::string& access) const
  {
    mpScheduler->ReserveRegisterByIndex(threadId, size, index, regType, access);
  }

  void PyInterface::ReserveRegister(uint32 threadId, const std::string& name,  const std::string& access) const
  {
    mpScheduler->ReserveRegister(threadId, name, access);
  }

  void PyInterface::UnreserveRegisterByIndex(uint32 threadId, uint32 size, uint32 index, const string& regType, const std::string& access) const
  {
    mpScheduler->UnreserveRegisterByIndex(threadId, size, index, regType, access);
  }

  void PyInterface::UnreserveRegister(uint32 threadId, const std::string& name, const std::string& access, const std::string& reserveType) const
  {
    mpScheduler->UnreserveRegister(threadId, name, access, reserveType);
  }

  py::object PyInterface::ReadRegister(uint32 threadId, const std::string& name, const std::string& field) const
  {
    py::tuple reg_tuple;
    uint64 reg_value = 0;
    if (mpScheduler->ReadRegister(threadId, name, field, reg_value) ) {
      reg_tuple = py::tuple(2);
      reg_tuple[0] = reg_value;
      reg_tuple[1] = 1;
    } else {
      reg_tuple[0] = reg_value;
      reg_tuple[1] = 0;
    }
    return reg_tuple;
    //LOG(warn) << "Unsupported Register name \"" << name << "\"." << endl;
    //return py::none();
  }

  void PyInterface::WriteRegister(uint32 threadId, const std::string& name, const std::string& field, const py::object& value, bool follow_iss)
  {
    if (py::isinstance<py::int_>(value)) {
      uint64 reg_value = value.cast<uint64>();
      mpScheduler->WriteRegister(threadId, name, field, reg_value, follow_iss);
    } else {
      LOG(warn) << "{PyInterface::WriteRegister} unsupported value type." << endl;
    }
  }

  void PyInterface::InitializeRegister(uint32 threadId, const std::string& name, const std::string& field, const py::object& value)
  {
    if (py::isinstance<py::int_>(value))
    {
      uint64 reg_value = value.cast<uint64>();
      mpScheduler->InitializeRegister(threadId, name, field, reg_value);
    }
    else if (py::isinstance<py::list>(value))
    {
      //vector of vals assumes we are setting a large register, so field param is dropped
      std::vector<uint64> reg_values = value.cast<vector<uint64> >();
      mpScheduler->InitializeRegister(threadId, name, reg_values);
    }
    else
    {
      LOG(warn) << "{PyInterface::InitializeRegister} unsupported value type." << endl;
    }
  }

  void PyInterface::InitializeRegisterFields(uint32 threadId, const std::string& registerName, const py::dict& field_value_map)
  {
    mpScheduler->InitializeRegisterFields(threadId, registerName, field_value_map.cast<map<string, uint64> >());
  }

  void PyInterface::RandomInitializeRegister(uint32 threadId, const std::string& name, const std::string& field)
  {
    mpScheduler->RandomInitializeRegister(threadId, name, field);
  }

  void PyInterface::RandomInitializeRegisterFields(uint32 threadId, const std::string& registerName, const py::list& fieldList)
  {
    mpScheduler->RandomInitializeRegisterFields(threadId, registerName, fieldList.cast<vector<string> >());
  }

  py::object PyInterface::GetRegisterFieldMask(uint32 threadId, const std::string& regName, const py::list& fieldList) const
  {
    py::tuple reg_tuple = py::tuple(2);
    uint64 mask = mpScheduler->GetRegisterFieldMask(threadId, regName, fieldList.cast<vector<string> >());
    reg_tuple[0] = mask;
    reg_tuple[1] = ~mask;
    return reg_tuple;
  }

  void PyInterface::GenSequence(uint32 threadId, const string& sequenceType, const py::dict& rParams) const
  {
    // instantiate request object.
    // the object ownership will be transfered into the Generator, don't need to delete here.
    GenSequenceRequest* gen_req = GenSequenceRequest::GenSequenceRequestInstance(sequenceType);

    // setup query parameters.
    process_transaction_parameters<GenRequest>(rParams, gen_req);

    // send the request.
    mpScheduler->GenSequence(threadId, gen_req);
  }

  void PyInterface::AddMemoryRange(uint32 bank, uint64 start, uint64 end)
  {
    mpScheduler->AddMemoryRange(bank, start, end);
  }

  void PyInterface::SubMemoryRange(uint32 bank, uint64 start, uint64 end)
  {
    mpScheduler->SubMemoryRange(bank, start, end);
  }

  void PyInterface::AddArchitectureMemoryAttributes(cuint32 bank, cuint64 start, cuint64 end, const string& rMemAttributes, cuint32 threadId)
  {
    vector<EMemoryAttributeType> mem_attributes;

    StringSplitter ss(rMemAttributes, ',');
    while (!ss.EndOfString()) {
      string sub_str = ss.NextSubString();
      mem_attributes.push_back(string_to_EMemoryAttributeType(sub_str));
    }

    mpScheduler->AddArchitectureMemoryAttributes(threadId, bank, start, end, mem_attributes);
  }

  void PyInterface::AddImplementationMemoryAttributes(cuint32 bank, cuint64 start, cuint64 end, const string& rMemAttributes, cuint32 threadId)
  {
    vector<string> mem_attributes;

    StringSplitter ss(rMemAttributes, ',');
    while (!ss.EndOfString()) {
      mem_attributes.push_back(ss.NextSubString());
    }

    mpScheduler->AddImplementationMemoryAttributes(threadId, bank, start, end, mem_attributes);
  }

  py::object PyInterface::Query(uint32 threadId, const string& queryName, const string& primaryString, const py::dict& rParams) const
  {
    // instantiate query object.
    GenQuery* gen_query = GenQuery::GenQueryInstance(queryName);
    unique_ptr<GenQuery> storage_ptr(gen_query); // responsible for releasing the storage when going out of scope.

    // setup query parameters.
    gen_query->SetPrimaryString(primaryString);

    if (rParams.size()) {
      process_transaction_parameters<GenQuery>(rParams, gen_query);
    }

    // run the query.
    mpScheduler->Query(threadId, *gen_query);

    // obtain and return results
    py::object py_results = py::none();
    gen_query->GetResults(py_results);
    return py_results;
  }

  py::object PyInterface::VirtualMemoryRequest(uint32 threadId, const string& reqName, const py::dict& rParams) const
  {
    // instantiate request object.
    GenVirtualMemoryRequest* gen_req = GenVirtualMemoryRequest::GenVirtualMemoryRequestInstance(reqName);
    unique_ptr<GenRequest> storage_ptr(gen_req); // responsible for releasing the storage when going out of scope.

    // setup query parameters.
    process_transaction_parameters<GenRequest>(rParams, gen_req);

    // send the request.
    mpScheduler->GenVmRequest(threadId, gen_req);

    // obtain and return results
    py::object py_results = py::none();
    gen_req->GetResults(py_results);

    return py_results;
  }

  py::object PyInterface::StateRequest(uint32 threadId, const string& actionName, const string& stateName, const py::object& stateValue, const py::dict& rParams) const
  {
    // instantiate request object.
    GenStateRequest* gen_req = GenStateRequest::GenStateRequestInstance(stateName);
    unique_ptr<GenRequest> storage_ptr(gen_req); // responsible for releasing the storage when going out of scope.

    gen_req->SetAction(actionName);

    // setup state request primary parameter.
    process_primary_parameter<GenRequest>(stateValue, gen_req);

    // setup state request parameters.
    if (rParams.size()) {
      process_transaction_parameters<GenRequest>(rParams, gen_req);
    }

    // send the request.
    mpScheduler->StateRequest(threadId, gen_req);

    // obtain and return results
    py::object py_results = py::none();
    gen_req->GetResults(py_results);
    return py_results;
  }

  py::object PyInterface::ExceptionRequest(uint32 threadId, const string& rReqName, const py::dict& rParams) const
  {
    // instantiate request object.
    GenExceptionRequest* gen_req = GenExceptionRequest::GenExceptionRequestInstance(rReqName);
    unique_ptr<GenRequest> storage_ptr(gen_req); // responsible for releasing the storage when going out of scope.

    // setup query parameters.
    process_transaction_parameters<GenRequest>(rParams, gen_req);

    // send the request.
    mpScheduler->ExceptionRequest(threadId, gen_req);

    // obtain and return results
    py::object py_results = py::none();
    gen_req->GetResults(py_results);
    return py_results;
  }

  void PyInterface::BeginStateRestoreLoop(cuint32 threadId, cuint32 loopRegIndex, cuint32 simCount, cuint32 restoreCount, const py::list& restoreExclusions) const
  {
    set<ERestoreExclusionGroup> restore_exclusions;
    for (auto restoreExclusion : restoreExclusions) {
      restore_exclusions.insert(restoreExclusion.cast<ERestoreExclusionGroup>());
    }

    auto gen_req = new GenRestoreRequest(ESequenceType::BeginRestoreLoop, loopRegIndex, simCount, restoreCount, restore_exclusions);
    mpScheduler->GenSequence(threadId, gen_req);
  }

  void PyInterface::EndStateRestoreLoop(cuint32 threadId, cuint32 loopId) const
  {
    auto gen_req = new GenRestoreRequest(ESequenceType::EndRestoreLoop);
    gen_req->SetLoopId(loopId);
    mpScheduler->GenSequence(threadId, gen_req);
  }

  void PyInterface::GenerateLoopRestoreInstructions(cuint32 threadId, cuint32 loopId) const
  {
    auto gen_req = new GenRestoreRequest(ESequenceType::RestoreLoopState);
    gen_req->SetLoopId(loopId);
    mpScheduler->GenSequence(threadId, gen_req);
  }

  py::object PyInterface::GetOption(const std::string& optName) const
  {
    py::tuple opt_tuple = py::tuple(2);

    Config* cfg_ptr = Config::Instance();
    bool valid = false;
    uint64 opt_value = cfg_ptr->GetOptionValue(optName, valid);
    if (not valid) {
      string opt_str = cfg_ptr->GetOptionString(optName, valid);
      opt_tuple[0] = opt_str;
    } else {
      opt_tuple[0] = opt_value;
    }
    opt_tuple[1] = valid;

    return opt_tuple;
  }

  void PyInterface::ReserveMemory(uint32 threadId, const string& name, const string& range, uint32 bank, bool isVirtual)
  {
    mpScheduler->ReserveMemory(threadId, name, range, bank, isVirtual);
  }

  void PyInterface::UnreserveMemory(uint32 threadId, const string& name, const string& range, uint32 bank, bool isVirtual)
  {
    mpScheduler->UnreserveMemory(threadId, name, range, bank, isVirtual);
  }

  void PyInterface::ModifyVariable(uint32 threadId, const string& name, const string& value, const string& var_type)
  {
    mpScheduler->ModifyVariable(threadId, name, value, var_type);
  }

  string PyInterface::GetVariable(uint32 threadId, const string& name, const string& var_type) const
  {
    return mpScheduler->GetVariable(threadId, name, var_type);
  }

  void PyInterface::RegisterModificationSet(uint32 threadId, const py::object& choicesType, uint32 set_id)
  {
    uint64 choices_type  = choicesType.cast<uint64>();
    mpScheduler->RegisterModificationSet(threadId,  (EChoicesType)choices_type, set_id);
  }

  bool PyInterface::VerifyVirtualAddress(uint32 threadId, uint64 va, uint64 size, bool isInstr) const
  {
    return mpScheduler->VerifyVirtualAddress(threadId, va, size, isInstr);
  }

  void PyInterface::LockThreadScheduler(uint32 threadId)
  {
    mpScheduler->LockThreadScheduler(threadId);
  }

  void PyInterface::UnlockThreadScheduler(uint32 threadId)
  {
    mpScheduler->UnlockThreadScheduler(threadId);
  }

  void PyInterface::PartitionThreadGroup(const std::string& policy, const py::dict& params)
  {
    auto part_policy = string_to_EPartitionThreadPolicy(policy);
    switch (part_policy) {
    case EPartitionThreadPolicy::Random: {
      PartitionArgument part_arg;
      for (const auto & dict_pair : params) {
        string key = dict_pair.first.cast<string>();
        const py::handle& value_obj = dict_pair.second;
        if (key == "group_num") {
          part_arg.mGroupNum = cast_py_int(value_obj);
        }
        else if (key == "group_size") {
          part_arg.mGroupSize = cast_py_int(value_obj);
        }
        else {
          LOG(fail) << "{PyInterface::PartitionThreadGroup} unknown key word: " << key << endl;
          FAIL("unknown-key-word");
        }
      }
      mpScheduler->PartitionThreadGroup(part_policy, &part_arg);
      break;
    }
    case EPartitionThreadPolicy::SameCore:
    case EPartitionThreadPolicy::SameChip:
    case EPartitionThreadPolicy::DiffChip:
    case EPartitionThreadPolicy::DiffCore:
      mpScheduler->PartitionThreadGroup(part_policy);
      break;
    default:
      LOG(fail) << "{PyInterface::PartitionThreadGroup} Unhandled policy: " << policy << endl;
      FAIL("unhandled-policy");
    }
  }

  void PyInterface::SetThreadGroup(uint64 groupId, const std::string& job, const std::string& threads)
  {
    mpScheduler->SetThreadGroup(groupId, job, threads);
  }

  py::object PyInterface::QueryThreadGroup(const py::object& groupId) const
  {
    uint32 group_id = 0;
    if (py::isinstance<py::none>(groupId))
      group_id = -1u;
    else
      group_id = groupId.cast<uint32>();

    py::list tg_list;
    vector<ThreadGroup*> thread_groups;

    mpScheduler->QueryThreadGroup(group_id, thread_groups);
    for (const auto grp : thread_groups) {
      // << "{PyInterface::QueryThreadGroup} thread group " << grp->ToString() << endl;
      py::tuple tg_tuple = py::tuple(3);
      tg_tuple[0] = py::int_(grp->GetId());
      tg_tuple[1] = py::str(grp->GetJob());
      tg_tuple[2] = py::str(grp->GetThreads()->ToSimpleString());
      tg_list.append(tg_tuple);
    }

    return tg_list;
  }

  py::object PyInterface::GetThreadGroupId(uint32 threadId) const
  {
    auto group_id = mpScheduler->GetThreadGroupId(threadId);
    if (group_id == -1u)
      return py::none();
    else
      return py::int_(group_id);
  }

  py::object PyInterface::GetFreeThreads() const
  {
    py::list tg_list;
    vector<uint32> free_threads;

    mpScheduler->GetFreeThreads(free_threads);
    for (const auto thread : free_threads) {
      tg_list.append(thread);
    }

    return tg_list;
  }

  py::object PyInterface::GenSemaphore(uint32 threadId, const std::string& name, uint64 counter, uint32 bank, uint32 size)
  {
    uint64 pa;
    bool reverse_endian;
    auto valid = mpScheduler->GenSemaphore(threadId, name, counter, bank, size, pa, reverse_endian);

    py::tuple ret_tuple = py::tuple(3);
    ret_tuple[0] = pa;
    ret_tuple[1] = reverse_endian;
    ret_tuple[2] = valid;
    return ret_tuple;
  }

  py::object PyInterface::GenFreePagesRange(uint32 threadId, const py::dict& parms) const
  {
    GenFreePageRequest free_page_req;
    process_transaction_parameters<GenRequest>(parms, &free_page_req);
    mpScheduler->GenVmRequest(threadId, &free_page_req);

    // obtain and return results
    py::object py_results = py::none();
    free_page_req.GetResults(py_results);
    return py_results;
  }

  static void process_synchronize_barrier_parameters(const py::dict& params, const Scheduler* scheduler_ptr, ConstraintSet& threads)
  {
    auto mergeGroup = [&](ThreadGroup* grp) { threads.MergeConstraintSet(*(grp->GetThreads())); };
    for (const auto & dict_pair : params) {
      string key   = dict_pair.first.cast<string>();
      string value = dict_pair.second.cast<string>();

      if (key == "ThreadId") {
        threads.MergeConstraintSet(ConstraintSet(value));
      }
      else if (key == "GroupId") {
        ConstraintSet groupid_constr(value);
        std::vector<uint64> group_vec;
        groupid_constr.GetValues(group_vec);
        for (const auto grp_id : group_vec) {
          vector<ThreadGroup*> thread_groups;
          scheduler_ptr->QueryThreadGroup(grp_id, thread_groups);
          if (thread_groups.empty()) {
            LOG(fail) << "{PyInterface::SynchronizeWithBarrier} GroupId " << grp_id << " not found" << endl;
            FAIL("group-id-not-found");
          }
          std::for_each(thread_groups.begin(), thread_groups.end(), mergeGroup);
        }
      }
      else if (key == "GroupJob") {
        StringSplitter ss(value, ',');
        while (!ss.EndOfString()) {
          const string grp_job = ss.NextSubString();
          vector<ThreadGroup*> thread_groups;
          scheduler_ptr->QueryThreadGroup(grp_job, thread_groups);
          if (thread_groups.empty()) {
            LOG(fail) << "{PyInterface::SynchronizeWithBarrier} GroupJob " << grp_job << " not found" << endl;
            FAIL("group-job-not-found");
          }
          std::for_each(thread_groups.begin(), thread_groups.end(), mergeGroup);
        }
      }
      else {
        LOG(fail) << "{PyInterface::SynchronizeWithBarrier} key param " << key << " not supported" << endl;
        FAIL("key-param-not-supported");
      }
    }
  }

  void PyInterface::SynchronizeWithBarrier(uint32 threadId, const py::dict& params)
  {
    ConstraintSet synchronized_threads;
    process_synchronize_barrier_parameters(params, mpScheduler, synchronized_threads);
    if (synchronized_threads.IsEmpty()) {
      uint32 total = mpScheduler->NumberOfChips() * mpScheduler->NumberOfCores() * mpScheduler->NumberOfThreads();
      synchronized_threads.AddRange(0, total - 1);
    }
    mpScheduler->SynchronizeWithBarrier(threadId, synchronized_threads);
  }

}
