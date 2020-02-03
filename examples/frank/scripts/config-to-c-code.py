import os
import platform

MYAPP = 'frank'
if platform.system() == 'Windows':
    MYPYTHON = 'python'
    MYCODEROOT = 'c:/coderoot'
else:
    MYPYTHON = 'python3'
    MYCODEROOT = '/coderoot'
JSONTOOL = MYCODEROOT + '/bin/linux/json'
BINTOC = MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/bin-to-c.py'
SIGNALSTOC = MYPYTHON + ' ' + MYCODEROOT + '/iocom/scripts/signals-to-c.py'

MYCONFIG = MYCODEROOT + '/iocom/examples/' + MYAPP + '/config'
MYINCLUDE = MYCONFIG + '/include'
MYSIGNALS = MYCONFIG  + '/signals/signals'
MYNETDEFAULTS = MYCONFIG + '/network/network-defaults'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

runcmd(JSONTOOL + ' --t2b -title ' + MYSIGNALS + '.json ' + MYSIGNALS + '.binjson')
runcmd(JSONTOOL + ' --b2t ' + MYSIGNALS + '.binjson ' + MYSIGNALS + '-check.json')
runcmd(SIGNALSTOC + ' -a controller-static ' + MYSIGNALS + '.json -o ' + MYCONFIG + '/include/signals.c')
runcmd(BINTOC + ' -v ioapp_signal_config ' + MYSIGNALS + '.binjson -o ' + MYINCLUDE + '/info-mblk-binary.c')

runcmd(JSONTOOL + ' --t2b -title ' + MYNETDEFAULTS + '.json ' + MYNETDEFAULTS + '.binjson')
runcmd(JSONTOOL + ' --b2t ' + MYNETDEFAULTS + '.binjson ' + MYNETDEFAULTS + '-check.json')
runcmd(BINTOC + ' -v ioapp_network_defaults ' + MYNETDEFAULTS + '.binjson -o ' + MYINCLUDE + '/network-defaults.c')

print("*** Check that the output files have been generated (error checks are still missing).")
print("*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies.")