import os
import platform

MYAPP = 'selectwifi'
if platform.system() == 'Windows':
    MYPYTHON = 'python'
    MYCODEROOT = 'c:/coderoot'
    JSONTOOL = MYCODEROOT + '/bin/win32/json'
else:
    MYPYTHON = 'python3'
    MYCODEROOT = '/coderoot'
    JSONTOOL = MYCODEROOT + '/bin/linux/json'
BINTOC = MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/bin-to-c.py'
SIGNALSTOC = MYPYTHON + ' ' + MYCODEROOT + '/iocom/scripts/signals-to-c.py'

MYCONFIG = MYCODEROOT + '/iocom/extensions/' + MYAPP + '/config'
MYINCLUDE = MYCONFIG + '/include'
MYSIGNALS = MYCONFIG + '/signals/signals'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

runcmd(JSONTOOL + ' --t2b -title ' + MYSIGNALS + '.json ' + MYSIGNALS + '.binjson')
runcmd(JSONTOOL + ' --b2t ' + MYSIGNALS + '.binjson ' + MYSIGNALS + '-check.json')
runcmd(BINTOC + ' -v ioapp_signal_config ' + MYSIGNALS + '.binjson -o ' + MYINCLUDE + '/swf-info-mblk.c')
runcmd(SIGNALSTOC + ' ' + MYSIGNALS + '.json -o ' + MYINCLUDE + '/swf-signals.c')

print("*** Check that the output files have been generated (error checks are still missing).")
print("*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies.")
