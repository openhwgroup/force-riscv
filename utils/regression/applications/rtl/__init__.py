#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0
#  (the "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
# OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from .RtlInit import (
    RtlCmdLineOptions,
    RtlParametersProcessor,
    process_rtl_control_data,
)
from .RtlExecutor import RtlExecutor
from .RtlReporter import RtlReporter

# Command line options for the module
CmdLineOptions = RtlCmdLineOptions

# Tag for the application
Tag = "rtl"

ParametersProcessorClass = RtlParametersProcessor

ProcessControlData = process_rtl_control_data

ExecutorClass = RtlExecutor

ReporterClass = RtlReporter
