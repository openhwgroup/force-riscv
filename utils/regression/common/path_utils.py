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
import errno
import glob
import os
import os.path
import shutil
import sys
from pathlib import Path

from common.datetime_utils import DateTime
from common.msg_utils import Msg


class PathUtils(object):
    # linux path delimiter
    path_delimiter = "/"
    # constants used with os.walk(...)
    path_root = 0
    path_dirs = 1
    path_files = 2
    #
    walk_dirinfo = 0

    # checks to see if the path string passed has a trailing path delimiter and
    # adds one if not exists and returns the resulting string
    @classmethod
    def include_trailing_path_delimiter(cls, arg_path):
        arg_path = str(arg_path)
        if not arg_path.endswith(PathUtils.path_delimiter):
            arg_path += PathUtils.path_delimiter
        return arg_path

    # checks to see if the path string passed has a trailing path delimiter and
    # removes it if it exists and returns the resulting string
    @classmethod
    def exclude_trailing_path_delimiter(cls, arg_path):
        arg_path = str(arg_path)
        if arg_path.endswith(PathUtils.path_delimiter):
            arg_path = arg_path[:-1]
        return arg_path

    # checks to see if the path string passed has a leading path delimiter and
    # adds one if not exists and returns the resulting string
    @classmethod
    def include_leading_path_delimiter(cls, arg_path):
        arg_path = str(arg_path)
        if not arg_path.startswith(PathUtils.path_delimiter):
            arg_path += PathUtils.path_delimiter + arg_path
        return arg_path

    # checks to see if the path string passed has a leading path delimiter and
    # removes it if it exists and returns the resulting string
    @classmethod
    def exclude_leading_path_delimiter(cls, arg_path):
        arg_path = str(arg_path)
        if arg_path.startswith(PathUtils.path_delimiter):
            arg_path = arg_path[1:]
        return arg_path

    # Class Method that checks to see if the path string passed exists
    @classmethod
    def valid_path(cls, arg_path):
        arg_path = str(arg_path)
        return os.path.exists(arg_path)

    # Class Method that checks to see if file a can execute
    @classmethod
    def check_exe(cls, arg_path):
        if PathUtils.check_file(arg_path):
            return os.access(str(arg_path), os.X_OK)
        return False

    # Class Method that checks to see if a file exists
    @classmethod
    def check_file(cls, arg_path):
        if PathUtils.valid_path(arg_path):
            my_file = Path(str(arg_path))
            return my_file.is_file()
        return False

    # Class Method that checks to see if a directory exists
    @classmethod
    def check_dir(cls, arg_path):
        if PathUtils.valid_path(arg_path):
            my_dir = Path(str(arg_path))
            return my_dir.is_dir()
        return False

    @classmethod
    def check_found(cls, arg_path):
        # see if file pattern exists
        my_list = glob.glob(arg_path)
        return isinstance(my_list, list) and my_list

    # Class method that changes into a directory if that directory exists
    @classmethod
    def set_dir(cls, arg_dir):
        if PathUtils.valid_path(arg_dir):
            os.chdir(arg_dir)

    # Class method, takes a path and appends additional directories
    @classmethod
    def append_path(cls, arg_path, arg_added_path):
        return PathUtils.include_trailing_path_delimiter(
            arg_path
        ) + PathUtils.exclude_leading_path_delimiter(arg_added_path)

    # splits a path into a file name and a directory and returns both
    @classmethod
    def split_path(cls, arg_path):
        if PathUtils.check_dir(arg_path):
            return arg_path, None
        my_dir, my_fname = os.path.split(arg_path)
        if not (len(my_dir)):
            my_dir = None
        return my_dir, my_fname

    # splits a path into a file name and a directory and returns both
    @classmethod
    def split_dir(cls, arg_path):
        my_path = PathUtils.exclude_trailing_path_delimiter(arg_path)
        if not PathUtils.check_dir(my_path):
            return None, arg_path
        try:
            my_root, my_dir = os.path.split(my_path)
        except BaseException:
            return None, arg_path

        if my_root is None or len(my_root) in [0, 1]:
            my_root = "/"
        return my_root, my_dir

    # retrieves the basename of the string passed
    @classmethod
    def base_name(cls, arg_path):
        return os.path.basename(arg_path)

    @classmethod
    def list_files(cls, arg_path):
        Msg.dbg(arg_path)
        return glob.glob(arg_path)

    @classmethod
    def list_dirs(cls, arg_path=None):
        if arg_path is None:
            return PathUtils.list_dirs(PathUtils.current_dir())
        my_rawinfo = list(os.walk(arg_path))
        Msg.dbg(str(my_rawinfo))
        my_dirinfo = my_rawinfo[0]
        my_dirs = list(filter(lambda my_dir: not my_dir.startswith("."), my_dirinfo[1]))

        return my_dirs

    @classmethod
    def dir_count(cls, arg_path=None):
        try:
            return len(PathUtils.list_dirs(arg_path))
        except BaseException:
            Msg.error_trace()
            print("Exception")
        return 0

    @classmethod
    def file_count(cls, arg_path):
        return len(list(filter(os.path.isfile, PathUtils.list_files(arg_path))))

    # returns the fully qualified working directory
    @classmethod
    def real_path(cls, arg_path):
        return os.path.realpath(arg_path)

    # get the parent directory unless that directory is the root
    # if it is the root return None
    @classmethod
    def parent_dir(cls, arg_path):
        if os.path.realpath(arg_path) != "/":
            return os.path.realpath(arg_path)
        return None

    # returns the current directory
    @classmethod
    def current_dir(cls):
        return os.getcwd()

    # checks to see if a directory exists then changes into that directory,
    # returns True on success
    @classmethod
    def chdir(cls, arg_dir, arg_force=False):

        Msg.dbg("PathUtils::chdir( %s, %s )" % (arg_dir, str(arg_force)))
        if not PathUtils.check_dir(PathUtils.exclude_trailing_path_delimiter(arg_dir)):
            if arg_force:
                Msg.dbg(
                    "Directory Does Not Exist, " "Attempting to Create Directory: %s" % (arg_dir)
                )
                if not PathUtils.mkdir(arg_dir):
                    Msg.dbg("Failed Create: %s" % (arg_dir))
                    return False
                Msg.dbg("Success Created: %s " % (arg_dir))
            else:
                Msg.dbg("Failed Change Directory: %s" % (arg_dir))
                return False
        try:
            real_path = PathUtils.real_path(arg_dir)
            os.chdir(arg_dir)
            Msg.dbg("Success Changed Directory: %s (real path: %s)" % (arg_dir, real_path))
        except BaseException:
            return False

        return True

    # creates a new directory with default permissions
    @classmethod
    def mkdir(cls, arg_dir):
        Msg.dbg("PathUtils.mkdir( %s )" % (arg_dir))
        try:
            os.makedirs(PathUtils.exclude_trailing_path_delimiter(arg_dir))

        except OSError as arg_ex:
            if arg_ex.errno != errno.EEXIST:
                Msg.dbg("Failed Create Directory: " + str(arg_ex))
                return False

            Msg.dbg("Success, Directory Exists: " + arg_dir)

        return True

    @classmethod
    def rmdir(cls, arg_dir, arg_force=False):
        Msg.dbg("PathUtils.rmdir( %s, %s )" % (arg_dir, str(arg_force)))
        try:
            if arg_force:
                # remove a directory regardless of the contents
                shutil.rmtree(arg_dir)
            else:
                os.rmdir(arg_dir)
            Msg.dbg("Success: Directory Removed: %s" % (arg_dir))

        except OSError as arg_ex:
            if PathUtils.check_dir(arg_dir):
                Msg.err("Fail, Unable to Remove Directory: %s" % (arg_dir))
                return False
            Msg.warn("Directory does not exists, Remove Failed: %s " % (arg_dir))
        return True

    @classmethod
    def remove(cls, arg_path, arg_force=False):
        try:
            os.remove(arg_path)
            Msg.dbg("Success, File Removed: %s" % (arg_path))
        except OSError as arg_ex:
            Msg.err(str(arg_ex))
            return False

    @classmethod
    def move(cls, arg_src, arg_target="."):
        Msg.dbg("PathUtils::move( %s, %s )" % (arg_src, arg_target))
        try:
            shutil.move(arg_src, arg_target)

        except shutil.Error as arg_ex:
            Msg.err(str(arg_ex))
            return False

        return True

    @classmethod
    def rename(cls, arg_src, arg_tgt):
        Msg.dbg("PathUtils::rename( %s, %s )" % (arg_src, arg_tgt))
        try:
            os.rename(arg_src, arg_tgt)

        except Exception as arg_ex:
            Msg.err(str(arg_ex))
            return False

        return True

    @classmethod
    def copy_file(cls, arg_src, arg_dest="."):
        try:
            shutil.copy(arg_src, arg_dest)

        except shutil.SameFileError as arg_ex:
            Msg.err(str(arg_ex))
            return False

        except OSError as arg_ex:
            Msg.err(str(arg_ex))
            return False

        return True

    @classmethod
    def purge_dirs(cls, arg_basedir):
        try:
            my_dirlist = sorted(os.listdir(arg_basedir))
            Msg.lout(my_dirlist, "dbg")

            for my_dir in my_dirlist:
                # PathUtils.rmdir( my_dir, True )
                PathUtils.rmdir(
                    PathUtils.include_trailing_path_delimiter(arg_basedir) + my_dir,
                    True,
                )

        except OSError as arg_ex:
            Msg.error_trace()
            Msg.err(str(arg_ex))
            return False

        except Exception as arg_ex:
            Msg.error_trace()
            Msg.err(str(arg_ex))
            return False

        return True

    @classmethod
    def archive_dir(cls, arg_srcdir):

        try:
            # get the base name
            my_basename = "%s" % (str(DateTime.YMD()))
            Msg.dbg("Base Name: %s" % (my_basename))

            # set the directory mask
            my_srcdir = PathUtils.exclude_trailing_path_delimiter(arg_srcdir)
            my_srcmask = "%s_%s_???" % (my_srcdir, my_basename)
            Msg.dbg("Directory Mask: %s" % (my_srcmask))

            # list any previous copies
            my_dirlist = sorted(PathUtils.list_files(my_srcmask))

            Msg.lout(my_dirlist, "dbg")
            my_findex = 0

            # there are only two possiblities here
            # 1. there is only one match, in which case the mask does not
            #       include a number
            # 2. there are more than one match in which case the last match
            #       should contain a number

            if len(my_dirlist) > 0:
                # remove the wildcards
                my_srcmask = my_srcmask.replace("???", "")
                Msg.dbg("New File Mask: %s" % (my_srcmask))

                my_tmp = my_dirlist[-1]
                Msg.dbg("Last Filename: %s" % (my_tmp))

                my_tmp = my_tmp.replace(my_srcmask, "")
                Msg.dbg("My Index Last Filename: %s" % (my_tmp))

                my_findex = int(my_tmp) + 1
                Msg.dbg("My New Index Filename: %s" % (my_findex))

            # get the target name
            my_tgtdir = "%s_%s_%0.3d" % (my_srcdir, my_basename, my_findex)
            Msg.dbg("Target Directory: %s" % (my_tgtdir))
            return PathUtils.move(my_srcdir, my_tgtdir)

        except Exception as arg_ex:
            Msg.error_trace(str(arg_ex))
            Msg.err(str(arg_ex))
            return False

    @classmethod
    def next_dir(cls):
        # list any previous copies
        my_dirlist = sorted(PathUtils.list_files("."))
        Msg.lout(my_dirlist, "dbg")
        return len(my_dirlist)

    @classmethod
    def expire(cls, arg_rootdir, arg_expiredate, arg_srcmask):
        my_targetbase = PathUtils.base_name(arg_rootdir)

        # list any previous copies
        my_dirlist = sorted(PathUtils.list_files(arg_srcmask))

        for my_dir in my_dirlist:
            if os.path.getmtime(my_dir) < arg_expiredate:
                PathUtils.rmdir(my_dir, True)

    @classmethod
    def move_files(cls, arg_target):
        PathUtils.mkdir(arg_target)
        my_target = "%s%s" % (
            PathUtils.include_trailing_path_delimiter(PathUtils.current_dir()),
            arg_target,
        )
        Msg.dbg("Current Directory: %s " % (PathUtils.current_dir()))

        my_flist = PathUtils.list_files(
            PathUtils.include_trailing_path_delimiter(PathUtils.current_dir()) + "*.*"
        )
        for my_file in my_flist:
            Msg.dbg("Source: %s, Target: %s" % (my_file, my_target))
            PathUtils.move(my_file, arg_target)

    @classmethod
    def write_file(cls, aFilePath, aContent, aFileType):

        with open(aFilePath, "w") as my_ofile:
            try:
                return my_ofile.write(aContent) > 0

            except Exception as arg_ex:
                Msg.error_trace()
                Msg.err("Error Writing %s File, %s" % (aFileType, str(arg_ex)))

            finally:
                my_ofile.close()

        return False

    # Return time last modified of a file or directory in timestamp format
    # (floating point number)
    @classmethod
    def time_modified(cls, aFilePath):
        file_stat = os.stat(aFilePath)
        mod_time = file_stat.st_mtime
        return mod_time

    @classmethod
    def touch(cls, arg_fpath):
        file_path = Path(arg_fpath)
        file_path.touch()

    @classmethod
    def chmod(cls, aFPath, aOctalPermissions):
        try:
            os.chmod(aFPath, aOctalPermissions)
        except OSError:
            Msg.dbg("Failed to modify permissions of file: " + str(aFPath))
            raise

    # Returns the full file path from a string or quits out if the file cannot
    # be found
    @classmethod
    def resolvePath(cls, aFilePath):
        try:
            return str(Path(aFilePath).resolve())  # return as a string instead of a Path object
        except FileNotFoundError:
            Msg.err("Unable to locate %s. Please ensure that it exists." % aFilePath)
            # Quit to prevent continuing with a file that was probably
            # improperly specified
            sys.exit(1)

    @classmethod
    def expandVars(cls, aPath):
        return os.path.expandvars(aPath)
