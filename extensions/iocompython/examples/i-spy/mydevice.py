from kivy.config import ConfigParser
from iocompython import Root, MemoryBlock, bin2json
import json

from mysettings import MySettingsDisplay 


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

class MyDevice(ConfigParser):
    def set_device(self, ioc_root, settings, device_path):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.ioc_settings = settings;
        self.my_signal_panel = None

    def create_signal_display(self):
        p = MySettingsDisplay()
        self.my_signal_panel = p;

        info = MemoryBlock(self.ioc_root, mblk_name='info.' + self.device_path)
        json_bin = info.read();        
        json_text = bin2json(json_bin)
        self.process_json(json_text)

        info.delete()
        return p;

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

        self.my_signal_panel.new_settings_group("signals - ", self.device_path, 1)
            
        for mblk in mblks:
            self.process_mblk(mblk)

    def process_mblk(self, data):
        mblk_name = data.get("name", "no_name")
        section_name = mblk_name.replace("_", "-")
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

        signals = data.get("signals", None)
        if signals == None:
            return;

        self.my_signal_panel.new_settings_group(group_name, mblk_name, 2)

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
