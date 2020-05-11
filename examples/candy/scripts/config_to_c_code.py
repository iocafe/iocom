#!/usr/bin/env python3
import os
import platform

MYAPP = 'candy'
MYHW = 'espcam'
if platform.system() == 'Windows':
    MYPYTHON = 'python'
    MYCODEROOT = 'c:/coderoot'
    JSONTOOL = MYCODEROOT + '/bin/win32/json'
else:
    MYPYTHON = 'python3'
    MYCODEROOT = '/coderoot'
    JSONTOOL = MYCODEROOT + '/bin/linux/json'
PINSTOC = MYPYTHON + ' ' + MYCODEROOT + '/pins/scripts/pins_to_c.py'
BINTOC = MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/bin_to_c.py'
SIGNALSTOC = MYPYTHON + ' ' + MYCODEROOT + '/iocom/scripts/signals_to_c.py'

MYCONFIG = MYCODEROOT + '/iocom/examples/' + MYAPP + '/config'
MYINCLUDE = MYCONFIG + '/include/' + MYHW
MYSIGNALS = MYCONFIG + '/signals/signals'
MYPINS = MYCONFIG + '/pins/' + MYHW + '/pins-io'
MYNETDEFAULTS = MYCONFIG + '/network/network-defaults'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

runcmd(PINSTOC + ' ' + MYPINS + '.json -o ' + MYINCLUDE + '/pins-io.c -s ' + MYSIGNALS + '.json')
runcmd(JSONTOOL + ' --t2b -title ' + MYSIGNALS + '.json ' + MYSIGNALS + '.binjson')
runcmd(JSONTOOL + ' --b2t ' + MYSIGNALS + '.binjson ' + MYSIGNALS + '-check.json')

runcmd(BINTOC + ' -v ioapp_signal_config ' + MYSIGNALS + '.binjson -o ' + MYINCLUDE + '/info-mblk.c')
runcmd(SIGNALSTOC + ' ' + MYSIGNALS + '.json -p ' + MYPINS + '.json -o ' + MYINCLUDE + '/signals.c')

runcmd(JSONTOOL + ' --t2b -title ' + MYNETDEFAULTS + '.json ' + MYNETDEFAULTS + '.binjson')
runcmd(JSONTOOL + ' --b2t ' + MYNETDEFAULTS + '.binjson ' + MYNETDEFAULTS + '-check.json')
runcmd(BINTOC + ' -v ioapp_network_defaults ' + MYNETDEFAULTS + '.binjson -o ' + MYINCLUDE + '/network-defaults.c')

print("*** Check that the output files have been generated (error checks are still missing).")
print("*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies.")

