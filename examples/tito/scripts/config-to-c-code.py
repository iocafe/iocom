#!/usr/bin/env python3
import os
import platform

MYAPP = 'tito'
MYHW1 = 'esphw'
MYHW2 = 'megahw'

if platform.system() == 'Windows':
    MYPYTHON = 'python'
    MYCODEROOT = 'c:/coderoot'
    JSONTOOL = MYCODEROOT + '/bin/win32/json'
else:
    MYPYTHON = 'python3'
    MYCODEROOT = '/coderoot'
    JSONTOOL = MYCODEROOT + '/bin/linux/json'
PINSTOC = MYPYTHON + ' ' + MYCODEROOT + '/pins/scripts/pins-to-c.py'
BINTOC = MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/bin-to-c.py'
SIGNALSTOC = MYPYTHON + ' ' + MYCODEROOT + '/iocom/scripts/signals-to-c.py'

MYCONFIG = MYCODEROOT + '/iocom/examples/' + MYAPP + '/config'
MYINCLUDE1 = MYCONFIG + '/include/' + MYHW1
MYINCLUDE2 = MYCONFIG + '/include/' + MYHW2
MYSIGNALS = MYCONFIG  + '/signals/signals'
MYPINS1 = MYCONFIG + '/pins/' + MYHW1 + '/pins-io'
MYPINS2 = MYCONFIG + '/pins/' + MYHW2 + '/pins-io'
MYNETDEFAULTS = MYCONFIG + '/network/network-defaults'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

def do_hardware(MYPINS, MYINCLUDE):
    runcmd(PINSTOC + ' ' + MYPINS + '.json -o ' + MYINCLUDE + '/pins-io.c -s ' + MYSIGNALS + '.json')
    
    runcmd(JSONTOOL + ' --t2b -title ' + MYSIGNALS + '.json ' + MYSIGNALS + '.binjson')
    runcmd(JSONTOOL + ' --b2t ' + MYSIGNALS + '.binjson ' + MYSIGNALS + '-check.json')
    runcmd(SIGNALSTOC + ' -a controller-static ' + MYSIGNALS + '.json -o ' + MYINCLUDE + '/signals.c')

    runcmd(BINTOC + ' -v ioapp_signal_config ' + MYSIGNALS + '.binjson -o ' + MYINCLUDE + '/info-mblk-binary.c')

    runcmd(SIGNALSTOC + ' -a controller-static ' + MYCODEROOT + '/iocom/examples/gina/config/signals/signals.json -o ' + MYINCLUDE + '/gina-for-' + MYAPP + '.c')
    runcmd(SIGNALSTOC + ' -a controller-static ' + MYCODEROOT + '/iocom/examples/candy/config/signals/signals.json -o ' + MYINCLUDE + '/candy-for-' + MYAPP + '.c')

    runcmd(JSONTOOL + ' --t2b -title ' + MYNETDEFAULTS + '.json ' + MYNETDEFAULTS + '.binjson')
    runcmd(JSONTOOL + ' --b2t ' + MYNETDEFAULTS + '.binjson ' + MYNETDEFAULTS + '-check.json')
    runcmd(BINTOC + ' -v ioapp_network_defaults ' + MYNETDEFAULTS + '.binjson -o ' + MYINCLUDE + '/network-defaults.c')

do_hardware(MYPINS1, MYINCLUDE1)
do_hardware(MYPINS2, MYINCLUDE2)

print("*** Check that the output files have been generated (error checks are still missing).")
print("*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies.")

