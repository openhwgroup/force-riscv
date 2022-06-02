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
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

#include "optionparser.h"

#include "ConfigFPIX.h"
#include "EnumsFPIX.h"
#include "Log.h"
#include "Random.h"
#include "StringUtils.h"
#include "usage.h"

using namespace std;

namespace Force
{
struct Arg: public option::Arg
{
    static void printError(const char* apMsg1, const option::Option& arOpt, const char* apMsg2)
    {
        std::cerr << apMsg1 << arOpt.name << apMsg2 << endl;
    }

    static option::ArgStatus Unknown(const option::Option& arOption, bool aMsg)
    {
        if (aMsg) printError("Unknown option '", arOption, "'.");
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus NonEmpty(const option::Option& arOption, bool aMsg)
    {
        if (arOption.arg != 0 && arOption.arg[0] != 0)
            return option::ARG_OK;

        if (aMsg) printError("Option '", arOption, "' requires a non-empty argument.");
            return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Numeric(const option::Option& arOption, bool aMsg)
    {
        bool error_status = false;

        parse_uint64(arOption.arg, &error_status);
        if (!error_status)
            return option::ARG_OK;

        if (aMsg) printError("Option '", arOption, "' requires a numeric argument.");
        return option::ARG_ILLEGAL;
    }
};


  const option::Descriptor usage[] =
    {
      {EOptionIndexBaseType(EOptionIndex::UNKNOWN), 0, "", "", Arg::None, "\nUSAGE: fpix [options]\n\n" "Options:"},
      {EOptionIndexBaseType(EOptionIndex::CFG), 0, "c", "cfg", Arg::NonEmpty, "  --cfg, -c \tConfiguration file path."},
      {EOptionIndexBaseType(EOptionIndex::HELP), 0, "h", "help", Arg::None, "  --help  \tPrint usage and exit."},

      // fpix options here...

      {EOptionIndexBaseType(EOptionIndex::LOGLEVEL), 0, "", "log", Arg::NonEmpty, "  --log \tLog level."},
      {EOptionIndexBaseType(EOptionIndex::SEED), 0, "", "seed", Arg::Numeric, "  --seed \tSeed."},
      {EOptionIndexBaseType(EOptionIndex::PLUGIN), 0, "", "plugin", Arg::NonEmpty, "  --plugin \tPath to plugin shared object."},
      {EOptionIndexBaseType(EOptionIndex::PLUGINS_OPTIONS), 0, "", "plugins_options", Arg::NonEmpty, "  --plugins_options \tComma separated list of plugin options. Example: --plugins_options plug1.opt1=45,all_plugins_work,plug2.enable_mode2"},

      {EOptionIndexBaseType(EOptionIndex::MAX_INSTS_OPT), 0, "i", "max_insts", Arg::Numeric, "  --max_insts, -i \tNumber of instructions to run til exiting simulator"},
      {EOptionIndexBaseType(EOptionIndex::RAILHOUSE_OPT), 0, "T", "railhouse", Arg::NonEmpty, "  --railhouse, -T \tWrite RAILHOUSE trace to this file name"},
      {EOptionIndexBaseType(EOptionIndex::DECODING_OPT), 0, "d", "decoding", Arg::Numeric, "  --decoding, -d \tPrint instruction decoding during execution"},
      {EOptionIndexBaseType(EOptionIndex::CORE_NUM_OPT), 0, "C", "core_num", Arg::Numeric, "  --core_num, -C \tset core numbers"},
      {EOptionIndexBaseType(EOptionIndex::CLUSTER_NUM_OPT), 0, "cl", "cluster_num", Arg::Numeric, "  --cluster_num, -cl \tset cluster numbers"},
      {EOptionIndexBaseType(EOptionIndex::THREADS_PER_CPU_OPT), 0, "T", "threads_per_cpu", Arg::Numeric, "  --threads_per_cpu, -T \tset num threads per cpu"},
      {EOptionIndexBaseType(EOptionIndex::PA_SIZE_OPT), 0, "P", "pa_size", Arg::Numeric, "  --pa_size, -P \tdefualt pa size is 44 bits"},
      {EOptionIndexBaseType(EOptionIndex::EXIT_LOOP_OPT), 0, "X", "exit_loop", Arg::Numeric, "  --exit_loop, -X \texit when an instruction jumps to itself"},
      {EOptionIndexBaseType(EOptionIndex::AUTO_INIT_MEM_OPT), 0, "", "auto_init_mem", Arg::None, "  --auto_init_mem  \tautomatically initialize simulator memory on access; enabling this could potentially mask errors in the test"},
      {EOptionIndexBaseType(EOptionIndex::UNKNOWN), 0, "", "", Arg::None, "\nExamples:\n"
                                                         "  fpix -i 10000 -D \n"
                                                         "  fpix --unknown -- --this_is_no_option\n"},


      {0,0,0,0,0,0}
    };

  void load_program_options(ConfigFPIX &arCfg, vector<string> &arTestFiles, vector<string> &plugins_cl_args, int argc, char *argv[], const string& rDefaultConfig)
  {
    Logger::Initialize();
    Random::Initialize();

    string program_name = argv[0];
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    stringstream parameters; // collect command line arguments to form a command for rerun.
    for (int i = 0; i < argc; ++ i)
    {
       parameters << " " << argv[i];
    }

    option::Stats stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);

    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    if (parse.error())
    {
      //FAIL("argument-error");
      LOG(fail) << "One or more command line options failed to parse.." << endl;
      exit(-1);
    }

    if (parse.nonOptionsCount())
    {
      const char** options_strings = parse.nonOptions();
      for (EOptionIndexBaseType i = 0; options_strings[i] != 0; i++)
      {
        string opt_str = options_strings[i];
        string lower_case;
        transform(opt_str.begin(), opt_str.end(), lower_case.begin(), ::tolower);
        if (lower_case.find(".elf") != string::npos)
        {
          arTestFiles.push_back(opt_str);
          continue;
        }
        else if (lower_case.find("riscv") != string::npos)
        {
          arTestFiles.push_back(opt_str);
          continue;
        }

        cout << "\n### Potential unknown command line option: " << options_strings[i] << endl;
        plugins_cl_args.push_back(opt_str);
      }

      if(arTestFiles.empty())
      {
        cout << "\n### Trying first unknown string as a potential ELF file: " << options_strings[0] << endl;
        arTestFiles.push_back(std::string(options_strings[0]));
      }
    }

    if (options[EOptionIndexBaseType(EOptionIndex::HELP)] || argc == 0)
    {
      //option::printUsage(std::cout, usage);
      std::cout << fpixUsage << std::endl;
      exit(0);
    }

    if (options[EOptionIndexBaseType(EOptionIndex::LOGLEVEL)])
    {
      option::Option* log_level = options[EOptionIndexBaseType(EOptionIndex::LOGLEVEL)].last();
      SET_LOG_LEVEL(log_level->arg);
    }

    LOG(info) << "   Command line:\n" << "      " << program_name << parameters.str() << endl;

    if (options[EOptionIndexBaseType(EOptionIndex::SEED)])
    {
      option::Option* seed_opt = options[EOptionIndexBaseType(EOptionIndex::SEED)].last();
      uint64 test_seed = parse_uint64(seed_opt->arg);
      Random::Instance()->Seed(test_seed);
      LOG(trace) << "User specified seed 0x" << hex << test_seed << dec << endl;
    }
    else
    {
      uint64 test_seed = Random::Instance()->RandomSeed();
      LOG(trace) << "Picked random seed 0x" << hex << test_seed << dec << endl;
    }

    // load user-specified config, or default config first...

    string config_file = rDefaultConfig;
    arCfg.LoadConfigFile(config_file, program_name);

    if (options[EOptionIndexBaseType(EOptionIndex::CFG)])
    {
      option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::CFG)].last();
      config_file = my_opt->arg;
      LOG(info) << "      'cfg' (custom) : " << config_file << endl;
      arCfg.LoadConfigFile(config_file, program_name);
    }
    else
    {
      LOG(info) << "      'cfg' (default) : " << config_file << endl;
    }

    if (options[EOptionIndexBaseType(EOptionIndex::PLUGIN)])
    {
      for (option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::PLUGIN)]; my_opt; my_opt = my_opt->next())
    {
         string plugin_name = my_opt->arg;
         string plugin_file_path;
         if (arCfg.PluginExists(plugin_name,plugin_file_path))
        {
           // a plugin with this name already exists...
           LOG(info) << "A plugin with name '" << plugin_name << "' is already in the config for this project. Cmdline option ignored..." << endl;
         }
         else
        {
           // assume the plugin-name is in fact the path to a plugin file...
           plugin_file_path = arCfg.LookUpFile(plugin_name);
           arCfg.AddPlugin(plugin_name.c_str(), plugin_file_path.c_str());
           LOG(info) << "User plugin: " << plugin_name << " (file-path: " << plugin_file_path << ")" << endl;
         }
      }
    }

    if (options[EOptionIndexBaseType(EOptionIndex::PLUGINS_OPTIONS)])
    {
      for (option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::PLUGINS_OPTIONS)]; my_opt; my_opt = my_opt->next())
    {
        string options_string = my_opt->arg;
        arCfg.appendPluginsOptions(options_string);
        LOG(info) << "Adding plugins options to global string: "  << options_string << endl;
      }
    }

    // process all options that represent strings...

    if (options[EOptionIndexBaseType(EOptionIndex::RAILHOUSE_OPT)])
    {
      option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::RAILHOUSE_OPT)].last();
      string railhouse_file = my_opt->arg;
      arCfg.SetRailhouseFile(railhouse_file);
      LOG(info) << "      'railhouse' : " << railhouse_file << endl;
    }

    // all other options are processed as ints...

    if (options[EOptionIndexBaseType(EOptionIndex::MAX_INSTS_OPT)])
    {
      option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::MAX_INSTS_OPT)].last();
      unsigned int nval = parse_uint64(my_opt->arg);
      arCfg.SetMaxInsts(nval);
      LOG(info) << "      'max_insts' : " << nval << endl;
    }
    if (options[EOptionIndexBaseType(EOptionIndex::DECODING_OPT)])
    {
      option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::DECODING_OPT)].last();
      unsigned int nval = parse_uint64(my_opt->arg);
      arCfg.SetEnableDecoding(nval);
      LOG(info) << "      'decoding' : " << nval << endl;
    }
    if (options[EOptionIndexBaseType(EOptionIndex::CLUSTER_NUM_OPT)])
    {
      option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::CLUSTER_NUM_OPT)].last();
      unsigned int nval = parse_uint64(my_opt->arg);
      arCfg.SetClusterCount(nval);
      LOG(info) << "      'cluster_num' : " << nval << endl;
    }
    if (options[EOptionIndexBaseType(EOptionIndex::CORE_NUM_OPT)])
    {
      option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::CORE_NUM_OPT)].last();
      unsigned int nval = parse_uint64(my_opt->arg);
      arCfg.SetNumberOfCores(nval);
      LOG(info) << "      'core_num' : " << nval << endl;
    }
    if (options[EOptionIndexBaseType(EOptionIndex::THREADS_PER_CPU_OPT)])
    {
      option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::THREADS_PER_CPU_OPT)].last();
      unsigned int nval = parse_uint64(my_opt->arg);
      arCfg.SetThreadsPerCpu(nval);
      LOG(info) << "      'threads_per_cpu' : " << nval << endl;
      cerr << "NOTE: Ignoring 'threads_per_cpu' cmdline arg for now. NOT supported by current Handcar libraries..." << endl;
    }

    uint32_t total_PE_count = arCfg.ClusterCount() * arCfg.NumberOfCores() * arCfg.ThreadsPerCpu();

    if (options[EOptionIndexBaseType(EOptionIndex::EXIT_LOOP_OPT)])
    {
      option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::EXIT_LOOP_OPT)].last();
      uint32_t nval = parse_uint64(my_opt->arg);
      LOG(info) << "      'exit_loop' : " << nval << endl;
      if (nval > total_PE_count)
        {
        LOG(fail) << "exit_loop count specified (" << nval << ") exceeds the core/thread count (" << total_PE_count << ")" << endl;
        exit(-1);
      }
      else if (nval == 0)
        {
        LOG(fail) << "Disabling exit_loop is currently not supported." << endl;
        exit(-1);
      }
      arCfg.SetExitOnBranchToSelf(nval);
    }

    if (options[EOptionIndexBaseType(EOptionIndex::AUTO_INIT_MEM_OPT)])
    {
      arCfg.SetAutoInitMem(true);
      LOG(info) << "      'auto_init_mem' : enabled" << endl;
    }

    if (options[EOptionIndexBaseType(EOptionIndex::PA_SIZE_OPT)])
    {
      option::Option* my_opt = options[EOptionIndexBaseType(EOptionIndex::PA_SIZE_OPT)].last();
      unsigned int nval = parse_uint64(my_opt->arg);
      arCfg.SetPhysicalAddressSize(nval);
      LOG(info) << "      'pa_size' : " << nval << endl;
    }

    LOG(info) << "   Test files:\n     ";
    for (vector<string>::iterator i = arTestFiles.begin(); i != arTestFiles.end(); ++i)
    {
       LOG(info) << " " << *i;
    }
    LOG(info) << endl;
}

}
