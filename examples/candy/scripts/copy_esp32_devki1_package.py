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

MYSOURCEFILE += '/tmp/candy_ioboard_tmp/esp32doit-devkit-v1/firmware.bin'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

runcmd(MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/copy_package.py ' + MYSOURCEFILE + ' -a candy -s esp32 -h devkit1 -o iocafe')

