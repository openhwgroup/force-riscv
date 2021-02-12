#!/usr/bin/env python3
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

import abc
import collections
import enum_string_hash
import importlib
import os
import string
import sys


class _EnumFileSpec:
    def __init__(
        self,
        input_file_name,
        base_output_dir,
        output_file_name,
        unit_test_output_dir,
    ):
        self._base_output_dir = base_output_dir
        self._input_file_name = input_file_name
        self._output_file_name = output_file_name
        self._unit_test_output_dir = unit_test_output_dir

    @property
    def base_output_dir(self):
        return self._base_output_dir

    @property
    def input_file_name(self):
        return self._input_file_name

    @property
    def output_file_name(self):
        return self._output_file_name

    @property
    def unit_test_output_dir(self):
        return self._unit_test_output_dir


class _EnumSpec:
    def __init__(self, class_name, raw_type, comments, values):
        self._class_name = class_name
        self._raw_type = raw_type
        self._comments = comments
        self._values = values

    @property
    def class_name(self):
        return self._class_name

    @property
    def raw_type(self):
        return self._raw_type

    @property
    def comments(self):
        return self._comments

    @property
    def values(self):
        return self._values


class _CppHeaderGenerator:
    def __init__(self, script_path, file_name):
        self._script_path = script_path
        self._file_name = file_name
        self._header_template = _TemplateLoader.load_template("cpp_header.txt")
        self._declaration_template = (
            _TemplateLoader.get_enum_declaration_template()
        )
        self._indent = " " * 2

    def generate(self, enum_specs):
        declarations = self._generate_declarations(enum_specs)
        return self._header_template.substitute(
            file_name=self._file_name,
            enum_declarations=declarations,
            script_path=self._script_path,
        )

    def _generate_declarations(self, enum_specs):
        declarations = ""
        for enum_spec in enum_specs:
            declarations += self._generate_declaration(enum_spec)

        return declarations[:-1]

    def _generate_declaration(self, enum_spec):
        value_settings = self._generate_value_settings(enum_spec.values)

        declaration_template_mapping = {
            "class_name": enum_spec.class_name,
            "raw_type": enum_spec.raw_type,
            "comments": enum_spec.comments,
            "value_settings": value_settings,
        }

        return self._declaration_template.substitute(
            declaration_template_mapping
        )

    def _generate_value_settings(self, enum_values):
        value_settings = ""
        for enum_value in enum_values:
            value_settings += self._indent * 2 + "%s = %s,\n" % enum_value

        return value_settings[:-1]


class _CppSourceGenerator:
    def __init__(self, script_path, file_name):
        self._script_path = script_path
        self._file_name = file_name
        self._source_template = _TemplateLoader.load_template("cpp_source.txt")
        self._definition_template = (
            _TemplateLoader.get_enum_definition_template()
        )
        self._indent = " " * 2

    def generate(self, enum_specs):
        definitions = self._generate_definitions(enum_specs)
        return self._source_template.substitute(
            file_name=self._file_name,
            enum_definitions=definitions,
            script_path=self._script_path,
        )

    def _generate_definitions(self, enum_specs):
        definitions = ""
        for enum_spec in enum_specs:
            definitions += self._generate_definition(enum_spec)

        return definitions[:-1]

    def _generate_definition(self, enum_spec):
        (
            to_enum_function_body,
            try_to_enum_function_body,
        ) = self._generate_to_enum_function_bodies(enum_spec)
        to_string_cases = self._generate_to_string_cases(enum_spec)

        definition_template_mapping = {
            "class_name": enum_spec.class_name,
            "raw_type": enum_spec.raw_type,
            "comments": enum_spec.comments,
            "enum_size": len(enum_spec.values),
            "to_enum_function_body": to_enum_function_body,
            "try_to_enum_function_body": try_to_enum_function_body,
            "to_string_cases": to_string_cases,
            "default_string_value": enum_spec.values[0][0],
        }

        return self._definition_template.substitute(
            definition_template_mapping
        )

    # Combine generation of string_to_<enum_type> and
    # try_string_to_<enum_type> functions into a single method to avoid the
    # cost of having to generate the nested hash table twice.
    def _generate_to_enum_function_bodies(self, enum_spec):
        nested_hash_table = enum_string_hash.find_limited_char_nested_hash(
            enum_spec.values
        )

        to_enum_function_generator = _CppStringToEnumFunctionGenerator(
            enum_spec, nested_hash_table
        )
        to_enum_function_body = (
            to_enum_function_generator.generate_function_body()
        )

        try_to_enum_function_generator = _CppTryStringToEnumFunctionGenerator(
            enum_spec, nested_hash_table
        )
        try_to_enum_function_body = (
            try_to_enum_function_generator.generate_function_body()
        )

        return (to_enum_function_body, try_to_enum_function_body)

    def _generate_to_string_cases(self, enum_spec):
        to_string_cases = ""
        for enum_value in enum_spec.values:
            to_string_cases += (
                self._indent * 2
                + 'case E%s::%s: return "%s";\n'
                % (enum_spec.class_name, enum_value[0], enum_value[0])
            )

        return to_string_cases[:-1]


class _CppUnitTestGenerator:
    def __init__(self, script_path, file_name):
        self._script_path = script_path
        self._file_name = file_name
        self._unit_test_template = _TemplateLoader.load_template(
            "cpp_unit_test.txt"
        )
        self._unit_test_case_template = (
            _TemplateLoader.get_unit_test_case_template()
        )
        self._indent = " " * 2

    def generate(self, enum_specs):
        unit_test_cases = self._generate_unit_test_cases(enum_specs)
        return self._unit_test_template.substitute(
            file_name=self._file_name,
            unit_test_cases=unit_test_cases,
            script_path=self._script_path,
        )

    def _generate_unit_test_cases(self, enum_specs):
        unit_test_cases = ""
        for enum_spec in enum_specs:
            unit_test_cases += self._generate_unit_test_case(enum_spec)

        return unit_test_cases[:-1]

    def _generate_unit_test_case(self, enum_spec):
        to_string_tests = self._generate_to_string_tests(enum_spec)
        to_enum_tests = self._generate_to_enum_tests(enum_spec)
        to_enum_fail_tests = self._generate_to_enum_fail_tests(enum_spec)
        try_to_enum_tests = self._generate_try_to_enum_tests(enum_spec)
        try_to_enum_fail_tests = self._generate_try_to_enum_fail_tests(
            enum_spec
        )

        unit_test_case_template_mapping = {
            "class_name": enum_spec.class_name,
            "to_string_tests": to_string_tests,
            "to_enum_tests": to_enum_tests,
            "to_enum_fail_tests": to_enum_fail_tests,
            "try_to_enum_tests": try_to_enum_tests,
            "try_to_enum_fail_tests": try_to_enum_fail_tests,
        }

        return self._unit_test_case_template.substitute(
            unit_test_case_template_mapping
        )

    def _generate_to_string_tests(self, enum_spec):
        to_string_tests = ""
        for enum_value in enum_spec.values:
            to_string_tests += (
                self._indent * 3
                + 'EXPECT(E%s_to_string(E%s::%s) == "%s");\n'
                % (
                    enum_spec.class_name,
                    enum_spec.class_name,
                    enum_value[0],
                    enum_value[0],
                )
            )

        return to_string_tests[:-1]

    def _generate_to_enum_tests(self, enum_spec):
        to_enum_tests = ""
        for enum_value in enum_spec.values:
            to_enum_tests += (
                self._indent * 3
                + 'EXPECT(string_to_E%s("%s") == E%s::%s);\n'
                % (
                    enum_spec.class_name,
                    enum_value[0],
                    enum_spec.class_name,
                    enum_value[0],
                )
            )

        return to_enum_tests[:-1]

    def _generate_to_enum_fail_tests(self, enum_spec):
        fail_test_string = self._generate_fail_test_string(
            enum_spec.values[0][0]
        )
        to_enum_fail_tests = (
            self._indent * 3
            + 'EXPECT_THROWS_AS(string_to_E%s("%s"), EnumTypeError);'
            % (enum_spec.class_name, fail_test_string)
        )

        return to_enum_fail_tests

    def _generate_try_to_enum_tests(self, enum_spec):
        try_to_enum_tests = ""
        for enum_value in enum_spec.values:
            try_to_enum_tests += (
                self._indent * 3
                + 'EXPECT(try_string_to_E%s("%s", okay) == E%s::%s);\n'
                % (
                    enum_spec.class_name,
                    enum_value[0],
                    enum_spec.class_name,
                    enum_value[0],
                )
            )

            try_to_enum_tests += self._indent * 3 + "EXPECT(okay);\n"

        return try_to_enum_tests[:-1]

    def _generate_try_to_enum_fail_tests(self, enum_spec):
        fail_test_string = self._generate_fail_test_string(
            enum_spec.values[0][0]
        )
        try_to_enum_fail_tests = (
            self._indent * 3
            + 'try_string_to_E%s("%s", okay);\n'
            % (enum_spec.class_name, fail_test_string)
        )

        try_to_enum_fail_tests += self._indent * 3 + "EXPECT(!okay);"
        return try_to_enum_fail_tests

    # Generate a string that doesn't correspond to any enum value. However,
    # it is intended to be similar enough to a string that does match an enum
    # value such that in many cases, the generated string will hash to one of
    # the case values and fail at the full equality check.
    def _generate_fail_test_string(self, enum_value_name):
        if len(enum_value_name) > 1:
            fail_test_string = enum_value_name[0] + "_" + enum_value_name[2:]
        else:
            fail_test_string = "_" + enum_value_name[1:]

        return fail_test_string


class _CppStringToEnumFunctionGeneratorBase(abc.ABC):
    def __init__(self, enum_spec):
        self._enum_spec = enum_spec
        self._indent = " " * 2

    @property
    @abc.abstractmethod
    def error_statements(self):
        ...

    @property
    def enum_spec(self):
        return self._enum_spec

    @property
    def indent(self):
        return self._indent

    @abc.abstractmethod
    def generate_function_body(self):
        ...

    @abc.abstractmethod
    def generate_validation(self, class_name, string_value, context):
        ...

    def generate_nested_function_body(self, nested_hash_table, context):
        to_enum_function_body = self._generate_hash_computation(
            nested_hash_table.char_indexes, context
        )
        to_enum_function_body += (
            self._indent * (2 + context.indent_level)
            + "switch (hash_value%s) {\n" % context.var_name_suffix
        )

        for table_entry in nested_hash_table.entries:
            to_enum_function_body += (
                self._indent * (2 + context.indent_level)
                + "case %d:\n" % table_entry.key
            )

            if table_entry.has_multiple_values():
                to_enum_function_body += (
                    self._indent * (3 + context.indent_level) + "{\n"
                )
                nested_context = _CppFunctionGenerationContext(
                    context.indent_level + 2, "_%d" % table_entry.key
                )

                to_enum_function_body += self.generate_nested_function_body(
                    table_entry.inner_hash_table, nested_context
                )

                to_enum_function_body += (
                    self._indent * (3 + context.indent_level) + "}\n"
                )
            else:
                # Validate the input string inside the case statement to
                # guard against hash collisions for strings that don't match
                # any of the enum values.
                to_enum_function_body += self.generate_validation(
                    self._enum_spec.class_name,
                    table_entry.get_only_value(),
                    context,
                )

        to_enum_function_body += (
            self._indent * (2 + context.indent_level) + "default:\n"
        )

        for error_statement in self.error_statements:
            to_enum_function_body += (
                self._indent * (3 + context.indent_level) + error_statement
            )

        to_enum_function_body += (
            self._indent * (2 + context.indent_level) + "}\n"
        )

        return to_enum_function_body

    def _generate_hash_computation(self, char_indexes, context):
        hash_computation = self._generate_string_size_expression(
            char_indexes, context
        )

        hash_computation += (
            self._indent * (2 + context.indent_level)
            + "char hash_value%s = " % context.var_name_suffix
        )
        hash_computation += self._generate_char_retrieval_expression(
            char_indexes[0], context
        )

        for char_index in char_indexes[1:]:
            char_retrieval_expression = self._generate_char_retrieval_expression(
                char_index, context
            )
            hash_computation += " ^ " + char_retrieval_expression

        hash_computation += ";\n\n"

        return hash_computation

    def _generate_string_size_expression(self, char_indexes, context):
        if (len(char_indexes) == 1) and (char_indexes[0] == 0):
            string_size_expression = ""
        else:
            string_size_expression = (
                self._indent * (2 + context.indent_level)
                + "size_t size%s = in_str.size();\n" % context.var_name_suffix
            )

        return string_size_expression

    def _generate_char_retrieval_expression(self, char_index, context):
        if char_index == 0:
            char_retrieval_expression = "in_str.at(%d)" % char_index
        else:
            # Use the modulo operator in case the character index is larger
            # than the string size, but not otherwise, as it degrades
            # performance.
            char_retrieval_expression = (
                "in_str.at(%d < size%s ? %d : %d %% size%s)"
                % (
                    char_index,
                    context.var_name_suffix,
                    char_index,
                    char_index,
                    context.var_name_suffix,
                )
            )

        return char_retrieval_expression


class _CppStringToEnumFunctionGenerator(_CppStringToEnumFunctionGeneratorBase):
    def __init__(self, enum_spec, nested_hash_table):
        super().__init__(enum_spec)
        self._nested_hash_table = nested_hash_table
        self._error_statements = [
            "unknown_enum_name(enum_type_name, in_str);\n"
        ]

    @property
    def error_statements(self):
        return self._error_statements

    def generate_function_body(self):
        context = _CppFunctionGenerationContext(0, "")
        to_enum_function_body = self.generate_nested_function_body(
            self._nested_hash_table, context
        )
        return to_enum_function_body[:-1]

    def generate_validation(self, class_name, string_value, context):
        validation = self.indent * (
            3 + context.indent_level
        ) + 'validate(in_str, "%s", enum_type_name);\n' % (string_value)

        validation += self.indent * (
            3 + context.indent_level
        ) + "return E%s::%s;\n" % (class_name, string_value)

        return validation


class _CppTryStringToEnumFunctionGenerator(
    _CppStringToEnumFunctionGeneratorBase
):
    def __init__(self, enum_spec, nested_hash_table):
        super().__init__(enum_spec)
        self._nested_hash_table = nested_hash_table
        self._error_statements = [
            "okay = false;\n",
            "return E%s::%s;\n"
            % (enum_spec.class_name, enum_spec.values[0][0]),
        ]

    @property
    def error_statements(self):
        return self._error_statements

    def generate_function_body(self):
        try_to_enum_function_body = self.indent * 2 + "okay = true;\n"
        context = _CppFunctionGenerationContext(0, "")
        try_to_enum_function_body += self.generate_nested_function_body(
            self._nested_hash_table, context
        )

        return try_to_enum_function_body[:-1]

    def generate_validation(self, class_name, string_value, context):
        validation = (
            self.indent * (3 + context.indent_level)
            + 'okay = (in_str == "%s");\n' % string_value
        )
        validation += self.indent * (
            3 + context.indent_level
        ) + "return E%s::%s;\n" % (class_name, string_value)
        return validation


class _TemplateLoader:
    @classmethod
    def load_template(cls, template_file_name):
        template_dir = "templates"
        with open(
            os.path.join(template_dir, template_file_name)
        ) as template_file:
            template_file_contents = template_file.read()

        return string.Template(template_file_contents)

    @classmethod
    def get_enum_declaration_template(cls):
        import templates.cpp_enum_declaration

        return string.Template(templates.cpp_enum_declaration.template_string)

    @classmethod
    def get_enum_definition_template(cls):
        import templates.cpp_enum_definition

        return string.Template(templates.cpp_enum_definition.template_string)

    @classmethod
    def get_unit_test_case_template(cls):
        import templates.cpp_unit_test_case

        return string.Template(templates.cpp_unit_test_case.template_string)


class _CppFunctionGenerationContext:
    def __init__(self, indent_level, var_name_suffix):
        self._indent_level = indent_level
        self._var_name_suffix = var_name_suffix

    @property
    def indent_level(self):
        return self._indent_level

    @property
    # Used to generate unique names for variables serving similar purposes
    # within the same method.
    def var_name_suffix(self):
        return self._var_name_suffix


def generate_enum_files(app_name):
    enum_file_specs = _create_enum_file_specs(app_name)
    for enum_file_spec in enum_file_specs:
        _generate_enum_file(enum_file_spec)


def _generate_enum_file(enum_file_spec):
    script_path = os.path.join("utils", "enum_classes", "create_enum_files.py")
    enum_specs = _load_enum_specs(enum_file_spec.input_file_name)

    cpp_header_generator = _CppHeaderGenerator(
        script_path, enum_file_spec.output_file_name
    )
    cpp_header_contents = cpp_header_generator.generate(enum_specs)
    cpp_source_generator = _CppSourceGenerator(
        script_path, enum_file_spec.output_file_name
    )
    cpp_source_contents = cpp_source_generator.generate(enum_specs)
    cpp_unit_test_generator = _CppUnitTestGenerator(
        script_path, enum_file_spec.output_file_name
    )
    cpp_unit_test_contents = cpp_unit_test_generator.generate(enum_specs)

    cpp_header_path = os.path.join(
        enum_file_spec.base_output_dir,
        "inc",
        ("%s.h" % enum_file_spec.output_file_name),
    )
    _write_enum_file(cpp_header_path, cpp_header_contents)
    cpp_source_path = os.path.join(
        enum_file_spec.base_output_dir,
        "src",
        ("%s.cc" % enum_file_spec.output_file_name),
    )
    _write_enum_file(cpp_source_path, cpp_source_contents)
    cpp_unit_test_path = os.path.join(
        enum_file_spec.unit_test_output_dir,
        enum_file_spec.output_file_name,
        ("%s_test.cc" % enum_file_spec.output_file_name),
    )
    _write_enum_file(cpp_unit_test_path, cpp_unit_test_contents)


def _create_enum_file_specs(app_name):
    enum_file_specs = []
    force_path = os.path.join("..", "..")
    if app_name == "Force":
        base_enum_file_spec = _EnumFileSpec(
            "base_enum_classes",
            os.path.join(force_path, "base"),
            "Enums",
            os.path.join(force_path, "unit_tests", "tests", "base"),
        )
        enum_file_specs.append(base_enum_file_spec)
        riscv_enum_file_spec = _EnumFileSpec(
            "riscv_enum_classes",
            os.path.join(force_path, "riscv"),
            "EnumsRISCV",
            os.path.join(force_path, "unit_tests", "tests", "riscv"),
        )
        enum_file_specs.append(riscv_enum_file_spec)
    elif app_name == "Fpix":
        base_enum_file_spec = _EnumFileSpec(
            "fpix_base_enum_classes",
            os.path.join(force_path, "fpix"),
            "EnumsFPIX",
            os.path.join(force_path, "unit_tests", "tests", "fpix"),
        )
        enum_file_specs.append(base_enum_file_spec)
    else:
        raise ValueError("Unknown application name %s" % app_name)

    return enum_file_specs


def _load_enum_specs(source_file_name):
    # Import the list used to define the enums
    source_module = importlib.import_module(source_file_name)

    enum_specs = []
    for (
        class_name,
        raw_type,
        comments,
        values,
    ) in source_module.enum_classes_details:
        enum_specs.append(_EnumSpec(class_name, raw_type, comments, values))

    return enum_specs


def _write_enum_file(file_path, file_contents):
    with open(file_path, "w") as enum_file:
        enum_file.write(file_contents)


if __name__ == "__main__":
    app_name = "Force"
    if len(sys.argv) > 1:
        app_name = sys.argv[1]

    generate_enum_files(app_name)
