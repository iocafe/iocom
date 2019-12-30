from kivy.config import ConfigParser
import json
from iocompython import Root,bin2json, json2bin
from mysettings import MySettingsDisplay 


class MyProgram(MySettingsDisplay):
    def set_device(self, ioc_root, device_path):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.my_signal_panel = None

        # Load static default network conguration and user network conguration. 
        # Persitent block number 4 is user account configuration
        my_config = None
        my_data = ioc_root.getconf(device_path, select=4)
        if my_data == None:
            print("Loading user account configuration from " + device_path + " failed")

        else:
            json_text = bin2json(my_data)
            if json_text == None:
                print("Unable to parse binary user account json from " + device_path)

            else:
                del my_data
                my_config = json.loads(json_text)
                if my_config == None:
                    print("Unable to parse user account  text json from " + device_path)
                del json_text

        self.process_json(my_config)

    def delete(self):
        pass

    def run(self):
        pass

    def process_json(self, my_config):
        pass
        
        '''
        net_d = my_default_config.get("network", None)
        if net_d == None:
            print("'network' not found in default configuration")
            return

        net = None
        if my_config != None:
            net = my_config.get("network", None)
            if net == None:
                print("'network' not found in configuration")
        '''
        self.new_settings_group("program", self.device_path, 1)
    '''        
        self.new_settings_group("general", None, 2)
        self.process_network(net_d, net)

        self.new_button("save", self)

    def process_network(self, data_d, data):
        for item_d in data_d:
            if item_d != 'connect' and item_d != 'nic' and item_d != 'security' and item_d != 'wifi':
                value_d = data_d[item_d]
                value = None
                if data != None:
                    value = data.get(item_d, None)

                description = ''
                description += "default: " + str(value_d)

                self.new_setting(self.ioc_root, item_d, data_d, value_d, value, description)

        g_d = data_d.get("connect", None)
        if g_d != None:
            g = None
            if data != None:
                g = data.get("connect", None)
            self.process_network_array("connect", g_d, g)

        g_d = data_d.get("nic", None)
        if g_d != None:
            g = None
            if data != None:
                g = data.get("nic", None)
            self.process_network_array("nic", g_d, g)

        g_d = data_d.get("wifi", None)
        if g_d != None:
            g = None
            if data != None:
                g = data.get("wifi", None)
            self.process_network_array("wifi", g_d, g)

        g_d = data_d.get("security", None)
        if g_d != None:
            g = None
            if data != None:
                g = data.get("security", None)
            self.process_network_array("security", g_d, g)

    def process_network_array(self, label, data_d, data):
        n = len(data_d)
        for i in range(n):
            a_d = data_d[i];
            a = None
            if data != None:
                if i < len(data):
                    a = data[i]

            mylabel = label
            if n > 1:
                mylabel += " " + str(i+1)                    

            self.new_settings_group(mylabel, None, 2)
            self.process_network(a_d, a)

    # Save configuration to device  
    def my_settings_button_pressed(self, i):
        json_text = json.dumps(self.my_merged_config)
        if json_text == None:
            print("Unable to generate json")
            return

        print(json_text)

        my_data = json2bin(json_text)
        if my_data == None:
            print("Compressing json failed")
            return;

        print(my_data)

        rval = self.ioc_root.setconf(self.device_path, my_data, select=2)
        print(rval)
    '''
