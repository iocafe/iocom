#!/usr/bin/env python3
import os
import platform

if platform.system() == 'Windows':
    MYPYTHON = 'python'
    MYCODEROOT = 'c:/coderoot'
else:
    MYPYTHON = 'python3'
    MYCODEROOT = '/coderoot'

# Slave device hardware, like espcam, specifies from which HW subfolder signals are taken. Use '*' for same HW as in this device.
MYAPPCONFIG = MYCODEROOT + '/iocom/examples/tito/config'
MYSLAVEDEVICES = ' -d ' + MYCODEROOT + '/iocom/examples/candy,espcam'
MYSLAVEDEVICES += ' -d '+ MYCODEROOT + '/iocom/examples/gina,carol'
MYCONFSCRIPTS = ' -l ' + MYCODEROOT + '/iocom/extensions/ioserver'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

cmd = MYCODEROOT + '/iocom/scripts/generate_c_code.py ' + MYAPPCONFIG
cmd += ' -r ' + MYCODEROOT + ' -p ' + MYPYTHON 
cmd += MYSLAVEDEVICES + MYCONFSCRIPTS
cmd += ' -a controller-static'
runcmd(cmd)

print("*** Check that output files have been generated (error checks are imperfect).")
print("*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies.")
print("*** For clean build delete contents of config/intermediate and config/include directories before running this script.")

