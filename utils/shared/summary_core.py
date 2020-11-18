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
# Threading Base
import re
from collections import defaultdict

from shared.path_utils import PathUtils
from shared.sys_utils import SysUtils
from shared.msg_utils import Msg
from shared.datetime_utils import DateTime
from shared.collections import HiThreadedProducerConsumerQueue
from shared.threads import HiThread, workers_done_event, summary_done_event

class SummaryLevel:
    Silent = 0      # only show summary info, log all results to summary log
    Fail = 1        # show only the fails,
    Any  = 2

class SummaryDetail:
    Nothing     = 0x00
    ForceCmd    = 0x01
    ForceResult = 0x02
    IssCmd      = 0x04
    IssResult   = 0x08
    AnyDetail   = 0xFF

# class SummaryThread( HiThread ): pass


class SummaryQueueItem( object ):
    def __init__( self, arg_process_info ): # arg_frun_path, arg_parent_fctrl, arg_fctrl_item, arg_group ):
        self.process_info = arg_process_info

class SummaryErrorQueueItem( object ):
    def __init__( self, arg_error_info ):
        self.error_info = arg_error_info

class SummaryErrorItem( object ):
    def load( self, arg_error_qitem ):
        self.extract_error( arg_error_qitem )
        self.report()

    def extract_error( self, arg_error_qitem ):
        self.error_item = arg_error_qitem.error_info

    def get_err_line( self ):
        my_err_type = str( self.error_item["type"] ).replace( "<class '","" ).replace( "'>","" )

        return  "[ERROR] Type: %s, Error: %s, Path: \"%s\", Message: %s" % ( my_err_type
                                                                       , str( self.error_item["error"] )
                                                                       , str( self.error_item["path"] )
                                                                       , str( self.error_item["message"] )
                                                                       )

    def report( self ):
        pass
        # self.prepare

class SummaryQueue( HiThreadedProducerConsumerQueue ):
    def __init__( self ):
        super().__init__(True)

class SummaryItem( object ):

    def load( self, arg_queue_item ):
        try:
            # my_usr_lbl = Msg.set_label( "user", "SUMMARY_ITEM" )
            self.load_process_info( arg_queue_item.process_info )
            # Msg.lout( self, "user", "Summary Item(load_process_info)" )
            self.load_task_info( )
            # Msg.lout( self, "user", "Summary Item(load_task_info)" )
            self.load_process_log()

            # Msg.lout( self, "user", "Summary Item(load_process_log)" )
            self.load_force_log()

            # Msg.lout( self, "user", "Summary Item(load_force_log)" )
            self.load_force_elog()
            # Msg.lout( self, "user", "Summary Item(load_force_elog)" )
            self.prepare()
            self.report()
            # Msg.set_label( "user", my_usr_lbl )

        except Exception as arg_ex:
            Msg.error_trace()
            Msg.err( str( arg_ex ))

        except:
            Msg.error_trace()

    def prepare( self ):
        pass

    def report( self ):

        if self.default    is None: self.default    = 0
        if self.secondary is None: self.secondary = 0
        if self.total     is None: self.total     = 0

        Msg.info( "Task Id: %s, Task Index: %d" % ( self.task_id, self.task_index ))
        my_msg = "Process Log Contains "
        if self.detail_flags & SummaryDetail.AnyDetail   == SummaryDetail.Nothing    : my_msg = "Process No Found or is Empty"
        else:
            if self.detail_flags & SummaryDetail.ForceCmd    == SummaryDetail.ForceCmd   : my_msg += "ForceCommand, "
            if self.detail_flags & SummaryDetail.ForceResult == SummaryDetail.ForceResult: my_msg += "ForceResult, "
            if self.detail_flags & SummaryDetail.IssCmd      == SummaryDetail.IssCmd     : my_msg += "IssCommand, "
            if self.detail_flags & SummaryDetail.IssResult   == SummaryDetail.IssResult  : my_msg += "IssResult, "
            my_msg = my_msg[:-2]

        # Msg.user( my_msg )

        # Msg.info( "Originating Control File: %s"  % ( self.parent_fctrl ))
        # Msg.info( "Control File Item: %s"  % ( str(  self.fctrl_item )))
        # Msg.info( "F-Run Control File: %s" % ( self.frun_path ))
        Msg.dbg( "Force Ret Code: %s" % ( str( self.force_retcode )))
        Msg.info( "[%s], Generate Command: %s" % ( SysUtils.ifthen( SysUtils.success( self.force_retcode ), "SUCCESS", "FAILED" ), str( self.force_cmd )))
        Msg.info( "Instructions Generated - Default: %d, Secondary: %d, Total: %d" % ( self.default, self.secondary, self.total ))

        if self.iss_cmd is not None:
            if self.iss_success():
                Msg.info( "[SUCCESS], ISS Command: %s" % ( str( self.iss_cmd   )))
            else:
                Msg.info( "[FAILED], ISS Command: %s" % ( str( self.iss_cmd   )))
            #Msg.fout( self.iss_log, "user" )
        Msg.blank()

    def iss_success( self ):
        return True

    def has_generate( self ):
        return self.force_cmd is not None

    def has_simulate( self ):
        return self.iss_cmd is not None

    def load_process_info( self, arg_process_info ):


        # update the user label and send iteration message to the screen if user is active
        # my_usr_lbl = Msg.set_label( "user", "PROCESS-RESULT" )
        # Msg.user( "Executing Iteration #%d of Test File: %s" % ( my_ndx + 1, arg_task_file ))

        Msg.dbg( "self.process_result: (%s)" % ( str( arg_process_info.get("process-result", None ))))
        # Msg.set_label( "user", my_usr_lbl )

        self.process_cmd    = arg_process_info.get("process-cmd"   , None )
        self.process_log    = arg_process_info.get("process-log"   , None )
        self.process_result = arg_process_info.get("process-result", None )
        self.frun_path      = arg_process_info.get("frun-path"     , None )
        self.parent_fctrl   = arg_process_info.get("parent-fctrl"  , None )
        self.fctrl_item     = arg_process_info.get("fctrl-item"    , None )
        self.item_group     = arg_process_info.get("item-group"    , None )
        self.fctrl_content  = arg_process_info.get("content"       , None )

        self.detail_flags = SummaryDetail.Nothing


    def load_task_info( self ):


        self.task_id   = None
        self.task_index = None
        self.work_dir  = None
        self.work_dir, my_tmp = PathUtils.split_path( self.frun_path )
        my_tmp, my_index = PathUtils.split_dir( self.work_dir )
        my_tmp, self.task_id = PathUtils.split_dir( my_tmp )
        self.task_index = int( my_index )


    def load_process_log( self ):

        self.force_cmd     = None
        self.force_elog    = None
        self.force_log     = None
        self.force_retcode = None
        self.force_stderr  = None
        self.force_stdout  = None
        self.force_level   = SummaryLevel.Any
        self.force_start   = 0.00
        self.force_end     = 0.00

        self.iss_cmd       = None
        self.iss_log       = None
        self.iss_retcode   = None
        self.max_instr     = None
        self.min_instr     = None

        # Msg.fout( self.process_log, "dbg" )
        with open( self.process_log , "r" ) as my_flog:
            try:
                for my_line in my_flog:
                    Msg.dbg("Load: %s" % my_line)
                    self.load_process_line( my_line )
            except Exception as arg_ex:
                Msg.error_trace( arg_ex )
                Msg.err( str( arg_ex ))
            finally:
                my_flog.close()

        # here is the lowdown, if a generate line exists in the process log then a Result line must also exists,

        if ( self.detail_flags & SummaryDetail.ForceCmd ) and not ( self.detail_flags & SummaryDetail.ForceResult ):
            self.load_gen_result( { "force-retcode": self.process_result[0]
                                  , "force-stdout" : self.process_result[2]
                                  , "force-stderr" : self.process_result[2]
                                  , "force_start"  : self.process_result[3]
                                  , "force_end"    : self.process_result[4]
                                  } )

        elif ( self.detail_flags & SummaryDetail.IssCmd ) and not ( self.detail_flags & SummaryDetail.IssResult ):
            self.load_iss_result( { "iss-retcode" : self.process_result[0]
                                  , "iss-log"     : None
                                  } )


    def load_force_log( self, arg_seed_only = False ):

        self.default = None
        self.secondary = None
        self.total = None
        self.seed = None
        self.task_path = PathUtils.include_trailing_path_delimiter( self.work_dir )

        my_glog = "%s%s" % ( self.task_path, self.force_log )

        Msg.dbg( "Path: %s" % my_glog)
        # Msg.user( "Opening Generator Log File: %s" % ( my_glog ))
        with open( my_glog, "r" ) as my_flog:
            try:
                for my_line in my_flog:

                    if SysUtils.found( my_line.find( "Secondary Instructions Generated" )):
                        # my_secondary = my_line.replace( "[notice]Secondary Instructions Generated:", "" ).strip()
                        # self.secondary = int( my_secondary )
                        my_lpos = my_line.find( ':' )
                        my_count = int( my_line[my_lpos+2:].strip())
                        # Msg.user( "Secondary Instructions: %d" % ( my_count ))
                        self.secondary = my_count

                    elif SysUtils.found( my_line.find( "Default Instructions Generated" )):
                        # my_pos = my_line.find( ":" ) + 2
                        # my_default = my_line[ my_pos: ].strip()
                        # get the count for this instruction type

                        my_lpos = my_line.find( ':' )
                        my_count = int( my_line[my_lpos+2:].strip())
                        # Msg.user( "Default Instructions: %d" % ( my_count ))
                        self.default = my_count

                        # my_default = my_line.replace( "[notice]Default Instructions Generated:", "" ).strip()
                        # self.default = int( my_default )

                    if SysUtils.found( my_line.find( "Total Instructions Generated" )):
                        self.total = int( my_line.replace( "[notice]Total Instructions Generated: ", "" ).strip())

                        my_lpos = my_line.find( ':' )
                        my_count = int( my_line[my_lpos+2:].strip())
                        # Msg.user( "Total Instructions: %d" % ( my_count ))
                        self.total = my_count

                    if SysUtils.found( my_line.find( "Initial seed" )):
                        self.seed = my_line.replace( "[notice]", "" ).replace( "Initial seed = ", "" ).strip()
                        # Msg.dbg( "Seed: %s" % ( self.seed ))

                        # for simulation only the seed is needed
                        if arg_seed_only:
                            break
                    if not( self.seed is None or self.total is None or self.secondary is None  or self.default is None ):
                        break
            except Exception as arg_ex:
                # NOTE: Determine the possible errors and handle accordingly, for now just keep processing
                Msg.error_trace()
                Msg.err( str( arg_ex ))

            finally:
                my_flog.close()


    def load_force_elog( self ):
        self.force_msg = None
        my_elog = "%s%s" % ( PathUtils.include_trailing_path_delimiter( self.work_dir ), self.force_elog )
        # Msg.dbg( my_elog )

        if SysUtils.failed( self.force_retcode ):
            Msg.fout( my_elog, "dbg" )
            with open(  my_elog , "r" ) as my_flog:
                try:
                    for my_line in my_flog:

                        if SysUtils.found( my_line.find( "[fail]" )):
                            self.force_msg = my_line.replace( "[fail]", "" ).strip()
                            # Msg.dbg( "Message: %s" % ( str( self.force_msg  )))
                            break
                finally:
                    my_flog.close()


    def load_process_line( self, arg_line ):

        if SysUtils.found( arg_line.find( "ForceCommand" )):
            my_glb, my_loc = SysUtils.exec_content( arg_line )
            self.load_gen_info( my_loc["ForceCommand"] )
            self.detail_flags |= SummaryDetail.ForceCmd

        elif SysUtils.found( arg_line.find( "ForceResult"  )):
            my_glb, my_loc = SysUtils.exec_content( arg_line )
            self.load_gen_result( my_loc["ForceResult" ] )
            self.detail_flags |= SummaryDetail.ForceResult

        elif SysUtils.found( arg_line.find( "ISSCommand"   )):
            my_glb, my_loc = SysUtils.exec_content( arg_line )
            self.load_iss_info( my_loc["ISSCommand"] )
            self.detail_flags |= SummaryDetail.IssCmd

        elif SysUtils.found( arg_line.find( "ISSResult"    )):
            my_glb, my_loc = SysUtils.exec_content( arg_line )
            self.load_iss_result( my_loc["ISSResult" ] )
            self.detail_flags |= SummaryDetail.IssResult





    # load the force generate information
    def load_gen_info( self, arg_dict ):

        # Msg.lout( arg_dict, "user", "Generate Info Dictionary ... " )
        self.force_cmd = arg_dict["force-command"]
        self.force_log = arg_dict["force-log"]
        self.force_elog= arg_dict["force-elog"]
        self.max_instr = arg_dict["max-instr"]
        self.min_instr = arg_dict["min-instr"]


    # load the force generate results
    def load_gen_result( self, arg_dict ):

        # Msg.lout( arg_dict, "dbg", "Generate Results Dictionary ... " )
        try:
            self.force_retcode = int( str( arg_dict["force-retcode"] ).strip() )
        except :
            self.force_retcode = -1
            Msg.err( "Generate Return Code in unrecognizable format" )

        self.force_stdout = arg_dict["force-stdout" ]
        self.force_stderr = arg_dict["force-stderr" ]

        if SysUtils.failed( self.force_retcode ):
            self.force_level = SummaryLevel.Fail

        self.force_start = float( arg_dict.get("force-start", 0.00 ))
        self.force_end   = float( arg_dict.get("force-end"  , 0.00 ))

        # Msg.lout( self, "dbg", "General Summary Item ... " )


    # load the iss execution information
    def load_iss_info( self, arg_dict ):
        # Msg.lout( arg_dict, "user", "ISS Info Dictionary ... " )
        self.iss_cmd = arg_dict["iss-command"]


    # load the iss execution results
    def load_iss_result( self,arg_dict ):

        # Msg.lout( arg_dict, "user", "ISS Results Dictionary ... " )
        self.iss_log = arg_dict["iss-log"]
        try:
            self.iss_retcode = int( arg_dict["iss-retcode"] )
        except :
            self.iss_retcode = -1
            Msg.err( "ISS Return Code in unrecognizable format" )

    def commit( self ):

        my_gen_cnt = 0
        my_gen_ret = 0
        my_sim_cnt = 0
        my_sim_ret = 0
        my_tgt_name = ""

        if self.has_generate():
            #Msg.user( "if self.has_generate(): True" )
            my_gen_cnt = 1
            my_gen_ret = self.commit_generate()

        if self.has_simulate():

            #Msg.user( "if self.has_simulate(): True" )
            my_sim_cnt = 1
            my_sim_ret = self.commit_simulate()
            my_tgt_name = "%s%s" % ( self.task_path, SysUtils.ifthen( bool( my_sim_ret ), "PASS", "FAIL" ))
        else:
            my_tgt_name = "%s%s" % ( self.task_path, SysUtils.ifthen( bool( my_gen_ret ), "PASS", "FAIL" ))

        my_src_name = "%s%s" % ( self.task_path, "STARTED" )
        PathUtils.move( my_src_name, my_tgt_name )

        return ( my_gen_cnt, my_gen_ret, my_sim_cnt, my_sim_ret )

class SummaryGroups( object ):

    def __init__( self ):
        self.groups = {}
        self.queue = SummaryQueue()
        # self.group_lookup = []


    def update_groups( self, arg_group ):
        if not arg_group in self.groups:
            self.groups[ arg_group ] = []


    # adds an item to a group list if the group does not exist the list is created
    def add_item( self, arg_item ):
        # Msg.dbg( "Item Group: %s"% ( arg_item.item_group ))
        if not arg_item.item_group in self.groups:
            self.groups[ arg_item.item_group ] = []
        self.groups[ arg_item.item_group ].append( arg_item )

        # Msg.dbg( "Group Count: %d, Group %s Membership Count: %d" % ( len( self.groups ), arg_item.item_group, len( self.groups[ arg_item.item_group ])  ))

    # returns the item list associated with the group passed as argument
    def group_items( self, arg_group  ):
        return self.groups[ arg_group ]

    # return the list of groups
    def task_groups( self ):
        return self.groups

class SummaryThread( HiThread ):
    def __init__( self, sq, summary):
        self.summary_queue = sq
        self.summary = summary
        # We do not want the thread to launch until we've loaded all the properties
        super().__init__(True)

    def commit_item( self, arg_item ):

        if not arg_item.task_id in self.summary.tasks:
            self.summary.tasks[ arg_item.task_id ] = []
            self.summary.task_lookup.append( arg_item.task_id  )
        self.summary.groups.update_groups( arg_item.item_group  )
        return 0

    def run( self ):
        # Block on process queue while we have threads running and stuff to do

        # == Replaced ==>> while True:
        # == Replaced ==>>     # Pop off the top of the process queue (should block if the queue is empty)
        # == Replaced ==>>     try:
        # == Replaced ==>>         next_item = self.summary_queue.dequeue(0)
        # == Replaced ==>>     except TimeoutError:
        # == Replaced ==>>         if (workers_done_event.isSet()):
        # == Replaced ==>>             summary_done_event.Signal()
        # == Replaced ==>>             return
        # == Replaced ==>>         else:
        # == Replaced ==>>             self.HeartBeat()
        # == Replaced ==>>             continue
        # == Replaced ==>>     my_item = self.summary.create_summary_item()
        # == Replaced ==>>     my_item.load( next_item )
        # == Replaced ==>>     self.summary.commit_item( my_item )
        # == Replaced ==>>     next_item = None

        try:
          while True: #not workers_done_event.isSet():
                # Pop off the top of the process queue (should block if the queue is empty)
                try:
                    # my_item = self.summary.create_summary_item()
                    # my_qitem = self.summary_queue.dequeue(0)
                    # my_item.load( my_qitem )
                    # self.summary.commit_item( my_item )
                    # my_qitem = None
                    # my_qitem = self.summary_queue.dequeue(0)

                    my_qitem = self.summary_queue.dequeue(0)
                    if isinstance( my_qitem, SummaryErrorQueueItem):
                        Msg.user( str( my_qitem.error_info ), "SUMMARY_ERROR" )
                        my_eitem = SummaryErrorItem()
                        my_eitem.load( my_qitem )
                        self.summary.commit_error_item( my_eitem )
                    else:
                        my_item = self.summary.create_summary_item()
                        my_item.load( my_qitem )
                        self.summary.commit_item( my_item )
                    my_qitem = None
                except TimeoutError as arg_ex:
                    # {{{TODO}}} Implement proper heartbeat
                    # Msg.dbg( str( arg_ex ) )
                    if (workers_done_event.isSet()):
                        break
                    else:
                        self.HeartBeat()
                        continue
                except Exception as arg_ex:
                    Msg.error_trace()
                    Msg.err( str( arg_ex ))
                    raise
                except:
                    Msg.error_trace()
                    raise
        finally:
            summary_done_event.Signal()


class  Summary( object ):

    def __init__( self, arg_summary_dir ):
        self.summary_dir = arg_summary_dir
        self.tasks = defaultdict(list)
        self.errors = []
        self.groups = SummaryGroups()
        self.queue = SummaryQueue()

        # Launch a Summary Thread
        self.summary_thread = SummaryThread(self.queue, self)

    def lock( self ):
        return True;

    def unlock( self ):
        True == True

    def commit_error_item( self, arg_error ):
        self.errors.append( arg_error )

    def process_errors( self, arg_ofile ):

        if len( self.errors ) > 0:
            arg_ofile.write( "\n" )
            Msg.blank()
            for my_eitem in self.errors:
                my_err_line = str( my_eitem.get_err_line())
                Msg.info( my_err_line )
                arg_ofile.write( "%s\n" % ( my_err_line ))

    # abstracts
    def create_summary_item( self ): pass
    def commit_item( self, arg_item ): pass
    def process_summary( self, sum_level = SummaryLevel.Fail ): pass

