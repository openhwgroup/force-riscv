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
import sys
import traceback


class MsgLevel:
    nomsg = 0x0000  # 0000 0000 = 0
    crit = 0x0001  # 0000 0001 = 1
    err = 0x0002  # 0000 0010 = 2
    warn = 0x0004  # 0000 0100 = 4
    info = 0x0008  # 0000 1000 = 8
    dbg = 0x0010  # 0001 0000 = 16
    user = 0x0020  # 0010 0000 = 32
    trace = 0x0040  # 0100 0000 = 64
    noinfo = 0x0080  # 1000 0000 = 128

    @classmethod
    def translate(cls, arg_str):
        my_lev = MsgLevel.nomsg
        if arg_str[0] != "+" and arg_str[0] != "-":
            arg_str = "+" + arg_str

        if arg_str.find("all") >= 0:
            my_lev |= (
                MsgLevel.trace
                | MsgLevel.user
                | MsgLevel.dbg
                | MsgLevel.info
                | MsgLevel.warn
                | MsgLevel.err
                | MsgLevel.crit
            )
        if arg_str.find("+crit") >= 0:
            my_lev |= MsgLevel.crit
            # print( "Critical On" )
        if arg_str.find("+err") >= 0:
            # print( "Error On" )
            my_lev |= MsgLevel.err
        if arg_str.find("+warn") >= 0:
            my_lev |= MsgLevel.warn
            # print( "Warning On" )
        if arg_str.find("+info") >= 0:
            my_lev |= MsgLevel.info
            # print( "Info On" )
        if arg_str.find("+dbg") >= 0:
            my_lev |= MsgLevel.dbg
            # print( "Debug On" )
        if arg_str.find("+user") >= 0:
            my_lev |= MsgLevel.user
        if arg_str.find("+trace") >= 0:
            # print( "Trace On" )
            my_lev |= MsgLevel.trace
        if arg_str.find("+noinfo") >= 0:
            my_lev |= MsgLevel.noinfo
            # print( "NoInfo On" )

        if arg_str.find("-crit") >= 0:
            # print( "Critical Off" )
            my_lev &= ~MsgLevel.crit
        if arg_str.find("-err") >= 0:
            # print( "Error Off" )
            my_lev &= ~MsgLevel.err
        if arg_str.find("-warn") >= 0:
            # print( "Warn Off" )
            my_lev &= ~MsgLevel.warn
        if arg_str.find("-info") >= 0:
            # print( "Info Off" )
            my_lev &= ~MsgLevel.info
        if arg_str.find("-dbg") >= 0:
            # print( "Debug Off" )
            my_lev &= ~MsgLevel.dbg
        if arg_str.find("-user") >= 0:
            # print( "Debug Off" )
            my_lev &= ~MsgLevel.user
        if arg_str.find("-trace") >= 0:
            # print( "Trace Off" )
            my_lev &= ~MsgLevel.trace
        if arg_str.find("-noinfo") >= 0:
            my_lev &= ~MsgLevel.noinfo
            # print( "No Info Off" )

        return my_lev

    @classmethod
    def lev_as_str(cls, arg_lev):

        if arg_lev & (
            MsgLevel.trace
            & MsgLevel.user
            & MsgLevel.dbg
            & MsgLevel.info
            & MsgLevel.warn
            & MsgLevel.err
            & MsgLevel.crit
        ):
            return "all"

        my_str = ""
        if arg_lev & MsgLevel.trace:
            my_str += "+trace"
        if arg_lev & MsgLevel.user:
            my_str += "+user"
        if arg_lev & MsgLevel.dbg:
            my_str += "+dbg"
        if arg_lev & MsgLevel.info:
            my_str += "+info"
        if arg_lev & MsgLevel.warn:
            my_str += "+warn"
        if arg_lev & MsgLevel.err:
            my_str += "+err"
        if arg_lev & MsgLevel.crit:
            my_str += "+crit"

        return my_str


class MsgLabel:
    crit = "CRIT"
    err = "ERROR"
    warn = "WARN"
    info = "INFO"
    dbg = "DEBUG"
    user = "USER"
    trace = "CALLSTACK"

    # set custom label returns old value
    @classmethod
    def set_label(cls, arg_lev, arg_label):
        if isinstance(arg_lev, str):
            # little recursion solves a couple of problems
            return MsgLabel.set_label(MsgLevel.translate(arg_lev), arg_label)

        # now as a string set the new label and return as a
        my_ret = None
        if arg_lev == MsgLevel.info:
            my_ret = MsgLabel.info
            MsgLabel.info = arg_label
        elif arg_lev == MsgLevel.dbg:
            my_ret = MsgLabel.dbg
            MsgLabel.dbg = arg_label
        elif arg_lev == MsgLevel.user:
            # print( "user label: %s" % (MsgLabel.user))
            my_ret = MsgLabel.user
            MsgLabel.user = arg_label
        elif arg_lev == MsgLevel.crit:
            my_ret = MsgLabel.crit
            MsgLabel.crit = arg_label
        elif arg_lev == MsgLevel.err:
            my_ret = MsgLabel.err
            MsgLabel.err = arg_label
        elif arg_lev == MsgLevel.warn:
            my_ret = MsgLabel.warn
            MsgLabel.warn = arg_label
        elif arg_lev == MsgLevel.trace:
            my_ret = MsgLabel.trace
            MsgLabel.trace = arg_label
        return my_ret

    # get Msg Level Label
    @classmethod
    def get_label(cls, arg_lev):
        if arg_lev is str:
            # little recursion solves a couple of problems
            return MsgLabel.get_label(MsgLevel.translate(arg_lev))
        if arg_lev & MsgLevel.info:
            return str(MsgLabel.info)
        elif arg_lev & MsgLevel.dbg:
            return str(MsgLabel.dbg)
        elif arg_lev & MsgLevel.user:
            return str(MsgLabel.user)
        elif arg_lev & MsgLevel.crit:
            return str(MsgLabel.crit)
        elif arg_lev & MsgLevel.err:
            return str(MsgLabel.err)
        elif arg_lev & MsgLevel.warn:
            return str(MsgLabel.warn)
        elif arg_lev & MsgLevel.trace:
            return str(MsgLabel.trace)


class Msg:
    lev = (
        MsgLevel.crit
        | MsgLevel.err
        | MsgLevel.warn
        | MsgLevel.info
        | MsgLevel.noinfo
    )
    # TODO: This is a hack to default messaging to master run's specified
    #  default logging level. In the future, make the message utility
    #  completely independent of master run.

    # set the default message level
    @classmethod
    def set_level(cls, arg_lev=MsgLevel.info):
        Msg.lev = int(arg_lev)
        # print( "Message Level: %x" % (  arg_lev))

    # gets the current default level
    @classmethod
    def get_level(cls):
        return Msg.lev

    # gets the current default level
    @classmethod
    def get_level_as_str(cls):
        return MsgLevel.lev_as_str(Msg.lev)

    # translate the message level
    @classmethod
    def translate_levelstr(cls, arg_str):
        # print( "Level Str: %s" % ( str( arg_str )))
        return MsgLevel.translate(arg_str)

    # customize the message labels. It is recommended that only the user, info
    # and dbg label be changed however there is no prohibition against doing
    # this
    @classmethod
    def set_label(cls, arg_lev, arg_label):
        return MsgLabel.set_label(arg_lev, arg_label)

    # returns the current label for the specified level, if no level is passed
    # then the current log level is used
    @classmethod
    def get_label(cls, arg_lev=None):
        if arg_lev is None:
            arg_lev = Msg.lev
        return MsgLabel.get_label(arg_lev)

    # returns the current labels for the current msg level.
    @classmethod
    def get_names(cls, arg_lev=None):
        if arg_lev is None:
            arg_lev = Msg.lev
        my_str = ""
        if arg_lev & MsgLevel.Crit:
            my_str += "Critical Errors + "
        if arg_lev & MsgLevel.Err:
            my_str += "General Errors + "
        if arg_lev & MsgLevel.Warn:
            my_str += "Warning Messages + "
        if arg_lev & MsgLevel.Run:
            my_str += "Infomation Messages +"
        if arg_lev & MsgLevel.Dbg:
            my_str += "Trace Messages + "
        # strip the end and return the results
        if len(my_str) > 0:
            return my_str[:-3]
        return ""

    # write posts the message. Expand as needed to log to specific file
    @classmethod
    def write(cls, arg_msg, arg_level):
        if Msg.lev != MsgLevel.nomsg:
            if (arg_level & MsgLevel.info) and (Msg.lev & MsgLevel.noinfo):
                print(str(arg_msg).strip())
            else:
                print(
                    "[%s] - %s"
                    % (MsgLabel.get_label(arg_level), str(arg_msg).strip())
                )

    @classmethod
    def write(cls, arg_msg, arg_level):
        if Msg.lev != MsgLevel.nomsg:
            if (arg_level & MsgLevel.info) and (Msg.lev & MsgLevel.noinfo):
                print(str(arg_msg).strip())
            else:
                print(
                    "[%s] - %s"
                    % (MsgLabel.get_label(arg_level), str(arg_msg).strip())
                )

    @classmethod
    def write_nostrip(cls, arg_msg, arg_level, arg_notag=False):
        if Msg.lev != MsgLevel.nomsg:
            if arg_notag or (
                (arg_level & MsgLevel.info) and (Msg.lev & MsgLevel.noinfo)
            ):
                print(str(arg_msg).strip())
            else:
                print(
                    "[%s] - %s" % (MsgLabel.get_label(arg_level), str(arg_msg))
                )

    @classmethod
    def user(cls, arg_msg, arg_label=None):
        if Msg.lev & MsgLevel.user:
            if arg_label is not None:
                my_usr_lbl = Msg.set_label("user", str(arg_label))
                try:
                    Msg.write(arg_msg, MsgLevel.user)
                finally:
                    Msg.set_label("user", my_usr_lbl)
            else:
                Msg.write(arg_msg, MsgLevel.user)

    @classmethod
    def dbg(cls, arg_msg=None, arg_label=None):
        if Msg.lev & MsgLevel.dbg:
            if arg_msg is None:
                # add an empty line
                print()
            elif arg_label is not None:
                my_usr_lbl = Msg.set_label("dbg", str(arg_label))
                try:
                    Msg.write(arg_msg, MsgLevel.user)
                finally:
                    Msg.set_label("dbg", my_usr_lbl)
            else:
                Msg.write(arg_msg, MsgLevel.dbg)

    @classmethod
    def info(cls, arg_msg=None, arg_flush=False):

        # print( Msg.lev )
        if Msg.lev & MsgLevel.info:
            if arg_msg is None:
                # add an empty line
                print()
            else:
                Msg.write(arg_msg, MsgLevel.info)

        if arg_flush:
            sys.stdout.flush()
            sys.stderr.flush()

    @classmethod
    def warn(cls, arg_msg):
        # Msg.trace()
        if Msg.lev & MsgLevel.warn:
            Msg.write(arg_msg, MsgLevel.warn)

    @classmethod
    def err(cls, arg_msg):
        if Msg.lev & MsgLevel.err:
            Msg.write(arg_msg, MsgLevel.err)

    @classmethod
    def crit(cls, arg_msg):
        if Msg.lev & MsgLevel.crit:
            Msg.write(arg_msg, MsgLevel.crit)

    @classmethod
    def blank(cls, arg_lev=MsgLevel.info):
        if isinstance(arg_lev, str):
            arg_lev = Msg.blank(MsgLevel.translate(arg_lev))

        elif Msg.lev & arg_lev:
            print()

    @classmethod
    def trace(cls, arg_msg="Call Stack "):
        print()
        Msg.write(arg_msg, MsgLevel.trace)
        if Msg.lev & MsgLevel.trace:
            traceback.print_stack()
            sys.stdout.flush()
            sys.stderr.flush()

    @classmethod
    def error_trace(cls, arg_msg="Call Stack "):
        if Msg.lev & MsgLevel.trace:
            print()
            Msg.write(arg_msg, MsgLevel.trace)
            traceback.print_exc(file=sys.stdout)

    @classmethod
    def fout(cls, arg_fpath, arg_lev):
        if isinstance(arg_lev, str):
            my_lev = MsgLevel.translate(arg_lev)
        if my_lev & Msg.lev:
            with open(arg_fpath, "r") as my_file:
                my_lines = my_file.readlines()
                for my_line in my_lines:
                    Msg.write(my_line, my_lev)

    @classmethod
    def lout(cls, arg_obj, arg_lev, arg_lbl=None, arg_indent=""):

        if not arg_obj:
            return
        if isinstance(arg_obj, (list, dict)):
            if not len(arg_obj) > 0 or not arg_obj:
                return

        elif not hasattr(arg_obj, "__class__"):
            return

        if isinstance(arg_lev, str):
            Msg.lout(arg_obj, MsgLevel.translate(arg_lev), arg_lbl)
            return

        if not arg_lev & Msg.lev:
            return

        arg_indent += "\t"

        if arg_lbl is not None:
            Msg.write_nostrip(arg_indent + str(arg_lbl), arg_lev)

        if isinstance(arg_obj, list):
            my_ndx = 0
            for my_item in arg_obj:
                my_ndx += 1
                if isinstance(my_item, (list, dict)):
                    Msg.lout(
                        my_item, arg_lev, str(my_ndx), "%s" % (arg_indent)
                    )
                else:
                    Msg.write_nostrip(
                        "%s[%02d] = %s" % (arg_indent, my_ndx, str(my_item)),
                        arg_lev,
                        True,
                    )

        elif isinstance(arg_obj, dict):

            for my_key in arg_obj:
                if isinstance(arg_obj[my_key], (list, dict)):
                    Msg.lout(
                        arg_obj[my_key], arg_lev, my_key, "%s" % (arg_indent)
                    )
                else:
                    Msg.write_nostrip(
                        "%s[%s] = %s"
                        % (arg_indent, my_key, str(arg_obj[my_key])),
                        arg_lev,
                        True,
                    )

        elif hasattr(arg_obj, "__class__"):

            Msg.lout(arg_obj.__dict__, arg_lev, None, "%s" % (arg_indent))

        else:
            raise Exception(
                "Argument 1 needs to be of list or dictionary type"
            )

    @classmethod
    def flush(cls):
        sys.stdout.flush()
        sys.stderr.flush()
