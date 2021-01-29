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
import argparse
import os
import sys
from common.path_utils import PathUtils

# A thin wrapper around Python argparse module.
#
# Pass in a command line parameter specification object to the __init__ method.
#
####################################################################################################################################
# Example:
# class CommandLineParameters(object):                                                                                             #
#                                                                                                                                  #
####################################################################################################################################

class CmdLineParser(object):

    """
        In order to get the usage help to have the desired sections, arguments need to be added as groups into the parser. This form of the constructor facilitates this choice.
    """
    def __init__(self, cmd_line_spec, group_names=None, group_descriptions=None,  group_parameters=None, add_help=False):
        init_dict = {}
        init_parameters = ("prog", "usage", "description", "epilog", "parents", "formatter_class", "prefix_chars", "add_help", "allow_abbrev")

        for init_parm in init_parameters:
            try:
                init_parm_value = getattr(cmd_line_spec, init_parm)
            except AttributeError:
                # for example, if the cmd_line_spec has prog attribute, we will pass it along to ArgumentParser __init__
                pass
            else:
                init_dict[init_parm] = init_parm_value

        #Do we want the default add_help behavior, or for compatibility reasons maybe do we want to prevent this?
        if add_help:
            init_dict.update({"add_help":True})
        else:
            init_dict.update({"add_help":False})

        self.parser = argparse.ArgumentParser(**init_dict)
        self.args_namespace = None
        self.argument_groups = []

        #We want to be able to set up conceptual parsing groups so that the help screen retains some similarity according to the grouping that used to be presented with the old command line parsing.
        if group_parameters is not None:
            for index, parameters in enumerate(group_parameters):
                self.argument_groups.append(self.parser.add_argument_group(title=group_names[index], description=group_descriptions[index]))

                for (short_opt, long_opt, num_args, additional_specs, help_text) in parameters:
                    parm_list = (short_opt, long_opt)
                    parm_dict = { "help":help_text }
                    if num_args != 0 :
                        parm_dict["nargs"] = num_args
                    parm_dict.update(additional_specs) # pass along additional specifications
                    # print ("option %s parameter dict: %s" % (long_opt, str(parm_dict)))
                    self.argument_groups[-1].add_argument(*parm_list, **parm_dict)
        else:
            # parameters variable is required
            for (short_opt, long_opt, num_args, additional_specs, help_text) in cmd_line_spec.parameters:
                parm_list = (short_opt, long_opt)
                parm_dict = { "help":help_text }
                if num_args != 0 :
                    parm_dict["nargs"] = num_args
                parm_dict.update(additional_specs) # pass along additional specifications
                # print ("option %s parameter dict: %s" % (long_opt, str(parm_dict)))
                self.parser.add_argument(*parm_list, **parm_dict)

        # catch all for remainder arguments
        if cmd_line_spec.pass_remainder:
            self.parser.add_argument("xargs", nargs=argparse.REMAINDER)

    def parse_args(self, in_args):
        self.args_namespace = self.parser.parse_args(in_args)
        return self.args_namespace

    def parse_known_args(self, in_args):
        self.args_namespace, unknown = self.parser.parse_known_args(in_args)
        return self.args_namespace, unknown

    def set_parameters(self, in_parms_obj):
        args_dict = vars(self.args_namespace)

        for key, var in args_dict.items():
            if key == "xargs":
                in_parms_obj.xargs = var[1:]
            elif isinstance(var, list):
                if len(var) == 1:
                    setattr(in_parms_obj, key, var[0])
                elif len(var):
                    setattr(in_parms_obj, key, var)
            else:
                setattr(in_parms_obj, key, var)
            #print("key %s, var %s" % (key, str(var)))

    def print_help(self):
        self.parser.print_help()

##
#  A minimal object to hold the output of the parse_args methods from argparse as attributes
class AttributeContainer(object):
    def __init__(self):
        pass

##
# Parse and access parsed command line options. Seems to have overlapping functionality with ModuleRun, while other files like forrest_run.py depend on
# both classes simultaneously.
#
class CmdLineOpts( object ):

    def __init__( self, arg_CommandLineParameters, aSysArgv ):

        self.mProgramPath, self.mProgramName = PathUtils.split_path( PathUtils.real_path(aSysArgv[0]) )
        self.CommandLineParameters = arg_CommandLineParameters
        self.cmd_line_parser = None
        self.ap_args = None
        self.ap_unknown = None
        self.ap_opts = AttributeContainer()

        self.cmd_line_parser = CmdLineParser(self.CommandLineParameters, self.CommandLineParameters.group_names, self.CommandLineParameters.group_descriptions, self.CommandLineParameters.group_parameters, add_help=True)
        self.ap_args, self.ap_unknown = self.cmd_line_parser.parse_known_args(aSysArgv[1:])
        self.cmd_line_parser.set_parameters(self.ap_opts)
        try:
            if len(self.ap_unknown) > 0:
                raise Exception("Only Command Options are allowed: [" + os.path.realpath(aSysArgv[0] ) + "]\n       Found Argument(s): " + str( self.get_unknown_arguments() ))
        except Exception as e:
            print(str(e))
            sys.exit(40) #40 was the code originally used for getopt errors

    ##
    # Takes an argument name, looks it up in the parsed options but returns the externally provided default if none found.
    def option_def( self, arg_switch, arg_def_val = None ):
        _opt = vars(self.ap_opts).get(arg_switch.replace("-", "_"))
        if _opt is None:
            _opt = arg_def_val
        return _opt

    ##
    # returns collection of arguments that were recognized by the parser
    def get_arguments( self ):
        return self.ap_args

    ##
    # returns collection of arguments that were not recognized by the parser
    def get_unknown_arguments( self):
        return self.ap_unknown
    ##
    # allows printing the help string whenever desired
    def print_help(self):
        self.cmd_line_parser.print_help()

##
# Retrieves command line arguments and returns them as a list (or None if it does not exist)
def basicCommandLineArgumentRetrieval(aArguments, aShort, aLong, aType, aNumArgs):
    basic_parser = argparse.ArgumentParser(add_help=False)
    basic_parser.add_argument(aShort, aLong, type=aType, nargs=aNumArgs)
    (known_arguments, _) = basic_parser.parse_known_args(aArguments)
    return known_arguments

