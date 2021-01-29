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
# This module is intended to determine a way to distinguish strings from a
# list by taking a sampling of a few characters. In the event it cannot find a
# way to uniquely identify each string by some combination of up to
# _MAX_CHAR_INDEXES characters, it will create a nested mechanism in which
# a sampling of characters is used to divide the strings into groups and
# further samplings are used to further distinguish them until they can be
# identified uniquely.

import itertools
import operator

# Set to cap the number of character combinations attempted, as the number of
# combinations grows exponentially with _MAX_CHAR_INDEXES.
_MAX_CHAR_INDEXES = 3


class NestedHashTable:
    def __init__(self, char_indexes, hash_table):
        self.char_indexes = char_indexes
        self.entries = self._create_entries(hash_table)

    def _create_entries(self, hash_table):
        entries = []

        # Sorting isn't necessary for functionality, but it helps generate
        # patterned output such that case values can easily be written in
        # increasing order, which makes it a little easier to read.
        for (key, values) in sorted(
            hash_table.items(), key=operator.itemgetter(0)
        ):
            entries.append(NestedHashTableEntry(key, values))

        return entries


class NestedHashTableEntry:
    def __init__(self, key, values):
        self._key = key
        self._values = values
        self._inner_hash_table = None

    @property
    def key(self):
        return self._key

    @property
    def values(self):
        return self._values

    @property
    def inner_hash_table(self):
        return self._inner_hash_table

    @inner_hash_table.setter
    def inner_hash_table(self, value):
        self._inner_hash_table = value

    def has_multiple_values(self):
        return len(self._values) > 1

    def get_only_value(self):
        if not self.has_multiple_values():
            return self._values[0]
        else:
            return None


def find_limited_char_nested_hash(enum_values):
    string_values = _get_string_values(enum_values)
    return _generate_nested_hash_table(string_values)


def _generate_nested_hash_table(string_values):
    max_string_length = _get_max_string_length(string_values)

    # Tuple containing (char_indexes, hash_table, duplicate_count) where
    # duplicate_count is ultimately minimized; duplicate_count can't possibly
    # exceed len(string_values).
    min_duplicate_hash = ([], {}, len(string_values))

    for char_index_count in range(1, _MAX_CHAR_INDEXES + 1):

        # The loop below evaluates all possible combinations of
        # char_index_count character indexes; it will halt early if a given
        # combination yields a unique set of hash values.
        for char_indexes in itertools.product(
            range(max_string_length), repeat=(char_index_count)
        ):
            (hash_table, duplicate_count) = _compute_hash_table(
                char_indexes, string_values
            )
            if duplicate_count == 0:
                nested_hash_table = NestedHashTable(char_indexes, hash_table)
                return nested_hash_table
            elif duplicate_count < min_duplicate_hash[2]:
                min_duplicate_hash = (
                    char_indexes,
                    hash_table,
                    duplicate_count,
                )

    nested_hash_table = NestedHashTable(
        min_duplicate_hash[0], min_duplicate_hash[1]
    )
    for table_entry in nested_hash_table.entries:
        if table_entry.has_multiple_values():
            table_entry.inner_hash_table = _generate_nested_hash_table(
                table_entry.values
            )

    return nested_hash_table


def _get_string_values(enum_values):
    return [enum_value[0] for enum_value in enum_values]


def _compute_hash_table(indexes, string_values):
    hash_table = {}
    duplicate_count = 0
    for string_value in string_values:
        length = len(string_value)
        hash = 0

        for i in indexes:
            hash = hash ^ ord(string_value[i % length])

        if hash in hash_table:
            values = hash_table[hash]
            values.append(string_value)
            duplicate_count += 1
        else:
            hash_table[hash] = [string_value]

    return (hash_table, duplicate_count)


def _get_max_string_length(string_values):
    longest_string = max(string_values, key=len)
    return len(longest_string)
