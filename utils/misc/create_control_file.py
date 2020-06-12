#!/usr/bin/env python3
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
"""
create_control_file.py

## DESCRIPTION ##
Sets up a directory with template files and a control file to allow using master_run to reproduce the experiment. 
Also, and attempts to checkout relevant force revision into the reproduce directory.

## NOTES ##
When you diff the output, you may see different listing orders for the 'Command line' portion of the log; this is expected.
Assembly files should totally match and image files will differ by timestamp.

## DIRECTIONS ##
Follow the prompts to supply a generation log file and output directory.

## LIMITATIONS ##
When using a particular seed value, master_run will output on the 'Command line' portion of the log *two* seed values, one
in base-16 that will be wrong, and one in base-10 that will match the numerical value of the base-16 seed value apearing
in the source log. Diffing the output will confirm that the correct seed value is actually used.  
"""
from pathlib import Path 
import os
import glob
import shutil
import re
import getopt
import sys
from subprocess import call
import subprocess

def usage():
    usage_str = """Create a reproduce directory and control file from gen.log
    -g, --gen specify relative path to which gen.log file to parse
    -o, --outdir specify the relative path for the output directory for control file and template copies
    -h, --help display this help message
Example:
%s -g gen.log -o ../bugs/jiraV4_123
""" % sys.argv[0]
    print(usage_str)

def getCommandLine(genLogPath):
    try:
        with open(os.path.join(os.path.sep, os.getcwd(), genLogPath)) as myFile:
            maxcount = 20
            for linecount, line in enumerate(myFile):
                if "Command line:" in line:
                    return line.rstrip("\n")
                if linecount > maxcount:
                    break
            print("Command line missing from log file.")
            exit(1)
    except:
        print("Could not read " + genLogPath + ", does it exist?")
        exit(1)

def createReproduceDirectory(outputDirectory):
    try:
        new_dir = os.path.join(os.path.sep, os.getcwd(), outputDirectory)
        os.mkdir(new_dir)
    except:
        print("Failed to create reproduce directory. Check if it exists already.")
        exit(1)
    return new_dir

def copyTemplates(reproduce_directory):
    pyfiles = glob.glob("*.py")
    try:
        for item in pyfiles:
            shutil.copy2( os.path.join(os.path.sep, os.getcwd(), item), os.path.join(os.path.sep, reproduce_directory))
    except:
        print("Failed to copy Python files to reproduce directory.")
        exit(1)

def findDashOption(signatureSubStr, line):
    index = line.find(signatureSubStr)
    if index != -1:
        a = line[index+1:].split(" ")[1]
        #print("free option: ",signatureSubStr, a)
        return a
    return None

def findFreeSwitch(signatureSubStr, line):
    index = line.find(signatureSubStr)
    if index != -1:
        #print("switch option: ",signatureSubStr)
        return True
    return None 

def updateControlItem(control_item, sig, value):
    slot_array = sig[2].split(":")
    if(len(slot_array)==1):
        control_item.update({slot_array[0]:value})
    elif(len(slot_array)==2):
        if slot_array[0] not in control_item:
            control_item.update({slot_array[0]:dict()})
        try:
            num = int(value)
            control_item[slot_array[0]].update({slot_array[1]:num})
        except:
            #Logic to account for seed values so that they appear without quote marks in the final output.
            try:
                num = int(value, 16)
                control_item[slot_array[0]].update({slot_array[1]:hex(num)})
            except:
                control_item[slot_array[0]].update({slot_array[1]:value})
    return control_item

def formatOptionsString(control_item):
    #close the quote mark on the --options section
    if "generator" in control_item:
        if "--options" in control_item["generator"]:
            previous = control_item["generator"]["--options"]
            control_item["generator"].update({"--options":"\\"+"\""+previous+"\\"+"\""})
    return control_item

def createControlItem(line):
    #Do the parsing, iterating through the implemented signatures, assuming the 'Command line' was listed in the first 20 lines of the supplied log file
    control_item = dict() 

    #(signature, handler, control_item_key, value_type)
    signatures=[(" -t", "free_option", "fname", "filepath"),
                ("--test", "free_option", "fname", "filepath"),
                (" -g", "free_option", "generator:--global-modifier", "filepath"),
                ("--global-modifier", "free_option", "generator:--global-modifier", "filepath"),
                (" -d", "cluster_option", "generator:--dump", "string"),
                ("--dump", "cluster_option", "generator:--dump", "string"),
                (" -s", "free_option", "generator:-s", "number"),
                ("--seed", "free_option", "generator:-s", "number"),
                (" -c", "free_option", "generator:-c", "filepath"),
                ("--cfg", "free_option", "generator:--cfg", "filepath"),
                (" -l", "free_option", "generator:-l", "string"),
                ("--log", "free_option", "generator:--log", "string"),
                ("--max-instr", "free_option", "options:max-instr", "number"), 
                ("--num-chips", "free_option", "options:num-chips", "number"),
                ("--num-cores", "free_option", "options:num-cores", "number"),
                ("--num-threads", "free_option", "options:num-threads", "number"),
                #
                ("--failOverride", "free_switch", "generator:--failOverride", "none"),
                ("--noiss", "free_switch", "generator:--noiss", "none"),            
                ("--outputwithseed", "free_switch", "generator:--outputwithseed", "none"),            
                ("--img", "free_switch", "generator:--img", "none"),            
                ("--noasm", "free_switch", "generator:--noasm", "none"),            
                ]

    for sig in signatures:
        value = None

        if sig[1] in ["free_option", "cluster_option"]:
            value = findDashOption(sig[0], line)
            if value is not None:
                control_item = updateControlItem(control_item, sig, value)
    
        elif sig[1] == "free_switch":
            value = findFreeSwitch(sig[0], line)
            if value is not None:
                #Even though at this point we identified a valid free_switch, subsequent logic needs the value to be 'None' so that printing works correctly
                value = None
                control_item = updateControlItem(control_item, sig, value)
    
    #Add --options to generator sub-dictionary
    optionsString = findDashOption("--options", line)
    if optionsString != "":
        if "generator" not in control_item:
            control_item.update({"generator":dict()})
        control_item["generator"].update({"--options":optionsString})
        control_item = formatOptionsString(control_item)  


    return control_item

#This method is needed basically just to ensure that when the seed value is printed, it is not surrounded with quote marks
def writeControlFile(control_item, reproduce_directory):
    #Write the actual control file
    real_repro_path = os.path.realpath(reproduce_directory)
    control_file_path = os.path.join(os.path.sep, real_repro_path, "_reproduce_fctrl.py")
    with open(control_file_path, "w+") as control_file:
        print("\nPrinting formatted control file text to: ", control_file_path)
        control_file.write("control_items = [\n{\n")
        for key, value in control_item.items():
            if key=="generator":
                string = "\"generator\" : {"
                for gkey, gvalue in control_item["generator"].items():
                    if gkey in ["--seed","-s"]:
                        string += "\"--seed\" : " + gvalue + ", "
                    else:
                        if type(gvalue) is None:
                            string += "\"" + gkey + "\"" + " : None, "
                        elif type(gvalue) is str:
                            string += "\"" + gkey + "\"" + " : \"" + gvalue + "\", "
                        else:
                            string += "\"" + gkey + "\"" + " : " + str(gvalue) + ", "
                control_file.write(string + "},\n")
            else:
                if type(value) is None:
                    control_file.write("\"" + key + "\" : None")
                elif type(value) is dict:
                    control_file.write("\"" + key + "\" : "+ str(value)+ ",")
                else:
                    control_file.write("\"" + key + "\" : \"" + value + "\",")
        control_file.write("},\n]")

#Control flow start
if __name__ == "__main__":
    opts = None
    args = None

    if len(sys.argv) == 1:
        print("Specify options.")
        usage()
        sys.exit(1)

    try: 
        opts, args = getopt.getopt(sys.argv[1:], "hg:o:", ["help", "gen=", "outdir="])
    except getopt.GetoptError as err:
        print(err)
        usage()
        sys.exit(1)

    genLogPath = None
    outputDirectory = None

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-g", "--gen"):
            genLogPath = a
        elif o in ("-o", "--outdir"):
            outputDirectory = a
        else:
            assert False, "Unhandled option: " + o
        
    print("\n#### Force Control File Auto Generate Script ####")
    print("current directory: ", os.getcwd(), "\nlog file name: ", genLogPath, "\noutput directory: ", outputDirectory)
    
    commandLine = getCommandLine(genLogPath)

    if outputDirectory is not None:
        reproduce_directory = createReproduceDirectory(outputDirectory)
        print("First copying over all the Python files from: ", os.getcwd(), " into: ", reproduce_directory)
        copyTemplates(reproduce_directory)
    else:
        reproduce_directory = "."


    print("\nGenerating and writing new control file to reproduce directory.")
    print("Parsing "+ genLogPath +" to create control file...")

    control_item = createControlItem(commandLine)

    writeControlFile(control_item, reproduce_directory)

    #get force path and checkout the repository
    if outputDirectory is not None:
        print("Obtaining the version of force used to generate the log and checking out into the reproduce directory.")
        index = commandLine.find("/bin/force")
        if index != -1:
            force_path = commandLine[:index].split(" ")[-1]
            current_path = os.getcwd()
            os.chdir(force_path)
            p = subprocess.check_output(["svn", "info"])
            url_string = ""
            for line in p.splitlines():
                decoded = line.decode('utf-8')
                if "URL: htt" in decoded:
                    url_string += decoded.split(" ")[1]
                elif "Revision:" in decoded:
                    url_string += "@" + decoded.split(" ")[1]
                
            call(["svn", "checkout", url_string, reproduce_directory + "/Force"])

    print("Control file generator done.")
