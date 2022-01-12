#!/usr/bin/env bash
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

# was this script sourced?
[[ "$0" == "${BASH_SOURCE[0]}" ]] && sourced=False || sourced=True

# was the current environment sourced?
[[ -n "$FORCE_SOURCED" ]] && force_env_sourced=True || force_env_sourced=False

# pick correct exit method
if [ "$sourced" = "True" ]; then
  ret="return"
else
  ret="exit"
fi

CURPATH=$(pwd)
ROOTPATH="${CURPATH}"

# capture current PYTHONPATH before modifying it
if [ -n "$PYTHONPATH" ]; then
  OLD_PYTHONPATH="${PYTHONPATH}"
else
  OLD_PYTHONPATH="False"
fi

declare -a PATHS=(
  "3rd_party/py"
  "utils"
  "utils/builder"
  "utils/builder/test_builder"
  "utils/builder/shared"
  "utils/regression"
)

# already run?
if [ -n "$FORCE_ORIG_PYTHONPATH" ]; then

  # requesting cleanup?
  if [ "$1" = "-u" ]; then

    # is existing env sourced?
    if [ "$force_env_sourced" = "True" ]; then

      # do env reset
      if [ "$FORCE_ORIG_PYTHONPATH" = "False" ]; then
        unset PYTHONPATH
      else
        export PYTHONPATH="$FORCE_ORIG_PYTHONPATH"
      fi
      unset FORCE_ORIG_PYTHONPATH
      unset FORCE_SOURCED

    else
      echo
      echo You are in a FORCE environment subshell
      echo Type 'exit' to exit
      $ret 1
    fi

  # warn already run and exit
  else
    echo "Already run -- FORCE environment already in place"
  fi

# not already run
else
  if [ "$1" = "-u" ]; then
    echo Environment not set, exiting without action
    $ret 2
  fi

  # setup PYTHONPATH
  for val in "${PATHS[@]}"; do
    if [ -n "$PYTHONPATH" ]; then
      PYTHONPATH="$ROOTPATH"/"$val":"$PYTHONPATH"
    else
      PYTHONPATH="$ROOTPATH"/"$val"
    fi
  done
  echo New PYTHONPATH = "${PYTHONPATH}"
  export PYTHONPATH
  export FORCE_ORIG_PYTHONPATH="${OLD_PYTHONPATH}"

  # if not sourced, create new shell with FORCE env in place
  if [ "$sourced" = "False" ]; then
    echo
    echo Creating new shell with FORCE environment
    echo Type 'exit' to exit
    $SHELL
  else
    export FORCE_SOURCED=True
  fi
fi

