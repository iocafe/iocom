# parameters_to_c.py 8.1.2020/pekka
# Converts communication parameter map written in JSON to C source and header files.
import json
import os
import sys
import re

IOC_parameter_NAME_SZ = 32
IOC_NAME_SZ = 16

# Size excludes parameter state byte.
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
    cfile.write('/* This file is gerated by parameters_to_c.py script, do not modify. */\n')
    hfile.write('/* This file is gerated by parameters_to_c.py script, do not modify. */\n')
    hfile.write('OSAL_C_HEADER_BEGINS\n\n')

def finish_c_files():
    global cfile, hfile
    hfile.write('\nOSAL_C_HEADER_ENDS\n')
    cfile.close()
    hfile.close()

def write_parameter_to_c_header(parameter_name, parameter):
    global hfile, current_type

    type = parameter.get('type', current_type);
    if type != current_type:
        current_type = type

    if type == 'str':
        showtype = 'os_char'
    else:
        showtype = 'os_' + type

    hfile.write('  ' + showtype +  ' ' + parameter_name)

    array_n = parameter.get('array', 1);
    if array_n > 1:
        hfile.write('[' + str(array_n) + ']')
        array_n = 1

    hfile.write(';\n')

def calc_parameter_memory_sz(type, array_n):
    if type == "boolean":
        if array_n <= 1:
            return 1
        else:
            return ((array_n + 7) >> 3) + 1;

    type_sz = osal_typeinfo[type];
    return array_n * type_sz + 1

def write_parameter_to_c_source_for_iodevice(pin_type, parameter_name, parameter):
    global cfile, hfile, array_list, parameter_list, reserved_addrs
    global current_type, current_addr, max_addr, parameter_nr, nro_parameters, handle

    addr = parameter.get('addr', current_addr);
    if addr != current_addr:
        current_addr = addr

    type = parameter.get('type', current_type);
    if type != current_type:
        current_type = type

    array_n = parameter.get('array', 1);
    if array_n < 1:
        array_n = 1

    sname = device_name + '_' + block_name + '_' + parameter_name
    parameter_list.append('#define ' + sname.upper())
    if array_n > 1:
        arr_name = sname.upper() + '_ARRAY_SZ'
        array_list.append('#define ' + arr_name + ' ' + str(array_n))

    mem_sz_bytes = calc_parameter_memory_sz(type, array_n);
    reserved_addrs += list(range(current_addr, current_addr + mem_sz_bytes))

    current_addr += mem_sz_bytes
    if current_addr > max_addr:
        max_addr = current_addr

    my_name = device_name + '.' + block_name + '.' + parameter_name

    if parameter_nr == 1:
        cfile.write('\n  {\n    {"' + block_name + '", ' + 'handle' + ', ' + str(nro_parameters) + ', ')
        define_name = device_name + '_' + block_name + "_pblk_SZ"
        cfile.write(define_name.upper() + ', ')
        cfile.write('(iocparameter*)&' + my_name + '},\n')

    cfile.write('    {')

    cfile.write(str(addr) + ", ")
    cfile.write(str(array_n) + ", ")
    cfile.write('OS_' + type.upper())

    cfile.write(', ' + 'handle')

    cfile.write('}')

    if parameter_nr < nro_parameters:
        cfile.write(',')

    parameter_nr = parameter_nr + 1
    cfile.write(' /* ' + parameter_name + ' */\n')

def process_parameter(group_name, parameter):
    global block_name
    parameter_name = parameter.get("name", None)
    if parameter_name == None:
        print("'name' not found for parameter in '" + block_name + "' group '" + group_name + "'")
        exit()

    check_valid_name("parameter", parameter_name, IOC_parameter_NAME_SZ, True)

    write_parameter_to_c_header(parameter_name, parameter)
    write_parameter_to_c_source_for_iodevice(group_name, parameter_name, parameter)

def process_group_block(group):
    global current_type
    group_name = group.get("name", None)
    if group_name == None:
        print("'name' not found for group in " + block_name)
        exit()
    parameters = group.get("parameters", None)
    if parameters == None:
        print("'parameters' not found for " + block_name + " " + group_name)
        exit()

    if group_name == 'inputs' or group_name == 'outputs':
        current_type = "boolean"
    else:
        current_type = "ushort"

    for parameter in parameters:
        process_parameter(group_name, parameter)

def check_if_duplicates(mylist):
    return len(mylist) != len(set(mylist))

def write_my_error(msg):
    global cfile
    print(msg)
    cfile.write('\n\n***************************************\n' + msg + '\n***************************************\n\n')

def process_parameter_block(name, pblk):
    global cfile, hfile, reserved_addrs, block_name
    global handle, define_list, pblk_nr
    global current_addr, max_addr, parameter_nr, nro_parameters

    block_name = name
    groups = pblk.get("groups", None)
    if groups == None:
        print("'groups' not found for " + block_name)
        exit()

    current_addr = 0
    reserved_addrs = []
    max_addr = 32
    parameter_nr = 1

    nro_parameters = 0
    for group in groups:
        parameters = group.get("parameters", None)
        if parameters != None:
            for parameter in parameters:
                nro_parameters += 1
        else:
            gname = group.get("name", "noname")
            print("'parameters' not found for '" + block_name + "' group '" + gname + "'")
            exit()

    struct_name = device_name + '_' + block_name
    hfile.write('\ntypedef struct ' + struct_name + '\n{\n')
    # hfile.write('  iocpblkparameterHdr hdr;\n')

    for group in groups:
        process_group_block(group)

    hfile.write('}\n' + struct_name + ';\n')

    # define_name = device_name + '_' + block_name + "_pblk_SZ"
    # define_list.append("#define " + define_name.upper() + " " + str(max_addr) + "\n")

    cfile.write('  }')
    cfile.write(',\n')

    # if check_if_duplicates(reserved_addrs):
    #    write_my_error('ERROR: Duplicated parameter addressess found in JSON!')


def process_source_file(path):
    global cfile, hfile, array_list, parameter_list
    global device_name, define_list
    read_file = open(path, "r")
    if read_file:
        data = json.load(read_file)
        if device_name == None:
            device_name = data.get("name", "unnameddevice")
        check_valid_name("Device", device_name, IOC_NAME_SZ, False)

        array_list = []
        parameter_list = []

        pblk = data.get("persistent", None)
        if pblk != None:
            process_parameter_block('persistent', pblk)

        pblk = data.get("volatile", None)
        if pblk != None:
            process_parameter_block('volatile', pblk)

        pblk = data.get("network", None)
        if pblk != None:
            process_parameter_block('network', pblk)

        cfile.write('\n')
        '''
        cfile.write('};\n')

        hfile.write('}\n' + struct_name + ';\n\n')

        for d in define_list:
            hfile.write(d)

        hfile.write('\nextern OS_FLASH_MEM_H ' + struct_name + ' ' + device_name + ';\n')

        list_name = device_name + "_pblk_list"
        cfile.write('\nstatic OS_FLASH_MEM iocpblkparameterHdr * OS_FLASH_MEM ' + list_name + '[] =\n{\n  ')
        isfirst = True
        for p in pblk_list:
            if not isfirst:
                cfile.write(',\n  ')
            isfirst = False
            cfile.write('&' + p + '.hdr')
        cfile.write('\n};\n\n')
        cfile.write('OS_FLASH_MEM iocDeviceHdr ' + device_name + '_hdr = {(iocpblkparameterHdr**)' + list_name + ', sizeof(' + list_name + ')/' + 'sizeof(iocpblkparameterHdr*)};\n')
        hfile.write('extern OS_FLASH_MEM_H iocDeviceHdr ' + device_name + '_' + 'hdr;\n\n')

        if len(array_list) > 0:
            hfile.write('\n/* Array length defines. */\n')
            for p in array_list:
                hfile.write(p + '\n')

        if len(parameter_list) > 0:
            hfile.write('\n/* Defines to check in code with #ifdef to know if parameter is configured in JSON. */\n')
            for p in parameter_list:
                hfile.write(p + '\n')
        '''

    else:
        print ("Opening file " + path + " failed")

def mymain():
    global cfilepath, hfilepath, device_name

    # Get command line arguments
    n = len(sys.argv)
    sourcefiles = []
    outpath = None
    expectpath = True
    device_name = None
    for i in range(1, n):
        if sys.argv[i][0] == "-":
            if sys.argv[i][1] == "o":
                outpath = sys.argv[i+1]
                expectpath = False

            if sys.argv[i][1] == "d":
                device_name = sys.argv[i+1]
                expectpath = False

        else:
            if expectpath:
                sourcefiles.append(sys.argv[i])
            expectpath = True

    if len(sourcefiles) < 1:
        print("No source files")
#        exit()

    sourcefiles.append('/coderoot/iocom/examples/candy/config/parameters/parameters.json')
    outpath = '/coderoot/iocom/examples/candy/config/include/espcam/parameters.c'

    if outpath is None:
        outpath = sourcefiles[0]

    filename, file_extension = os.path.splitext(outpath)
    cfilepath = filename + '.c'
    hfilepath = filename + '.h'

    print("Writing files " + cfilepath + " and " + hfilepath)

    start_c_files()

    for path in sourcefiles:
        process_source_file(path)

    '''
    struct_name = device_name + '_t'
        define_list = []
        hfile.write('typedef struct ' + struct_name + '\n{')
        cfile.write('OS_FLASH_MEM struct ' + struct_name + ' ' + device_name + ' = \n{')
    '''

    hfile.write('\n#ifndef IOBOARD_DEVICE_NAME\n')
    hfile.write('#define IOBOARD_DEVICE_NAME \"' + device_name + '\"\n#endif\n')

    finish_c_files()

mymain()