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
    hfile.write("  iocSignal " + signal_name + ";\n")

def calc_signal_memory_sz(type, array_n):
    if type == "boolean":
        if array_n == 1:
            return 1
        #else            
            return (array_n + 7) / 8 + 1;

    type_sz = osal_typeinfo[type];
    return array_n * type_sz + 1

def write_signal_to_c_source(pin_type, signal_name, signal):
    global current_type, current_addr, max_addr, signal_nr, nro_signals, handle, pinlist

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

    my_name = prefix + block_name + '.' + signal_name
    if signal_nr == 1:
        cfile.write('&' + my_name + '},\n')

    cfile.write('  {')

    cfile.write(str(addr) + ", ")
    cfile.write(str(array_n) + ", ")
    cfile.write('OS_' + type.upper() + ', ')
    cfile.write('0, ' + handle)

    if signal_name in pinlist:
        cfile.write(', ' + pin_prefix + signal_name)

    cfile.write('}')

    if signal_nr < nro_signals:
        cfile.write(',')
        
    signal_nr = signal_nr + 1

    cfile.write(' /* ' + signal_name + ' */\n')


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
    global prefix, block_name, handle, pin_prefix
    global current_addr, max_addr, signal_nr, nro_signals

    block_name = mblk.get("name", "MBLK")
    prefix = mblk.get('prefix', block_name + '_')
    pin_prefix = mblk.get('pin_prefix', '&io_')
    handle = mblk.get("handle", "OS_NULL")         
    groups = mblk.get("groups", None)
    if groups == None:
        print("'groups' not found for " + block_name)
        exit()

    current_addr = 0
    max_addr = 32
    signal_nr = 1

    instance_name = prefix + block_name
    struct_name = instance_name + "_t"

    nro_signals = 0
    for group in groups:
        signals = group.get("signals", None);
        if signals != None:
            for signal in signals:
                nro_signals += 1

    hfile.write('typedef struct ' + struct_name + '\n{\n')
    hfile.write('  iocSignalStructHeader hdr;\n')

    cfile.write(struct_name + ' ' + instance_name + '\n= {')
    cfile.write('{' + handle + ', ' + str(nro_signals) + ', ')

    for group in groups:
        process_group_block(group)

    hfile.write('}\n' + struct_name + ';\n\n')
    hfile.write('extern ' + struct_name + ' ' + instance_name + ';\n')
    define_name = prefix + block_name + "_MBLK_SZ"
    hfile.write("#define " + define_name.upper() + " " + str(max_addr) + "\n\n")

    cfile.write('};\n\n')

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

def list_pins_rootblock(rootblock):
    groups = rootblock.get("groups", None)

    for group in groups:
        pins  = group.get("pins", None);
        if pins != None:
            for pin in pins:
                name = pin.get('name', None)
                if name is not None:
                    pinlist.append(name)

def list_pins_in_pinsfile(path):
    pins_file = open(path, "r")
    if pins_file:
        data = json.load(pins_file)
        rootblocks = data.get("io", None)
        if rootblocks == None:
            print("'io' not found")
            exit()

        for rootblock in rootblocks:
            list_pins_rootblock(rootblock)

    else:
        printf ("Opening file " + path + " failed")
            
def mymain():
    global cfilepath, hfilepath, pinlist

    # Get command line arguments
    n = len(sys.argv)
    sourcefiles = []
    outpath = None
    pinspath = None
    expectpath = True
    for i in range(1, n):
        if sys.argv[i][0] is "-":
            if sys.argv[i][1] is "o":
                outpath = sys.argv[i+1]
                expectpath = False

            if sys.argv[i][1] is "p":
                pinspath = sys.argv[i+1]
                expectpath = False

        else:
            if expectpath:
                sourcefiles.append(sys.argv[i])
            expectpath = True    

    if len(sourcefiles) < 1:
        print("No source files")
        exit()

#    sourcefiles.append('/coderoot/iocom/examples/gina/config/signals/gina-signals.json')
#    pinspath = '/coderoot/iocom/examples/gina/config/pins/carol/gina-io.json'

    if outpath is None:
        outpath = sourcefiles[0]

    pinlist = []
    if pinspath is not None:
        list_pins_in_pinsfile(pinspath)

    filename, file_extension = os.path.splitext(outpath)
    cfilepath = filename + '.c'
    hfilepath = filename + '.h'

    print("Writing files " + cfilepath + " and " + hfilepath)

    start_c_files()

    for path in sourcefiles:
        process_source_file(path)

    finish_c_files()

mymain()