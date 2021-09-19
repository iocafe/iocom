#!/usr/bin/env python3
import os
import platform

if platform.system() == 'Windows':
    MYPYTHON = 'python'
    MYCODEROOT = 'c:/coderoot'
    MYSOURCEFILE = "c:"
else:
    MYPYTHON = 'python3'
    MYCODEROOT = '/coderoot'
    MYSOURCEFILE = ""

MYSOURCEFILE += '/tmp/tealight_ioboard_tmp/esp32dev/firmware.bin'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

runcmd(MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/copy_package.py ' + MYSOURCEFILE + ' -a tealight -s esp32 -h espcam -o iocafe')

