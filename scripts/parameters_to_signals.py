# parameters_to_signals.py 11.6.2020/pekka
# Convert parameter JSON file to signals format.
import json
import os
import sys
import copy

def setup_group(x_groups, group_name):
    for group in x_groups:
        if group_name == group.get("name", None):
            return group["signals"]

    group = {"name" : group_name, "signals" : []} 
    x_groups.append(group)
    return group["signals"]

def append_parameter(x_signals, parameter, prefix, is_persistent):
    p = copy.deepcopy(parameter)
    if prefix != None:
        name = p.get("name", "noname")
        p["name"] = prefix + name

    if is_persistent:
        p["prm_flags"] = 3
    else:
        p["prm_flags"] = 1

    x_signals.append(p)
 
def merge_group(merged, data, exp_groups, imp_groups, is_persistent):
    group_name = data.get("name", "noname")
    parameters = data.get("parameters", None)
    if parameters == None:
        print("No parameters for " + name + "?")
        return

    exp_signals = setup_group(exp_groups, group_name)
    imp_signals = setup_group(imp_groups, group_name)

    for parameter in parameters:
        append_parameter(exp_signals, parameter, None, is_persistent)
        append_parameter(imp_signals, parameter, "set_", is_persistent)

def merge(merged, data, exp_groups, imp_groups, is_persistent):
    title = data.get("title", "untitled")
    groups = data.get("groups", None)
    if groups == None:
        print('"' + title + '" is empty?')
        return

    for group in groups:
        merge_group(merged, group, exp_groups, imp_groups, is_persistent)

def process_source_file(merged, path, exp_groups, imp_groups):
    read_file = open(path, "r")
    if read_file:
        data = json.load(read_file)

        device_name = data.get("name", None)
        if device_name != None:
            merged["name"] = device_name

        persistent = data.get("persistent", None)
        if persistent != None:
            merge(merged, persistent, exp_groups, imp_groups, True)

        volatile = data.get("volatile", None)
        if volatile != None:
            merge(merged, volatile, exp_groups, imp_groups, False)
    else:
        print ("Opening file " + path + " failed")

def mymain():
    n = len(sys.argv)
    sourcefiles = []
    outpath = None
    expect = None
    for i in range(1, n):
        if sys.argv[i][0] == "-":
            if sys.argv[i][1] == "o":
                expect = 'o'
        else:
            if expect=='o':
                outpath = sys.argv[i]
                expect = None
            else:
                sourcefiles.append(sys.argv[i])

    if len(sourcefiles) < 1:
        print("No source files")
#        exit()

        sourcefiles.append('/coderoot/iocom/examples/candy/config/parameters/parameters.json')
#        sourcefiles.append('/coderoot/iocom/config/parameters/wifi_dhcp_device_network_parameters.json')

    # If output path is not given as argument.
    if outpath is None:
        path, file_extension = os.path.splitext(sourcefiles[0])
        dir_path, file_name =  os.path.split(path)
        outpath = dir_path + '/intermediate/' + file_name + '-as-signals' + file_extension

    # Make sure that "intermediate" directory exists.
    dir_path, file_name =  os.path.split(outpath)
    try:
        os.makedirs(dir_path)
    except FileExistsError:
        pass

    print("Writing file " + outpath)

    exp = {"name":"exp", "flags": "up"}
    imp = {"name":"imp", "flags": "down"}
    exp_groups = []
    exp["groups"] = exp_groups
    imp_groups = []
    imp["groups"] = imp_groups
    mblk = [exp, imp]
    merged = {"mblk" : mblk}

    for path in sourcefiles:
        process_source_file(merged, path, exp_groups, imp_groups)

    with open(outpath, "w") as outfile:
        json.dump(merged, outfile, indent=4)

mymain()
