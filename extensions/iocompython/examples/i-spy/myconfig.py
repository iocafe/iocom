from kivy.config import ConfigParser
import json

from iocompython import Root,bin2json
# from iocompython import Root, EndPoint, Signal, Stream, json2bin


from mysignal import MySignalDisplay 


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

class MyConfig(MySignalDisplay):
    def set_device(self, ioc_root, device_path):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.my_signal_panel = None

        # Load static default network conguration and user network conguration.
        self.my_default_config = ioc_root.getconf(device_path, 3)
        if self.my_default_config == None:
            print("Loading default network configuration from " + device_path + " failed")
            return;
        self.my_config = ioc_root.getconf(device_path, 2)

        if self.my_config == None:
            print("Loading network configuration from " + device_path + " failed")
            self.my_config = self.my_default_config

        json_text = bin2json(self.my_config)
        if json_text == None:
            print("Unable to parse received binary json from " + device_path)
            return;

        print(json_text)
        self.process_json(json_text)


    ''' def get_network_conf(self, device_path):

        exp_mblk_path = 'conf_exp.' + device_path
        imp_mblk_path = 'conf_imp.' + device_path

        stream = Stream(root, frd = "frd_buf", tod = "tod_buf", exp = exp_mblk_path, imp = imp_mblk_path, select = 2)
        stream.start_read()

        while True:
            s = stream.run()
            if s != None:
                break
            time.sleep(0.01) 

        if s == 'completed':
            data = stream.get_data();
            print(data)

        else:
            print(s)

        stream.delete() 
    '''


    def delete(self):
        pass

    def run(self):
        pass

    def process_json(self, json_text):
        self.signals = {}

        data = json.loads(json_text)

        mblks = data.get("mblk", None)
        if mblks == None:
            print("'mblk' not found")
            return
            
        for mblk in mblks:
            self.process_mblk(mblk)

    def process_mblk(self, data):
        mblk_name = data.get("name", "no_name")
        section_name = mblk_name.replace("_", "-")
        # title = data.get("title", "no_title")
        mblk_flags = data.get("flags", "none")
        self.signal_addr = 0
        groups = data.get("groups", None)
        if groups == None:
            return;

        for group in groups:
            self.process_group(group, mblk_name, mblk_flags, section_name)
        
    def process_group(self, data, mblk_name, mblk_flags, section_name):
        group_name = data.get("name", "no_name")
        if group_name == 'inputs' or group_name == 'outputs':
            self.signal_type = 'boolean'
        else:
            self.signal_type = 'ushort'

        # title = data.get("title", "no_title")
        signals = data.get("signals", None)
        if signals == None:
            return;

        self.my_signal_panel.new_signal_group(group_name, mblk_name)

        for signal in signals:
            self.process_signal(signal, group_name, mblk_name, mblk_flags)

    def process_signal(self, data, group_name, mblk_name, mblk_flags):
        signal_name = data.get("name", "no_name")
        signal_type = data.get("type", None)
        if signal_type != None:
            self.signal_type = signal_type
        signal_addr = data.get("addr", None)
        if signal_addr != None:
            self.signal_addr = signal_addr
        n = data.get("array", 1)

        self.my_signal_panel.new_signal(self.ioc_root, signal_name, self.signal_addr, 
            self.signal_type, n, mblk_name, mblk_flags, self.device_path);
                
        if self.signal_type == "boolean":
            if n <= 1:
                self.signal_addr += 1
            else:
                self.signal_addr += 1 + (n + 7) // 8
        else:
            type_sz = osal_typeinfo[self.signal_type]
            self.signal_addr += 1 + type_sz * n
