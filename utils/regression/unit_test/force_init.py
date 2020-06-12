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
import sys, traceback
import os, os.path

def force_root():
    my_path=os.path.realpath(sys.argv[0])
    while True:
        # split the path into a head and a tail
        my_head, my_tail = os.path.split( my_path )
        # was the app path found, return the head which will be the app root
        if os.path.exists( my_head + "/config/force.config" ):
            return str( my_head )
        # not found has the root been reached?
        if my_head == "/":
            raise Exception( "Reached the system root, force root not found" )
        # not found and not at root move to the next directory up the chain
        my_path = my_head

def force_init():

    # need the force root
    my_force_path = force_root()

    # need to terminate the root str with a path delimiter
    if not my_force_path.endswith( "/"):
        my_force_path += "/"

    # check to see if "utils" path exist if not then something has really gone wrong
    # and recovery from second case is impossible raise a fatal exception
    if not( os.path.exists( my_force_path + "/utils/" )):
        # raise OSError( "\"" + my_force_path + "/utils/\" does not exists, check that it exists as a sub directory of Force" )
        raise OSError( "\"" + my_force_path + "/utils/\" does not exists, check that it exists as a sub directory of Force" )

    return my_force_path

the_force_root = force_init()

# next we need to set the import paths for the utils and tests directories both of which are used for
# the rest of the module run, keep in mind the tests may change in later releases
sys.path.append( the_force_root + "utils/" )
sys.path.append( the_force_root + "utils/regression" )
# sys.path.append( the_force_root + "tests/" )

# if and when another utility is written that uses the same strategy then the following lines need to move to a
# file specific to master_run

def force_usage( arg_msg ):
    #display usage message
    print( arg_msg )
    sys.exit(1)


