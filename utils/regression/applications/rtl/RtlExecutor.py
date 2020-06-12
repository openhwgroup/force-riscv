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
# file: rtl_executor.py
# comment: implements RtlExecutor which serves as a Class Wrapper for
#          for executing the rtl simulator family in client processing apps

from common.path_utils import PathUtils
from common.msg_utils import Msg
from common.sys_utils import SysUtils
from common.errors import *
# from launchers.lsf_launcher import LsfLauncher

from executors.iss_executor import *
from classes.control_item import ControlItem, CtrlItmKeys
# import shutil
import re


class RtlDefaults(object):
    skyros_mem_default     = 0
    RVBAR                  = 80000000
    tbench_elf64           = "%s"
    s_mem_init             = "fm_mem_init_s.hex"
    ns_mem_init            = "fm_mem_init_ns.hex"
    goldmem_level          = 1
    UVM_TC_PATH            = "."
    UVM_TC_NAME            = "%s"
    UVM_TC_SEED            = "%s"
    ntb_random_seed        = "%s"
    UVM_TEST_DIR           = "."
    assert_proc            = "nopostproc"
    RERUN_CMD_FILENAME     = "rerun_cmd"
    BSUB_RERUN_CMD_FILENAME= "bsub_rerun_cmd"
    RERUN_DUMP_CMD_FILENAME= "rerun_dump_cmd"
    BSUB_RERUN_DUMP_CMD_FILENAME="bsub_rerun_dump_cmd"


class RtlOption(object):
    def __init__( self, arg_op_action, arg_op_code, arg_op_val = None ):
        self.action = arg_op_action
        self.code   = arg_op_code
        self.val    = arg_op_val
        
    def __str__(self):
        my_ret = " %s%s" % ( self.action, self.code )
        if self.val is not None:
            return "%s=%s"% (my_ret, str( self.val )) 
        return my_ret


class RtlResult(ProcessResult):
    rtl_cycle_count = 0
    rtl_message     = 1
    rtl_passed      = 2

class RtlKeys( object ):
    rtl_retcode   = "retcode"
    rtl_stdout    = "stdout"
    rtl_stderr    = "stderr"
    rtl_start     = "start"
    rtl_end       = "end"
    rtl_count     = "count"
    rtl_message   = "message"
    # {{{TODO}}} following line deprecated
    rtl_log       = "log"
    
class RtlExecutor( IssExecutor ):

    def __init__( self ):
        super().__init__( )
        self.rtl_cmd = None
        self.rtl_log = None
        self.rtl = None
        self.yes_preload_boot_code = False
        self.preload_cmd = None

    def use_rtl(self):
        return len(self.ctrl_item.rtl) > 0
    
    def skip(self):
        if not self.use_rtl():
            Msg.user( "[RtlExecutor::skip] skipping due to no rtl information specified" )
            return True

        return False

    def load( self, arg_ctrl_item ):
        super().load(arg_ctrl_item )
        self.rtl = self.ctrl_item.rtl

    def filterPlusArgs(self, aPlusArgs):
        ## TODO These are mismatch between what we have with what the RTL make file produce, will need to verify if these are needed.
        args_list = aPlusArgs.split(' ')
        filtered_list = [ ]
        for arg in args_list:
            if arg in [
                    'goldmem_level=0', # ???
                    '+redistr_pl_override=16',
                    '+lsu_seq_cnt=0',
                    '+lsu_per_cnt=0',
                    '+ifu_seq_cnt=0',
                    '+force_keep_cycles=0',
                    '+mmu_ecc_random=10000',
                    '+l2c_seq_cnt=0',
                    '+mmu_seq_cnt=0',
                    '+lsu_ecc_random=10000',
                    '+ifu_per_cnt=0',
                    '+vcs+flush+log',
                    '+gic_bfm_test=gic_cpuif_upwrite_sv',
                    '+ifu_ecc_random=10000',
                    '+redistr_vl_override=16',
                    '+goldmem_level=0',
                    '+mmu_per_cnt=0',
                    '+l3_ecc_random=10000',
                    '+WATCH_DOG=6000',
                    '+force_hwp_en',
                    '+pre_cnt_corner_int_mode=10000',
                    '+int_cpu_num=6',
                    '+warn=noSC-TCMM-V5',
                    '+preload_tt=0',
                    '+force_delay_cycles=0',
                    '+l2c_ecc_random=10000',
                    '+CPU_NUM=0',
                    '+l2c_per_cnt=0' ]:
                continue
            else:
                #This plus arg is always present when force init is on
                if arg == "+FORCE_INIT":
                    self.yes_preload_boot_code = True
                #Since we're dealing with other scripts meant for use without Force, we have to peel back a few of their assumptions about directory structure
                elif "+force_reg_init_0_0=" in arg: 
                    last = arg.split('/')[-1]
                    arg = "+force_reg_init_0_0=" + last
                filtered_list.append(arg)
                    
        return ' '.join(filtered_list)

    def get_plus_args(self):
        rtl_root = self.rtl.get("root")
        meta_args = self.rtl.get("meta_args", "tc_mode=force railhouse=on rvbar=80000000 prefetch_drop_int=10 no_commit_cycles=6000") # by default set railhouse on

        # look for additional meta args if any
        add_meta_args = self.rtl.get("add_meta_args", "")
        if len(add_meta_args) > 0:
            meta_args = ' '.join([meta_args, add_meta_args])

        meta_converter = self.rtl.get("meta_converter")
        convert_cmd = meta_converter + " -r %s -m \"%s\"" % (rtl_root, meta_args)
        plus_args, is_valid = SysUtils.get_command_output(convert_cmd)
        Msg.user("Meta args converting command: %s" % convert_cmd, "META-CONV")
        if not is_valid:
            Msg.user("Conversion failed.", "META-CONV")
            raise Exception("Meta conversion failed: %s" % meta_args)
        else:
            Msg.user("Converted to plus args: %s" % plus_args, "META-CONV")

        if self.rtl.get("filter"):
            plus_args = self.filterPlusArgs(plus_args)
            Msg.user("Filtered plus args: %s" % plus_args, "META-CONV")
        return plus_args
        
    def build_cmd( self ):
        rtl_root = self.rtl.get("root")
        regr_val = self.rtl.get("regr")
        bld_cfg_val = self.rtl.get("bld_cfg")
        exe_val = self.rtl.get("exe")
        rtl_executable = "%s/verif/top/sim/%s/build/%s/%s" % (rtl_root, regr_val, bld_cfg_val, exe_val)

        plus_args = self.get_plus_args()        
        
        # finding ELF Files
        my_elf   = self.locate_test_case("*.Default.ELF"   , self.task_name)
        my_elfns = self.locate_test_case("*.Secondary.ELF", self.task_name)

        my_task_name = my_elf.replace(".Default.ELF", "")

        Msg.user( "Default Elf: %s, Secondary: %s" % ( str( my_elf ), str( my_elfns )))     

        self.rtl_log = ( "%s.log" % my_task_name )

        my_params = list()
        my_params.append( RtlOption( "+", "skyros_mem_default"             , RtlDefaults.skyros_mem_default             ))
        my_params.append( RtlOption( "+", "tbench_elf64"                   , RtlDefaults.tbench_elf64 % my_task_name    ))
        my_params.append( RtlOption( "+", "s_mem_init"                     , RtlDefaults.s_mem_init                     ))
        my_params.append( RtlOption( "+", "ns_mem_init"                    , RtlDefaults.ns_mem_init                    ))
        my_params.append( RtlOption( "+", "goldmem_level"                  , RtlDefaults.goldmem_level                  ))
        my_params.append( RtlOption( "+", "UVM_TC_PATH"                    , RtlDefaults.UVM_TC_PATH                    ))
        my_params.append( RtlOption( "+", "UVM_TC_NAME"                    , RtlDefaults.UVM_TC_NAME % my_task_name     ))
        my_params.append( RtlOption( "+", "UVM_TC_SEED"                    , RtlDefaults.UVM_TC_SEED % self.ctrl_item.seed ))
        my_params.append( RtlOption( "+", "ntb_random_seed"                , RtlDefaults.ntb_random_seed % self.ctrl_item.seed ))
        my_params.append( RtlOption( "+", "UVM_TEST_DIR"                   , RtlDefaults.UVM_TEST_DIR                   ))

        # next one is a bit of an oddball 
        my_params.append( RtlOption( "-", "reportstats -assert %s -l %s" % (RtlDefaults.assert_proc, self.rtl_log), None))  

        #################################################
        ## NOTE: DO NOT USE format to assemble string  ##
        #################################################
        
        self.rtl_cmd = rtl_executable + " " + plus_args + " " + self.assemble_cmd( my_params )
        
        #If FORCE_INIT is a detected argument we are preloading the boot code, assemble the appropriate commands
        if self.yes_preload_boot_code:
            img_modify_script = "%s/bin/%s" % (rtl_root, self.rtl.get("img_modify"))
            tst_handler_script = "%s/bin/%s" % (rtl_root, self.rtl.get("tst_handler"))
            current_dir = PathUtils.current_dir()
            name_in_dir = current_dir + "/" + my_task_name
            tst_filename = "%s/%s.tst" % (current_dir, my_task_name)

            #These are options from the makefile and their default values which have some meaning to the preload scripts.
            #Since we don't support MP, its not clear if they should be optional yet. 
            force_thread1 = "0"
            clusters = "1" 
            gpro_48 = "1"
            gpro_core_switch = "0"

            seed = self.ctrl_item.seed
            self.preload_cmd = "%s %s %s; %s %s %s %s %s %s %s %s; mkdir ELF; cp ./*.ELF ELF; cp ./*.img ELF;" % (img_modify_script, name_in_dir, current_dir, tst_handler_script, tst_filename, current_dir, force_thread1, clusters, gpro_48, gpro_core_switch, seed)
            #simply ";" splice the different commands together and run compound command like usual.
            self.rtl_cmd = self.preload_cmd + self.rtl_cmd
            #self.rtl_cmd.replace("UVM_NONE", "UVM_HIGH")#DEBUG


        Msg.user( "rtl_cmd: " + str( self.rtl_cmd ), "RTL-EXEC")
    
    ##
    #   Makes a copy of the waves_fsdb.do file found in a default location specified in the config file and copies it with a name
    #   change into the current working directory in order to support the rerun command scripts that have the dump option enabled.
    #
    #   By default the do file may enable a volume of signals that can lead to a significantly large wave dump that in some cases
    #   upon using the rerun scripts may exceed the disk quota of the unprepared.
    #
    def copyWavesFsdbDoFile(self):
        dofile_path = self.rtl.get("fsdb_do")
        
        if dofile_path is not None and type(dofile_path) is str:
            SysUtils.exec_cmd("cp %s %s/%s_waves_fsdb.do" % (dofile_path, PathUtils.current_dir(), self.task_name))
        
    ##
    #   Provides a meaningful rerun script at the disk location expected in the gathered report, such as in gathered.xml
    #
    #   Caveat: the fidelity of the rerun will depend on the environment variables set during the previous execution
    #   
    def outputRerunScript(self):
        #use current test directory as output location and write script that allows rerun of at least the RTL portion of the test.
        output_directory = PathUtils.current_dir()

        rerun_script_path = PathUtils.append_path(output_directory, RtlDefaults.RERUN_CMD_FILENAME)
        rerun_dump_script_path = PathUtils.append_path(output_directory, RtlDefaults.RERUN_DUMP_CMD_FILENAME)
        bsub_rerun_script_path = PathUtils.append_path(output_directory, RtlDefaults.BSUB_RERUN_CMD_FILENAME)
        bsub_rerun_dump_script_path = PathUtils.append_path(output_directory, RtlDefaults.BSUB_RERUN_DUMP_CMD_FILENAME)

        base_rerun_cmd = self.rtl_cmd.replace("UVM_NONE", "UVM_HIGH")
        base_rerun_dump_cmd = base_rerun_cmd.replace(self.rtl.get("exe"), self.rtl.get("debug_exe"))
        rerun_dump_cmd = base_rerun_dump_cmd + str(" +fsdbfile+./%s.fsdb -ucli -do ./%s_waves_fsdb.do" % (self.task_name, self.task_name))

        class LsfDefaults(object):
            Group = "trg"
            Queue = "normal"
            ThreadCount = 16
            CoreFileSize = 1
            lsf_log = "lsf.%J"

        bsub_prepend = "bsub -K -G %s -q %s -C %s -o " % (LsfDefaults.Group, LsfDefaults.Queue, LsfDefaults.CoreFileSize) + LsfDefaults.lsf_log 

        try:
            with open(str(rerun_script_path), "w+") as out_file:
                rerun_cmd_string = base_rerun_cmd 
                out_file.write(rerun_cmd_string)    
            PathUtils.chmod(rerun_script_path, 0o755) #The numeric argument here is an octal literal 

            with open(str(rerun_dump_script_path), "w+") as out_file:
                rerun_cmd_string = rerun_dump_cmd
                out_file.write(rerun_cmd_string)    
            PathUtils.chmod(rerun_dump_script_path, 0o755) #The numeric argument here is an octal literal 

            with open(str(bsub_rerun_script_path), "w+") as out_file:
                rerun_cmd_string = bsub_prepend + " \"" + base_rerun_cmd + "\""
                out_file.write(rerun_cmd_string)    
            PathUtils.chmod(bsub_rerun_script_path, 0o755) #The numeric argument here is an octal literal 

            with open(str(bsub_rerun_dump_script_path), "w+") as out_file:
                rerun_cmd_string = bsub_prepend + " \"" + rerun_dump_cmd + "\""
                out_file.write(rerun_cmd_string)    
            PathUtils.chmod(bsub_rerun_dump_script_path, 0o755) #The numeric argument here is an octal literal 

        except IOError as e:
            print("IO error({0}): {1}".format(e.errno, e.strerror))
            raise
        except:
            print("[RTL-SIM] Unhandled exception while attempting to write rerun command files")
            raise

    def extract_results( self, arg_result, arg_log, arg_elog ):

        # extract information from the generate log
        my_result, my_error = self.query_logs( arg_log, arg_elog )
        Msg.user( "Process: %s" % ( str( arg_result )), "RTL-SIM" )
        Msg.user( "Log[%s]: []" % ( str( arg_log    )), "RTL-SIM" )

        process_ret_code = int(arg_result[ RtlResult.process_retcode ])
        test_passed = my_result[RtlResult.rtl_passed]

        if ((process_ret_code == 0) and not test_passed):
            Msg.warn( "[RTL-SIM] Test Passed=%s, but process return code is %d, changing return code to 1" % ( str (test_passed), process_ret_code))
            process_ret_code = 1
        
        my_process_data = { RtlKeys.rtl_retcode : process_ret_code
                          , RtlKeys.rtl_stdout  : str(arg_result[ RtlResult.process_stdout  ])
                          , RtlKeys.rtl_stderr  : str(arg_result[ RtlResult.process_stderr  ])
                          , RtlKeys.rtl_start   : str(arg_result[ RtlResult.process_start   ])
                          , RtlKeys.rtl_end     : str(arg_result[ RtlResult.process_end     ])
                          , RtlKeys.rtl_count   : int(my_result[ RtlResult.rtl_cycle_count ])
                          , RtlKeys.rtl_message : str(my_result[ RtlResult.rtl_message     ])
                          # {{{TODO}}} following line deprecated
                          , RtlKeys.rtl_log     : arg_log
                          }

        return my_process_data

    def execute( self ):

        my_result = None
        test_passed = True

        try:
            self.build_cmd()

            self.copyWavesFsdbDoFile()
            self.outputRerunScript()
            
            # report the command line
            Msg.info( "RTLCommand = " + str( { "rtl-command": self.rtl_cmd } ))

            # execute the simulation
            my_result = SysUtils.exec_process( self.rtl_cmd, self.rtl_log, self.rtl_log, self.ctrl_item.timeout, True )

            Msg.user( "Results: %s" % ( str( my_result )), "RTLEX-RESULT" "")
            my_extract_results = self.extract_results( my_result, "./" + self.rtl_log, None )

            # report the results
            Msg.info( "RTLResult = " + str( my_extract_results ))

        except Exception as arg_ex:
            Msg.error_trace( "RTL Process Failure" )
            Msg.err( "RTL did not properly execute, Reason: %s" % ( str( arg_ex )))
            return False

        finally:
            pass

        #return SysUtils.success( int(my_result[ RtlResult.process_retcode ]) )
        return test_passed

    def open_log_file( self, pFileName, pOpenMode ):
        from lib.file_read_backwards import FileReadBackwards
        return FileReadBackwards( pFileName )

    # used to avoid abstract method error
    def query_result_log( self, arg_hfile ):
        cycle_count = 0
        test_passed = True
        error_found = False
        message = None

        success_keywords = ["PASSED", "TC_PASSED"]
        fail_keywords = ["FAILED", "failed", "corruption problem", "Error", "ERROR", "Segmentation fault", "TC_FAILED"]
        tool_fail_keywords = ["SLI error", "core dumped", "SIGHUP"]

        #for line in arg_hfile:
        for line in arg_hfile:
            if line.find("Total cycles : ") == 0:
                cycle_count = int(line [15:])
                continue

            if test_passed:
                if any(kw in line for kw in success_keywords):
                    break

                if any(kw in line for kw in fail_keywords):
                    test_passed = False
                    message = ""
                    continue
            
                if any(kw in line for kw in tool_fail_keywords):
                    test_passed = False
                    message = "TOOL FAIL: " + line + " ; "
                    continue
            else:
                if not error_found:
                    if line.find("UVM_FATAL") != -1:  
                        error_found = True
                        message += line
                        break
                    if line.find("ERROR") != -1:
                        error_found = True
                        message += line
                        break

        return cycle_count, message, test_passed

    def assemble_cmd( self, arg_params ):
        my_cmd = ""
        for my_index, my_param in enumerate( arg_params ):
            my_cmd += str( my_param )
        return my_cmd[1:]
        
        
        

