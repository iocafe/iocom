# generate_c_code.py 11.5.2020/pekka
import json
import os
import platform
import sys


def setup_environment(confpath, hw, coderoot, pythoncmd):
    global MYHW,  MYPYTHON,  CODEROOT, JSONTOOL, PINSTOC, BINTOC, SIGNALSTOC, MERGEJSON, MYIMPORTS
    global MYCONFIG, MYSIGNALS, MYPINS, MYPARAMETERS, MYNETWORK, MYINCLUDE, MYINTERMEDIATE
     
    MYHW = hw
    if platform.system() == 'Windows':
        MYPYTHON = 'python'
        MYCODEROOT = 'c:/coderoot'
        JSONTOOL = MYCODEROOT + '/bin/win32/json'
    else:
        MYPYTHON = 'python3'
        MYCODEROOT = '/coderoot'
        JSONTOOL = MYCODEROOT + '/bin/linux/json'

    if coderoot != None:
        MYCODEROOT = coderoot

    if pythoncmd != None:
        MYPYTHON = pythoncmd

    PINSTOC = MYPYTHON + ' ' + MYCODEROOT + '/pins/scripts/pins_to_c.py'
    BINTOC = MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/bin_to_c.py'
    SIGNALSTOC = MYPYTHON + ' ' + MYCODEROOT + '/iocom/scripts/signals_to_c.py'
    MERGEJSON = MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/merge_json.py'
    MYIMPORTS  = MYCODEROOT + '/iocom/config'

    MYCONFIG = confpath
    MYSIGNALS = MYCONFIG + '/signals'
    MYPINS = MYCONFIG + '/pins/' + MYHW + '/pins-io'
    MYPARAMETERS = MYCONFIG + '/parameters'
    MYNETWORK = MYCONFIG + '/network'
    MYINCLUDE = MYCONFIG + '/include'
    MYINTERMEDIATE = MYCONFIG + '/intermediate'

    # MYNETDEFAULTS = MYCONFIG + '/network/network-defaults'
    # MYINTERMEDIATESIG = MYINTERMEDIATE + '/signals-merged'
    # MYINTERMEDIATENET = MYINTERMEDIATE + '/network-defaults'

def append_subdirectories(subdirs, path):
    try:
        dirlist = [fname for fname in os.listdir(path) if os.path.isdir(os.path.join(path, fname))]    
    except FileNotFoundError:
        return
    for fname in dirlist:
        if not fname in subdirs:
            subdirs.append(fname)

def get_exact_path(fname, confdir):
    path = MYCONFIG + '/' + confdir + '/' + MYHW + '/' + fname
    if os.path.exists(path):
        return path
    path = MYCONFIG + '/' + confdir + '/' + fname
    if os.path.exists(path):
        return path
    path = MYIMPORTS + '/' + confdir + '/' + MYHW + '/' + fname
    if os.path.exists(path):
        return path
    path = MYIMPORTS + '/' + confdir + '/' + fname
    if os.path.exists(path):
        return path
    return None

def read_json(fname, confdir):
    path = get_exact_path(fname, confdir)
    if path == None:
        return None
    read_file = open(path, "r")
    if read_file:
        data = json.load(read_file)
        return data
    print ("Reading JSON file " + path + " failed")
    return None

def merge_jsons(default_file, confdir):
    merge_data = read_json('merge.json', confdir)
    if merge_data == None:
        merge_list = [default_file]
    else:        
        merge_list = merge_data.get('merge', None)
        if merge_list == None:
            print("Merge data for '" + confdir + "' is erronous.")
            exit()

    cmd = MERGEJSON 
    for f in merge_list:
        path = get_exact_path(f, confdir)
        if path == None:
            if merge_data == None:
                return
            print("File '" + f + "' not found for '" + confdir + "'.")
            exit()
        cmd += ' ' + path 
    filename, file_extension = os.path.splitext(default_file)
    cmd += ' -o ' + MYINTERMEDIATE + '/' + MYHW + '/' + filename + '-merged.json'
    runcmd(cmd)

def pins_to_c():
    cmd = PINSTOC + ' ' + MYINTERMEDIATE + '/' + MYHW + '/pins-io-merged.json '
    cmd += '-o ' + MYINCLUDE + '/' + MYHW + '/pins-io.c -s ' + MYINTERMEDIATE + '/' + MYHW + '/signals-merged.json'
    runcmd(cmd)

def mymakedir(path):
    # Make sure that "include" "intermediate" directory exists. 
    try:
        os.makedirs(path)
    except FileExistsError:
        pass        

def generate_c_for_hardware():
    mymakedir(MYINCLUDE + '/' + MYHW)
    mymakedir(MYINTERMEDIATE + '/' + MYHW)
    merge_jsons('signals.json', 'signals')
    merge_jsons('parameters.json', 'parameters')
    merge_jsons('pins-io.json', 'pins')
    merge_jsons('network-defaults.json', 'network')
    pins_to_c()


def generate_c_for_io_application(confpath, coderoot, pythoncmd):
    # Generate list of HW configurations
    hw_list = []
    append_subdirectories(hw_list, confpath + '/signals')
    append_subdirectories(hw_list, confpath + '/pins')
    append_subdirectories(hw_list, confpath + '/parameters')
    append_subdirectories(hw_list, confpath + '/network')

    # Loop trough hardware configurations. If none, use 'generic'
    if len(hw_list) == 0:
        setup_environment(confpath, 'generic', coderoot, pythoncmd)
        generate_c_for_hardware()
    else:
        for hw in hw_list:
            setup_environment(confpath, hw, coderoot, pythoncmd)
            generate_c_for_hardware()

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

def mymain():
    n = len(sys.argv)
    sourcepaths = []
    expect = None
    coderoot = None
    pythoncmd = None
    for i in range(1, n):
        if sys.argv[i][0] == "-":
            switch = sys.argv[i][1]
            if switch == 'r' or switch == 'p':
                expect = switch
        else:
            if expect=='r':
                coderoot = sys.argv[i]
                expect = None
            elif expect=='p':
                pythoncmd = sys.argv[i]
                expect = None
            else:
                sourcepaths.append(sys.argv[i])

    if len(sourcepaths) < 1:
        print("No source files")
#        exit()

        sourcepaths.append('/coderoot/iocom/examples/candy/config')

    for confpath in sourcepaths:
        print("Processing path " + confpath)
        generate_c_for_io_application(confpath, coderoot, pythoncmd)


mymain()
