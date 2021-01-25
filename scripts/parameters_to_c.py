# parameters_to_c.py 11.6.2020/pekka
# Convert parameter JSON file to C code.
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


def start_c_files():
    global cfile, hfile, cfilepath, hfilepath
    cfile = open(cfilepath, "w")
    hfile = open(hfilepath, "w")
    cfile.write('/* This file is generated by parameters_to_c.py script, do not modify. */\n')
    hfile.write('/* This file is generated by parameters_to_c.py script, do not modify. */\n')
    path, fname = os.path.split(hfilepath)
    fname, ext = os.path.splitext(fname)
    macroname = 'IOC_' + fname.upper() + '_INCLUDED'
    hfile.write('#ifndef ' + macroname + '\n')
    hfile.write('#define ' + macroname + '\n')
    hfile.write('OSAL_C_HEADER_BEGINS\n\n')

def finish_c_files():
    global cfile, hfile
    hfile.write('\nOSAL_C_HEADER_ENDS\n')
    hfile.write('#endif\n')
    cfile.close()
    hfile.close()

def calc_signal_memory_sz(type, array_n):
    if type == "boolean":
        if array_n <= 1:
            return 1
        else:
            return ((array_n + 7) >> 3) + 1

    type_sz = osal_typeinfo[type];
    return array_n * type_sz + 1

# Set address and type, if not set
def set_offset_and_type(parameter):
    global current_type, current_offset

    type = parameter.get('type', None)
    if type != None:
        current_type = type
    else:
        parameter['type'] = current_type

    array_n = parameter.get('array', 1)
    if array_n < 1:
        array_n = 1

    mem_sz_bytes = calc_signal_memory_sz(current_type, array_n)
    parameter['my_offset'] = current_offset
    parameter['my_bytes'] = mem_sz_bytes

    current_offset += mem_sz_bytes


def append_init_parameter_to_c(parameter):
    init = parameter.get('init', None);
    if init == None:
        return;

    name = parameter.get("name", None);
    if name == None:
        print("Parameter without name");
        return

    type = parameter["type"]
    array_n = parameter.get('array', 1);

    cfile.write('  ')

    if array_n > 1:
        if type == 'str':
            cfile.write('ioc_set_str(&sigs->.exp.' + name + ', "' + str(init) + '");\n')
        else:
            cfile.write('static OS_CONST os_' + type + ' ioc_idata_' + name + '[] = {')
            init_list = init.split(',')
            is_first = True
            for i in init_list:
                if not is_first:
                    cfile.write(', ')
                is_first = False
                cfile.write(str(i))
            cfile.write('};\n  ')
            cfile.write('ioc_set_array(&sigs->exp.' + name + ', ' + 'ioc_idata_' + name + ');\n')
    else:
        cfile.write('ioc_set(&sigs->exp.' + name + ', ' + str(init) + ');\n')


def append_load_parameter_to_c(parameter):
    name = parameter.get("name", None);
    if name == None:
        return

    dsig ='sigs->exp.' + name
    cfile.write('    ioc_write(' + dsig + '.handle, ' + dsig + '.addr, buf + ' + str(parameter['my_offset']) + ', ' + str(parameter['my_bytes']) + ', 0);\n')

def append_save_parameter_to_c(parameter):
    name = parameter.get("name", None);
    if name == None:
        return

    dsig = 'sigs->exp.' + name
    cfile.write('  ioc_read(' + dsig + '.handle, ' + dsig + '.addr, buf + ' + str(parameter['my_offset']) + ', ' + str(parameter['my_bytes']) + ', 0);\n')

def process_struct(step, data):
    global current_type, current_offset, persistent_struct_sz

    is_first = True
    current_type = "ushort"
    current_offset = 0

    current_type = "ushort"
    title = data.get("title", "untitled")
    groups = data.get("groups", None)
    if groups == None:
        print('"' + title + '" is empty?')
        return

    for group in groups:
        parameters = group.get("parameters", None)
        if parameters == None:
            print("Group without parameters?")
        elif step == 'setoffsets':
            for parameter in parameters:
                set_offset_and_type(parameter)
        elif step == 'init':
            for parameter in parameters:
                append_init_parameter_to_c(parameter)
        elif step == 'load':
            for parameter in parameters:
                append_load_parameter_to_c(parameter)
        elif step == 'save':
            for parameter in parameters:
                append_save_parameter_to_c(parameter)

    if step == 'setoffsets':
        persistent_struct_sz = current_offset

def process_source_data(step, sourcedata):
    for data in sourcedata:
        persistent = data.get("persistent", None)
        if persistent != None:
            process_struct(step, persistent)

        if step == 'init':
            volatile = data.get("volatile", None)
            if volatile != None:
                process_struct(step, volatile)

def load_source_file(path):
    global device_name
    read_file = open(path, "r")
    if read_file:
        data = json.load(read_file)

        if device_name == None:
            device_name = data.get("name", "NONAME")
    return data;

def mymain():
    global cfilepath, hfilepath, device_name
    n = len(sys.argv)
    sourcefiles = []
    outpath = None
    expect = None
    device_name = None
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
        exit()
        # sourcefiles.append('/coderoot/iocom/examples/candy/config/parameters/parameters.json')

    if outpath is None:
        outpath = sourcefiles[0]

    filename, file_extension = os.path.splitext(outpath)
    cfilepath = filename + '.c'
    hfilepath = filename + '.h'

    # Make sure that "intermediate" directory exists.
    dir_path, file_name =  os.path.split(outpath)
    try:
        os.makedirs(dir_path)
    except FileExistsError:
        pass

    sourcedata = []
    for path in sourcefiles:
        data = load_source_file(path)
        sourcedata.append(data)

    print("Writing files " + cfilepath + " and " + hfilepath)

    start_c_files()

    # Set offset, size in bytes and type for every parameter
    # and structure sizes for volatile and persistent parameters.
    process_source_data('setoffsets', sourcedata)
    
    # Write initialization function, set offset and size for 
    cfile.write('void ioc_initialize_' + device_name + '_parameters(const struct ' + device_name + '_t *sigs, os_int block_nr, void *reserved)\n{\n')
    cfile.write('  os_memclear(&ioc_prm_storage, sizeof(ioc_prm_storage));\n')
    cfile.write('  ioc_prm_storage.block_nr = block_nr;\n')
    process_source_data('init', sourcedata)
    cfile.write('}\n\n')

    # Write persistent load function
    cfile.write('osalStatus ioc_load_' + device_name + '_parameters(const struct ' + device_name + '_t *sigs)\n{\n')
    cfile.write('#if OSAL_PERSISTENT_SUPPORT\n')
    cfile.write('  os_char buf[' + str(persistent_struct_sz) + '];\n')
    cfile.write('  osalStatus s;\n\n')
    cfile.write('  ioc_prm_storage.changed = OS_FALSE;\n')
    cfile.write('  s = os_load_persistent(ioc_prm_storage.block_nr, buf, sizeof(buf));\n')
    cfile.write('  if (!OSAL_IS_ERROR(s)) {\n')
    process_source_data('load', sourcedata)
    cfile.write('  }\n')
    cfile.write('  return s;\n')
    cfile.write('#else\n')
    cfile.write('  return OSAL_STATUS_NOT_SUPPORTED;\n')
    cfile.write('#endif\n')
    cfile.write('}\n\n')

    # Write persistent save function
    cfile.write('osalStatus ioc_save_' + device_name + '_parameters(const struct ' + device_name + '_t *sigs)\n{\n')
    cfile.write('#if OSAL_PERSISTENT_SUPPORT\n')
    cfile.write('  os_char buf[' + str(persistent_struct_sz) + '];\n')
    cfile.write('  osalStatus s;\n\n')
    process_source_data('save', sourcedata)
    cfile.write('  s = os_save_persistent(ioc_prm_storage.block_nr, buf, sizeof(buf), OS_FALSE);\n')
    cfile.write('  ioc_prm_storage.changed = OS_FALSE;\n')
    cfile.write('  return s;\n')
    cfile.write('#else\n')
    cfile.write('  return OSAL_STATUS_NOT_SUPPORTED;\n')
    cfile.write('#endif\n')
    cfile.write('}\n\n')

    # Write autosave function
    cfile.write('osalStatus ioc_autosave_' + device_name + '_parameters(const struct ' + device_name + '_t *sigs)\n{\n')
    cfile.write('#if OSAL_PERSISTENT_SUPPORT\n')
    cfile.write('  if (ioc_prm_storage.changed) {\n')
    cfile.write('    if (os_has_elapsed(&ioc_prm_storage.ti, 3000)) {\n')
    cfile.write('       ioc_save_' + device_name + '_parameters(sigs);\n')
    cfile.write('    }\n')
    cfile.write('  }\n')
    cfile.write('  return OSAL_SUCCESS;\n')
    cfile.write('#else\n')
    cfile.write('  return OSAL_STATUS_NOT_SUPPORTED;\n')
    cfile.write('#endif\n')
    cfile.write('}\n\n')

    # Write heade file
    hfile.write('struct ' + device_name + '_t;\n')
    hfile.write('void ioc_initialize_' + device_name + '_parameters(const struct ' + device_name + '_t *sigs, os_int block_nr, void *reserved);\n')
    hfile.write('osalStatus ioc_load_' + device_name + '_parameters(const struct ' + device_name + '_t *sigs);\n')
    hfile.write('osalStatus ioc_save_' + device_name + '_parameters(const struct ' + device_name + '_t *sigs);\n')
    hfile.write('osalStatus ioc_autosave_' + device_name + '_parameters(const struct ' + device_name + '_t *sigs);\n')

    finish_c_files()

mymain()
