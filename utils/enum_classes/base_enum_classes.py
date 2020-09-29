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
    ["LimitType", "unsigned char", "Various limitation values",
     [("ThreadsLimit", 0), ("CoresLimit", 1), ("ChipsLimit", 2), ("PhysicalAddressLimit", 3), ("MaxInstructions", 4), ("PerMonRegisterNumber", 5), ("DependencyHistoryLimit", 6),
      ("BranchNotTakenLimit", 7), ("SpeculativeBntLevelLimit", 8), ("MaxPhysicalVectorLen", 9),  ("ErrRegisterNumber", 10), ("SpeculativeBntInstructionLimit", 11)]
    ],
    ["OperandType", "unsigned char", "Operand types in the instruction files",
     [("Constant", 0), ("Immediate", 1), ("Choices", 2), ("Register", 3), ("GPR", 4), ("GPRSP", 5), ("FPR", 6), ("SIMDSR", 7), ("SIMDVR", 8), ("SysReg", 9), ("Branch", 10), ("LoadStore", 11), ("AuthBranch", 12), ("AuthLoadStore", 13), ("SystemOp", 14), ("VECREG", 15), ("PREDREG", 16), ("ALU", 17), ("DataProcessing", 18), ("VectorLayout", 19)]
    ],
     ["MemDataType", "unsigned char", "Memory data types in memory model",
      [("Init", 1),("Instruction", 2), ("Data", 4), ("Both", 6)]
     ],
     ["RegAttrType", "unsigned char", "Register attribute types in register file",
      [("Read", 1<<0), ("Write", 1<<1), ("ReadWrite", 3), ("HasValue", 1<<2), ("Unpredictable", 1<<3), ("UpdatedFromISS", 1<<4)]
     ],
     ["MemAccessType", "unsigned char", "Memory access types",
      [("Unknown", 0), ("Read", 1), ("Write", 2), ("ReadWrite", 3)]
     ],
     ["GenAgentType", "unsigned char", "Generator agent types",
      [("GenInstructionAgent", 0), ("GenSequenceAgent", 1), ("GenVirtualMemoryAgent", 2), ("GenQueryAgent", 3), ("GenStateAgent", 4), ("GenExceptionAgent", 5), ("GenCallBackAgent", 6), ("GenStateTransitionAgent", 7)]
      ],
     ["ChoicesType", "unsigned char", "Choices types",
      [("OperandChoices", 0), ("RegisterFieldValueChoices", 1), ("PagingChoices", 2), ("GeneralChoices", 3), ("DependenceChoices", 4)]
      ],
     ["RegisterType", "unsigned char", "Register types in the register files",
      [("GPR", 0), ("FPR", 1), ("SIMDR", 2), ("SIMDVR", 3), ("VECREG", 4), ("PREDREG", 5), ("SysReg", 6), ("SP", 7), ("ZR", 8), ("PC", 9), ("Internal", 10)]
     ],
     ["ConstraintType", "unsigned char", "Constraint types",
      [("Value", 0), ("Range", 1)]
      ],
     ["ConstraintResultType", "unsigned char", "Constraint operation result types",
      [("Consumed", 0), ("Replace", 1), ("Remove", 2)]
     ],
     ["ConstraintMoveDirection", "unsigned char", "In ConstraintSet assembling, the direction toward which to move involved Constraints.",
      [("Shrink", 0), ("Expand", 1)]
     ],
     ["TranslationResultType", "unsigned char", "Address translation result type",
       [("NotMapped", 0), ("Mapped", 1), ("AddressError", 2)]
      ],
     ["VmStateType", "unsigned char", "Vm State types",
      [("Uninitialized", 0), ("Invalid", 1), ("Initialized", 2), ("Active", 3)]
      ],
     ["VmStateBits", "unsigned char", "Vm State Bit Masks",
      [("ActiveMask", 1), ("InitMask", 2)]
      ],
     ["AddrModeStateType", "unsigned char", "Addressing mode state types",
      [("Free", 1<<0), ("Mapped", 1<<1)]
      ],
     ["SequenceType", "unsigned char", "Sequence types",
      [("BootLoading", 0), ("EndOfTest", 1), ("Summary",2), ("LoadRegister", 3), ("CommitInstruction", 4), ("JumpToStart", 5), ("BranchToTarget", 6), ("InitialSetup", 7), ("RegisterReservation", 8), ("EscapeCollision", 9), ("BranchNotTaken", 10), ("BntNode", 11), ("ReExecution", 12), ("UpdatePeState", 13), ("UpdateRegisterField", 14), ("SetRegister", 15), ("ConfirmSpace", 16), ("ReloadRegister", 17), ("BatchReloadRegisters", 18), ("SpeculativeBntNode", 19), ("ThreadSummary", 20), ("InitializeAddrTables", 21), ("LoadLargeRegister", 22), ("BeginRestoreLoop", 23), ("EndRestoreLoop", 24), ("RestoreLoopState", 25), ("LoopReconverge", 26)]
      ],
     ["CallBackType", "unsigned char", "CallBack types",
      [("Bnt", 0), ("Eret", 1)]
      ],
     ["ExtendType", "unsigned char", "Extend types",
      [("UXTW", 0), ("LSL", 1), ("SXTW", 2), ("SXTX", 3), ("UXTX", 4)]
      ],
     ["VmRequestType", "unsigned char", "Virtual memory request types",
      [("GenPA", 0), ("GenVA", 1), ("GenVAforPA", 2), ("GenPage", 3), ("GenFreePage", 4), ("PhysicalRegion", 5), ("GenVMVA",6), ("GenVmContext",7), ("UpdateVm",8)]
      ],
     ["InstrBoolAttrType", "unsigned char", "Instruction boolean attribute types",
      [("NoSkip", 0), ("NoRestriction", 1), ("UnalignedPC", 2), ("AlignedData", 3), ("AlignedSP", 4), ("NoBnt", 5), ("NoPreamble", 6), ("SpeculativeBnt", 7), ("NoDataAbort", 8), ("SharedTarget", 9)]
      ],
     ["InstrConstraintAttrType", "unsigned char", "Instruction constraint attribute types",
      [("LSTarget", 0), ("BRTarget", 1), ("CondTaken", 2)]
      ],
     ["BntAttributeType", "unsigned char", "BNT attribute types",
      [("Taken", 0), ("Conditional", 1), ("Accurate", 2)]
      ],
     ["AluOperationType", "unsigned char", "ALU operation type",
      [("Unknown", 0), ("ADD", 1), ("SUB", 2)]
      ],
     ["DataProcessingOperationType", "unsigned char", "Data processing operation type",
      [("Unknown", 0), ("MulAdd", 1), ("Mul", 2), ("UDiv", 3), ("SDiv", 4), ("AddWithCarry", 5), ("SubWithCarry", 6), ("AndShift32", 7), ("AndShift64", 8), 
      ("BicClearShift32", 9), ("BicClearShift64", 10), ("EonExcOrNotShift32", 11), ("EonExcOrNotShift64", 12), ("EorExcOrShift32", 13), ("EorExcOrShift64", 14),
      ("OrrIncOrShift32", 15), ("OrrIncOrShift64", 16), ("MvnNotShift32", 17), ("MvnNotShift64", 18), ("OrnIncOrNotShift32", 19), ("OrnIncOrNotShift64", 20), 
      ("MsubMulSub32", 21), ("MsubMulSub64", 22), ("MnegMulNot32", 23), ("MnegMulNot64", 24), ("SmaddlSigMulAddLong", 25), ("SmsublSigMulSubLong", 26), 
      ("SmneglSigMulNotLong", 27), ("SmullSigMulLong", 28), ("SmulhSigMulHigh", 29), ("UmaddlMulAddLong", 30 ), ("UmsublMulSubLong", 31), 
      ("UmneglMulNotLong", 32), ("UmullMulLong", 33), ("UmulhMulHigh", 34), ("AndImm32", 35), ("AndImm64", 36), ("EorExcOrImm32", 37), ("EorExcOrImm64", 38), 
      ("OrrIncOrImm32", 39), ("OrrIncOrImm64", 40), ("AddExt32", 41), ("AddExt64", 42), ("AddShift32", 43), ("AddShift64", 44), ("SubExt32", 45), 
      ("SubExt64", 46), ("SubShift32", 47), ("SubShift64", 48), ("AddsAddSetFlagsExt32", 49), ("AddsAddSetFlagsExt64", 50 ), ("AddsAddSetFlagsShift32", 51), 
      ("AddsAddSetFlagsShift64", 52), ("SubsSubSetFlagsExt32", 53), ("SubsSubSetFlagsExt64", 54), ("SubsSubSetFlagsShift32", 55), 
      ("SubsSubSetFlagsShift64", 56), ("NegNegateShift32", 57), ("NegNegateShift64", 58), ("NegsNegateSetFlagsShift32", 59), ("NegsNegateSetFlagsShift64", 60),
      ("NgcNegateWithCarry32", 61), ("NgcNegateWithCarry64", 62), ("NgcsNegateWithCarrySetFlags32", 63), ("NgcsNegateWithCarrySetFlags64", 64), 
      ("AndsAndSetFlagsShift32", 65), ("AndsAndSetFlagsShift64", 66), ("AndsAndSetFlagsImm32", 67), ("AndsAndSetFlagsImm64", 68), 
      ("BicsClearSetFlagsShift32", 69), ("BicsClearSetFlagsShift64", 70)]
      ],
     ["PteType", "unsigned char", "Page table entry types",
      [("P4K", 0), ("P16K", 1), ("P64K", 2), ("P2M", 3), ("P32M", 4), ("P512M", 5), ("P1G", 6), ("P512G", 7)]
      ],
     ["PteCategoryType", "unsigned char", "Page table entry category types",
      [("Page", 0), ("Table", 1)]
      ],
     ["NotificationType", "unsigned char", "Notification types",
      [("RegisterUpdate", 0), ("RegisterInitiation", 1),  ("ChoiceUpdate", 2), ("VariableUpdate", 3), ("ConditionUpdate", 4), ("PhysicalRegionAdded", 5), ("PCUpdate", 6)]
      ],
     ["PageRequestAttributeType", "unsigned char", "GenPageRequest attributes for configuring page generation",
      [("VA", 0), ("IPA", 1), ("PA", 2), ("PageSize", 3), ("MemAttrArch", 4), ("MemAttrImpl", 5), ("AliasPageId", 6)]
      ],
     ["PageGenAttributeType", "unsigned char", "Paging attribute of generated page that is useful in later test generation.",
      [("MemAttrImpl", 0), ("Invalid", 1), ("AddrSizeFault", 2), ("DataAccessPermission", 3), ("InstrAccessPermission", 4)]
      ],
     ["DataAccessPermissionType", "unsigned char", "Data access permission types.",
      [("NoAccess", 0), ("ReadWrite", 1), ("ReadOnly", 2), ("ReadWriteNoUser", 3), ("ReadOnlyNoUser", 4), ("ReadWriteUserOnly", 5), ("ReadOnlyUserOnly", 6)]
      ],
     ["InstrAccessPermissionType", "unsigned char", "Instruction access permission types.",
      [("Execute", 0), ("NoExecute", 1), ("PrivilegedNoExecute", 2)]
      ],
     ["GlobalStateType", "unsigned char", "Global state types",
      [("ResetPC", 0), ("PageTableRegionSize", 1), ("PageTableRegionAlign", 2), ("PageTableRegionStart", 3), ("MemoryFillPattern", 4), ("ElfMachine", 5) ]
      ],
     ["AddrType", "unsigned char", "Address types",
      [("DataAddr", 1), ("InstructionAddr", 2) ]
      ],
     ["BootElementActionType", "unsigned char", "Boot loading element action types",
      [("LoadRegister", 0), ("InstructionBarrier", 1), ("DataBarrier", 2), ("LoadLargeRegister", 3)]
      ],
     ["QueryType", "unsigned char", "Query types",
      [("RegisterIndex", 0), ("RegisterReloadValue", 1), ("InstructionRecord", 2), ("RegisterInfo", 3), ("GenState", 4), ("PageInfo", 5), ("BranchOffset", 6), ("RegisterFieldInfo", 7), ("ChoicesTreeInfo", 8), ("SimpleExceptionsHistory", 9), ("AdvancedExceptionsHistory", 10), ("MaxAddress", 11), ("ValidAddressMask", 12), ("HandlerSetMemory", 13), ("ExceptionVectorBaseAddress", 14), ("ResourceEntropy", 15), ("SoftwareStepPrivLevs", 16), ("SoftwareStepReady", 17), ("PickedValue", 18), ("GetVmContextDelta", 19), ("RestoreLoopContext", 20), ("GenData", 21), ("GetVmCurrentContext", 22)]
      ],
     ["GenModeType", "unsigned int", "Generator mode types",
      [("NoIss", 1<<0), ("SimOff", 1<<1), ("NoEscape", 1<<2), ("NoJump", 1<<3), ("ReExe", 1<<4), ("Exception", 1<<5), ("NoSkip", 1<<6), ("InLoop", 1<<7), ("DelayInit", 1<<8), ("LowPower", 1 << 9), ("Filler", 1<< 10), ("Speculative", 1 << 11), ("AddressShortage", 1 << 12), ("RecordingState", 1 << 13), ("RestoreStateLoop", 1 << 14)]
      ],
     ["GenStateType", "unsigned char", "Generator state types",
      [("GenMode", 0), ("PC", 1), ("InitialPC", 2), ("BootPC", 3), ("EL", 4), ("Loop", 5), ("CPSR", 6), ("LinearBlock", 7), ("LastPC", 8), ("BntHook", 9), ("PostLoopAddress", 10), ("LoopReconvergeAddress", 11), ("PrivilegeLevel", 12), ("Endianness", 13)]
      ],
     ["GenStateActionType", "unsigned char", "Action types for generator states",
      [("Push", 0), ("Pop", 1), ("Set", 2), ("Enable", 3), ("Disable", 4)]
      ],
     ["GenExceptionDetatilType", "unsigned char", "Generator exception detail types",
      [("RegisterNotInitSetValue", 0)]
      ],
     ["ExceptionRequestType", "unsigned char", "Types of exception related request.",
      [("HandleException", 0), ("SystemCall", "1"), ("UpdateHandlerInfo", "2")]
      ],
     ["ExceptionEventType", "unsigned char", "Types of exception events.",
      [("ExceptionReturn", 0)]
      ],
     ["ExceptionConstraintType", "unsigned char", "Types of exception constraint.",
      [("Allow", 0), ("Prevent", 1), ("Trigger", 2), ("PreventHard", 3), ("TriggerHard", 4), ("Invalid", 5)]
      ],
     ["PhysicalRegionType", "unsigned char", "Types of physical memory regions.",
      [("HandlerMemory", 0), ("ExceptionStack", 1), ("PageTable", 2), ("BootRegion", 3), ("AddressTable", 4), ("ResetRegion", 5)]
      ],
      ["DataType", "unsigned char", "Data Type to override operand data",
      [("INT8", 0x10), ("INT16", 0x11), ("INT32", 0x12), ("INT64", 0x13),
       ("FIX8", 0x20), ("FIX16", 0x21), ("FIX32", 0x22),  ("FIX64", 0x23), 
       ("FP8", 0x40), ("FP16", 0x41), ("FP32", 0x42), ("FP64", 0x43)]
      ],
      ["DumpType", "unsigned char", "Types of dump options.",
      [("Asm", 0), ("Elf", 1), ("Mem", 2), ("Page", 3), ("FailOnly", 4), ("Handlers", 5)]
      ],
     ["ResourceType", "unsigned char", "Types of resource for dependency.",
      [("GPR", 0), ("FPR", 1), ("PREDREG", 2)]
      ],
     ["DependencyType", "unsigned char", "Types of resource dependency.",
      [("OnSource", 0), ("OnTarget", 1), ("NoDependency", 2)]
      ],
     ["AccessAgeType", "unsigned char", "Types of resource access age.",
      [("Invalid", 0), ("Read", 1), ("Write", 2)]
      ],
     ["EntropyStateType", "unsigned char", "Types of entropy state.",
      [("WarmUp", 0), ("Stable", 1), ("CoolDown", 2)]
      ],
     ["VariableType", "unsigned char", "Types of variables.",
      [("Choice", 0), ("Value", 1), ("String", 2)]
      ],
     ["AddressSolutionFilterType", "unsigned char", "Type of AddressSolutionFilters.",
      [("BaseDependency", 0), ("IndexDependency", 1), ("SpAlignment", 2)]
      ],
     ["CallBackTemplateType", "unsigned char", "Types of functions calling back template.",
      [("SetBntSeq", 0), ("RunBntSeq", 1), ("SetEretPreambleSeq", 2), ("RunEretPreambleSeq", 3) ]
      ],
     ["RegReserveType", "unsigned char", "Register types to reserve register",
      [("User", 0 ), ("Exception", 1), ("Unpredictable", 2)]
     ],
     ["DataAlignedType", "unsigned char", "Types of data alignment",
      [("SingleDataAligned", 0), ("Unaligned", 1), ("WholeDataAligned", 2)]
     ],
     ["SpeculativeBntActionType", "unsigned char", "Types of Speculative Bnt Action",
      [("Execute", 0 ), ("Restore", 1), ("Pop", 2)]
      ],
     ["ResourcePeStateType", "unsigned char", "Types of Resource Pe State",
      [("RegisterPeState", 0 ), ("MemoryPeState", 1), ("DependencePeState", 2), ("ExceptionPeState", 3)]
      ],
     ["ReloadingMethodType", "unsigned char", "Types of reloading method",
      [("Move", 0), ("Load", 1)]
     ],
    ["SpAlignedType", "unsigned char", "Types of sp alignment",
     [("Aligned", 0), ("Unaligned", 1)]
    ],
    ["RestoreExclusionGroup", "unsigned char", "Categories of PE state to restore",
     [("GPR", 0), ("SIMDFP", 1), ("VECREG", 2), ("System", 3), ("Memory", 4)]
    ],
    ["SchedulingState", "unsigned char", "Scheduling state types",
     [("Random", 0), ("Finishing", 1), ("Locked", 2)]
    ],
    ["RestoreGroup", "unsigned char", "Categories of PE state to restore",
     [("GPR", 0), ("VECREG", 1), ("PREDREG", 2), ("System", 3), ("Memory", 4)]
    ],
    ["PartitionThreadPolicy", "unsigned char", "Partition thread group policy",
     [("Random", 0), ("SameCore", 1), ("SameChip", 2), ("DiffChip", 3), ("DiffCore", 4)]
    ],
    ["LargeConstraintSetState", "unsigned char", "State of a LargeConstraintSet object",
     [("Clean", 0), ("AddCached", 1), ("SubCached", 2)]
    ],
    ["AddressReuseType", "unsigned char", "Types of address reuse",
     [("ReadAfterRead", 1), ("ReadAfterWrite", 2), ("WriteAfterRead", 4), ("WriteAfterWrite", 8)]
    ],
    ["StateElementDuplicateMode", "unsigned char", "Mode determining how to treat duplicate or repeated StateElements",
     [("Fail", 0), ("Replace", 1), ("Ignore", 2)]
    ],
    ["StateTransitionType", "unsigned char", "Type of StateTransition",
     [("Boot", 0), ("Explicit", 1)]
    ],
    ["StateTransitionOrderMode", "unsigned char", "Mode determining the order in which StateElements are processed",
     [("UseDefault", 0), ("AsSpecified", 1), ("ByStateElementType", 2), ("ByPriority", 3)]
    ],
    ["StateElementType", "unsigned char", "Types of StateElements for capturing system State",
     [("Memory", 0), ("SystemRegister", 1), ("VectorRegister", 2), ("GPR", 3), ("VmContext", 4), ("PrivilegeLevel", 5), ("PC", 6), ("FloatingPointRegister", 7), ("PredicateRegister", 8)]
    ],
    ["Endianness", "unsigned char", "Byte order",
     [("LittleEndian", 0), ("BigEndian", 1)]
    ],
]
