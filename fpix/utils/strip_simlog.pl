#!/usr/bin/perl -w
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

#------------------------------------------------------------------------
# strip down fpix simulation 'log' (stdout) to allow 'diff's
# between logs...
#------------------------------------------------------------------------

my $core = '';

$core = $ARGV[0] if defined($ARGV[0]);

while(<STDIN>) {
    # print instruction decode from log, and only core#,instr#,address...

    next if ($core ne '') and not /^$core/;

    if (/^c\d+\s+\d+/) {
	print $` . "\n" if /\:/;
    }
}
