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
# -*- coding: utf-8 -*-
"""Tests for `file_read_backwards` module."""

import itertools
import os
import tempfile
import unittest

from collections import deque

from file_read_backwards.file_read_backwards import FileReadBackwards
from file_read_backwards.file_read_backwards import supported_encodings
from file_read_backwards.buffer_work_space import new_lines


# doing this xrange/range dance so that we don't need to add additional dependencies of future or six modules
try:
    xrange
except NameError:
    xrange = range


created_files = set()


def helper_write(t, s, encoding="utf-8"):
    """A helper method to write out string s in specified encoding."""
    t.write(s.encode(encoding))


def helper_create_temp_file(generator=None, encoding='utf-8'):
    global created_files
    if generator is None:
        generator = ("line {}!\n".format(i) for i in xrange(42))
    temp_file = tempfile.NamedTemporaryFile(delete=False)
    for line in generator:
        helper_write(temp_file, line, encoding)
    temp_file.close()
    print('Wrote file {}'.format(temp_file.name))
    created_files.add(temp_file)
    return temp_file


def helper_destroy_temp_file(temp_file):
    temp_file.close()
    os.unlink(temp_file.name)


def helper_destroy_temp_files():
    global created_files
    while created_files:
        helper_destroy_temp_file(created_files.pop())


def tearDownModule():
    helper_destroy_temp_files()


class TestFileReadBackwards(unittest.TestCase):

    """Class that contains various test cases for actual FileReadBackwards usage."""
    @classmethod
    def setUpClass(cls):
        cls.empty_file = helper_create_temp_file(generator=(_ for _ in []))
        cls.long_file = helper_create_temp_file()

    @classmethod
    def tearDownClass(cls):
        helper_destroy_temp_files()

    def test_with_completely_empty_file(self):
        """Test with a completely empty file."""
        f = FileReadBackwards(self.empty_file.name)
        expected_lines = deque()
        lines_read = deque()
        for l in f:
            lines_read.appendleft(l)
        self.assertEqual(expected_lines, lines_read)

    def test_file_with_a_single_new_line_char_with_different_encodings(self):
        """Test a file with a single new line character."""
        for encoding, new_line in itertools.product(supported_encodings, new_lines):
            temp_file = helper_create_temp_file((l for l in [new_line]), encoding=encoding)
            f = FileReadBackwards(temp_file.name)
            expected_lines = deque([""])
            lines_read = deque()
            for l in f:
                lines_read.appendleft(l)
            self.assertEqual(
                expected_lines,
                lines_read,
                msg="Test with {0} encoding with {1!r} as newline".format(encoding, new_line))

    def test_file_with_one_line_of_text_with_accented_char_followed_by_a_new_line(self):
        """Test a file with a single line of text with accented char followed by a new line."""
        b = b'Caf\xc3\xa9'  # accented e in utf-8
        s = b.decode("utf-8")
        for new_line in new_lines:
            temp_file = helper_create_temp_file((l for l in [s, new_line]))
            f = FileReadBackwards(temp_file.name)
            expected_lines = deque([s])
            lines_read = deque()
            for l in f:
                lines_read.appendleft(s)
            self.assertEqual(expected_lines, lines_read, msg="Test with {0!r} as newline".format(new_line))

    def test_file_with_one_line_of_text_followed_by_a_new_line_with_different_encodings(self):
        """Test a file with just one line of text followed by a new line."""
        for encoding, new_line in itertools.product(supported_encodings, new_lines):
            temp_file = helper_create_temp_file((l for l in ["something{0}".format(new_line)]), encoding=encoding)
            f = FileReadBackwards(temp_file.name)
            expected_lines = deque(["something"])
            lines_read = deque()
            for l in f:
                lines_read.appendleft(l)
            self.assertEqual(
                expected_lines,
                lines_read,
                msg="Test with {0} encoding with {1!r} as newline".format(encoding, new_line))

    def test_file_with_varying_number_of_new_lines_and_some_text_in_chunk_size(self):
        """Test a file with varying number of new lines and text of size custom chunk_size."""
        chunk_size = 3
        s = "t"
        for number_of_new_lines in xrange(21):
            for new_line in new_lines:  # test with variety of new lines
                temp_file = helper_create_temp_file((l for l in [new_line * number_of_new_lines, s * chunk_size]))
                f = FileReadBackwards(temp_file.name, chunk_size=chunk_size)
                expected_lines = deque()
                for _ in xrange(number_of_new_lines):
                    expected_lines.append("")
                expected_lines.append(s * chunk_size)
                lines_read = deque()
                for l in f:
                    lines_read.appendleft(l)
                self.assertEqual(
                    expected_lines,
                    lines_read,
                    msg="Test with {0} of new line {1!r} followed by {2} of {3!r}".format(number_of_new_lines, new_line,
                                                                                          chunk_size, s))

    def test_file_with_new_lines_and_some_accented_characters_in_chunk_size(self):
        """Test a file with many new lines and a random text of size custom chunk_size."""
        chunk_size = 3
        b = b'\xc3\xa9'
        s = b.decode("utf-8")
        for number_of_new_lines in xrange(21):
            for new_line in new_lines:  # test with variety of new lines
                temp_file = helper_create_temp_file((l for l in [new_line * number_of_new_lines, s * chunk_size]))
                f = FileReadBackwards(temp_file.name, chunk_size=chunk_size)
                expected_lines = deque()
                for _ in xrange(number_of_new_lines):
                    expected_lines.append("")
                expected_lines.append(s * chunk_size)
                lines_read = deque()
                for l in f:
                    lines_read.appendleft(l)
                self.assertEqual(
                    expected_lines,
                    lines_read,
                    msg="Test with {0} of new line {1!r} followed by {2} of \\xc3\\xa9".format(number_of_new_lines,
                                                                                               new_line, chunk_size))

    def test_unsupported_encoding(self):
        """Test when users pass in unsupported encoding, NotImplementedError should be thrown."""
        with self.assertRaises(NotImplementedError):
            _ = FileReadBackwards(self.empty_file.name, encoding="not-supported-encoding")  # noqa: F841

    def test_file_with_one_line_of_text_readline(self):
        """Test a file with a single line of text followed by a new line."""
        s = "Line0"
        for new_line in new_lines:
            temp_file = helper_create_temp_file((l for l in [s, new_line]))
            with FileReadBackwards(temp_file.name) as fp:
                l = fp.readline()
                expected_line = s + os.linesep
                self.assertEqual(l, expected_line)

                # the file contains only 1 line
                second_line = fp.readline()
                expected_second_line = ""
                self.assertEqual(second_line, expected_second_line)

    def test_file_with_two_lines_of_text_readline(self):
        """Test a file with a two lines of text followed by a new line."""
        line0 = "Line0"
        line1 = "Line1"
        for new_line in new_lines:
            line0_with_n = "{}{}".format(line0, new_line)
            line1_with_n = "{}{}".format(line1, new_line)
            temp_file = helper_create_temp_file((l for l in [line0_with_n, line1_with_n]))
            with FileReadBackwards(temp_file.name) as fp:
                l = fp.readline()
                expected_line = line1 + os.linesep
                self.assertEqual(l, expected_line)

                second_line = fp.readline()
                expected_second_line = line0 + os.linesep
                self.assertEqual(second_line, expected_second_line)

                # EOF
                third_line = fp.readline()
                expected_third_line = ""
                self.assertEqual(third_line, expected_third_line)


class TestFileReadBackwardsAsContextManager(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp_file = helper_create_temp_file()

    @classmethod
    def tearDownClass(cls):
        helper_destroy_temp_files()

    def test_behaves_as_classic(self):
        with FileReadBackwards(self.temp_file.name) as f:
            lines_read = deque()
            for l in f:
                lines_read.appendleft(l)
        f2 = FileReadBackwards(self.temp_file.name)
        lines_read2 = deque()
        for l2 in f2:
            lines_read2.appendleft(l2)
        self.assertEqual(
            lines_read,
            lines_read2,
            msg="The Context Manager way should behave exactly the same way as without using one."
        )


class TestFileReadBackwardsCloseFunctionality(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp_file = helper_create_temp_file()

    @classmethod
    def tearDownClass(cls):
        helper_destroy_temp_files()

    def test_close_on_iterator(self):
        with FileReadBackwards(self.temp_file.name) as f:
            it = iter(f)
            for count, i in enumerate(it):
                if count == 2:
                    break
            self.assertFalse(it.closed, msg="The fp should not be closed when not exhausted")
            it.close()
            self.assertTrue(it.closed, msg="Calling close() on the iterator should close it")

    def test_not_creating_new_iterator(self):
        with FileReadBackwards(self.temp_file.name) as f:
            it1 = iter(f)
            it2 = iter(f)
            self.assertTrue(it1 is it2, msg="FileReadBackwards will return the same iterator")

    def test_close_on_iterator_exhausted(self):
        with FileReadBackwards(self.temp_file.name) as f:
            it = iter(f)
            for _ in it:
                pass
            self.assertTrue(it.closed, msg="The fp should be closed automatically when the iterator is exhausted.")

    def test_close_on_reader_exit(self):
        with FileReadBackwards(self.temp_file.name) as f:
            it = iter(f)
        self.assertTrue(it.closed,
                        msg="Iterator created by a reader should have its fp closed when the reader gets closed.")

    def test_close_on_reader_explicitly(self):
        f = FileReadBackwards(self.temp_file.name)
        it = iter(f)
        self.assertFalse(it.closed, msg="Iterator should not have its fp closed at this point.")
        f.close()
        self.assertTrue(it.closed,
                        msg="Iterator created by a reader should have its fp closed when the reader closes it.")

    def test_close_on_reader_with_already_closed_iterator(self):
        with FileReadBackwards(self.temp_file.name) as f:
            it = iter(f)
            it.close()
        self.assertTrue(it.closed, msg="It should be okay to close (through the reader) an already closed iterator.")

    def test_cannot_iterate_when_closed(self):
        with FileReadBackwards(self.temp_file.name) as f:
            it = iter(f)
            it.close()
            for _ in it:
                self.fail(msg="An iterator should be exhausted when closed.")
