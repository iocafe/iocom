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
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

runcmd(MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/make_debian_package.py ' + MYCODEROOT + '/iocom/examples/minion -a minion -s linux -h amd64 -o iocafe -d "Minion camera application"')

