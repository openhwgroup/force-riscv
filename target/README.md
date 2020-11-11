# Target directories

riscvOVPsim_rv64gc - directory used as current example of generator and instruction functional coverage

- UVM_cv32e40p - awaiting generator 32bit support
- cv32e40p - awaiting generator 32bit support
- cva6 - awaiting specification of cva6 to be able to configure generator and simulator

# Downloading and using riscvOVPsim to get instruction functional coverage

There are two versions of the free Imperas simulator:

- [riscvOVPsim](https://github.com/riscv-ovpsim/imperas-riscv-tests) - free compliance simulator from github
- [riscvOVPsimPlus](https://www.ovpworld.org/riscvOVPsimPlus/) - free from OVPworld - includes additional development features including trace, debug

Please refer to their instructions for installation and run time features and options.

Download the simulator at the same level as FORCE-RISCV, e.g.:
- force-riscv
- riscv-ovpsim-plus
- riscv-toolchains (pre-compiled ones can be obtained from [Imperas riscv-toolchains](https://github.com/Imperas/riscv-toolchains) for objdump etc.)


# An example run

This assumes you have installed the generator and simulator.


    $ cd target/riscvOVPsim_rv64gc/
    $ ls
    riscvOVPsim.ic            # this is a configuration file to define the device specification
    RUN_riscvOVPsimPlus.sh    # script to run

The run script sets some variables, selects an example test script, then runs the friscv executable.

    $ ./RUN_riscvOVPsimPlus.sh
    
    [notice]Initial seed = 0x1
    [notice]FORCE_PATH = /home/ec2-user/imperas/force-riscv/force-riscv
    [notice]Loading config file: /home/ec2-user/imperas/force-riscv/force-riscv/config/riscv.config ...
    [notice]Generating using test template "../../tests/riscv/loop/loop_broad_random_instructions_force.py"...
    [notice]Simulating instructions during test generation.
    ...
    [notice]{GenSequenceAgent::HandleRequest} HandleRequest is called with : Summary
    [notice]Instruction Summary
    [notice]Default Instructions Generated: 1833
    [notice]Total Instructions Generated: 1833
    
It then runs the Imperas simulator with RV64IMC instruction functional coverage enabled.

    riscvOVPsimPlus (64-Bit) v20201109.0 Open Virtual Platform simulator from www.IMPERAS.com.
    Copyright (c) 2005-2020 Imperas Software Ltd.  Contains Imperas Proprietary Information.
    Licensed Software, All Rights Reserved.
    Visit www.IMPERAS.com for multicore debug, verification and analysis solutions.

    riscvOVPsimPlus started: Mon Nov  9 12:13:29 2020

    Info (OR_OF) Target 'riscvOVPsim/cpu' has object read from 'loop_broad_random_instructions_force.Default.ELF'
    Info (OR_PH) Program Headers:
    Info (OR_PH) Type           Offset             VirtAddr           PhysAddr
    Info (OR_PH)                FileSiz            MemSiz             Flags  Align
    Info (OR_PD) LOAD           0x0000000000000cb8 0x000000007fe674d0 0x000000007fe674d0
    ...
    Info (EXC_FS) opcode match: Stopping at 0x2b3fabcfb28
    Info (ICV_WCR) Writing coverage report riscvOVPsim.basic.cover.txt
    Info ---------------------------------------------------
    Info TOTAL INSTRUCTION COVERAGE : RVI,RVM,RVIC
    Info   Threshold             : 1
    Info   Instructions counted  : 5315
    Info   Unique instructions   : 46/95 :  48.42%
    Info   Coverage points hit   : 2124/5532 :  38.39%
    Info ---------------------------------------------------
    Info
    Info ---------------------------------------------------
    Info CPU 'riscvOVPsim/cpu' STATISTICS
    Info   Type                  : riscv (RVB64I+MAFDCSU)
    Info   Nominal MIPS          : 100
    Info   Final program counter : 0x2b3fabcfb28
    Info   Simulated instructions: 5,328
    Info   Simulated MIPS        : run too short for meaningful result
    Info ---------------------------------------------------
    Info
    Info ---------------------------------------------------
    Info SIMULATION TIME STATISTICS
    Info   Simulated time        : 0.00 seconds
    Info   User time             : 0.04 seconds
    Info   System time           : 0.00 seconds
    Info   Elapsed time          : 0.04 seconds
    Info ---------------------------------------------------

    riscvOVPsimPlus finished: Mon Nov  9 12:13:29 2020


    riscvOVPsimPlus (64-Bit) v20201109.0 Open Virtual Platform simulator from www.IMPERAS.com.
    Visit www.IMPERAS.com for multicore debug, verification and analysis solutions.

Note that the run script sets a fixed seed so runs are reproducible.

Please read the FORCE-RISCV user guide to configure and select different tests.

Please read the riscvOVPsim/riscvOVPsimPlus user guide for all the different coverage options.

If you are using the riscvOVPsimPlus simulator you can get full instructions trace and disassembly by adding to the simulator run command in the run script:

    --trace --tracechange --tracemode --traceshowicount

For example:

    Info 4509: 'riscvOVPsim/cpu', 0x000000007ff034cc(text1+3c): Machine 0056d493 srli    s1,a3,0x5
    Info   s1 fffffffffffff890 -> 0000000000000000
    Info 4510: 'riscvOVPsim/cpu', 0x000000007ff034d0(text1+40): Machine 00d311b3 sll     gp,t1,a3
    Info   gp 0000000000000000 -> 000000001ea8d5b2
    Info 4511: 'riscvOVPsim/cpu', 0x000000007ff034d4(text1+44): Machine c70dcf93 xori    t6,s11,-912
    Info   t6 0000000000000000 -> fffffffffffffc70





