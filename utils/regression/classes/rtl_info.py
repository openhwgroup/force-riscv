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
# File: rtl_info.py
# Comment: class to manage the data necessary to run simulations using rtl
# Contributors: Howard Maler, Amit Kumar, Jingliang(Leo) Wang

# since there will be a significant amount of directory manipulation import the path utils

from common.path_utils import PathUtils
from common.sys_utils import SysUtils
from common.datetime_utils import DateTime
from common.msg_utils import Msg
from common.errors import *

# REGR_ID = regr_id, can modify with make option regr_id
# OUTPUT_DIR = ./logs, can be modified with make option regr
# BUILD_DIR := $(OUTPUT_DIR)/build
# BUILD_CFG = target_config, can modify with make option bld_cfg
# BUILD_CFG_DIR := $(BUILD_DIR)/$(BUILD_CFG)
# SIM_EXE_FILE := $(BUILD_CFG_DIR)/uvm_simv_opt
# SIM_EXE_FILE := $(BUILD_CFG_DIR)/uvm_simv_dbg if dump=on

class RtlInfo( object ):

    def __init__( self, arg_rtl_data ):
        
        self.root = arg_rtl_data.get( "root", "$(PROJ_ROOT)")
        self.path = arg_rtl_data.get( "path", "verif/top/sim"  )
        self.regr = arg_rtl_data.get( "regr", "$(RTL_REGR)"    ) #"logs"          )
        self.tgt  = arg_rtl_data.get( "tgt" , "build"          )  
        self.cfg  = arg_rtl_data.get( "cfg" , "$(RTL_CFG)"     ) # "target_config"  )  
        self.name = arg_rtl_data.get( "name", "$(RTL_EXE)"     ) # "uvm_simv_opt" ) 
        self.cmd  = arg_rtl_data.get( "cmd" , "%s/%s/%s/%s/%s/%s" )   # root/path/regr/tgt/cfg/name
        self.debug_cfg = arg_rtl_data.get("debug_cfg", "target_config_waves") #replaces cfg for building debug rerun command for getting wave dumps
        self.debug_name = arg_rtl_data.get("debug_name", "uvm_simv_dbg") #name of the debug rtl executable, used for getting wave dumps
        self.default_fsdb_do = arg_rtl_data.get("default_fsdb_do", "/verif/top/tests/wave_fsdb.do") #copies a user modifiable do file into the test results directory to help them select signals for a debug wave dump rerun
        self._command = None    
        self._debug_command = None

        self.prepare()

        if self.root is None:
            raise Exception( "Unable to extract RTL Root, check config or $(PROJ_ROOT) set by sourcing ./scripts/project.cshrc" )

    def prepare( self ):
        
        if self.root.startswith( "$(") and self.root.endswith( ")" ):
            self.root = PathUtils.exclude_trailing_path_delimiter( SysUtils.envar( self.root[2:-1], None ))             

        self.default_fsdb_do = PathUtils.append_path(self.root, self.default_fsdb_do)
        
        if self.regr.startswith( "$(") and self.regr.endswith( ")" ):
            self.regr = PathUtils.exclude_trailing_path_delimiter( SysUtils.envar( self.regr[2:-1], "logs" ))  
        else:    
            self.regr = "logs" 
        
        if self.cfg.startswith( "$(") and self.cfg.endswith( ")" ):
            self.cfg = PathUtils.exclude_trailing_path_delimiter( SysUtils.envar( self.cfg[2:-1], "target_config" ))   
        else: 
            self.cfg = "config_name"
            
        if self.name.startswith( "$(") and self.name.endswith( ")" ):
            self.name = PathUtils.exclude_trailing_path_delimiter( SysUtils.envar( self.name[2:-1], "uvm_simv_opt" ))   
        else: 
            self.cfg = "uvm_simv_opt"

        self.path = PathUtils.exclude_trailing_path_delimiter( self.path )
        self.tgt  = PathUtils.exclude_trailing_path_delimiter( self.tgt  )

        self._debug_command = self.cmd % ( self.root, self.path, self.regr, self.tgt, self.debug_cfg, self.debug_name ) 
            
    @property    
    def command( self ):
        if self._command is None:
            self._command = self.cmd % ( self.root, self.path, self.regr, self.tgt, self.cfg, self.name ) 
        return self._command
        
