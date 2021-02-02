#!/bin/bash

# select a test
if [[ ${TEST} == "" ]]; then
    TEST=../../tests/riscv/loop/loop_broad_random_instructions_force.py
fi
STUB=$(basename $TEST | sed 's/.py//')

if [[ ${RISCVOVPSIM} == "" ]]; then
    RISCVOVPSIM=../../../riscv-ovpsim-plus/bin/Linux64/riscvOVPsimPlus.exe
fi

if [[ ${FORCERISCV} == "" ]]; then
    FORCERISCV=../../bin/friscv
fi

# run force-riscv
# use --seed to get same results each run
${FORCERISCV} -t ${TEST} -s 1 | tee friscv.log

# run riscvOVPsim or risvOVPsimPlus
# if using a riscvOVPsim after  v20201109, then finish use: --finishonopcode 0x0000006f
# if using a riscvOVPsim before v20201109, then finish after a number of instructions with --finishafter 3000
# note you need riscvOVPsimPlus for trace options: --trace --tracechange --tracemode --traceshowicount
${RISCVOVPSIM}  \
    "--program ${STUB}.Default.ELF" \
    --controlfile ./riscvOVPsim.ic \
    --addressbits 64 \
    --finishonopcode 0x0000006f \
    --override riscvOVPsim/cpu/simulateexceptions=T \
    --override riscvOVPsim/cpu/defaultsemihost=F \
    --cover basic --showuncovered --extensions RVI,RVM,RVIC --reportfile riscvOVPsim.basic.cover.txt \
    "$@" | tee riscvOVPsim.basic.log
