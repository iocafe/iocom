import json
import re
from kivy.config import ConfigParser
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.boxlayout import BoxLayout
from kivy.core.window import Window
from kivy.uix.widget import Widget
from kivy.uix.popup import Popup
from kivy.uix.label import Label
from kivy.metrics import dp

from time import sleep

from panel import Panel
from item import make_my_text_input
from item_heading import HeadingItem
from item_button import ButtonItem
from iconbutton import IconButton
from iocompython import Root, bin2json, json2bin

class ConfigurationPanel(Panel):
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
            temp = re.findall(r'[0-9]', device_path)
            self.my_select = int(temp[0]) + 10

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
        for s in self.run_list:
            s.update_signal()
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

        title = HeadingItem()
        title.set_group_label("configure", self.device_path, 1)
        self.add_my_widget(title)
        b = ButtonItem()
        b.setup_button("save to device", self)
        self.add_my_widget(b)

        self.add_heading("general", None, 2)
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

                self.add_configuration_item(self.ioc_root, item_d, data_d, value_d, value, description)

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

            self.add_heading(mylabel, None, 2)
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

        title = HeadingItem()
        title.set_group_label("user", self.device_path, 1)
        g = GridLayout()
        g.cols = 2
        g.size_hint_y = None
        g.height = 60 
        g.add_widget(title)
        b = ButtonItem()
        b.setup_button("save to device", self)
        g.add_widget(b)
        self.add_my_widget(g)

        grouplist = {"requests":["new devices", "accept,delete,blacklist"], "alarms":["alarms", "delete"]}
        for g in grouplist:
            k = grouplist[g]
            self.make_notification_group(g, k[0], k[1])

        grouplist = {"accounts": ["user accounts", "edit,delete,blacklist"], "whitelist":["whitelist", "edit,delete,blacklist"], "blacklist":["blacklist","edit,delete"]}
        for g in grouplist:
            group_d = accounts_d.get(g, None)
            group = None
            if accounts != None:
                group = accounts.get(g, None)
            k = grouplist[g]
            self.process_accounts_group(accounts_d, g, k[0], group_d, group, k[1])

        ##################xxx

    def make_notification_group(self, groupname, label, flags):
        self.add_heading(label, None, 2)
        self.add_notification(self.ioc_root, "new1_text", "exp", self.device_path, flags)
        
    def process_accounts_group(self, groupdict, groupname, label, group_d, group, flags):
        if group_d == None:
            return
            
        title = self.add_heading(label, None, 2)
        if label != 'new devices' and label != 'alarms':
            b = IconButton()
            b.set_image("new", groupdict, groupname)
            b.bind(on_release = self.new_account_item)
            title.add_widget(b)

        # Merge loaded data into default configuration
        if group != None:
            groupdict[groupname] = []
            group_d = groupdict[groupname]
            for item in group:
                group_d.append(item)

        for item_d in group_d:
            u = self.add_user_account_item(self.ioc_root, groupdict, groupname, group_d, item_d, flags)
            u.bind(on_remake_page = self.remake_accounts_page)

    def new_account_item(self, source_object):
        titlelist = {"accounts": "new user account", "whitelist":"whitelist user or IP addresses", "blacklist":"blacklist user or IP addresses"}
        self.my_groupdict = source_object.my_groupdict
        groupname = source_object.my_groupname
        self.my_groupname = groupname

        # create grid of text inputs
        grid = GridLayout()
        grid.cols = 2;
        grid.spacing = [6, 6]
        nrows = 1
        self.user_name_input = make_my_text_input("")
        grid.add_widget(Label(text='user name'));
        grid.add_widget(self.user_name_input)
        if groupname == "accounts":
            self.password_input = make_my_text_input("")
            grid.add_widget(Label(text='password'));
            grid.add_widget(self.password_input)

            self.privileges_input = make_my_text_input("")
            grid.add_widget(Label(text='privileges'));
            grid.add_widget(self.privileges_input)
            nrows += 2

        if groupname == "blacklist" or groupname == "whitelist":
            self.ip_input = make_my_text_input("")
            grid.add_widget(Label(text='ip'));
            grid.add_widget(self.ip_input)
            nrows += 1
        
        content = GridLayout()
        content.padding = [6, 6]
        content.cols = 1
        popup_width = min(0.95 * Window.width, dp(500))
        self.popup = popup = Popup(
            title=titlelist[groupname], content=content, size_hint=(None, None),
            size=(popup_width, str(160 + nrows * 70) + 'dp'))

        # construct the content, empty widgets are used as a spacers
        content.add_widget(Widget())
        content.add_widget(grid)
        content.add_widget(Widget())
        content.add_widget(Widget())

        # 2 buttons are created for accept or cancel the current value
        btnlayout = BoxLayout(size_hint_y=None, height='50dp', spacing='5dp')
        btn = Button(text='ok')
        btn.bind(on_release=self.my_new_account_ok_button_pressed)
        btnlayout.add_widget(btn)
        btn = Button(text='cancel')
        btn.bind(on_release=popup.dismiss)
        btnlayout.add_widget(btn)
        content.add_widget(btnlayout)

        # all done, open the popup
        popup.open()
        self.user_name_input.focus = True

    def my_new_account_ok_button_pressed(self, instance):
        groupname = self.my_groupname
        groupdict = self.my_groupdict
        item = {}
        item['user'] = self.user_name_input.text;

        if groupname == "accounts":
            item['password'] = self.password_input.text;
            if self.privileges_input.text != "":
                item['privileges'] = self.privileges_input.text;

        if groupname == "blacklist" or groupname == "whitelist":
            item['ip'] = self.ip_input.text;

        groupdict[groupname].append(item)
        self.popup.dismiss()
        self.clear_widgets()
        self.process_accounts_json(self.my_default_config, None)

    def remake_accounts_page(self, source_object, *args):
        self.clear_widgets()
        self.process_accounts_json(self.my_default_config, None)
