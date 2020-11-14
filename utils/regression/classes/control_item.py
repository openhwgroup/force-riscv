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
# File: control_item.py
# Comment: manages control item data
# Contributors: Howard Maler, Noah Sherrill, Amit Kumar, Jingliang(Leo) Wang


# since there will be a significant amount of directory manipulation import the path utils
import seedgen
from common.path_utils import PathUtils
from common.sys_utils import SysUtils
from common.datetime_utils import DateTime
from common.msg_utils import Msg
from common.file_locator import FileLocator
import copy

class CtrlItmKeys( object ):
    # Control File Location
    fname        = "fname"
    fdir         = "control-dir"    # control item file directory, could be relative, absolute, or current
    fctrl_name   = "fctrl-name"     # control item file name {{{TODO}}} should be renamed to item-fname
    fctrl_dir    = "fctrl-dir"
    work_dir     = "work-dir"
    parent_fctrl = "parent-fctrl"
    fctrl_item   = "fctrl-item"

    # Process Defaults (persistent Values)

    # control item options
    group        = "group"
    iterations   = "iterations"
    min_instr    = "min-instr"
    max_instr    = "max-instr"
    num_chips    = "num-chips"
    num_cores    = "num-cores"
    num_threads  = "num-threads"
    no_sim       = "no-sim"
    delay_run    = "delay-"
    seed         = "seed"
    suffix       = "suffix"

    # grouped options
    options      = "options"
    performance  = "performance"
    regression   = "regression"
    timeout       = "timeout"

class CtrlItmDefs( object ):
    # Control File Location
    fctrl_name  = "_def_fctrl.py"
    fctrl_dir   = "."  # "tests"
    work_dir    = "."

    # Process Defaults (persistent Values)

    # control item options
    group       = "default"
    iterations  = 1
    min_instr   = 1
    max_instr   = 10000
    num_chips   = 1
    num_cores   = 1
    num_threads = 1
    no_sim      = False
    seed        = None
    suffix      = None

    # grouped options
    options     = {}
    performance = {}
    regression  = {}
    timeout  = 600

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
        raise Exception( "Unable to translate string value: %s, to ControlItemActionType" % ( arg_str ))

    @classmethod
    def asstr( cls, arg_val ):
        if arg_val == ControlItemActionType.WriteOnly : return "write-only"
        if arg_val == ControlItemActionType.Immediate : return "immediate"
        if arg_val == ControlItemActionType.Delay     : return "delay"
        if arg_val == ControlItemActionType.NoWrite   : return "no-write"
        raise Exception( "Unable to translate value to string  %s, to ControlItemActionType" % (str( arg_val ) ))

class ControlItem( object ):

    def __init__( self ):
        self.parent_fctrl = None
        self.fctrl_item   = None
        self.fctrl_dir    = None
        self.iterations   = None
        self.group        = None
        self.min_instr    = None
        self.max_instr    = None
        self.num_chips    = None
        self.num_cores    = None
        self.num_threads  = None
        self.no_sim       = None
        self._seed        = None
        self.suffix       = None
        self.work_dir     = None

    @property
    def seed(self):
        if self._seed is None:
            return hex(seedgen.seedgen())
        return self._seed

    @seed.setter
    def seed(self, value):
        self._seed = value

    def load( self, aAppsInfo, aItemDict, aParentItem = None ):

        # Msg.lout( aItemDict, "dbg", "Loaded Control Item Values" )
        self.validate_item_data( aAppsInfo, aItemDict )

        # fname is the key in the control file
        self.fctrl_name  = aItemDict.get( CtrlItmKeys.fname, CtrlItmDefs.fctrl_name  )

        work_dir = self.load_item_values( aAppsInfo, aItemDict, aParentItem )

        try:
            self.resolve_file_location(aAppsInfo, work_dir)
        except:
            raise

        Msg.user( "File Directory: %s, File Name: %s, Work Directory: %s" % ( self.fctrl_dir, self.fctrl_name, work_dir ))

        # grouped options
        #
        if aParentItem is None:
            self.performance = aItemDict.get( CtrlItmKeys.performance, CtrlItmDefs.performance )
            self.regression  = aItemDict.get( CtrlItmKeys.regression , CtrlItmDefs.regression  )
        else:
            self.performance = aItemDict.get( CtrlItmKeys.performance, aParentItem.performance )
            self.regression  = aItemDict.get( CtrlItmKeys.regression , aParentItem.regression  )
        for seq_app_cfg in aAppsInfo.mAllAppsOrder[1:]:
            self.processAppControlData(seq_app_cfg, aItemDict, aParentItem)

    def processAppControlData(self, aAppCfg, aItemDict, aParentItem):
        app_tag = aAppCfg.tag()
        Msg.user("processing %s application" % app_tag)

        if aParentItem is None:
            # no parent item.
            app_dict = aItemDict.get( app_tag  , { } )
        else:
            # have parent item, get default from parent item
            if app_tag in aItemDict:
                app_dict = aItemDict[ app_tag ]
            else:
                parent_dict = getattr( aParentItem, app_tag )
                app_dict = copy.deepcopy( parent_dict )

        setattr( self, app_tag, app_dict)
        aAppCfg.processControlData(app_dict)

    def item_type( self ):
        if self.fctrl_name.endswith( "_force.py" ):
            # Msg.dbg( self.fctrl_name + ": Is a Control Template" )
            return ControlItemType.TaskItem

        if self.fctrl_name.endswith( "_fctrl.py" ):
            # Msg.dbg( self.fctrl_name + ": Is a Control File" )
            return ControlItemType.FileItem

        if self.fctrl_name.endswith( "_frun.py" ):
            # Msg.dbg( self.fctrl_name + ": Is a Control File" )
            return ControlItemType.FileItem

        raise Exception( "\"" + self.fctrl_name + "\": Unknown Control Item Type ..." )

    def file_path( self ):

        return PathUtils.append_path( self.fctrl_dir, self.fctrl_name )

    # perform some minimal validation
    def validate_item_data( self, aAppsInfo, aItemDict ):
        # although there are many defaults there are a few things things that are required
        # 1. The Control item must exists
        if aItemDict is None:
            raise Exception( "Control Item Data Dictionary Not Initialized" )

        # it must be a dictionary type
        if not isinstance( aItemDict, dict ):
            raise TypeError( "Control Item Data: Must Be a Dictionary, Control Item: %s" % ( str( aItemDict )))

        # check for valid data in control item line
        my_keys = [ CtrlItmKeys.options, CtrlItmKeys.performance, CtrlItmKeys.regression ]

        for seq_app_cfg in aAppsInfo.mAllAppsOrder[1:]:
            my_keys.append( seq_app_cfg.tag() )

        for my_key in my_keys:
            my_tmp_dict = aItemDict.get( my_key, None )
            if my_tmp_dict is not None:
                if not isinstance( my_tmp_dict, dict ):
                    raise TypeError( "Control Item[\"%s\"]: Must Be a Dictionary, \"%s\":%s" % (my_key, my_key, str(my_tmp_dict)))
        return True

    # this control item datsa is in the form of a dictionary, populate the control item with the extracted values, where the values do not exist
    # use the
    def load_item_values( self, aAppsInfo, aItemDict, aParentItem):

        if aParentItem is None:
            parent_options = { }
            work_dir = self.fctrl_dir
        else:
            parent_options = aParentItem.values(aAppsInfo)
            work_dir = aParentItem.fctrl_dir

        my_options = aItemDict.get( CtrlItmKeys.options, CtrlItmDefs.options )

        # Control File Location
        self.fctrl_dir = my_options.get( CtrlItmKeys.fdir, CtrlItmDefs.fctrl_dir )

        # Populated from control item options
        self.iterations  = my_options.get( CtrlItmKeys.iterations, CtrlItmDefs.iterations  )

        # Populated from control item options or parent
        # Control File Location
        self.group       = my_options.get( CtrlItmKeys.group       , parent_options.get( CtrlItmKeys.group       , CtrlItmDefs.group       ))
        self.min_instr   = my_options.get( CtrlItmKeys.min_instr   , parent_options.get( CtrlItmKeys.min_instr   , CtrlItmDefs.min_instr   ))
        self.max_instr   = my_options.get( CtrlItmKeys.max_instr   , parent_options.get( CtrlItmKeys.max_instr   , CtrlItmDefs.max_instr   ))
        self.num_chips   = my_options.get( CtrlItmKeys.num_chips   , parent_options.get( CtrlItmKeys.num_chips   , CtrlItmDefs.num_chips   ))
        self.num_cores   = my_options.get( CtrlItmKeys.num_cores   , parent_options.get( CtrlItmKeys.num_cores   , CtrlItmDefs.num_cores   ))
        self.num_threads = my_options.get( CtrlItmKeys.num_threads , parent_options.get( CtrlItmKeys.num_threads , CtrlItmDefs.num_threads ))
        self.no_sim      = my_options.get( CtrlItmKeys.no_sim      , parent_options.get( CtrlItmKeys.no_sim      , CtrlItmDefs.no_sim      ))
        self.timeout     = my_options.get( CtrlItmKeys.timeout     , parent_options.get( CtrlItmKeys.timeout     , CtrlItmDefs.timeout     ))
        self.seed        = my_options.get( CtrlItmKeys.seed        , parent_options.get( CtrlItmKeys.seed        , CtrlItmDefs.seed        ))

        return work_dir

    def resolve_file_location( self, aAppsInfo, aParentPath ):

        control_file_name = self.fctrl_name
        control_dir = self.fctrl_dir

        if control_file_name.startswith( "$/" ):
            control_file_name = control_file_name.replace( "$/", aAppsInfo.mTestBaseDir )

        the_dir, the_filename = PathUtils.split_path(control_file_name)

        if the_dir is None:
            control_file_name = control_dir + "/" + the_filename

        prefix_list = [aParentPath, aAppsInfo.mTestBaseDir, aAppsInfo.mToolPath, aAppsInfo.mMainAppPath]
        control_file_path, path_valid = FileLocator.resolveFileLocation(control_file_name, prefix_list)

        control_dir, control_file_name = PathUtils.split_path(control_file_path)
        control_dir = PathUtils.include_trailing_path_delimiter(control_dir)

        if not path_valid:
            raise Exception( "File [%s] Not Found at Locations Specified %s" % ( control_file_name, str(prefix_list) ))

        self.fctrl_name = control_file_name
        self.fctrl_dir = control_dir
        self.item_path = control_file_path

    def __str__( self ):
        raise Exception("Not implemented.")

    def values( self, aAppsInfo ):
        self.vals =  { CtrlItmKeys.fname         : self.fctrl_name
                     , CtrlItmKeys.fctrl_dir     : self.fctrl_dir
                     , CtrlItmKeys.group         : self.group
                     , CtrlItmKeys.iterations    : self.iterations
                     , CtrlItmKeys.min_instr     : self.min_instr
                     , CtrlItmKeys.max_instr     : self.max_instr
                     , CtrlItmKeys.num_chips     : self.num_chips
                     , CtrlItmKeys.num_cores     : self.num_cores
                     , CtrlItmKeys.num_threads   : self.num_threads
                     , CtrlItmKeys.no_sim        : self.no_sim
                     , CtrlItmKeys.suffix        : self.suffix
                     , CtrlItmKeys.performance   : self.performance
                     , CtrlItmKeys.regression    : self.regression
                     , CtrlItmKeys.timeout       : self.timeout
                     , CtrlItmKeys.seed          : self._seed
                     , CtrlItmKeys.parent_fctrl  : None
                     }

        for seq_app_cfg in aAppsInfo.mAllAppsOrder[1:]:
            app_tag = seq_app_cfg.tag()
            app_dict = copy.deepcopy( getattr( self, app_tag ) )
            if app_tag in self.vals:
                raise Exception("Application tag \"%s\"already exist in self.vals dict." % app_tag)
            self.vals[ app_tag ] = app_dict

        # # Msg.lout( my_vals, "user", "Control Item Values" )
        return self.vals

    def catalogues( self, aAppsInfo ):

        my_dict = { CtrlItmKeys.fname      : self.fctrl_name
                  , CtrlItmKeys.options: { CtrlItmKeys.group       : self.group
                                         , CtrlItmKeys.fdir        : self.fctrl_dir
                                         , CtrlItmKeys.min_instr   : self.min_instr
                                         , CtrlItmKeys.max_instr   : self.max_instr
                                         , CtrlItmKeys.num_chips   : self.num_chips
                                         , CtrlItmKeys.num_cores   : self.num_cores
                                         , CtrlItmKeys.num_threads : self.num_threads
                                         , CtrlItmKeys.no_sim      : self.no_sim
                                         , CtrlItmKeys.suffix      : self.suffix
                                         , CtrlItmKeys.timeout     : self.timeout
                                         , CtrlItmKeys.seed        : self.seed
                                         }
                  , CtrlItmKeys.performance: self.performance
                  , CtrlItmKeys.regression: self.regression
                  }

        for seq_app_cfg in aAppsInfo.mAllAppsOrder[1:]:
            app_tag = seq_app_cfg.tag()
            app_dict = getattr( self, app_tag )
            if app_tag in my_dict:
                raise Exception("Application tag \"%s\"already exist in my_dict." % app_tag)
            my_dict[ app_tag ] = app_dict

        return my_dict

    def print_vals( self, arg_obj, arg_indent = "" ):
        my_str = ""
        if isinstance( arg_obj, dict ):
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
                elif isinstance( arg_obj[ my_key ], (int, bool, list)):
                    my_str += "%s%s'%s': %s" % ( arg_indent, my_sep, my_key, str( arg_obj[ my_key ]  ))
                else:
                    my_str += "%s%s'%s': '%s'" % ( arg_indent, my_sep, my_key, str( arg_obj[ my_key ]  ))
                my_sep = "  "
                my_term = ",\n"
            my_str += "\n%s}" % ( arg_indent )
        # # Msg.user( my_str )
        return my_str

    def prepare( self, aAppsInfo, aTaskFile ):
        my_catalogues = self.catalogues( aAppsInfo )
        my_catalogues[ CtrlItmKeys.fname ] = aTaskFile
        my_contents = "control_items = [\n"
        my_contents += self.print_vals( my_catalogues, "\t" )
        # my_contents += str( my_catalogues )
        my_contents += "\n]"
        #Msg.user( str( my_contents ), "FRUN"  )

        return my_contents

