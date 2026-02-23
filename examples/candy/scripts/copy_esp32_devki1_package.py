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
    try:
        stream = os.popen(cmd)
        output = stream.read()
        exit_status = stream.close()

        if exit_status is not None:
            print ("copy_esp32_devki1.py: Command \'" + cmd + "\'failed with status " + str(exit_status))

    except Exception as e:
        print(f"copy_esp32_devki1,config_to_c_code.py: os.popen(\'" + cmd + "\') failed, exception:" + str(e))


runcmd(MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/copy_package.py ' + MYSOURCEFILE + ' -a candy -s esp32 -h devkit1 -o iocafe')

