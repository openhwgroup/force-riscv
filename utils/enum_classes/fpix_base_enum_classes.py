#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
enum_classes_details = [
    ["SimThreadEventType", "unsigned char", "Types of simulator events",
        [("START_TEST", 0), ("END_TEST", 1), ("RESET", 2), ("BOOT_CODE", 3), ("FIRST_INSTRUCTION", 4), ("PRE_STEP", 5), ("STEP", 6), ("POST_STEP", 7), ("MEMORY_UPDATE", 8), ("MMU_EVENT", 9), ("REGISTER_UPDATE", 10), ("DELAY", 11), ("INTERRUPT", 12), ("EXCEPTION_EVENT", 13), ("UNKNOWN_EVENT", 14)]
    ],
    ["OptionIndex", "unsigned char", "Simulator options",
        [("UNKNOWN", 0), ("CFG", 1), ("HELP", 2), ("LOGLEVEL", 3), ("SEED", 4), ("MAX_INSTS_OPT", 5), ("RAILHOUSE_OPT", 6), ("DECODING_OPT", 7), ("CORE_NUM_OPT", 8), ("CLUSTER_NUM_OPT", 9), ("THREADS_PER_CPU_OPT", 10), ("PA_SIZE_OPT", 11), ("EXIT_LOOP_OPT", 12), ("PLUGIN", 13), ("PLUGINS_OPTIONS", 14)]
    ],
    ["OperandType", "unsigned char", "Operand types in the instruction files",
        [("Register", 0), ("GPR", 1), ("VECREG", 2)]
    ],
    ["AccessAgeType", "unsigned char", "Types of resource access age.",
        [("Invalid", 0), ("Read", 1), ("Write", 2)]
    ],
    ["DependencyType", "unsigned char", "Types of resource dependency.",
        [("OnSource", 0), ("OnTarget", 1)]
    ],
]
