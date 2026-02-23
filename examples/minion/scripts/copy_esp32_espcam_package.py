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

MYSOURCEFILE += '/tmp/minion_ioboard_tmp/esp32cam/firmware.bin'

def runcmd(cmd):
    try:
        stream = os.popen(cmd)
        output = stream.read()
        exit_status = stream.close()

        if exit_status is not None:
            print ("make_esp32_espcam_package.py: Command \'" + cmd + "\'failed with status " + str(exit_status))

    except Exception as e:
        print(f"make_esp32_espcam_package.py: os.popen(\'" + cmd + "\') failed, exception:" + str(e))

runcmd(MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/copy_package.py ' + MYSOURCEFILE + ' -a minion -s esp32 -h espcam -o iocafe')

