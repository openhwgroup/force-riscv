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
import re


from common.path_utils import PathUtils
from common.sys_utils import SysUtils
from common.msg_utils import Msg
from common.datetime_utils import DateTime

from classes.summary import Summary, SummaryItem, SummaryGroups, SummaryLevel


class RegressionSummaryItem( SummaryItem ):

    # save the initial generation information for this task item
    def commit_generate( self ):

        # Msg.dbg( "RegressionSummaryItem::commit_generate" )
        self.force_result = SysUtils.ifthen( SysUtils.success( self.force_retcode ), "PASS", SysUtils.ifthen( self.signal_id is None, "FAIL", "INCOMPLETE" ) )
        self.force_level = SummaryLevel.Any

        if SysUtils.failed( self.force_retcode ):
            self.force_level = SummaryLevel.Fail
            Msg.user( "self.force_retcode = %s" % ( str( self.force_retcode )) )

        Msg.user( "self.force_level = %s" % ( str( self.force_level)) )
        return SysUtils.ifthen( SysUtils.success( self.force_retcode ), 1 , 0 )

    def commit_simulate( self ):

        try:
            # check the return code for error
            if SysUtils.success( self.iss_retcode ):
                # Check to see for instruction overrun
                if int( self.instr_count ) < self.max_instr:
                    if int( self.instr_count ) >= self.min_instr:
                        self.iss_result = "PASS"
                        self.iss_level  = SummaryLevel.Any
                        return 1

            self.iss_result = SysUtils.ifthen( self.signal_id is None, "FAIL", "INCOMPLETE" )
            self.iss_level  = SummaryLevel.Fail
            return 0
        finally:
            pass
            # Msg.user( "Iss Result: %s" % ( self.iss_result ))

    # save the initial RTL run information for this task item
    def commit_rtl( self ):
        # Msg.dbg( "RegressionSummaryItem::commit_generate" )
        if SysUtils.success( self.rtl_retcode ):
            self.rtl_result = "PASS"
            self.rtl_level = SummaryLevel.Any
            commit_count = 1
        else:
            self.rtl_result = "FAIL"
            self.rtl_level = SummaryLevel.Fail
            Msg.user( "self.rtl_retcode = %s" % ( str( self.rtl_retcode )) )
            commit_count = 0

        Msg.user( "self.rtl_level = %s" % ( str( self.rtl_level)) )
        return commit_count        

    def commit_trace_cmp( self ):

        # Msg.dbg( "RegressionSummaryItem::commit_generate" )
        self.trace_cmp_result = SysUtils.ifthen( SysUtils.success( self.trace_cmp_retcode ), "PASS", SysUtils.ifthen( self.signal_id is None, "FAIL", "INCOMPLETE" ) )
        self.trace_cmp_level = SummaryLevel.Any

        if SysUtils.failed( self.trace_cmp_retcode ):
            self.trace_cmp_level = SummaryLevel.Fail
            Msg.user( "self.trace_cmp_retcode = %s" % ( str( self.trace_cmp_retcode )) )

        Msg.user( "self.trace_cmp_level = %s" % ( str( SummaryLevel.Fail)) )
        return SysUtils.ifthen( SysUtils.success( self.trace_cmp_retcode ), 1 , 0 )


    def iss_success( self ):

        if SysUtils.success( self.iss_retcode ):
            # Check to see for instruction overrun
            if int( self.instr_count ) < self.max_instr:
                if int( self.instr_count ) >= self.min_instr:
                    return True
        return False


    def get_gen_line( self ):

        my_gline = "Generate[%s] - Return Code: %2d, Test Dir: %s/%05d" % ( self.force_result, self.force_retcode, self.task_id, self.task_index )

        my_msg = self.seed
        Msg.dbg( "Seed: %s" % ( my_msg))
        if not my_msg is None:
            my_gline += ", Seed: %s" % ( str( my_msg ))

        # show the seed if one exists
        my_msg = self.force_msg
        if not my_msg is None:
            my_gline += ", Message: %s" % ( str( my_msg ))
        my_gline += "\n"
        # Msg.user( "get_gen_line: %s" % ( my_gline ))
        return my_gline


    def get_iss_line( self ):

        my_sline = None
        my_sline = "ISS-Sim[%s] - Return Code: %2d, Test Dir: %s/%05d" % ( self.iss_result, self.iss_retcode, self.task_id, self.task_index )

        my_msg = self.iss_message
        if not my_msg is None:
            my_sline += ", Message: %s" % ( str( my_msg ))

        my_sline += "\n"
        return my_sline

    def get_rtl_line( self ):
        my_sline = None
        my_sline = "RTL-Sim[%s] - Return Code: %2d, Test Dir: %s/%05d" % ( self.rtl_result, self.rtl_retcode, self.task_id, self.task_index )

        my_msg = self.seed
        Msg.dbg( "Seed: %s" % ( my_msg))
        # show the seed if one exists
        if not my_msg is None:
            my_sline += ", Seed: %s" % ( str( my_msg ))

        # show the message if one exists
        my_msg = self.rtl_message
        if not my_msg is None:
            my_sline += ", Message: %s" % ( str( my_msg ))

        my_sline += "\n"
        return my_sline


    def get_trace_cmp_line( self ):
        Msg.user( "RegressionSummaryItem::get_trace_cmp_line - 0", "GOT HERE" )

        my_tline = None
        my_tline = "Compare[%s] - Return Code: %2d, Test Dir: %s/%05d" % ( self.trace_cmp_result, self.trace_cmp_retcode, self.task_id, self.task_index )

        Msg.user( "RegressionSummaryItem::get_trace_cmp_line - 1", "GOT HERE" )
        Msg.user( "Compare[%s] - Return Code: %2d, Test Dir: %s/%05d" % ( self.trace_cmp_result, self.trace_cmp_retcode, self.task_id, self.task_index ) )

        # show the seed if one exists
        my_msg = self.trace_cmp_msg
        Msg.user( "RegressionSummaryItem::get_trace_cmp_line - 2", "GOT HERE" )
        if not my_msg is None:
            Msg.user( "RegressionSummaryItem::get_trace_cmp_line - 3", "GOT HERE" )
            my_tline += ", Message: %s" % ( str( my_msg ))
        # show the seed if one exists
        my_tline += "\n"

        Msg.user( "RegressionSummaryItem::get_trace_cmp_line - 4", "GOT HERE" )

        return my_tline

class RegressionSummary( Summary ):

    def __init__( self, arg_summary_path, arg_keep ):

        super(). __init__( arg_summary_path, arg_keep )
        self.gen_total  = 0
        self.gen_passed = 0
        self.iss_total  = 0
        self.iss_passed = 0
        self.rtl_total = 0
        self.rtl_passed = 0
        self.trace_cmp_total  = 0
        self.trace_cmp_passed = 0
        self.task_total = 0
        self.start_time = DateTime.Time()
        self.total_cycle_count = 0
        self.total_instruction_count = 0

    def create_summary_item( self ):
        my_item = RegressionSummaryItem( self )
        return my_item

    def commit_item( self, arg_item ):

        # Msg.user( "def commit( self, arg_item ):" );
        self.task_total += 1

        # replaced tasks dict with defaultdict(list) in parent
        self.tasks[ arg_item.task_id ].append( arg_item )
        self.groups.add_item( arg_item  )

        my_results = arg_item.commit()
        # Msg.lout( arg_item, "user", "Final Committed SummayItem .... " )
        Msg.user( "Commit Results: %s" % ( str( my_results )), "CMT-RESULTS")

        self.gen_total  += my_results[0]
        self.gen_passed += my_results[1]
        self.iss_total  += my_results[2]
        self.iss_passed += my_results[3]
        self.rtl_total  += my_results[4]
        self.rtl_passed += my_results[5]
        self.trace_cmp_total  += my_results[6]
        self.trace_cmp_passed += my_results[7]



    # Create a summary file to contain the results from every attempted generation and simulation and process each operation and
    # produce summarized totals for quick analysis
    def process_summary( self, arg_sum_level = SummaryLevel.Fail ):

        # Msg.user( "process_summary()", "REG-SUMMARY" )
        my_utcdt = DateTime.UTCNow()
        my_file_name = "%sregression_summary.log" % ( PathUtils().include_trailing_path_delimiter( self.summary_dir ))
        # Msg.user( "Master Log File: %s" % ( my_file_name ))
        my_ofile = None
        myLines = []
        # First try to open file
        with open( my_file_name, "w" ) as my_ofile:
            try:
                my_ofile.write( "Date: %s\n" % ( DateTime.DateAsStr( my_utcdt )))
                my_ofile.write( "Time: %s\n" % ( DateTime.TimeAsStr( my_utcdt )))
                self.process_errors( my_ofile )
                my_instr_count, my_cycle_count = self.process_summary_tasks( my_ofile, arg_sum_level )
                self.process_summary_totals( my_ofile, my_instr_count, my_cycle_count )

            except Exception as arg_ex:
                Msg.error_trace()
                Msg.err( "Processing Summary, " + str( arg_ex ))

            finally:
                my_ofile.close()
                # Msg.set_label( "user", my_usr_lbl )

    # iterate through the tasks dictionary and process the iterms associated with each task
    def process_summary_tasks( self, arg_ofile, arg_sum_level ):
        Msg.blank( "info" )
        my_instr_count = 0
        my_cycle_count = 0
        # get the outer task
        for my_key in self.tasks:
            try:
                arg_ofile.write( "\nTask: %s\n" % ( my_key ))
                task_instr_count, task_cycle_count = self.process_summary_task( arg_ofile, self.tasks[ my_key ], arg_sum_level )
                my_instr_count += task_instr_count
                my_cycle_count += task_cycle_count
            except:
                Msg.error_trace()
                Msg.err( "Processing Task %s, Skipping Item ...." % ( my_key ))

        self.total_instruction_count = my_instr_count
        self.total_cycle_count = my_cycle_count
        return my_instr_count, my_cycle_count

    # iterate through and process each item associated with this task
    def process_summary_task( self, arg_ofile, arg_task, arg_sum_level ):
        my_instr_count = 0
        my_cycle_count = 0

        # Msg.lout( arg_task, "user", "process_summary_task" )
        for my_item in arg_task:
            try:
                # Msg.user("arg_sum_level: %s, my_item.force_level: %s " % (str( arg_sum_level ), str( my_item.force_level )))
                # Msg.lout( my_item, "user", "Process Summary Item" )
                if my_item.has_generate():
                    # post the generate results
                    my_outline = my_item.get_gen_line()
                    if not my_outline is None:
                        arg_ofile.write( my_outline )
                        if arg_sum_level >= my_item.force_level:
                            Msg.info( my_outline )

                if my_item.has_simulate():
                    # post the simulate results
                    my_outline = my_item.get_iss_line()
                    if not my_outline is None:
                        arg_ofile.write( my_outline )
                        if arg_sum_level >= my_item.iss_level:
                            Msg.info( my_outline )
                    my_instr_count += my_item.instr_count

                if my_item.has_rtl():
                    # post the rtl run results
                    my_outline = my_item.get_rtl_line()
                    if not my_outline is None:
                        arg_ofile.write( my_outline )
                        if arg_sum_level >= my_item.rtl_level:
                            Msg.info( my_outline )
                    my_cycle_count += my_item.cycle_count

                try:
                    if my_item.has_trace_cmp():
                        # post the simulate results
                        my_outline = my_item.get_trace_cmp_line()
                        if not my_outline is None:
                            arg_ofile.write( my_outline )
                            if arg_sum_level >= my_item.trace_cmp_level:
                                Msg.info( my_outline )

                except:

                    Msg.user( "Exception Raised" , "GOT HERE" )

            except:
                Msg.error_trace()
                Msg.err( "Processing Task Index: %s" % ( str( my_item.index )))

        return my_instr_count, my_cycle_count

    # output the tasks summary and echo to stdout
    def process_summary_totals( self, arg_ofile, arg_instr_count, arg_cycle_count ):
        Msg.blank("info")

        # generator totals
        my_gfails =  self.gen_total - self.gen_passed
        my_lines =  "\nGenerate    : %3d\n" % ( self.gen_total )
        my_lines += "Generate Fails: %3d\n" % ( my_gfails )
        my_lines += "Generate Success Rate : %5.2f%%\n" % ( SysUtils.percent( self.gen_passed, self.gen_total ))
        my_lines += "\n"

        my_sfails = 0
        if self.iss_total >0:
            # ISS sim totals
            my_sfails =  self.iss_total - self.iss_passed
            my_lines += "ISS Sim   : %3d\n" % ( self.iss_total )
            my_lines += "ISS Sim Fails: %3d\n" % ( my_sfails )
            my_lines += "ISS Sim Success Rate : %5.2f%%\n" % ( SysUtils.percent( self.iss_passed, self.iss_total ))
            my_lines += "\n"

            my_lines += "Total Instructions Emulated: %3d\n" % ( arg_instr_count )
            my_lines += "\n"

        my_rfails = 0
        if self.rtl_total >0:
            # RTL sim totals
            my_rfails =  self.rtl_total - self.rtl_passed
            my_lines += "RTL Sim   : %3d\n" % ( self.rtl_total )
            my_lines += "RTL Sim Fails: %3d\n" % ( my_rfails )
            my_lines += "RTL Sim Success Rate : %5.2f%%\n" % ( SysUtils.percent( self.rtl_passed, self.rtl_total ))
            my_lines += "\n"

            my_lines += "Total Cycle Count: %3d\n" % ( arg_cycle_count )
            my_lines += "\n"

        my_cfails = 0
        if self.trace_cmp_total > 0:
            # simulation totals
            my_cfails = self.trace_cmp_total - self.trace_cmp_passed
            my_lines += "Compared     : %3d\n" % ( self.trace_cmp_total )
            my_lines += "Compare Fails: %3d\n" % ( my_cfails )
            my_lines += "Compare Success Rate : %5.2f%%\n" % ( SysUtils.percent( self.trace_cmp_passed, self.trace_cmp_total ))
            my_lines += "\n"


        # task totals
        my_task_total = ( self.task_total)
        my_total_fails =  my_gfails + my_sfails + my_cfails + my_rfails
        my_lines += "Total Tasks     : %3d\n" % ( my_task_total )
        my_lines += "Task Fails      : %3d\n"   % ( my_total_fails )
        my_lines += "Task Success Rate: %5.2f%%\n" % ( SysUtils.percent( my_task_total - my_total_fails, ( my_task_total )))
        my_lines += "\n"
        my_lines += "Total Run Time: %0.5f Seconds" % ( DateTime.Time() - self.start_time )

        Msg.blank("info")
        Msg.info( my_lines )
        Msg.blank("info")

        arg_ofile.write( my_lines )

