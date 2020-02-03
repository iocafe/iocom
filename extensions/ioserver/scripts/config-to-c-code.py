import os
import platform

MYAPP = 'ioserver'
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
MYACCOUNTS = MYCONFIG + '/signals/accounts'
MYACCOUNTSDEFAULTS = MYCONFIG + '/network/default-accounts'

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

runcmd(JSONTOOL + ' --t2b -title ' + MYACCOUNTS + '.json ' + MYACCOUNTS + '.binjson')
runcmd(JSONTOOL + ' --b2t ' + MYACCOUNTS + '.binjson ' + MYACCOUNTS + '-check.json')
runcmd(SIGNALSTOC + ' -a controller-static ' + MYACCOUNTS + '.json -o ' + MYCONFIG + '/include/account-signals.c')
runcmd(BINTOC + ' -v ioserver_account_config ' + MYACCOUNTS + '.binjson -o ' + MYINCLUDE + '/accounts-mblk-binary.c')

runcmd(JSONTOOL + ' --t2b --hash-pw -title ' + MYACCOUNTSDEFAULTS + '.json ' + MYACCOUNTSDEFAULTS + '.binjson')
runcmd(JSONTOOL + ' --b2t  ' + MYACCOUNTSDEFAULTS + '.binjson ' + MYACCOUNTSDEFAULTS + '-check.json')
runcmd(BINTOC + ' -v ioserver_account_defaults ' + MYACCOUNTSDEFAULTS + '.binjson -o ' + MYINCLUDE + '/account-defaults.c')

print("*** Check that the output files have been generated (error checks are still missing).")
print("*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies.")
