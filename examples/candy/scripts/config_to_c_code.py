#!/usr/bin/env python3
import os
import platform

if platform.system() == 'Windows':
    MYPYTHON = 'python'
    MYCODEROOT = 'c:/coderoot'
else:
    MYPYTHON = 'python3'
    MYCODEROOT = '/coderoot'

MYAPPCONFIG = MYCODEROOT + '/iocom/examples/candy/config'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

runcmd(MYPYTHON + ' ' + MYCODEROOT + '/iocom/scripts/generate_c_code.py ' + MYAPPCONFIG + ' -r ' + MYCODEROOT + ' -p ' + MYPYTHON)

print("*** Check that output files have been generated (error checks are imperfect).")
print("*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies.")
print("*** For clean build delete contents of config/intermediate and config/include directories before running this script.")

