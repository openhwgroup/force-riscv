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
#include <Defines.h>
#include <TopLevelResources.h>
#include <UtilityFunctions.h>
#include <StringUtils.h>
#include <optionparser.h>
#include <Random.h>
#include <Architectures.h>
#include <Config.h>
#include <ObjectRegistry.h>
#include <MemoryManager.h>
#include <PcSpacing.h>
#include <InstructionResults.h>
#include <Log.h>
#include <Data.h>
#include <Dump.h>
#include <DataStation.h>
#include <ExceptionManager.h>
#include <FrontEndCall.h>
#include <RestoreLoop.h>
#include <StateTransition.h>
#include <ThreadGroupPartitioner.h>
#include <PyEnvironment.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

namespace Force {

  struct Arg: public option::Arg
  {
    static void printError(const char* msg1, const option::Option& opt, const char* msg2)
    {
      LOG(error) << msg1 << opt.name << msg2 << endl;
    }

    static option::ArgStatus Unknown(const option::Option& option, bool msg)
    {
      if (msg) printError("Unknown option '", option, "'.");
      return option::ARG_ILLEGAL;
    }

    static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
    {
      if (option.arg != 0 && option.arg[0] != 0)
        return option::ARG_OK;

      if (msg) printError("Option '", option, "' requires a non-empty argument.");
      return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Numeric(const option::Option& option, bool msg)
    {
      bool error_status = false;
      parse_uint64(option.arg, &error_status);
      if (!error_status)
        return option::ARG_OK;

      if (msg) printError("Option '", option, "' requires a numeric argument.");
      return option::ARG_ILLEGAL;
    }
  };

  enum OptionIndex { UNKNOWN, CFG, HELP, LOGLEVEL, DUMP, NOASM, IMG, OPTIONS, SEED, TEST, NOISS, MAXINSTR, NUMCHIPS, NUMCORES, NUMTHREADS, OUTPUTWITHSEED, FAILOVERRIDE, GLOBALMODIFIER, ISSTRACEFILE };
  const option::Descriptor usage[] =
    {
      {UNKNOWN,      0, "",   "",         Arg::None,     "USAGE: force [options]\n\n" "Options:" },
      {CFG,          0, "c", "cfg",       Arg::NonEmpty, "  --cfg, -c \tConfiguration file path."},
      {HELP,         0, "h", "help",      Arg::None,     "  --help  \tPrint usage and exit." },
      {LOGLEVEL,     0, "l", "log",       Arg::NonEmpty, "  --log, -l \tSpecify logging level."},
      {DUMP,         0, "d", "dump",      Arg::NonEmpty, "  --dump, -d \tSpecify dumping option."},
      {NOASM,        0, "",  "noasm",     Arg::None,     "  --noasm, \tIndicate not to output assembly code."},
      {IMG,          0, "",  "img",       Arg::None,     "  --img, \tIndicate to output memory and registers image."},
      {OPTIONS,      0, "o", "options",   Arg::NonEmpty, "  --options, -o \tSpecify test options."},
      {SEED,         0, "s", "seed",      Arg::Numeric,  "  --seed, -s  \tSpecify seed for test generation." },
      {TEST,         0, "t", "test",      Arg::NonEmpty, "  --test, -t  \tSpecify test template name to run." },
      {NOISS,        0, "n", "noiss",     Arg::None,     "  --noiss, -n \tIndicate to not simulate during test generation."},
      {MAXINSTR,     0, "m", "max-instr", Arg::Numeric,  "  --max-instr, -m \tMaximum number of instruction that can be simulated."},
      {NUMCHIPS,     0, "P", "num-chips", Arg::Numeric, "  --num-chips, -P \tNumber of chips."},
      {NUMCORES,     0, "C", "num-cores", Arg::Numeric,  "  --num-cores, -C \tNumber of cores per chip."},
      {NUMTHREADS,   0, "T", "num-threads", Arg::Numeric, "  --num-threads, -T \tNumber of threads per core."},
      {OUTPUTWITHSEED, 0, "w",  "outputwithseed",  Arg::None, "  --outputwithseed, -w \tIndicate to generate outputs with seed number."},
      {FAILOVERRIDE, 0, "f",  "failOverride",  Arg::None, "  --failOverride, -f \tFORCE will fail when operand override is invalid."},
      {GLOBALMODIFIER, 0, "g",  "global-modifier",  Arg::NonEmpty, "  --global-modifier, -g \tGlobal modification file path."},

//      {ISSTRACEFILE, 0, "",  "apitrace",  Arg::NonEmpty, "  --apitrace, \tPath to simulator API trace file."},
      {UNKNOWN,      0, "",  "",          Arg::None,     "\nExamples:\n"
                                                         "  force -s 0x123 -t utils/smoke/test_force.py\n"
                                                         "  force -s 0x123 -l info -t utils/smoke/test_force.py\n"
                                                         "  force --unknown -- --this_is_no_option\n"},
      {0,0,0,0,0,0}
    };

  static void parse_command_line_options(int argc, char* argv[], const char* pDefConfig)
  {
    string program_name = argv[0];
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    stringstream parameters; // collect command line arguments to form a command for rerun.
    for (int i = 0; i < argc; ++ i) {
      parameters << " " << argv[i];
    }

    option::Stats  stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    if (parse.error()) {
      FAIL("argument-error");
    }
    if (parse.nonOptionsCount()) {
      const char** non_options = parse.nonOptions();
      LOG(notice) <<"{parse command options} has unknown option:" << non_options[0] << endl;
      FAIL("argument-error");
    }

    if (options[HELP] || argc == 0) {
      option::printUsage(std::cout, usage);
      exit(0);
    }

    if (options[LOGLEVEL]) {
      option::Option* log_level = options[LOGLEVEL].last();
      SET_LOG_LEVEL(log_level->arg);
    }

    if (options[DUMP]) {
      option::Option* dump_option = options[DUMP].last();
      Dump::Instance()->SetOption(dump_option->arg);
    }

    uint64 test_seed = 0;
    bool seed_provided = false;
    if (options[SEED]) {
      option::Option* seed_opt = options[SEED].last();
      test_seed = parse_uint64(seed_opt->arg);
      seed_provided = true;
      LOG(trace) << "User specified seed 0x" << hex << test_seed << endl;
    } else {
      test_seed = Random::Instance()->RandomSeed();
      LOG(trace) << "Picked random seed 0x" << hex << test_seed << endl;
    }
    Random::Instance()->Seed(test_seed);

    string cfg_file = pDefConfig;
    if (options[CFG]) {
      option::Option* cfg_opt = options[CFG].last();
      cfg_file = cfg_opt->arg;
      LOG(trace) << "User specified config file \"" << cfg_file << "\"." << endl;
    }
    Config::Instance()->LoadConfigFile(cfg_file, program_name);

    if (options[TEST]) {
      option::Option* test_opt = options[TEST].last();
      string test_file = test_opt->arg;
      LOG(notice) << "Generating using test template \"" << test_file << "\"..." << endl;
      Config::Instance()->SetTestTemplate(test_file);
    } else {
      option::printUsage(std::cout, usage);
      exit(0);
    }

    if (options[NOASM]) {
      LOG(notice) << "Not outputing assembly code." << endl;
      Config::Instance()->SetOutputAssembly(false);
    }

    if (options[IMG]) {
      LOG(notice) << "Not to output memory and registers image." << endl;
      Config::Instance()->SetOutputImage(true);
    }

    if (options[NOISS]) {
      LOG(notice) << "NOT Simulating instructions during test generation." << endl;
    } else {
      LOG(notice) << "Simulating instructions during test generation." << endl;
      Config::Instance()->SetDoSimulate(true);
    }

    if (options[MAXINSTR]) {
      option::Option* max_instr_opt = options[MAXINSTR].last();
      uint64 max_instr = parse_uint64(max_instr_opt->arg);
      LOG(notice) << "Maximum number of instructions that can be simulated before the test terminate: " << dec << max_instr << endl;
      Config::Instance()->SetMaxInstructions(max_instr);
    }

    if (options[NUMCHIPS]) {
      option::Option* num_chips_opt = options[NUMCHIPS].last();
      uint64 num_chips = parse_uint64(num_chips_opt->arg);
      LOG(notice) << "Number of chips: " << dec << num_chips << endl;
      Config::Instance()->SetNumChips(num_chips);
    }

    if (options[NUMCORES]) {
      option::Option* num_cores_opt = options[NUMCORES].last();
      uint64 num_cores = parse_uint64(num_cores_opt->arg);
      LOG(notice) << "Number of cores per chip: " << dec << num_cores << endl;
      Config::Instance()->SetNumCores(num_cores);
    }

    if (options[NUMTHREADS]) {
      option::Option* num_threads_opt = options[NUMTHREADS].last();
      uint64 num_threads = parse_uint64(num_threads_opt->arg);
      LOG(notice) << "Number of threads per core: " << dec << num_threads << endl;
      Config::Instance()->SetNumThreads(num_threads);
    }

    if (options[ISSTRACEFILE]) {
      option::Option* apitrace_opt = options[ISSTRACEFILE].last();
      string apitrace_file = apitrace_opt->arg;
      LOG(notice) << "Simulator API trace file \"" << apitrace_file << "\"..." << endl;
      Config::Instance()->SetIssApiTraceFile(apitrace_file);
    }

    if (options[OUTPUTWITHSEED]) {
      LOG(notice) << "Generate outputs with seed number." << endl;
      Config::Instance()->SetOutputWithSeed(true, test_seed);
    }

    if (options[FAILOVERRIDE]) {
      LOG(notice) << "Generator will fail on invalid operand overrides" << endl;
      Config::Instance()->SetFailOnOperandOverrides();
    }

    if (options[GLOBALMODIFIER]) {
      option::Option* mod_opt = options[GLOBALMODIFIER].last();
      string mod_file = mod_opt->arg;
      Config::Instance()->SetChoicesModificationFile(mod_file);
      LOG(trace) << "User specified global modification file \"" << mod_file << "\"." << endl;
    }

    if (options[OPTIONS]) {
      option::Option* test_opt = options[OPTIONS].last();
      string opt_string = test_opt->arg;
      if (not Config::Instance()->ParseOptions(opt_string)) {
        LOG(fail) << "Incorrect format of test options string: \"" << opt_string << "\"." << endl;
        FAIL("incorrect-test-option-format");
      } else {
        LOG(notice) << "Test options specified: " << opt_string << "." << endl;
      }
    }

    if (not seed_provided) {
      parameters << " -s 0x" << hex << test_seed;
    }
    string command_line = program_name + parameters.str();
    Config::Instance()->SetCommandLine(command_line);
    LOG(notice) << "Command line: " << command_line << endl;

  }

  /*!
    Top level resources should implement Initialize and Destroy interface methods.

    The method calling order (in terms of modules involved) in initialize_top_level_resources() and destroy_top_level_resources() should be in opposite order.
 */
  void initialize_top_level_resources(int argc, char* argv[], std::vector<ArchInfo*>& archInfoObjs)
  {
    Logger::Initialize();
    Random::Initialize();
    DataStation::Initialize();

    Architectures::Initialize();
    Architectures * arch_top = Architectures::Instance();
    arch_top->AssignArchInfoObjects(archInfoObjs);

    Config::Initialize();
    FrontEndCall::Initialize();
    DataFactory::Initialize();
    Dump::Initialize();

    ExceptionManager::Initialize();

    parse_command_line_options(argc, argv, arch_top->DefaultArchInfo()->DefaultConfigFile());
    ObjectRegistry::Initialize();
    MemoryManager::Initialize();
    ThreadPartitionerFactory::Initialize();

    Config * config_ptr = Config::Instance();
    config_ptr->SetGlobalStateValue(EGlobalStateType::ElfMachine, arch_top->DefaultArchInfo()->ElfMachineType());

    arch_top->SetupSimAPIs();

    PcSpacing::Initialize();
    InstructionResults::Initialize();
    RestoreLoopManagerRepository::Initialize();

    PyEnvironment::initialize_python();

    // Everything below here is dependent on the Python runtime
    StateTransitionManagerRepository::Initialize();
  }

  /*!
    Call Destroy interface methods of top level resources to wind down.
   */
  void destroy_top_level_resources()
  {
    // Everything from here until the call to finalize_python() is dependent on the Python runtime
    StateTransitionManagerRepository::Destroy();

    PyEnvironment::finalize_python();

    RestoreLoopManagerRepository::Destroy();
    InstructionResults::Destroy();
    PcSpacing::Destroy();
    ThreadPartitionerFactory::Destroy();
    MemoryManager::Destroy();
    ExceptionManager::Destroy();
    ObjectRegistry::Destroy();
    FrontEndCall::Destroy();
    Config::Destroy();
    DataFactory::Destroy();
    Dump::Destroy();
    Architectures::Destroy();

    DataStation::Destroy();
    Random::Destroy();
    Logger::Destroy();
  }

}
