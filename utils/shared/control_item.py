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

# since there will be a significant amount of directory manipulation import the path utils
from shared.path_utils import PathUtils
from shared.sys_utils import SysUtils
from shared.datetime_utils import DateTime
from shared.msg_utils import Msg
from shared.summary_core import SummaryErrorQueueItem
# from shared.errors import EInvalidTypeError

class CtrlItmKeys( object ):
    # Control File Location
    fname        = "fname"
    fdir         = "control-dir"    # {{{TODO}}} control file directory, could be implicit, relitive, absolute, or current
    fctrl_name   = "fctrl-name"
    fctrl_dir    = "fctrl-dir"
    test_root    = "test-root"
    work_dir     = "work-dir"
    parent_fctrl = "parent-fctrl"
    fctrl_item   = "fctrl-item"

    # Process Defaults (persistant Values)
    mode         = "mode"
    # summary      = "summary"
    iss_path    = "path"         # {{{TODO}}} set make key iss-path instead of path
    force_path   = "force_path"   # {{{TODO}}} make follow the paradigm for keys

    # control item options
    exit_loop    = "exit-loop"
    group        = "group"
    iterations   = "iterations"
    min_instr    = "min-instr"
    max_instr    = "max-instr"
    num_cores    = "num-cores"
    no_asm       = "no-asm"
    no_sim       = "no-sim"
    delay_run    = "delay-"

    # grouped options
    options      = "options"
    generator    = "generator"
    iss          = "iss"
    performance  = "performance"
    regression   = "regression"
    parent_vals  = "parent-vals"

    # commands
    force_cmd    = "force-cmd"
    iss_cmd     = "iss-cmd"

    action      = "action"
    process_queue = "process-queue"
    process_max = "process-max"
    timeout  = "timeout"


class CtrlItmDefs( object ):
    # Control File Location
    fctrl_name  = "_def_fctrl.py"
    fctrl_dir   = "tests"
    test_root   = None
    work_dir    = "."
    force_path  = None
    process_queue = None

    # Process Defaults (persistant Values)
    mode        = "regress"
    # summary     = None
    iss_path   = None 

    # control item options
    exit_loop   = 1
    group       = "default"
    iterations  = 1
    min_instr   = 1
    max_instr   = 10000
    num_cores   = 1
    no_asm      = False
    no_sim      = False

    # grouped options
    options     = {}
    generator   = {}
    iss         = {}
    performance = {}
    regression  = {}
    parent_vals = {}
    process_max = 16
    #timeout  = 10
    timeout  = 600

    action = "immediate"


class ControlItemType:
    TaskItem=0
    FileItem=1

# this is slated to be a temporary solution, there will be times when we process files without threads or just generate the
# frun control files
class ControlItemActionType():
    WriteOnly=0   #
    Immediate=1   # run as soon as components identified
    Delay=2       # delay run until all components have been pre processed
    NoWrite=3     # execute single run for each control file generated

    @classmethod
    def translate( cls, arg_str ):
        if arg_str == "write-only": return ControlItemActionType.WriteOnly
        if arg_str == "immediate" : return ControlItemActionType.Immediate
        if arg_str == "delay"     : return ControlItemActionType.Delay
        if arg_str == "no-write"  : return ControlItemActionType.NoWrite
        raise Execption( "Unable to translate string value: %s, to ControlItemActionType" % ( arg_str ))

    @classmethod
    def asstr( cls, arg_val ):
        if arg_val == ControlItemActionType.WriteOnly : return "write-only"
        if arg_val == ControlItemActionType.Immediate : return "immediate"
        if arg_val == ControlItemActionType.Delay     : return "delay"
        if arg_val == ControlItemActionType.NoWrite   : return "no-write"
        raise Execption( "Unable to translate value to string  %s, to ControlItemActionType" % (str( arg_val ) ))


class ParentData( object ):

    def populate( self, arg_dict ):
        self.work_dir      = arg_dict[ CtrlItmKeys.fdir       ]
        #Msg.trace()
        Msg.user( "self.work_dir: %s" % ( self.work_dir ),"ParentData::populate()")
        self.test_root     = arg_dict[ CtrlItmKeys.test_root  ]
        self.mode          = arg_dict[ CtrlItmKeys.mode       ]
        self.force_path    = arg_dict[ CtrlItmKeys.force_path ]
        self.action        = arg_dict[ CtrlItmKeys.action     ]
        self.process_queue = arg_dict[ CtrlItmKeys.process_queue ]
        self.force_cmd     = arg_dict[ CtrlItmKeys.force_cmd  ]
        self.process_max   = arg_dict.get( CtrlItmKeys.process_max, CtrlItmDefs.process_max )
        self.iss_path     = arg_dict.get( CtrlItmKeys.iss_path, CtrlItmDefs.iss_path )

        if not SysUtils.found( self.iss_path.find( self.test_root )):
            # Msg.user( "Test Root Not Found ..." )
            # if not then prepend the test root
            self.iss_path = PathUtils.include_trailing_path_delimiter( self.test_root ) + PathUtils.exclude_leading_path_delimiter( self.iss_path )
        Msg.user( "Final Iss Path: %s" % ( self.iss_path ), "ISS_PATH" )
        self.timeout       = arg_dict.get( CtrlItmKeys.timeout, CtrlItmDefs.timeout )
        return self

    def clone( self, arg_data ):
        self.work_dir      = arg_data.work_dir
        self.test_root     = arg_data.test_root
        self.mode          = arg_data.mode
        self.force_path    = arg_data.force_path
        self.action        = arg_data.action
        self.process_queue = arg_data.process_queue
        self.force_cmd     = arg_data.force_cmd
        self.process_max   = arg_data.process_max
        self.iss_path     = arg_data.iss_path
        self.timeout       = arg_data.timeout
        return self

    def data( self ):
        return { CtrlItmKeys.fdir          : self.work_dir
               , CtrlItmKeys.test_root     : self.test_root
               , CtrlItmKeys.mode          : self.mode
               , CtrlItmKeys.force_path    : self.force_path
               , CtrlItmKeys.action        : self.action
               , CtrlItmKeys.process_queue : self.process_queue
               , CtrlItmKeys.force_cmd     : self.force_cmd
               , CtrlItmKeys.process_max   : self.process_max
               , CtrlItmKeys.iss_path     : self.iss_path
               , CtrlItmKeys.timeout       : self.timeout
               }

    def update_iss_path( self, arg_iss_path ):

        self.iss_path = arg_iss_path


class ControlItem( object ):
    def __init__( self ):
        self.parent_fctrl = None
        self.fctrl_item   = None

    def load( self, arg_item_dict, arg_parent_data ):

        # Msg.lout( arg_item_dict, "dbg", "Loaded Control Item Values" )

        self.validate_item_data( arg_item_dict )

        # fname is the key in the control file
        self.fctrl_name  = arg_item_dict.get( CtrlItmKeys.fname, CtrlItmDefs.fctrl_name  )

        # Msg.dbg( "Control Item File: %s" % ( self.fctrl_name ))
        # extract
        self.clone_parent_data( arg_parent_data )
        self.load_item_values( arg_item_dict )
        self.resolve_file_location()

        # grouped options
        self.generator   = arg_item_dict.get( CtrlItmKeys.generator  , CtrlItmDefs.generator   )
        self.iss         = arg_item_dict.get( CtrlItmKeys.iss        , CtrlItmDefs.iss         )

        self.extract_iss_data( arg_item_dict )
        self.performance = arg_item_dict.get( CtrlItmKeys.performance, CtrlItmDefs.performance )
        self.regression  = arg_item_dict.get( CtrlItmKeys.regression , CtrlItmDefs.regression  )
        # Msg.lout( self, "user", "Control Item Dump" )

    def item_type( self ):
        if self.fctrl_name.find( "_force.py" ) >= 0:
            # Msg.dbg( self.fctrl_name + ": Is a Control Template" )
            return ControlItemType.TaskItem

        if self.fctrl_name.find( "_fctrl.py" ) >= 0:
            # Msg.dbg( self.fctrl_name + ": Is a Control File" )
            return ControlItemType.FileItem

        raise Exception( "\"" + self.fctrl_name + "\": Unknown Control Item Type ..." )

    def summary( self ):
        return self.parent_data.process_queue.summary

    def file_path( self ):
        return "%s%s%s" % (self.parent_data.test_root, PathUtils.include_trailing_path_delimiter( self.fctrl_dir ), self.fctrl_name )

    def file_dir( self ):
        return "%s%s" % (self.parent_data.test_root, self.fctrl_dir )

    # perform some minimal validation
    def validate_item_data( self, arg_item_dict ):
        # although there are many defaults there are a few things things that are required
        # 1. The Control item must exists
        if arg_item_dict is None:
            raise Exception( "Control Item Data Dictionary Not Initialized" )

        # it must be a dictionary type
        if not isinstance( arg_item_dict, dict ):
            raise TypeError( "Control Item Data: Must Be a Dictionary, Control Item: %s" % ( str( arg_item_dict )))

        my_keys = [ CtrlItmKeys.options, CtrlItmKeys.generator, CtrlItmKeys.iss, CtrlItmKeys.performance, CtrlItmKeys.regression ]

        for my_key in my_keys:
            my_tmp_dict = arg_item_dict.get( my_key, None )
            if my_tmp_dict is not None:
                if not isinstance( my_tmp_dict, dict):
                    raise TypeError( "Control Item[\"%s\"]: Must Be a Dictionary, \"%s\":%s" % (my_key, my_key, str(my_tmp_dict)))

        return True

    # clone persistent values, the only exception is the work directory which is the location
    # of the control file from where this control item data was loaded
    def clone_parent_data( self, arg_data ):

        # Msg.lout( arg_parent_data, "user", "Parent Data Dictionary" )
        # Passed from parent item
        self.parent_data = ParentData()
        self.parent_data.clone(arg_data  )

        # self.work_dir   = arg_parent_data[ CtrlItmKeys.fdir       ]
        # self.test_root  = arg_parent_data[ CtrlItmKeys.test_root  ]
        # self.mode       = arg_parent_data[ CtrlItmKeys.mode       ]
        # # self.summary    = arg_parent_data[ CtrlItmKeys.summary    ]
        # self.force_path = arg_parent_data[ CtrlItmKeys.force_path ]
        # self.action     = arg_parent_data[ CtrlItmKeys.action     ]
        # self.process_queue = arg_parent_data[ CtrlItmKeys.process_queue ]
        #
        # self.force_cmd   = arg_parent_data[ CtrlItmKeys.force_cmd  ]
        # # self.iss_path   = arg_parent_data[ CtrlItmKeys.iss_path  ]
        #
        # self.process_max = arg_parent_data.get( CtrlItmKeys.process_max, CtrlItmDefs.process_max )
        # # self.timeout  = arg_parent_data.get( CtrlItmKeys.timeout , CtrlItmDefs.timeout  )

    # this control item datsa is in the form of a dictionary, populate the control item with the extracted values, where the values do not exist
    # use the
    def load_item_values( self, arg_item_dict ):

        my_options = arg_item_dict.get( CtrlItmKeys.options, CtrlItmDefs.options )
        my_parent_vals = arg_item_dict.get( CtrlItmKeys.parent_vals, {} )
        Msg.lout( my_parent_vals, "dbg", "Parent Values from Control Item" )
        Msg.lout( self.parent_data, "dbg", "Persistent Data" )

        # Control File Location
        self.fctrl_dir   = my_options.get( CtrlItmKeys.fdir , SysUtils.ifthen( self.parent_data.work_dir is None
                                                                             , CtrlItmDefs.fctrl_dir
                                                                             , self.parent_data.work_dir ))

        # Populated from control item options
        self.iterations  = my_options.get( CtrlItmKeys.iterations, CtrlItmDefs.iterations  )

        # Populated from control item options or parent
        self.exit_loop   = my_options.get( CtrlItmKeys.exit_loop , my_parent_vals.get( CtrlItmKeys.exit_loop , CtrlItmDefs.exit_loop   ))
        self.group       = my_options.get( CtrlItmKeys.group     , my_parent_vals.get( CtrlItmKeys.group     , CtrlItmDefs.group       ))
        self.min_instr   = my_options.get( CtrlItmKeys.min_instr , my_parent_vals.get( CtrlItmKeys.min_instr , CtrlItmDefs.min_instr   ))
        self.max_instr   = my_options.get( CtrlItmKeys.max_instr , my_parent_vals.get( CtrlItmKeys.max_instr , CtrlItmDefs.max_instr   ))
        self.num_cores   = my_options.get( CtrlItmKeys.num_cores , my_parent_vals.get( CtrlItmKeys.num_cores , CtrlItmDefs.num_cores   ))
        self.no_asm      = my_options.get( CtrlItmKeys.no_asm    , my_parent_vals.get( CtrlItmKeys.no_asm    , CtrlItmDefs.no_asm      ))
        self.no_sim      = my_options.get( CtrlItmKeys.no_sim    , my_parent_vals.get( CtrlItmKeys.no_sim    , CtrlItmDefs.no_sim      ))
        self.timeout     = my_options.get( CtrlItmKeys.timeout   , my_parent_vals.get( CtrlItmKeys.timeout   , CtrlItmDefs.timeout     ))


    def extract_iss_data( self, arg_item_dict ):

        # first get the iss dictionary
        # Msg.lout( arg_item_dict, "user", "RAW Item Dictionary ..." )
        my_iss_data = arg_item_dict.get( CtrlItmKeys.iss, CtrlItmDefs.iss )
        my_iss_path = my_iss_data.get( CtrlItmKeys.iss_path, None )

        if my_iss_path is not None:
            # if the iss path was not specified then the iss path currently in the parent data instance is the correct one
            Msg.dbg( "Iss Path: %s, Test Root: %s" % ( my_iss_path, self.parent_data.test_root ))
            # check to see if path is absolute path
            if not SysUtils.found( my_iss_path.find( self.parent_data.test_root )):
                Msg.dbg( "Test Root Not Found ..." )
                # if not then prepend the test root
                my_iss_path = PathUtils.include_trailing_path_delimiter( self.parent_data.test_root ) + PathUtils.exclude_leading_path_delimiter( my_iss_path )

            self.parent_data.update_iss_path( my_iss_path )

        Msg.dbg( "Final Iss Path: %s" % (self.parent_data.iss_path ))



    def resolve_file_location( self ):

        my_fctrl_dir, self.fctrl_name = PathUtils.split_path( str( self.fctrl_name  ))

        Msg.user( "Extracted File Path: %s, Control File Path: %s, Extracted File Name: %s" % ( str( my_fctrl_dir ), str( self.fctrl_dir ), str( self.fctrl_name )), "FCTRL-DIR" )
        Msg.user( "1 - Control Directory: %s, Force Directory: %s, Test Root: %s, Work Dir: %s, Control File: %s, self.fctrl_dir: %s" % ( my_fctrl_dir
                                                                                                                                        , self.parent_data.force_path
                                                                                                                                        , self.parent_data.test_root
                                                                                                                                        , self.parent_data.work_dir
                                                                                                                                        , self.fctrl_name
                                                                                                                                        , self.fctrl_dir
                                                                                                                                        ) , "FCTRL-DIR"  )
        # if the name does not contain a path use the contol directory
        if my_fctrl_dir is None:
            my_fctrl_dir = self.fctrl_dir

        Msg.user( "Control Directory: %s, Force Directory: %s, Test Root: %s, Work Dir: %s, Control File: %s" % ( my_fctrl_dir
                                                                                                                , self.parent_data.force_path
                                                                                                                , self.parent_data.test_root
                                                                                                                , self.parent_data.work_dir
                                                                                                                , self.fctrl_name
                                                                                                                ) , "FCTRL-DIR" )

        # because of the requirement of not knowing and needing to discover the directory
        # situation it is necessary to search for the control file
        my_tmp = None
        my_tmp_dir = None

        # if the a directory was specified in the control name or a specifies as part of the control item it is
        # necessary to check for either a absolute path or relative parent path
        # Msg.dbg( "Checking: [%s] for [%s]" % ( my_fctrl_dir, self.fctrl_name ))


        if my_fctrl_dir is None:
            raise Exception( "Control File Location was not specified, File[%s]" % ( self.fctrl_name ))

        if self.check_full_path( my_fctrl_dir ):
            Msg.user( "Using Real Path as Control Directory", "FILE_LOCATION" )
            pass

        elif self.check_parent_dir( my_fctrl_dir ):
            Msg.user( "Using Parent Directory as Control Directory", "FILE_LOCATION" )
            pass

        elif self.check_work_dir( my_fctrl_dir ):
            Msg.user( "Using Current Control File Location as Control Directory", "FILE_LOCATION" )
            pass

        elif self.check_ctrl_dir(  my_fctrl_dir ):
            Msg.user( "Using Specified Control Directory as Control Directory", "FILE_LOCATION" )
            pass

        else:

            if self.item_type() == ControlItemType.TaskItem:
                my_err_queue_item = SummaryErrorQueueItem( { "error"  : "Template Not Found at Specified Location"
                                                           , "message": "Template File Not Found ..."
                                                           , "path"   : self.file_path()
                                                           , "type"   : str( "FileNotFoundError" )
                                                           } )
                self.summary().queue.enqueue( my_err_queue_item )

            elif self.item_type() == ControlItemType.FileItem:
                my_err_queue_item = SummaryErrorQueueItem( { "error"  : "FileNotFoundError"
                                                           , "message": "Control File Not Found at Specified Location"
                                                           , "path"   : self.file_path()
                                                           , "type"   : str( "FileNotFoundError" )
                                                           } )
                self.summary().queue.enqueue( my_err_queue_item )

            raise Exception( "File[%s] Not Found at Location Specified[%s]" % ( self.fctrl_name, my_fctrl_dir ))

        # Msg.dbg( "Raw Control Directory: %s" % ( self.fctrl_dir ))
        # Msg.dbg( "Test Root: %s" % ( self.test_root ))
        # remove the test root if present
        if SysUtils.found( self.fctrl_dir.find( self.parent_data.test_root )):
            # Msg.dbg( "Test Root Found" )
            self.fctrl_dir = self.fctrl_dir.replace( self.parent_data.test_root, "" )

        # Msg.dbg( "Trimmed Control Directory: %s" % ( self.fctrl_dir ))

        # add the trailing path delimiter
        self.fctrl_dir = PathUtils.include_trailing_path_delimiter( self.fctrl_dir )
        Msg.user( "Delimited Control Directory: %s" % ( self.fctrl_dir ), "FILE_LOCATION")
        return True;


    def check_full_path( self,  arg_fctrl_dir ):
        # handle fully qualified path if found
        Msg.dbg( "Checking for control file with real path[%s][%s]" % (str( arg_fctrl_dir ), str( self.fctrl_name )))
        if arg_fctrl_dir.startswith( "/" ):
            # there is only one scenario where the control directory starts with the path delimiter and that is the path
            # is a real path from the root directory of the system, the control file must exist at that location
            if PathUtils.check_found( PathUtils.append_path( arg_fctrl_dir, self.fctrl_name )):
                self.fctrl_dir = arg_fctrl_dir
                return True

            raise Exception( "Control Item File: [%s] count not be located Not Found at [%s]" % ( self.fctrl_name, arg_fctrl_dir ))

        return False


    def check_parent_dir( self, arg_fctrl_dir ):

        Msg.dbg( "Checking for control file in parent directory" )
        Msg.user( "Test Root[%s], Work Dir[%s]" % ( str(self.parent_data.test_root ), str(self.parent_data.work_dir)), "CHECK-PARENT-DIR" )

        if arg_fctrl_dir.startswith( ".." ):

            my_search_dir = arg_fctrl_dir
            my_work_dir =  PathUtils.append_path( self.parent_data.test_root, self.parent_data.work_dir )
            my_test_root = self.parent_data.test_root
            Msg.user( "Updated Test Root[%s], Full Work Dir[%s], Search Dir[%s]" % ( my_test_root, my_work_dir, my_search_dir ), "CHECK-PARENT-DIR" )

            while my_search_dir.startswith( ".." ):

                # First get the parent directory
                # The search [updated] directory started with [..] split path for both the work directory and the search directory
                # this fact that the [updated] search directory still begins with a [..], means that the work directory
                my_test_root, my_dumy = PathUtils.split_dir( my_test_root )
                my_work_dir , my_dumy = PathUtils.split_dir( my_work_dir  )
                my_search_dir = my_search_dir[3:]
                Msg.user( "Updated Test Root[%s], Full Work Dir[%s], Search Dir[%s]" % ( my_test_root, my_work_dir, my_search_dir ), "CHECK-PARENT-DIR" )

                my_filepath = str( PathUtils.append_path( my_work_dir,  PathUtils.append_path( my_search_dir, self.fctrl_name )))
                Msg.user( "File Path[%s] ... " % ( my_filepath ), "CHECK-PARENT-DIR" )

                if PathUtils.check_found( my_filepath ):
                    self.fctrl_dir = PathUtils.append_path( my_work_dir, my_search_dir )
                    return True

                my_filepath = str( PathUtils.append_path( my_test_root, PathUtils.append_path( my_search_dir, self.fctrl_name )))
                Msg.user( "File Path[%s] ... " % ( my_filepath ), "CHECK-PARENT-DIR" )

                if PathUtils.check_found( my_filepath ):
                    self.fctrl_dir = PathUtils.append_path( my_test_root, my_search_dir )
                    return True

            raise Exception( "Control Item File[%s] Not Found in Parent Directory: [%s]" % ( self.fctrl_name, arg_fctrl_dir ))

        return False


    def check_work_dir( self, arg_fctrl_dir ):
        # check the work directory
        Msg.dbg( "Checking for control file using implict path[%s] of current control file" % ( str(arg_fctrl_dir )  ))
        if not self.parent_data.work_dir is None:

            my_tmp_dir = PathUtils.append_path( self.parent_data.work_dir, arg_fctrl_dir )
            if not SysUtils.found( my_tmp_dir.find( self.parent_data.test_root )):
                # Prepend the test_root on the work directory
                my_tmp_dir = PathUtils.append_path( self.parent_data.test_root, my_tmp_dir )

            if PathUtils.check_found( PathUtils.append_path( my_tmp_dir, self.fctrl_name )  ):
                self.fctrl_dir = my_tmp_dir
                # Msg.dbg( "Using Control File Directory: %s" % ( self.fctrl_dir ))
                return True

        return False


    def check_ctrl_dir( self, arg_fctrl_dir ):

        Msg.user( "Checking for control file at relative path[%s] from test root" % ( arg_fctrl_dir ), "RELATIVE PATH"  )
        my_tmp_dir = arg_fctrl_dir
        if not SysUtils.found( my_tmp_dir.find( self.parent_data.test_root )):
            # Prepend the test_root on the work directory
            Msg.user( "Building Control Directory ...", "RELATIVE PATH" )
            my_tmp_dir = PathUtils.append_path( self.parent_data.test_root, my_tmp_dir )

        if PathUtils.check_found(PathUtils.append_path( my_tmp_dir, self.fctrl_name )  ):
            self.fctrl_dir = my_tmp_dir
            Msg.user( "Setting Control Directory: %s ..." % ( self.fctrl_dir ), "RELATIVE PATH" )
            return True
        return False

    def __str__( self ):
        return self.print_vals( self.catalougs() )

    def print_vals( self, arg_obj, arg_indent = "" ):
        my_str = ""
        if isinstance( arg_obj, dict ):
            # # Msg.user( "Help Me Underdog....." )
            my_sep = "{ "
            my_term = ""
            for my_key in arg_obj:
                my_str += my_term
                # # Msg.user( "%s: %s " % ( my_key, str( arg_obj[ my_key ] )))
                if arg_obj[ my_key ] == {}:
                    my_str += "%s%s'%s': {}" % ( arg_indent, my_sep, my_key )
                elif arg_obj[ my_key ] is None:
                    my_str += "%s%s'%s': None" % ( arg_indent, my_sep, my_key )
                elif isinstance( arg_obj[ my_key ], dict ):
                    my_str += "%s%s'%s':\n" % ( arg_indent, my_sep, str( my_key ))
                    my_str += self.print_vals( arg_obj[ my_key ], "\t%s" % (arg_indent ))
                elif isinstance( arg_obj[ my_key ], (int, bool) ):
                    my_str += "%s%s'%s': %s" % ( arg_indent, my_sep, my_key, str( arg_obj[ my_key ]  ))
                else:
                    my_str += "%s%s'%s': '%s'" % ( arg_indent, my_sep, my_key, str( arg_obj[ my_key ]  ))
                my_sep = "  "
                my_term = ",\n"
            my_str += "\n%s}" % ( arg_indent )
        # # Msg.user( my_str )
        return my_str

    def values( self ):
        self.vals =  { CtrlItmKeys.fname         : self.fctrl_name
                     , CtrlItmKeys.fctrl_dir     : self.fctrl_dir
                     , CtrlItmKeys.exit_loop     : self.exit_loop
                     , CtrlItmKeys.group         : self.group
                     , CtrlItmKeys.iterations    : self.iterations
                     , CtrlItmKeys.min_instr     : self.min_instr
                     , CtrlItmKeys.max_instr     : self.max_instr
                     , CtrlItmKeys.num_cores     : self.num_cores
                     , CtrlItmKeys.no_asm        : self.no_asm
                     , CtrlItmKeys.no_sim        : self.no_sim
                     , CtrlItmKeys.generator     : self.generator
                     , CtrlItmKeys.iss           : self.iss
                     , CtrlItmKeys.performance   : self.performance
                     , CtrlItmKeys.regression    : self.regression
                     , CtrlItmKeys.timeout       : self.timeout
                     , CtrlItmKeys.parent_fctrl  : None
                     }


        my_parent_data = ParentData().populate( { CtrlItmKeys.action        : self.parent_data.action
                                                , CtrlItmKeys.process_queue : self.parent_data.process_queue
                                                , CtrlItmKeys.fdir          : self.fctrl_dir
                                                , CtrlItmKeys.test_root     : self.parent_data.test_root
                                                , CtrlItmKeys.mode          : self.parent_data.mode
                                                , CtrlItmKeys.force_cmd     : self.parent_data.force_cmd
                                                , CtrlItmKeys.force_path    : self.parent_data.force_path
                                                , CtrlItmKeys.process_max   : self.parent_data.process_max
                                                } )

        # # Msg.lout( my_vals, "user", "Control Item Values" )
        return ( self.vals, my_parent_data )

    def catalougs( self ):

        my_dict = { CtrlItmKeys.fname      : self.fctrl_name
                  , CtrlItmKeys.fdir       : self.fctrl_dir
                  , CtrlItmKeys.force_cmd  : self.parent_data.force_cmd
                  , CtrlItmKeys.options: { CtrlItmKeys.group      : self.group
                                         , CtrlItmKeys.exit_loop  : self.exit_loop
                                         , CtrlItmKeys.min_instr  : self.min_instr
                                         , CtrlItmKeys.max_instr  : self.max_instr
                                         , CtrlItmKeys.num_cores  : self.num_cores
                                         , CtrlItmKeys.no_asm     : self.no_asm
                                         , CtrlItmKeys.no_sim     : self.no_sim
                                         , CtrlItmKeys.timeout    : self.timeout
                                         }
                  , CtrlItmKeys.parent_vals: self.parent_data.data()
                                         #    { CtrlItmKeys.test_root  : self.test_root
                                         #    , CtrlItmKeys.mode       : self.mode
                                         #    , CtrlItmKeys.iss_path  : self.iss_path
                                         #    , CtrlItmKeys.force_path : self.force_path
                                         #    , CtrlItmKeys.mode       : self.mode
                                         #    , CtrlItmKeys.force_cmd  : self.force_cmd
                                         #    , CtrlItmKeys.force_path : self.force_path
                                         #    }
                  , CtrlItmKeys.generator  :self.generator
                  , CtrlItmKeys.iss: self.iss
                  , CtrlItmKeys.performance: self.performance
                  , CtrlItmKeys.regression: self.regression
                  }


        return my_dict


