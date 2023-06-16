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

# Introduction to FORCE-RISCV
FORCE-RISCV is an instruction sequence generator (ISG) for the RISC-V instruction set architecture.  It can be used to generate tests for design verification of RISC-V processors.  FORCE-RISCV uses randomization to choose instructions, registers, addresses and data for the tests, and can generate valid test sequences with very little input from the user.  However, FORCE-RISCV provides a set of APIs with extensive capabilities which gives the user a high level of control over how the instruction generation takes place.

Test templates (written in Python) are used to control FORCE-RISCV instruction generation by invoking the FORCE-RISCV APIs to define constraints and specify the instructions.  Because the test templates are normal Python code, the user has the power of the Python programming language to define the instruction generation sequence and the appropriate constraints.

FORCE-RISCV is integrated with the Handcar instruction simulator to model the behavior of the generated RISC-V instructions.  Handcar is based on the Spike open source RISC-V instruction simulator.  The format for the generation output is the standard *.ELF file and a disassembled text *.S file.

FORCE-RISCV provides support for the following aspects of the RISC-V ISA:
* RV64G (IMAFDCZicsr_Zifencei)
* RV32G
* V extension 1.0
* RISC-V privileged ISA, including full support for the U, S, and M privilege levels.
* Full support for the Sv48 virtual memory system, including 4KB, 2MB, 1GB and 512GB page sizes.
* Full support for the Sv39 virtual memory system, including 4KB, 2MB and 1GB page sizes.
* Full support for the Sv32 virtual memory system, including 4KB and 4MB page sizes.
* Fast exception handling.
* Non-trivial exception handlers.
* Full privilege mode switching support.
* Multiprocess/multithread instruction generation.

To run the output of FORCE-RISCV with the CORE-V-VERIF UVM testbench and to run the Imperas riscvOVPsim simulator get instruction functional coverage, see the sections below: Using riscvOVPsim to report coverage of generated tests.

# FORCE-RISCV Quick Start Guide for RISC-V

## User Manual
The user manual is located in the 'force-riscv/doc' directory.  The user manual is still a "work in progress".

  *  FORCE-RISCV\_User\_Manual-v0.8.docx
  *  FORCE-RISCV\_User\_Manual-v0.8.pdf

## Use FORCE-RISCV to generate a RISC-V testcase (ELF file) from a FORCE-RISCV test template

### Setup
* Clone the FORCE-RISCV repository:
  * The \[base directory\] in the description that follows is the directory from which you execute git clone.
  * `git clone http://path-to-the-repository/force-riscv.git`
  * `cd [base directory]/force-riscv/`
* Set the following environment variables.  FORCE-RISCV development requires gcc 5.1 or higher and Python 3.4.1 or higher version.  The Makefile will make a guess if you don't set them, which should be suitable for modern 64-bit GNU/Linux distributions.
  * `export FORCE_CC=/usr/bin/g++`
  * `export FORCE_PYTHON_VER=3.6`
  * `export FORCE_PYTHON_LIB=/usr/lib/x86_64-linux-gnu/`
  * `export FORCE_PYTHON_INC=/usr/include/python3.6`
* Build FORCE-RISCV
  * Single command
     * Run the setup script, which will perform some rudimentary environment checks, (warning if there are any issues,) and then run the next several commands.
     `./setup`
     * Skip to the section: **ELF file generation running FORCE-RISCV directly**
  * Manual steps
     * `source setenv.bash` if running bash; otherwise `./setenv.bash`
     * `make`
     * `make tests`
     * Run the main regression while in the force-riscv directory.
         `./utils/regression/master_run.py`
     * Run the unit tests while in the force-riscv directory.
         `./utils/regression/unit_tests.py`

### ELF file generation running FORCE-RISCV directly
FORCE-RISCV is run from the Linux/Unix command line, and you can specify command line options that alter how the test generation is run.  You can run this simple "smoke test" to demonstrate that the build was successful.
    `bin/friscv -t utils/smoke/test_force.py`
You can run FORCE-RISCV from any directory; just adjust the relative path names.  For example,
    `cd force-riscv/tmp`
    `../bin/friscv -t ../utils/smoke/test_force.py`
Note that the file name of a FORCE-RISCV test template always ends with "_force.py"; this serves to indicate that the file is a Python script, and that it is a FORCE-RISCV test template.

The FORCE-RISCV program will generate the following files:
    <templateName>.Default.S
    <templateName>.Default.ELF
Which in our example is:
    test_force.Default.S
    test_force.Default.ELF
The *.ELF files can be the input to an RTL simulation environment defining what instructions get executed during the simulation.  The *.S files are assembly code style files to help the user see what instructions have been generated.

Generation can also be run with the master_run.py utility script.  Here is an example command line specifying a *_fctrl.py file.  The *_fctrl.py file contains the path to and name of the template file along with options that get passed to master_run and FORCE-RISCV.
    utils/regression/master_run.py -f examples/riscv/_def_fctrl.py -c utils/regression/config/_riscv_rv64_fcfg.py -k all

Notes on contents of the control file (*_fctrl.py):
    * "--cfg":"config/riscv_rv64.config" is required in the "generator" options for any generation for the RISC-V RV64 architecture.

For example:  <base directory>/force-riscv/tests/riscv/fsuExamples/fsu_basic_fctrl.py

        control_items = [
            { "fname":"basic_random_01_force.py",
              "options":{"max-instr":50000, "iterations":1),
              "generator":{"--cfg": "config/riscv_rv64.config", "--options": "\"MMU=1\""} } ]



Notes on contents of template file (*_force.py):

    An example of a simple FORCE-RISCV template file is at

        <base directory>/force-riscv/examples/riscv/basic_random_01_force.py

    The strings that represent each instruction in the ISA can be found in the file:

        <base directory>/force-riscv/py/DV/riscv/trees/instruction_tree.py

For more details on the FORCE-RISCV test template and the FORCE-RISCV API, refer to the FORCE-RISCV User Manual.

Here are the contents of an example test template:

        from riscv.EnvRISCV import EnvRISCV
        from riscv.GenThreadRISCV import GenThreadRISCV
        from base.Sequence import Sequence

        class MainSequence(Sequence):

            def generate(self, **kargs):
                self.genInstruction("ADD##RISCV")
                self.genInstruction("SRA##RISCV")

        MainSequenceClass = MainSequence
        GenThreadClass = GenThreadRISCV
        EnvClass = EnvRISCV

This template generates an ADD instruction followed by an SRA instruction.  Addressing, register selection and register values are randomized and chosen by FORCE-RISCV automatically.

### Generating with RV32 enabled
FORCE-RISCV generates with RV64 by default. To enable RV32 when running with master_run while in the force-riscv directory, use:

    ./utils/regression/master_run.py -f <control file> -c utils/regression/config/_riscv_rv32_fcfg.py

To run the main regression with RV32 while in the force-riscv directory, use:

    ./utils/regression/master_run.py -f tests/riscv/_rv32_fctrl.py -c utils/regression/config/_riscv_rv32_fcfg.py

To run FORCE-RISCV directly with RV32 while in the force-riscv directory, use:

    ./bin/friscv -t <test_template> --cfg config/riscv_rv32.config

### Output files
The output files can be found in:

    <base directory>/force-riscv/output/regression/basic_random_01_force/00000
    where "basic_random_01_force" is the name of your template file (without the '.py' suffix).

Output Files | Brief Description
------------ | -----------------
fsu_basic_force_*.Default.ELF | The ELF file is a binary file that contains the generated test.  Typically, it is the input to the simulators.
fsu_basic_force_*.Default.S  | The *.S file is a text listing of the disassembled instruction stream and their associated memory addresses.  It can be used as a debug aid.
fpix_sim.log | A text file that contains the disassembled instruction stream, their associated memory addresses and the values of the architected registers as they are read and written in the execution of each instruction in the stream.  It can be used as a debug aid. It is the output of the execution that occurs after generation has been entirely completed.
gen.log | Detailed output from FORCE-RISCV regarding the generation process.
sim.log | A text file that contains the disassembled instruction stream as the instructions are being generated, along with the associated values of the registers and memory read and written by the instructions.  Similar to the fpix_sim.log but sim.log has some additional information and better formatting.

### The process to build FORCE-RISCV handcar_cosim.so
The Handcar cosim shared object (utils/handcar/handcar_cosim.so) is implemented based on Spike ISS (https://github.com/riscv/riscv-isa-sim).  We implemented additional code to package Spike code into a shared object with well-defined C APIs.  Since the build process is slightly involved, we have included a prebuilt handcar_cosim.so inside the utils/handcar directory.

The C header for the APIs can be found at: utils/handcar/handcar_cosim_wrapper.h

The exact command steps to build the handcar_cosim.so yourself are contained in this script: handcar/regenerate_and_build.bash

To build handcar_cosim.so:

    cd handcar
    ./regenerate_and_build.bash

If the regenerate_and_build.bash script fails for some reason, it might be a good idea to execute the commands in there step-by-step.  We intend to improve this process and make it more robust down the road.

# Using riscvOVPsim to report coverage of generated tests
The [riscvOVPsim](https://github.com/riscv-ovpsim/imperas-riscv-tests) free RISC-V simulator from [Imperas](https://www.imperas.com/) includes the capability to report instruction functional coverage when running RISC-V programs.

In this repository there are several [target](target) directories that provide configuration files and run scripts to make it easy to get started.

The [README.md](target/README.md) provides information on downloading and using riscvOVPsim with the targets.

# Using the generated tests with the OpenHW CORE-V-VERIF UVM testbench
Currently we are working on FORCE-RISCV providing support for 32bit RISC-V processors. When this is working we will provide scripts and notes so the generated *.ELF files can work easily with the [CV32E40P](https://github.com/openhwgroup/cv32e40p) RV32IMC core.   The UVM verification environment for CORE-V cores, including the cv32e40p is also in GitHub [here](https://github.com/openhwgroup/core-v-verif).

There is a placeholder target directory: [cv32e40p](target/riscvOVPsim_cv32e40p)

# Welcome to join openhw FORCE-RISCV VTG to track further developement of FORCE-RISCV
Follow the steps to reigister eclipse account:
https://www.openhwgroup.org/register/

Then join FORCE-RISCV VTG mattermost, which is openhw's official instant message communication tool:
https://mattermost.openhwgroup.org/all-users/channels/vtg-force-riscv#

Stage2 development of FORCE-RISCV will be tracked in FORCE-RISCV VTG mattermost.
Welcome to join.
