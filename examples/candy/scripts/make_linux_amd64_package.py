#!/usr/bin/env python3
import os
import platform

if platform.system() == 'Windows':
    MYPYTHON = 'python'
    MYCODEROOT = 'c:/coderoot'
else:
    MYPYTHON = 'python3'
    MYCODEROOT = '/coderoot'

def runcmd(cmd):
    try:
        stream = os.popen(cmd)
        output = stream.read()
        exit_status = stream.close()

        if exit_status is not None:
            print ("make_linux_amd64_package.py: Command \'" + cmd + "\'failed with status " + str(exit_status))

    except Exception as e:
        print(f"make_linux_amd64_package.py: os.popen(\'" + cmd + "\') failed, exception:" + str(e))


runcmd(MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/make_debian_package.py ' + MYCODEROOT + '/iocom/examples/candy -a candy -s linux -h amd64 -o iocafe -d "Candy camera application"')

