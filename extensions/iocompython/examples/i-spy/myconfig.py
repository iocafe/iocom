import json
from kivy.config import ConfigParser
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button

from iocompython import Root,bin2json, json2bin
from mysettings import MySettingsDisplay, MySettingsGroup, MyButton
from time import sleep

class MyConfig(MySettingsDisplay):
    def set_device(self, ioc_root, device_path):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.my_signal_panel = None

        # Load static default network conguration and user network conguration. 
        # Persitent block number 2 is network configuration and block number 3
        # is static default network configuration.
        my_default_data = ioc_root.getconf(device_path, select=3)
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

        sleep(0.1)

        if my_default_config.get("accounts", None) != None:
            self.my_select = 4

        else:            
            self.my_select = 2

        my_config = None
        my_data = ioc_root.getconf(device_path, select=self.my_select)
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

        if self.my_select == 2:
            self.process_json(my_default_config, my_config)
        else:
            self.my_default_config = my_default_config
            self.process_accounts_json(my_default_config, my_config)

        self.my_merged_config = my_default_config

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

        title = MySettingsGroup()
        title.set_group_label("configure", self.device_path, 1)
        self.my_add_widget(title)
        b = MyButton()
        b.setup_button("save to device", self)
        self.my_add_widget(b)

        self.new_settings_group("general", None, 2)
        self.process_network(net_d, net)

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
    def my_button_pressed(self, i):
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

        rval = self.ioc_root.setconf(self.device_path, my_data, select=self.my_select)
        print(rval)

    def process_accounts_json(self, my_default_config, my_config):
        self.cols = 1
        accounts_d = my_default_config.get("accounts", None)
        if accounts_d == None:
            print("'accounts' not found in default configuration")
            return

        accounts = None
        if my_config != None:
            accounts = my_config.get("accounts", None)

        title = MySettingsGroup()
        title.set_group_label("user", self.device_path, 1)
        g = GridLayout()
        g.cols = 2
        g.size_hint_y = None
        g.height = 60 
        g.add_widget(title)
        b = MyButton()
        b.setup_button("save to device", self)
        g.add_widget(b)
        self.my_add_widget(g)

        grouplist = {"valid": ["valid accounts", "edit,delete,blacklist"], "requests":["new device requests", "accept,dismiss,blacklist"], "alarms":["alarms", "dismiss"], "whitelist":["white list", "edit,delete,blacklist"], "blacklist":["black list","edit,delete"]}
        for g in grouplist:
            group_d = accounts_d.get(g, None)
            group = None
            if accounts != None:
                group = accounts.get(g, None)
            k = grouplist[g]
            self.process_accounts_group(accounts_d, k[0], group_d, group, k[1])
        
    def process_accounts_group(self, groupdict, label, group_d, group, flags):
        if group_d == None:
            return
            
        self.new_settings_group(label, None, 2)

        # Merge loaded data into default configuration
        if group != None:
            for item in group:
                group_d.append(item)

        for item_d in group_d:
            u = self.new_user(self.ioc_root, groupdict, group_d, item_d, flags)
            u.bind(on_remake_page = self.remake_accounts_page)

    def remake_accounts_page(self, source_object, *args):
        self.clear_widgets()
        self.process_accounts_json(self.my_default_config, None)
