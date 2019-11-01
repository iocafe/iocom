# signals-to-c.py 31.10.2019/pekka
# Converts communication signal map written in JSON to C source and header files. 
import json
import os
import sys

# Size excludes signal state byte. 
osal_typeinfo = {
    "undef" : 0,
    "boolean" : 1,
    "char" : 1,
    "uchar" : 1,
    "short" : 2,
    "ushort" : 2,
    "int" : 3, 
    "uint" : 3,
    "int64" : 8,
    "long" : 8,
    "float" : 4,
    "double" : 8,
    "dec01" : 2,
    "dec001" : 2,
    "str" : 1, 
    "object" : 0,
    "pointer" : 0}

def start_c_files():
    global cfile, hfile, cfilepath, hfilepath
    cfile = open(cfilepath, "w")
    hfile = open(hfilepath, "w")
    cfile.write('/* This file is gerated by signals-to-c.py script, do not modify. */\n')
    cfile.write('#include "iocom.h"\n')
    hfile.write('/* This file is gerated by signals-to-c.py script, do not modify. */\n')
    hfile.write('OSAL_C_HEADER_BEGINS\n')

def finish_c_files():
    global cfile, hfile
    hfile.write('OSAL_C_HEADER_ENDS\n')
    cfile.close()
    hfile.close()

def write_signal_to_c_header(signal_name):
    global prefix
    hfile.write("extern iocSignal " + prefix + signal_name + ";\n")

def calc_signal_memory_sz(type, array_n):
    if type == "boolean":
        if array_n == 1:
            return 1
        #else            
            return (array_n + 7) / 8 + 1;

    type_sz = osal_typeinfo[type];
    return array_n * type_sz + 1

def write_signal_to_c_source(pin_type, signal_name, signal):
    global prefix, block_name, prev_signals_c_name
    global current_type, current_addr, max_addr

    addr = signal.get('addr', current_addr);
    if addr != current_addr:
        current_addr = addr

    type = signal.get('type', current_type);
    if type != current_type:
        current_type = type

    array_n = signal.get('array', 1);
    if array_n < 1:
        array_n = 1

    current_addr += calc_signal_memory_sz(type, array_n)
    if current_addr > max_addr:
        max_addr = current_addr

    # Write pin name and type
    signals_c_name = prefix + signal_name
    cfile.write("iocSignal " + signals_c_name + ' = {')

    # Write address
    cfile.write(str(addr) + ", ")
    cfile.write(str(array_n) + ", ")
    cfile.write('OS_' + type.upper())

    # Setup linked list for all signals in this memory block
    cfile.write(", 0,  " + prev_signals_c_name)
    prev_signals_c_name = "&" + signals_c_name
    cfile.write("};\n")

def write_linked_list_heads():
    global prefix, block_name, prev_signals_c_name
    if prev_signals_c_name is not "OS_NULL":
        varname = prefix + block_name + "_signals";
        cfile.write("iocSignal *" + varname + " = " + prev_signals_c_name + ";\n")
        hfile.write("extern iocSignal *" + varname + ";\n")

        define_name = prefix + block_name + "_MBLK_SZ"
        hfile.write("#define " + define_name.upper() + " " + str(max_addr) + "\n")

def process_signal(group_name, signal):
    global block_name
    signal_name = signal.get("name", None)
    if signal_name == None:
        print("'name' not found for signal in " + block_name + " " + group_name)
        exit()
    write_signal_to_c_header(signal_name)
    write_signal_to_c_source(group_name, signal_name, signal)

def process_group_block(group):
    global current_type
    group_name = group.get("name", None);
    if group_name == None:
        print("'name' not found for group in " + block_name)
        exit()
    signals = group.get("signals", None);
    if signals == None:
        print("'signals' not found for " + block_name + " " + group_name)
        exit()

    if group_name == 'inputs' or group_name == 'outputs':
        current_type = "boolean";
    else:
        current_type = "ushort";

    for signal in signals:
        process_signal(group_name, signal)

def process_mblk(mblk):
    global prefix, block_name, prev_signals_c_name
    global current_addr, max_addr

    block_name = mblk.get("name", "MBLK")
    prefix = mblk.get("prefix", None)
    if prefix == None:
        prefix = block_name + '_'
    groups = mblk.get("groups", None)
    if groups == None:
        print("'groups' not found for " + block_name)
        exit()

    prev_signals_c_name = "OS_NULL"
    current_addr = 0
    max_addr = 32

    for group in groups:
        process_group_block(group)

    write_linked_list_heads()

def process_source_file(path):
    read_file = open(path, "r")
    if read_file:
        data = json.load(read_file)
        mblks = data.get("mblk", None)
        if mblks == None:
            print("'mblk' not found")
            exit()

        for mblk in mblks:
            process_mblk(mblk)

    else:
        printf ("Opening file " + path + " failed")
            
def mymain():
    global cfilepath, hfilepath

    # Get options
    n = len(sys.argv)
    sourcefiles = []
    outpath = None
    for i in range(1, n):
        if sys.argv[i][0] is "-":
            if sys.argv[i][1] is "o":
                outpath = sys.argv[i+1]
                i = i + 1
                if i >=n:
                    print("Output file name must follow -o")
                    exit()

            s = sys.argv[i]

        else:
            sourcefiles.append(sys.argv[i])

    if len(sourcefiles) < 1:
        print("No source files")
#        exit()

    sourcefiles.append('/coderoot/iocom/examples/gina/config/signals/gina-signals.json')

    if outpath is None:
        outpath = sourcefiles[0]

    filename, file_extension = os.path.splitext(outpath)
    cfilepath = filename + '.c'
    hfilepath = filename + '.h'

    print("Writing files " + cfilepath + " and " + hfilepath)

    start_c_files()

    for path in sourcefiles:
        process_source_file(path)

    finish_c_files()

mymain()