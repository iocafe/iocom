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
MERGEJSON = MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/merge_json.py'

MYCONFIG = MYCODEROOT + '/iocom/examples/' + MYAPP + '/config'
MYINCLUDE = MYCONFIG + '/include/' + MYHW
MYSIGNALS = MYCONFIG + '/signals/signals'
MYINTERMEDIATE = MYCONFIG + '/intermediate/' + MYHW
MYINTERMEDIATESIG = MYINTERMEDIATE + '/signals-merged'
MYPINS = MYCONFIG + '/pins/' + MYHW + '/pins-io'
MYNETDEFAULTS = MYCONFIG + '/network/network-defaults'
MYINTERMEDIATENET = MYINTERMEDIATE + '/network-defaults'
MYIMPORTS   = MYCODEROOT + '/iocom/config/'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

runcmd(MERGEJSON + ' ' + MYCONFIG + '/signals/signals.json ' + MYIMPORTS + '/signals/device-conf-signals.json ' + MYIMPORTS + '/signals/camera-buf8k-signals.json -o ' + MYINTERMEDIATESIG + '.json')
runcmd(PINSTOC + ' ' + MYPINS + '.json -o ' + MYINCLUDE + '/pins-io.c -s ' + MYINTERMEDIATESIG + '.json')

runcmd(JSONTOOL + ' --t2b -title ' + MYINTERMEDIATESIG + '.json ' + MYINTERMEDIATESIG + '.binjson')
runcmd(JSONTOOL + ' --b2t ' + MYINTERMEDIATESIG + '.binjson ' + MYINTERMEDIATESIG + '-check.json')

runcmd(BINTOC + ' -v ioapp_signal_config ' + MYINTERMEDIATESIG + '.binjson -o ' + MYINCLUDE + '/info-mblk.c')
runcmd(SIGNALSTOC + ' ' + MYINTERMEDIATESIG + '.json -p ' + MYPINS + '.json -o ' + MYINCLUDE + '/signals.c')

runcmd(JSONTOOL + ' --t2b -title ' + MYNETDEFAULTS + '.json ' + MYINTERMEDIATENET + '.binjson')
runcmd(JSONTOOL + ' --b2t ' + MYINTERMEDIATENET + '.binjson ' + MYINTERMEDIATENET + '-check.json')
runcmd(BINTOC + ' -v ioapp_network_defaults ' + MYINTERMEDIATENET + '.binjson -o ' + MYINCLUDE + '/network-defaults.c')

print("*** Check that the output files have been generated (error checks are still missing).")
print("*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies.")

