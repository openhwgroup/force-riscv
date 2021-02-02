#!/bin/bash

echo "This is a work in progress, and does not work at present."
echo "when FORCE-RISCV supports RV32 then this will be updated."
exit

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
# note you need riscvOVPsimPlus for trace options: --trace --tracechange --tracemode --traceshowicount
${RISCVOVPSIM}  \
    "--program ${STUB}.Default.ELF" \
    --controlfile ./riscvOVPsim.ic \
    --finishonopcode 0x0000006f \
    --override riscvOVPsim/cpu/simulateexceptions=T \
    --override riscvOVPsim/cpu/defaultsemihost=F \
    --cover basic --showuncovered --extensions RVI,RVM,RVIC --reportfile riscvOVPsim.basic.cover.txt \
    "$@" | tee riscvOVPsim.basic.log
