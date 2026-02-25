#!/usr/bin/env python3
# generate_c_code.py 11.5.2020/pekka
import json
import os
import platform
import sys

def setup_environment(confpath, hw, coderoot, pythoncmd):
    global MYHW, MYPYTHON,  CODEROOT, JSONTOOL, PINSTOC, BINTOC, SIGNALSTOC, MERGEJSON, MYIMPORTS
    global MYCONFIG, MYSIGNALS, MYPINS, MYPARAMETERS, MYNETWORK, MYINCLUDE, MYINTERMEDIATE, CFILES
    global PARAMETERSTOC, PARAMETERSTOSIGNALS, SETVERSION

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
    PARAMETERSTOC = MYPYTHON + ' ' + MYCODEROOT + '/iocom/scripts/parameters_to_c.py'
    PARAMETERSTOSIGNALS = MYPYTHON + ' ' + MYCODEROOT + '/iocom/scripts/parameters_to_signals.py'
    MERGEJSON = MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/merge_json.py'
    MYIMPORTS  = MYCODEROOT + '/iocom/config'

    MYCONFIG = confpath
    MYSIGNALS = MYCONFIG + '/signals'
    MYPINS = MYCONFIG + '/pins/' + MYHW + '/pins_io'
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

    # Check also intermediate directory
    if fname != "merge.json":
        path = MYCONFIG + '/intermediate/' + MYHW + '/' + fname
        if os.path.exists(path):
            return path
        path = MYCONFIG + '/intermediate/' + fname
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
    print ("oenerate_c_code.py: Reading JSON file " + path + " failed")
    return None

def merge_jsons(default_file, confdir):
    rval = default_file
    merge_data = read_json('merge.json', confdir)
    if merge_data == None:
        merge_list = [default_file]
    else:
        merge_list = merge_data.get('merge', None)
        if merge_list == None:
            print("oenerate_c_code.py: Merge data for '" + confdir + "' is erronous.")
            exit()
        rval = merge_list[0]
    rval, ext = os.path.splitext(rval)
    cmd = MERGEJSON
    for f in merge_list:
        rval2, ext2 = os.path.splitext(f)
        if ext2 == '.json':
            path = get_exact_path(f, confdir)
            if path == None:
                if merge_data == None:
                    return None
                print("oenerate_c_code.py: File '" + f + "' not found for '" + confdir + "'.")
                exit()
            cmd += ' ' + path
    cmd += ' -o ' + MYINTERMEDIATE + '/' + MYHW + '/' + rval + '-merged.json'
    runcmd(cmd)
    return rval

def compress_json(fname, extra_args):
    cmd = JSONTOOL + ' --t2b -title '
    if extra_args != None:
        cmd += extra_args + ' '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + fname + '.json '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + fname + '.binjson'
    runcmd(cmd)

    cmd = JSONTOOL + ' --b2t '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + fname + '.binjson '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + fname + '-check.json'
    runcmd(cmd)

def parameters_to_signals(parameters_name):
    cmd = PARAMETERSTOSIGNALS + ' ' + MYINTERMEDIATE + '/' + MYHW + '/' + parameters_name + '-merged.json'
    cmd += ' -o ' + MYINTERMEDIATE + '/' + MYHW + '/' + parameters_name + '-as-signals.json'
    runcmd(cmd)

def parameters_to_c(parameters_name):
    cmd = PARAMETERSTOC + ' ' + MYINTERMEDIATE + '/' + MYHW + '/' + parameters_name + '-merged.json'
    cmd += ' -o ' + MYINCLUDE + '/' + MYHW + '/' + parameters_name + '.c'
    CFILES.append(parameters_name)
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
    cmd += ' -h ' + MYHW
    if server_flag != None:
        cmd += ' -a ' + server_flag
    CFILES.append(signals_name)
    runcmd(cmd)

def slave_device_signals_to_c(slavepath, hw):
    if hw == '*':
        hw = MYHW
    path, slavedevicename = os.path.split(slavepath)
    cmd = SIGNALSTOC + ' -a slave-device ' + slavepath + '/config/intermediate/' + hw + '/signals-merged.json '
    # cmd = SIGNALSTOC + ' -a controller-static ' + slavepath + '/config/intermediate/' + hw + '/signals-merged.json '
    cmd += '-o ' + MYINCLUDE + '/' + MYHW + '/' + slavedevicename + '_signals.c'
    CFILES.append(slavedevicename + '_signals')
    runcmd(cmd)

def bin_json_to_c(v, src_json, c_file):
    cmd = BINTOC + ' -v ' + v + ' '
    cmd += MYINTERMEDIATE + '/' + MYHW + '/' + src_json + '.binjson '
    cmd += '-o ' + MYINCLUDE + '/' + MYHW + '/' + c_file + '.c'
    CFILES.append(c_file)
    runcmd(cmd)

# Write a file only if content has changed
def write_file_only_if_changed(filepath, content):
    if (os.path.isfile(filepath)):
        hfile = open(filepath, "r")
        old_content = hfile.read()
        hfile.close()
        if (content == old_content):
            return

    print("Writing generate_c_code.py top file " + filepath)

    hfile = open(filepath, "w")
    hfile.write(content)
    hfile.close()
    return

def make_common_header(common_c_file):
    content = '/* This file is generated by generate_c_code.py script, do not modify. */\n'
    for fname in CFILES:
        content += '#include "config/include/' + MYHW + '/' + fname + '.h"\n'
    write_file_only_if_changed(MYINCLUDE + '/' + MYHW + '/' + common_c_file + '.h', content)

def make_common_cfile(common_c_file):
    content = '/* This file is generated by generate_c_code.py script, do not modify. */\n'

    content += '#define IOCOM_IOBOARD\n'
    content += '#include "iocom.h"\n'
    if "pins_io" in CFILES:
        content += '#include "pinsx.h"\n'

    for fname in CFILES:
        content += '#include "config/include/' + MYHW + '/' + fname + '.h"\n'
    for fname in CFILES:
        content += '#include "config/include/' + MYHW + '/' + fname + '.c"\n'
    write_file_only_if_changed(MYINCLUDE + '/' + MYHW + '/' + common_c_file + '.c', content)
    pass

def mymakedir(path):
    try:
        os.makedirs(path)
    	# except FileExistsError:
    except:
        pass

def set_version():
    global MYPYTHOM, MYCODEROOT
    cmd = MYPYTHON + ' ' + MYCODEROOT + '/eosal/scripts/set_version.py'
    runcmd(cmd)

def generate_c_for_hardware(slavedevices, server_flag, common_c_file):
    mymakedir(MYINCLUDE + '/' + MYHW)
    mymakedir(MYINTERMEDIATE + '/' + MYHW)
    parameters_name = merge_jsons('parameters.json', 'parameters')
    if parameters_name != None:
        if os.path.exists(MYINTERMEDIATE + '/' + MYHW + '/' + parameters_name + '-merged.json'):
            parameters_to_signals(parameters_name)
            parameters_to_c(parameters_name)

    signals_name = merge_jsons('signals.json', 'signals')
    pins_name = merge_jsons('pins_io.json', 'pins')
    network_name = merge_jsons('network_defaults.json', 'network')
    accounts_name = merge_jsons('account_defaults.json', 'accounts')
    compress_json(signals_name + '-merged', None)
    signals_to_c(server_flag, signals_name, pins_name)
    bin_json_to_c('ioapp_' + signals_name + '_config', signals_name + '-merged', signals_name + '_info_mblk')
    if pins_name != None:
        if os.path.exists(MYINTERMEDIATE + '/' + MYHW + '/' + pins_name + '-merged.json'):
            pins_to_c(pins_name, signals_name)
    if network_name != None:
        if os.path.exists(MYINTERMEDIATE + '/' + MYHW + '/' + network_name + '-merged.json'):
            compress_json(network_name + '-merged', None)
            bin_json_to_c('ioapp_' + network_name, network_name + '-merged', network_name)
    if accounts_name != None:
        if os.path.exists(MYINTERMEDIATE + '/' + MYHW + '/' + accounts_name + '-merged.json'):
            compress_json(accounts_name + '-merged', '--hash-pw')
            bin_json_to_c('ioapp_' + accounts_name, accounts_name + '-merged', accounts_name)
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
    try:
        stream = os.popen(cmd)
        output = stream.read()
        exit_status = stream.close()

        if exit_status is not None:
            print ("generate_c_code.py: Command \'" + cmd + "\'failed with status " + str(exit_status))

    except Exception as e:
        print(f"generate_c_code.py: os.popen(\'" + cmd + "\') failed, exception:" + str(e))

def mymain():
    global MYPYTHON, MYCODEROOT
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
        print("oenerate_c_code.py: No source files")
        exit()

        # sourcepaths.append('/coderoot/iocom/examples/candy/config')
        # sourcepaths.append('/coderoot/iocom/examples/gina/config')

        # 'python c:/coderoot/iocom/scripts/generate_c_code.py c:/coderoot/iocom/examples/buster/config -r c:/coderoot -p python -d c:/coderoot/iocom/examples/minion,grumpy -l c:/coderoot/iocom/extensions/ioserver -a controller-static'
        # sourcepaths.append('/coderoot/iocom/examples/buster/config')
        # slavedevices.append("c:/coderoot/iocom/examples/minion,grumpy")
        # conflibs.append("c:/coderoot/iocom/extensions/ioserver")
        # server_flag = "controller-static"
    
    if platform.system() == 'Windows':
        MYPYTHON = 'python'
        MYCODEROOT = 'c:/coderoot'
    else:
        MYPYTHON = 'python3'
        MYCODEROOT = '/coderoot'

    # Set build version (date and time)
    set_version()

    # Run slave device and library configuration scripts
    for lib in conflibs:
        runcmd(MYPYTHON + " " + lib + '/scripts/config_to_c_code.py')

    for device in slavedevices:
        path_hw = device.split(',')
        runcmd(MYPYTHON + " " + path_hw[0] + '/scripts/config_to_c_code.py')

    for confpath in sourcepaths:
        print("Processing path " + confpath)
        generate_c_for_io_application(confpath, coderoot, pythoncmd, slavedevices, server_flag, common_c_file)

mymain()
