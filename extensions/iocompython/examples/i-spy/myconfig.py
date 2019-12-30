from kivy.config import ConfigParser
import json

from iocompython import Root,bin2json
# from iocompython import Root, EndPoint, Signal, Stream, json2bin


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

class MyConfig(MySettingsDisplay):
    def set_device(self, ioc_root, device_path):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.my_signal_panel = None

        # Load static default network conguration and user network conguration. 
        # Persitent block number 2 is network configuration and block number 3
        # is static default network configuration.
        my_default_data = ioc_root.getconf(device_path, 3)
        if my_default_data == None:
            print("Loading default network configuration from " + device_path + " failed")
            return;

        json_default_text = bin2json(my_default_data)
        if json_default_text == None:
            print("Unable to parse binary default network configuration json from " + device_path)
            return;

        del my_default_data

        my_default_config = json.loads(json_default_text)
        if my_default_config == None:
            print("Unable to parse default network configuration text json from " + device_path)
            return;

        del json_default_text

        my_config = None
        my_data = ioc_root.getconf(device_path, 2)
        if my_data == None:
            print("Loading network configuration from " + device_path + " failed")

        else:
            json_text = bin2json(my_data)
            if json_text == None:
                print("Unable to parse binary network configuration json from " + device_path)

            else:
                del my_data
                my_config = json.loads(json_text)
                if my_config == None:
                    print("Unable to parse network configuration text json from " + device_path)

                del json_text

        self.process_json(my_default_config, my_config)


    def delete(self):
        pass

    def run(self):
        pass

    def process_json(self, my_default_config, my_config):
        net_d = my_default_config.get("network", None)
        if net_d == None:
            print("'network' not found in default configuration")
            return

        net = None
        if my_config != None:
            net = my_config.get("network", None)
            if net == None:
                print("'network' not found in configuration")
            
        self.process_network(net_d, net)

    def process_network(self, net_d, net):
        # connect = data.get("connect", None)
        # nic = data.get("nic", None)
        # security = data.get("security", None)

        self.new_settings_group(self.device_path, "general")

        for item_d in net_d:
            if item_d != 'connect' and item_d != 'nic' and item_d != 'security':
                value_d = net_d[item_d]
                value = None
                if net != None:
                    value = net.get(item_d, None)

                description = ''
                description += "default: " + value_d

                self.new_setting(self.ioc_root, item_d, net_d, value_d, value, description)
        
