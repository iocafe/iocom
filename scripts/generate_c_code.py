#!/usr/bin/env python3
# generate_c_code.py 11.5.2020/pekka
import json
import os
import platform
import sys

def setup_environment(confpath, hw, coderoot, pythoncmd):
    global MYHW, MYPYTHON,  CODEROOT, JSONTOOL, PINSTOC, BINTOC, SIGNALSTOC, MERGEJSON, MYIMPORTS
    global MYCONFIG, MYSIGNALS, MYPINS, MYPARAMETERS, MYNETWORK, MYINCLUDE, MYINTERMEDIATE, CFILES
     
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
    CFILES = []

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
    rval = default_file
    merge_data = read_json('merge.json', confdir)
    if merge_data == None:
        merge_list = [default_file]
    else:        
        merge_list = merge_data.get('merge', None)
        if merge_list == None:
            print("Merge data for '" + confdir + "' is erronous.")
            exit()
        rval = merge_list[0]
    rval, ext = os.path.splitext(rval)
    cmd = MERGEJSON 
    for f in merge_list:
        path = get_exact_path(f, confdir)
        if path == None:
            if merge_data == None:
                return None
            print("File '" + f + "' not found for '" + confdir + "'.")
            exit()
        cmd += ' ' + path 
    # filename, file_extension = os.path.splitext(default_file)
    cmd += ' -o ' + MYINTERMEDIATE + '/' + MYHW + '/' + rval + '-merged.json'
    runcmd(cmd)
    return rval

def compress_json(fname):
    cmd = JSONTOOL + ' --t2b -title '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + fname + '.json '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + fname + '.binjson'
    runcmd(cmd)

    cmd = JSONTOOL + ' --b2t '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + fname + '.binjson '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + fname + '-check.json'
    runcmd(cmd)

def pins_to_c(pins_name, signals_name):
    cmd = PINSTOC + ' ' + MYINTERMEDIATE + '/' + MYHW + '/' + pins_name + '-merged.json '
    cmd += '-o ' + MYINCLUDE + '/' + MYHW + '/' + pins_name + '.c -s ' + MYINTERMEDIATE + '/' + MYHW + '/' + signals_name + '-merged.json'
    CFILES.append(pins_name)
    runcmd(cmd)

def signals_to_c(server_flag, signals_name, pins_name):
    cmd = SIGNALSTOC + ' ' + MYINTERMEDIATE + '/' + MYHW + '/' + signals_name + '-merged.json'
    if pins_name != None:
        pins_json = MYINTERMEDIATE + '/' + MYHW + '/' + pins_name + '-merged.json'
        if os.path.exists(pins_json):
            cmd += ' -p ' + pins_json
    cmd += ' -o ' + MYINCLUDE + '/' + MYHW + '/' + signals_name + '.c'
    if server_flag != None:
        cmd += ' -a ' + server_flag
    CFILES.append(signals_name)
    runcmd(cmd)

def slave_device_signals_to_c(slavepath, hw):
    if hw == '*':
        hw = MYHW
    path, slavedevicename = os.path.split(slavepath)        
    cmd = SIGNALSTOC + ' -a controller-static ' + slavepath + '/config/intermediate/' + hw + '/signals-merged.json '
    cmd += '-o ' + MYINCLUDE + '/' + MYHW + '/' + slavedevicename + '-signals.c'
    CFILES.append(slavedevicename + '-signals')
    runcmd(cmd)

def bin_json_to_c(v, src_json, c_file):
    cmd = BINTOC + ' -v ' + v + ' '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + src_json + '.binjson '
    cmd += '-o ' + MYINCLUDE + '/' + MYHW + '/' + c_file + '.c'
    CFILES.append(c_file)
    runcmd(cmd)

def make_common_header(common_c_file):
    hfile = open(MYINCLUDE + '/' + MYHW + '/' + common_c_file + '.h', "w")
    hfile.write('/* This file is gerated by generate_c_code.py script, do not modify. */\n')
    for fname in CFILES:
        hfile.write('#include "config/include/' + MYHW + '/' + fname + '.h"\n')
    hfile.close()

def make_common_cfile(common_c_file):
    cfile = open(MYINCLUDE + '/' + MYHW + '/' + common_c_file + '.c', "w")
    cfile.write('/* This file is gerated by generate_c_code.py script, do not modify. */\n')

    cfile.write('#define IOCOM_IOBOARD\n')
    cfile.write('#include "iocom.h"\n')
    if "pins-io" in CFILES:
        cfile.write('#include "pinsx.h"\n')

    for fname in CFILES:
        cfile.write('#include "config/include/' + MYHW + '/' + fname + '.h"\n')
    for fname in CFILES:
        cfile.write('#include "config/include/' + MYHW + '/' + fname + '.c"\n')
    cfile.close()
    pass

def mymakedir(path):
    # Make sure that "include" and "intermediate" directories exists. 
    try:
        os.makedirs(path)
    except FileExistsError:
        pass        

def generate_c_for_hardware(slavedevices, server_flag, common_c_file):
    mymakedir(MYINCLUDE + '/' + MYHW)
    mymakedir(MYINTERMEDIATE + '/' + MYHW)
    signals_name = merge_jsons('signals.json', 'signals')
    parameters_name = merge_jsons('parameters.json', 'parameters')
    pins_name = merge_jsons('pins-io.json', 'pins')
    network_name = merge_jsons('network_defaults.json', 'network')
    compress_json(signals_name + '-merged')
    signals_to_c(server_flag, signals_name, pins_name)
    bin_json_to_c('ioapp_' + signals_name + '_config', signals_name + '-merged', signals_name + '-info-mblk')
    if pins_name != None:
        if os.path.exists(MYINTERMEDIATE + '/' + MYHW + '/' + pins_name + '-merged.json'):
            pins_to_c(pins_name, signals_name)
    if network_name != None:
        if os.path.exists(MYINTERMEDIATE + '/' + MYHW + '/' + network_name + '-merged.json'):
            compress_json(network_name + '-merged')
            bin_json_to_c('ioapp_' + network_name, network_name + '-merged', network_name)
    for device in slavedevices:
        path_hw = device.split(',')
        slave_device_signals_to_c(path_hw[0], path_hw[1])
    make_common_header(common_c_file)
    make_common_cfile(common_c_file)

def generate_c_for_io_application(confpath, coderoot, pythoncmd, slavedevices, server_flag, common_c_file):
    # Generate list of HW configurations
    hw_list = []
    append_subdirectories(hw_list, confpath + '/signals')
    append_subdirectories(hw_list, confpath + '/pins')
    append_subdirectories(hw_list, confpath + '/parameters')
    append_subdirectories(hw_list, confpath + '/network')

    # Loop trough hardware configurations. If none, use 'generic'
    if len(hw_list) == 0:
        setup_environment(confpath, 'generic', coderoot, pythoncmd)
        generate_c_for_hardware(slavedevices, server_flag, common_c_file)
    else:
        for hw in hw_list:
            setup_environment(confpath, hw, coderoot, pythoncmd)
            generate_c_for_hardware(slavedevices, server_flag, common_c_file)

def runcmd(cmd):
    stream = os.popen(cmd)
    output = stream.read()
    print(output)

def mymain():
    n = len(sys.argv)
    sourcepaths = []
    conflibs = []
    slavedevices = []
    expect = None
    coderoot = None
    pythoncmd = None
    server_flag = None
    common_c_file = 'json_io_config'
    for i in range(1, n):
        if sys.argv[i][0] == "-":
            switch = sys.argv[i][1]
            if switch == 'r' or switch == 'p' or switch == 'l' or switch == 'd' or switch == 'a' or switch == 'c':
                expect = switch
        else:
            if expect=='r':
                coderoot = sys.argv[i]
                expect = None
            elif expect=='p':
                pythoncmd = sys.argv[i]
                expect = None
            elif expect=='l':
                conflibs.append(sys.argv[i])
                expect = None
            elif expect=='d':
                slavedevices.append(sys.argv[i])
                expect = None
            elif expect=='s':
                slavedevices.append(sys.argv[i])
                expect = None
            elif expect=='a':
                server_flag = sys.argv[i]
                expect = None
            elif expect=='c':
                common_c_file = sys.argv[i]
                expect = None
            else:
                sourcepaths.append(sys.argv[i])

    if len(sourcepaths) < 1:
        print("No source files")
#        exit()
#        sourcepaths.append('/coderoot/iocom/examples/candy/config')
        sourcepaths.append('/coderoot/dehec-ref/dref/config')

    # Run slave device and library configuration scripts
    for lib in conflibs:
        runcmd(lib + '/scripts/config_to_c_code.py')
    for device in slavedevices:
        path_hw = device.split(',')
        runcmd(path_hw[0] + '/scripts/config_to_c_code.py')

    for confpath in sourcepaths:
        print("Processing path " + confpath)
        generate_c_for_io_application(confpath, coderoot, pythoncmd, slavedevices, server_flag, common_c_file)

mymain()
