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
from pathlib import Path

class BuilderFactory:

    def get_rw_objects(self, arch, builder_type):
        return (get_reader(arch, builder_type), get_writer(arch, builder_type))

    def get_reader(self, arch, builder_type):
        if arch == "RISCV":
            if builder_type == "instruction":
                return InstructionReaderRISCV()
            #if builder_type == "register": return RegisterReaderRISCV() TODO enable w/ chris' code
            raise ValueError(builder_type)

        raise ValueError(arch)

    def get_writer(self, arch, builder_type):
        if arch_type == "RISCV":
            if builder_type == "instruction":
                return InstructionWriterRISCV()
            #if builder_type == "register": return RegisterWriterRISCV() TODO enable w/ chris' code
            raise ValueError(builder_type)

        raise ValueError(arch)

class BaseParser:
    def __init__(self, input_files=None, arch="", builder_type="", aggregate=False):
        #ensure input files not empty for parser class
        if input_files is None:
            raise ValueError(input_files)
        else:
            self.mInputFiles = input_files

        #setting aggregate to true will collect all input data and output an individual output file
        self.bAggregate = aggregate
        if self.bAggregate:
            self.sAggOutputFile = '{}_{}_out.xml'.format(arch, builder_type)

        #factory object to instantiate the reader/writer objects from the architecture/builder_type
        builder_factory = BuilderFactory(arch, builder_type)
        try:
            (self.mReader, self.mWriter) = builder_factory.get_rw_objets(arch, builder_type)
        except ValueError as err:
            print("Builder object failed to create, invalid input specified: {}".format(err.args))

        #aggregation of output data for debug or aggregated output mode
        self.mOutputData = []

    def output_name(self, f_name):
        try:
            p = Path(f_name)
            return '{}_out.xml'.format(p.stem)
        except IOError as err:
            print("Invalid input file(s) specified: {}".format(err.args))

    def run(self):
        for f_name in self.mInputFiles:
            reader_data = mReader.read_input(f_name)
            self.mOutputData.update(reader_data)
            #if not aggregate mode, write each individual file
            if not self.bAggregate:
                mWriter.write_output(output_name(f_name), reader_data)

        #if aggregate mode - write all output data to the aggregate output file
        if self.bAggregate:
            mWriter.write_output(self.sAggOutputFile, mOutputData)

#Base classes for reader/writer objects/interfaces
class BaseReader:
    def read_input(self, input_path):
        with open(input_path, 'r') as f_handle:
            return self.read(f_handle)

class BaseWriter:
    def write_output(self, output_path, output_data):
        with open(output_path, 'w') as f_handle:
            self.write(f_handle, output_data)
