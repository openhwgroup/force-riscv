
 Copyright (C) [2020] Futurewei Technologies, Inc.

 FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

 THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 FIT FOR A PARTICULAR PURPOSE.
 See the License for the specific language governing permissions and
 limitations under the License.

USAGE: fpix [options] <test image>

  Where [options] are command line options and a <test image> is either an ELF file(s) or a test 'checkpoint'.
  Note that the test-image file(s) signify the end of the command line options, and thus must appear last.

Command line options include:

  --cfg, -c                                  Configuration file path. The tool will always load a base config
                                             file from the fpix dev directory (config/base.config). Use this
                                             option to specify a user-defined config file.

  --help                                     Print usage and exit.

  --log                                      Specify logging level.
  --seed                                     Specify random seed initial value.
  --plugin                                   Specify path to plugin shared object.

  This is the preliminary set of simulator options to be supported:

  --core_num, -C                             Number of cores (defaults to four)
  --cluster_num, -cl                         Number of clusters (defaults to one)
  --threads_per_cpu, -T                      Number of  threads per cpu (defaults to one)
  --pa_size, -P                              Physical address size (defaults to 44 bits)
  --max_insts, -i                            Maximum number of instructions to simulate. If the number
                                             of simulated instructions exceeds this value the simulator
                                             will abort. (has no default value)
  --exit_loop, -X                            Exit when an instruction jumps to itself (always enabled)
                                             (Can be used to specify a numeric value that indicates how many
                                             'cores' should end execution in this manner)
  --decoding, -d                             Print instruction decoding during execution
  --railhouse, -T                               Write RAILHOUSE trace to this file name

Examples:

  fpix_riscv -i 10000 -T xyz.railhouse xyz.Default.ELF
  fpix_riscv -C 1 -P 48 -i exc_test.elf


