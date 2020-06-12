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
import getopt
import sys

from common.path_utils import PathUtils
from common.msg_utils import Msg

class CmdLineOpts( object ):

    def __init__( self, arg_switches, arg_opts ):
        self.opts, self.args = getopt.getopt(sys.argv[1:], arg_opts, arg_switches )

    # break command line into dictionary
    def get_options( self ):
        my_options = {}
        for my_opt, my_val in self.opts:
            if not my_val:
                my_val = True
            elif isinstance( my_opt, str ):
                 # Remove leading dashes
                #print( my_opt )
                while my_opt.startswith( "-" ):
                    # print( "Option Key: " + my_opt )
                    my_opt = my_opt[1:]

            my_options[ my_opt ] = my_val
        return my_options

    # looks for an option then returns the value if found
    def get_option( self, arg_switch, arg_def_val ):
        # print( "Switch: " + str( arg_switch ))
        my_val = None
        for my_key, my_val in self.opts:
            # print( "Key: " + my_key + " value : " + my_val)
            if( my_key in arg_switch ):

                my_val = (str( my_val )).strip()
                while my_val.startswith( "=" ):
                    # print( "Option Key: " + my_opt )
                    my_val = my_val[1:]

                my_val = my_val.strip()

                if not len( str( my_val )):
                    return True
                return my_val
        return arg_def_val

    # more general version of get_options
    def option_def( self, arg_switch, arg_def_val = None, arg_short = None ):
        if arg_short is not None:
            arg_sw = [ arg_short, "--" + arg_switch.replace("=", "" )]
            # Msg.user("arg_short %s " %  str (arg_sw) )
            return self.get_option( [ arg_short, "--" + arg_switch.replace("=", "" )], arg_def_val )
        return self.get_option( [ "--" + arg_switch.replace("=", "" )], arg_def_val )

    # returns a list of command line arguments in the order that they occurred on the command line
    def get_arguments( self ):
        return self.args


class ModuleRun( object ):

    # takes a list of supported switches from the command line
    def __init__( self, arg_force_path, arg_switches, arg_opts, arg_msg_lev, arg_def_lev, arg_short_lev  ):

        # print( "Force Path: %s"      % (str(arg_force_path )))
        # print( "Module Switches: %s" % (str(arg_switches   )))
        # print( "Options: %s"         % (str(arg_opts       )))
        # print( "Message Level: %s"   % (str(arg_msg_lev    )))
        # print( "Default Level: %s"   % (str(arg_def_lev    )))
        # print( "Short Level: %s"     % (str(arg_short_lev  )))

        # extract the arguments and the parameters
        self.opts, self.args = getopt.getopt(sys.argv[1:], arg_opts, arg_switches )
        self.module_dir, self.module_name = PathUtils.split_path( PathUtils.real_path(sys.argv[0]) )

        # set the message level
        self.load_message_levels( arg_msg_lev, arg_def_lev, arg_short_lev )

        # save the force path
        self.force_path = arg_force_path


    # load defaults
    def load( self ):
        pass


    # run, creating an abstract here prevents creation
    def run( self ):
        pass

    # resolves the desired message types
    def load_message_levels( self, arg_msg_lev, arg_def_lev, arg_short_lev ):
        # load from the command line if specified or use the default
        my_lev_str = self.option_def( arg_msg_lev, arg_def_lev, arg_short_lev  )
        #my_def = "crit+err+warn+info+noinfo"

        #my_lev_str = self.option_def( "all", my_def, "-l"  )

        # if a (+) or a (-) is found then the command line will be appended to or demoted by
        if ( my_lev_str[0] == '+' ) or ( my_lev_str[0] == '-' ):
            # use the default string to build a format string, then append the passed in value
            my_fmt_str = "%s%s%s"%( arg_def_lev,"\%","s" )
            my_lev_str =  my_fmt_str % ( my_lev_str )

        # print( my_lev_str )
        # finally no matter what set the levels that are to be active

        my_level = Msg.translate_levelstr( my_lev_str )
        # print( "Before: %x" % my_level )

        Msg.set_level( my_level )

        # print( "After: %x" % Msg.get_level())



    # return the force path
    def get_force_dir( self ):
        return self.force_path

    # break command line into dictionary
    def get_options( self ):
        my_options = {}
        for my_opt, my_val in self.opts:
            if not my_val:
                my_val = True
            elif isinstance( my_opt, str ):
                 # Remove leading dashes
                #print( my_opt )
                while my_opt.startswith( "-" ):
                    # print( "Option Key: " + my_opt )
                    my_opt = my_opt[1:]

            my_options[ my_opt ] = my_val
        return my_options

    # looks for an option then returns the value if found
    def get_option( self, arg_switch, arg_def_val ):
        # print( "Switch: " + str( arg_switch ))
        my_val = None
        for my_key, my_val in self.opts:
            # print( "Key: " + my_key + " value : " + my_val)
            if( my_key in arg_switch ):

                my_val = (str( my_val )).strip()
                while my_val.startswith( "=" ):
                    # print( "Option Key: " + my_opt )
                    my_val = my_val[1:]

                my_val = my_val.strip()

                if not len( str( my_val )):
                    return True
                return my_val
        return arg_def_val

    # more general version of get_options
    def option_def( self, arg_switch, arg_def_val = None, arg_short = None ):
        if arg_short is not None:
            arg_sw = [ arg_short, "--" + arg_switch.replace("=", "" )]
            # Msg.user("arg_short %s " %  str (arg_sw) )
            return self.get_option( [ arg_short, "--" + arg_switch.replace("=", "" )], arg_def_val )
        return self.get_option( [ "--" + arg_switch.replace("=", "" )], arg_def_val )



    # returns a list of command line arguments in the order that they occurred on the command line
    def get_arguments( self ):
        return self.args


