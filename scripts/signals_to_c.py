# signals-to_c.py 8.1.2020/pekka
# Converts communication signal map written in JSON to C source and header files.
import json
import os
import sys
import re

IOC_SIGNAL_NAME_SZ = 32
IOC_NAME_SZ = 16
IOC_PIN_NAME_SZ = 64
IOC_PIN_GROUP_NAME_SZ = 64

# Size excludes signal state byte.
osal_typeinfo = {
    "undef" : 0,
    "boolean" : 1,
    "char" : 1,
    "uchar" : 1,
    "short" : 2,
    "ushort" : 2,
    "int" : 4,
    "uint" : 4,
    "int64" : 8,
    "long" : 8,
    "float" : 4,
    "double" : 8,
    "dec01" : 2,
    "dec001" : 2,
    "str" : 1,
    "object" : 0,
    "pointer" : 0}

def check_valid_name(label, name, sz, allow_numbers):
    if name == None:
        print(label + " name is not defined.")
        exit()

    if len(name) >= sz:
        print(label + " name '" + name + "' is too long. Maximum is " + str(sz - 1) + "characters.")
        exit()

    if allow_numbers:
        if not re.match(r'^[A-Za-z0-9_]+$', name):
            print(label + " name '" + name + "' may contain only characters 'A' - 'Z', 'a' - 'z', '0' -'9' and underscore '_'.")
            exit()
    else:
        if not re.match(r'^[A-Za-z_]+$', name):
            print(label + " name '" + name + "' may contain only characters 'A' - 'Z', 'a' - 'z', and underscore '_'.")
            exit()

def start_c_files():
    global cfile, hfile, cfilepath, hfilepath
    cfile = open(cfilepath, "w")
    hfile = open(hfilepath, "w")
    cfile.write('/* This file is gerated by signals_to_c.py script, do not modify. */\n')
    hfile.write('/* This file is gerated by signals_to_c.py script, do not modify. */\n')
    hfile.write('OSAL_C_HEADER_BEGINS\n\n')

def finish_c_files():
    global cfile, hfile
    hfile.write('\nOSAL_C_HEADER_ENDS\n')
    cfile.close()
    hfile.close()

def write_signal_to_c_header(signal_name):
    global hfile
    hfile.write("    iocSignal " + signal_name + ";\n")

def calc_signal_memory_sz(type, array_n):
    if type == "boolean":
        if array_n <= 1:
            return 1
        else:
            return ((array_n + 7) >> 3) + 1;

    type_sz = osal_typeinfo[type];
    return array_n * type_sz + 1

def write_signal_to_c_source_for_iodevice(pin_type, signal_name, signal):
    global cfile, hfile, array_list, signal_list, reserved_addrs
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

    sname = device_name + '_' + block_name + '_' + signal_name
    signal_list.append('#define ' + sname.upper())
    if array_n > 1:
        arr_name = sname.upper() + '_ARRAY_SZ'
        array_list.append('#define ' + arr_name + ' ' + str(array_n))

    mem_sz_bytes = calc_signal_memory_sz(type, array_n);
    reserved_addrs += list(range(current_addr, current_addr + mem_sz_bytes))

    current_addr += mem_sz_bytes
    if current_addr > max_addr:
        max_addr = current_addr

    my_name = device_name + '.' + block_name + '.' + signal_name

    if signal_nr == 1:
        cfile.write('\n  {\n    {"' + block_name + '", ' + handle + ', ' + str(nro_signals) + ', ')
        define_name = device_name + '_' + block_name + "_MBLK_SZ"
        cfile.write(define_name.upper() + ', ')
        cfile.write('(iocSignal*)&' + my_name + '},\n')

    cfile.write('    {')

    cfile.write(str(addr) + ", ")
    cfile.write(str(array_n) + ", ")
    cfile.write('OS_' + type.upper())
    if signal_name in pinlist:
        cfile.write('|IOC_PIN_PTR')

    cfile.write(', ' + handle)

    if signal_name in pinlist:
        cfile.write(', ' + pinlist[signal_name])
    else:
        cfile.write(', OS_NULL')

    cfile.write('}')

    if signal_nr < nro_signals:
        cfile.write(',')

    signal_nr = signal_nr + 1
    cfile.write(' /* ' + signal_name + ' */\n')

def write_signal_to_c_source_for_controller(pin_type, signal_name, signal):
    global cfile, hfile, array_list, signal_list, reserved_addrs
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

    mem_sz_bytes = calc_signal_memory_sz(type, array_n);
    reserved_addrs += list(range(current_addr, current_addr + mem_sz_bytes))

    sname = device_name + '_' + block_name + '_' + signal_name
    signal_list.append('#define ' + sname.upper())
    if array_n > 1  and not is_dynamic:
        arr_name = sname.upper() + '_ARRAY_SZ'
        array_list.append('#define ' + arr_name + ' ' + str(array_n))

    current_addr += mem_sz_bytes
    if current_addr > max_addr:
        max_addr = current_addr

    if signal_nr == 1:
        my_name = '  s->' + block_name + '.hdr'
        cfile.write(my_name + '.mblk_name = "' + block_name + '";\n')
        cfile.write(my_name + '.n_signals = ' + str(nro_signals)  + ';\n')
        if not is_dynamic:
            define_name = device_name + '_' + block_name + "_MBLK_SZ"
            cfile.write(my_name + '.mblk_sz = ' + define_name.upper() + ';\n')
        cfile.write(my_name + '.first_signal = &s->' + block_name + '.' + signal_name + ';\n')

    cfile.write('\n /* ' + signal_name + ' */\n')
    my_name = '  s->' + block_name + '.' + signal_name
    if not is_dynamic:
        cfile.write(my_name + '.addr = ' + str(addr) + ';\n')
        cfile.write(my_name + '.n = ' + str(array_n) + ';\n')
        cfile.write(my_name + '.flags = OS_' + type.upper() + ';\n')

    if is_dynamic:
        cfile.write(my_name + '.ptr = \"' + signal_name + '\";\n')

    signal_nr = signal_nr + 1

def process_signal(group_name, signal):
    global block_name
    signal_name = signal.get("name", None)
    if signal_name == None:
        print("'name' not found for signal in '" + block_name + "' group '" + group_name + "'")
        exit()

    check_valid_name("Signal", signal_name, IOC_SIGNAL_NAME_SZ, True)

    write_signal_to_c_header(signal_name)
    if is_controller:
        write_signal_to_c_source_for_controller(group_name, signal_name, signal)
    else:
        write_signal_to_c_source_for_iodevice(group_name, signal_name, signal)

def process_group_block(group):
    global current_type
    group_name = group.get("name", None)
    if group_name == None:
        print("'name' not found for group in " + block_name)
        exit()
    signals = group.get("signals", None)
    if signals == None:
        print("'signals' not found for " + block_name + " " + group_name)
        exit()

    if group_name == 'inputs' or group_name == 'outputs':
        current_type = "boolean"
    else:
        current_type = "ushort"

    for signal in signals:
        process_signal(group_name, signal)

def check_if_duplicates(mylist):
    return len(mylist) != len(set(mylist))

def write_my_error(msg):
    global cfile
    print(msg)
    cfile.write('\n\n***************************************\n' + msg + '\n***************************************\n\n')

def process_mblk(mblk):
    global cfile, hfile, reserved_addrs
    global block_name, handle, define_list, mblk_nr, nro_mblks, mblk_list
    global current_addr, max_addr, signal_nr, nro_signals, is_controller

    block_name = mblk.get("name", "MBLK")
    check_valid_name("Memory block", block_name, IOC_NAME_SZ, True)
    handle = mblk.get("handle", "OS_NULL")
    if handle == "OS_NULL":
        handle = "&ioboard_" + block_name

    groups = mblk.get("groups", None)
    if groups == None:
        print("'groups' not found for " + block_name)
        exit()

    mblk_list.append(device_name + '.' + block_name)

    current_addr = 0
    reserved_addrs = []
    max_addr = 32
    signal_nr = 1

    nro_signals = 0
    for group in groups:
        signals = group.get("signals", None)
        if signals != None:
            for signal in signals:
                nro_signals += 1
        else:
            gname = group.get("name", "noname")
            print("'signals' not found for '" + block_name + "' group '" + gname + "'")
            exit()

    hfile.write('\n  struct ' + '\n  {\n')
    hfile.write('    iocMblkSignalHdr hdr;\n')

    for group in groups:
        process_group_block(group)

    hfile.write('  }\n  ' + block_name + ';\n')
    if not is_dynamic:
        define_name = device_name + '_' + block_name + "_MBLK_SZ"
        define_list.append("#define " + define_name.upper() + " " + str(max_addr) + "\n")

    if not is_controller:
        cfile.write('  }')
        if mblk_nr < nro_mblks:
            cfile.write(',\n')
    else:
        cfile.write('  s->mblk_list[' + str(mblk_nr - 1) + '] = &s->' + block_name + '.hdr;\n\n')

    if check_if_duplicates(reserved_addrs):
        write_my_error('ERROR: Duplicated signal addressess found in JSON!')

    mblk_nr = mblk_nr + 1

def write_assembly_item(prefix, ending, assembly_name):
    global cfile, is_controller, device_name
    if is_controller:
        cfile.write('  s->' + assembly_name + '.' + ending + ' =  &s->' + prefix + ending + ';\n')
    else:
        cfile.write('&' + device_name + '.' + prefix + ending + ',\n   ')

def process_assembly(assembly):
    global cfile, hfile, is_controller
    assembly_name = assembly.get("name", None)
    check_valid_name("Assembly", assembly_name, IOC_NAME_SZ, True)
    assembly_type = assembly.get("type", None)
    check_valid_name("Assembly type", assembly_type, IOC_NAME_SZ, True)

    if assembly_type == 'linecam' or assembly_type == 'camera' or assembly_type == 'microphone' or assembly_type == 'userinput':
        imp = assembly.get("imp", "imp.undefined_*")
        exp = assembly.get("exp", "exp.undefined_*")

        hfile.write('\n  iocStreamerSignals ' + assembly_name + ';\n')

        if is_controller:
            cfile.write('  /* ' + assembly_type + " '" + assembly_name + "' */\n")
        else:
            cfile.write(',\n\n  /* Signals for ' + assembly_type + " '" + assembly_name + "' */\n  {")
        write_assembly_item(imp, "cmd", assembly_name)
        write_assembly_item(imp, "select", assembly_name)
        write_assembly_item(exp, "buf", assembly_name)
        write_assembly_item(exp, "head", assembly_name)
        write_assembly_item(imp, "tail", assembly_name)
        write_assembly_item(exp, "state", assembly_name)
        if not is_controller:
            cfile.write('OS_FALSE}')

    elif assembly_type == 'display' or assembly_type == 'speaker':
        imp = assembly.get("imp", "imp.undefined_*")
        exp = assembly.get("exp", "exp.undefined_*")

        hfile.write('\n  iocStreamerSignals ' + assembly_name + ';\n')

        if is_controller:
            cfile.write(' /* ' + assembly_type + " '" + assembly_name + "' */\n")
        else:
            cfile.write(',\n\n  /* Signals for ' + assembly_type + " '" + assembly_name + "' */\n  {")
        write_assembly_item(imp, "cmd", assembly_name)
        write_assembly_item(imp, "select", assembly_name)
        write_assembly_item(imp, "buf", assembly_name)
        write_assembly_item(imp, "head", assembly_name)
        write_assembly_item(exp, "tail", assembly_name)
        write_assembly_item(exp, "state", assembly_name)
        if not is_controller:
            cfile.write('OS_TRUE}')

    else:
        print("Assembly '" + assembly_name + "' type '" + assembly_type + "' is uknown")
        exit()

def process_source_file(path):
    global cfile, hfile, array_list, signal_list
    global device_name, define_list, mblk_nr, nro_mblks, mblk_list
    read_file = open(path, "r")
    if read_file:
        data = json.load(read_file)
        if device_name == None:
            device_name = data.get("name", "unnameddevice")

        check_valid_name("Device", device_name, IOC_NAME_SZ, False)

        mblks = data.get("mblk", None)
        if mblks == None:
            print("'mblk' not found")
            exit()

        mblk_nr = 1;
        nro_mblks = 0
        for mblk in mblks:
            nro_mblks += 1

        struct_name = device_name + '_t'
        define_list = []
        hfile.write('typedef struct ' + struct_name + '\n{')

        if is_controller:
            hfile.write('\n  iocDeviceHdr hdr;\n')
            hfile.write('  iocMblkSignalHdr *mblk_list[' + str(nro_mblks) + '];\n')

            cfile.write('void ' + device_name + '_init_signal_struct(' + struct_name + ' *s)\n{\n')
            cfile.write('  os_memclear(s, sizeof(' + struct_name + '));\n')

        else:
            cfile.write('OS_FLASH_MEM struct ' + struct_name + ' ' + device_name + ' = \n{')

        mblk_list = []
        array_list = []
        signal_list = []

        for mblk in mblks:
            process_mblk(mblk)

        assemblies = data.get("assembly", None)
        if assemblies != None:
            for assembly in assemblies:
                process_assembly(assembly)

        cfile.write('\n')

        if is_controller:
            cfile.write('  s->hdr.n_mblk_hdrs = ' + str(nro_mblks) + ';\n')
            cfile.write('  s->hdr.mblk_hdr = s->mblk_list;\n')
            cfile.write('}\n')
        else:
            cfile.write('};\n')

        hfile.write('}\n' + struct_name + ';\n\n')

        for d in define_list:
            hfile.write(d)

        if not is_controller:
            hfile.write('\nextern OS_FLASH_MEM_H ' + struct_name + ' ' + device_name + ';\n')

            list_name = device_name + "_mblk_list"
            cfile.write('\nstatic OS_FLASH_MEM iocMblkSignalHdr * OS_FLASH_MEM ' + list_name + '[] =\n{\n  ')
            isfirst = True
            for p in mblk_list:
                if not isfirst:
                    cfile.write(',\n  ')
                isfirst = False
                cfile.write('&' + p + '.hdr')
            cfile.write('\n};\n\n')
            cfile.write('OS_FLASH_MEM iocDeviceHdr ' + device_name + '_hdr = {(iocMblkSignalHdr**)' + list_name + ', sizeof(' + list_name + ')/' + 'sizeof(iocMblkSignalHdr*)};\n')
            hfile.write('extern OS_FLASH_MEM_H iocDeviceHdr ' + device_name + '_' + 'hdr;\n\n')

        else:
            hfile.write('\nvoid ' + device_name + '_init_signal_struct(' + struct_name + ' *s);\n')

        if len(array_list) > 0:
            hfile.write('\n/* Array length defines. */\n')
            for p in array_list:
                hfile.write(p + '\n')

        if len(signal_list) > 0:
            hfile.write('\n/* Defines to check in code with #ifdef to know if signal is configured in JSON. */\n')
            for p in signal_list:
                hfile.write(p + '\n')

    else:
        print("Opening file " + path + " failed")

def list_pins_rootblock(rootblock):
    global pinlist
    prins_prefix = rootblock.get('prefix', "pins")
    groups = rootblock.get("groups", None)

    for group in groups:
        pins  = group.get("pins", None);
        if pins != None:
            pingroup_name = group.get("name", None);
            check_valid_name("Pin group", pingroup_name, IOC_PIN_GROUP_NAME_SZ, True)
            for pin in pins:
                name = pin.get('name', None)
                check_valid_name("Pin", name, IOC_PIN_NAME_SZ, True)
                if name is not None:
                    pinlist.update({name : '&' + prins_prefix + '.' + pingroup_name + '.' + name})

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
        print("Opening file " + path + " failed")

def mymain():
    global cfilepath, hfilepath, pinlist, device_name, is_controller, is_dynamic

    # Get command line arguments
    n = len(sys.argv)
    sourcefiles = []
    outpath = None
    pinspath = None
    expectpath = True
    device_name = None
    application_type = "iodevice"
    for i in range(1, n):
        if sys.argv[i][0] == "-":
            if sys.argv[i][1] == "o":
                outpath = sys.argv[i+1]
                expectpath = False

            if sys.argv[i][1] == "p":
                pinspath = sys.argv[i+1]
                expectpath = False

            if sys.argv[i][1] == "d":
                device_name = sys.argv[i+1]
                expectpath = False

            if sys.argv[i][1] == "a":
                # application_type:
                #   "iodevice" - IO board, etc
                #   "controller-static" Controller using static addressess and types to match data with IO device
                #   "controller-dynamic" Controller using signal names to match types and
                application_type = sys.argv[i+1]
                expectpath = False

        else:
            if expectpath:
                sourcefiles.append(sys.argv[i])
            expectpath = True

    if len(sourcefiles) < 1:
        print("No source files")
        exit()

#    sourcefiles.append('/coderoot/iocom/examples/gina/config/signals/signals.json')
#    outpath = '/coderoot/iocom/examples/gina/config/include/carol/signals.c'
#    outpath = '/coderoot/iocom/examples/tito/config/include/gina-for-tito.c'
#    pinspath = '/coderoot/iocom/examples/gina/config/pins/carol/gina-io.json'
#    application_type = "controller-static"
#    application_type = "controller-dynamic"

    is_controller = False
    is_dynamic = False
    if application_type == "controller-static":
        is_controller = True

    if application_type == "controller-dynamic":
        is_controller = True
        is_dynamic = True

    if outpath is None:
        outpath = sourcefiles[0]

    pinlist = {}
    if pinspath is not None:
        list_pins_in_pinsfile(pinspath)

    filename, file_extension = os.path.splitext(outpath)
    cfilepath = filename + '.c'
    hfilepath = filename + '.h'

    print("Writing files " + cfilepath + " and " + hfilepath)

    start_c_files()

    for path in sourcefiles:
        process_source_file(path)

    if not is_controller:
        hfile.write('\n#ifndef IOBOARD_DEVICE_NAME\n')
        hfile.write('#define IOBOARD_DEVICE_NAME \"' + device_name + '\"\n#endif\n')

    finish_c_files()

mymain()