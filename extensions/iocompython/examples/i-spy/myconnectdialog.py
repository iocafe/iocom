from kivy.config import Config
Config.set('input', 'mouse', 'mouse,multitouch_on_demand')

from kivy.app import App

from kivy.uix.settings import SettingsWithNoMenu
# from kivy.lang import Builder

from kivy.uix.settings import SettingsPanel
# from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.config import ConfigParser
from kivy.uix.settings import SettingItem

from kivy.core.window import Window
from kivy.uix.popup import Popup
from kivy.uix.boxlayout import BoxLayout
from kivy.metrics import dp
from kivy.uix.textinput import TextInput
from kivy.uix.widget import Widget            

# from functools import partial
# from kivy.clock import Clock 

# from iocompython import Root, MemoryBlock, Connection, EndPoint, Signal, json2bin


# This JSON defines entries we want to appear in our App configuration screen
json = '''
[
    {
        "type": "options",
        "title": "role",
        "desc": "Listen for incoming connections or connect actively?",
        "section": "My Label",
        "key": "conf_role",
        "options": ["SERVER", "CLIENT"]
    },
    {
        "type": "options",
        "title": "transport",
        "desc": "What kind of data transport: Secure TLS, plain socket or serial communication?",
        "section": "My Label",
        "key": "conf_transport",
        "options": ["TLS", "SOCKET", "SERIAL"]
    },
    {
        "type": "string",
        "title": "IP address",
        "desc": "IP address of the IO device to connect to.",
        "section": "My Label",
        "key": "conf_ip"
    },
    {
        "type": "string",
        "title": "serial port",
        "desc": "Serial port used to connect to the IO device.",
        "section": "My Label",
        "key": "conf_serport"
    },
    {
        "type": "string",
        "title": "user name",
        "desc": "Authenticate to the device as (vieview is client)",
        "section": "client",
        "key": "conf_user"
    },
    {
        "type": "string",
        "title": "certificate chain",
        "desc": "Chain of trust, PEM certificate bundle file (client)",
        "section": "client",
        "key": "conf_cert_chain"
    },
    {
        "type": "string",
        "title": "server certificate",
        "desc": "Certificate to present to clients (public)",
        "section": "server",
        "key": "conf_serv_cert"
    },
    {
        "type": "string",
        "title": "server key",
        "desc": "Server's private key (seacret)",
        "section": "server",
        "key": "conf_serv_key"
    },
    {
        "type": "buttons",
        "title": "",
        "desc": "",
        "section": "My Label",
        "key": "conf_button",
        "buttons": [{"title":"connect","id":"button_connect"}]
    }
]
'''

class SettingButtons(SettingItem):

    def __init__(self, **kwargs):
        self.register_event_type('on_release')

        kw = kwargs.copy()
        kw.pop('buttons', None)
        super(SettingItem, self).__init__(**kw)
        
        for aButton in kwargs["buttons"]:
            oButton=Button(text=aButton['title'], font_size= '15sp')
            oButton.ID=aButton['id']
            self.add_widget(oButton)
            oButton.bind (on_release=self.On_ButtonPressed)

    def set_value(self, section, key, value):
        # set_value normally reads the configparser values and runs on an error
        # to do nothing here
        return

    def On_ButtonPressed(self,instance):
        self.panel.settings.dispatch('on_config_change',self.panel.config, self.section, self.key, instance.ID)

class MyConnectDialog(SettingsWithNoMenu):
    def __init__(self, **kwargs):
        super(MyConnectDialog, self).__init__(**kwargs)
        
        self.register_type('buttons', SettingButtons)
        config = ConfigParser()
        config.setdefaults('My Label', {'conf_role': 'SERVER', 'conf_transport': 'TLS', 'conf_ip': '192.168.1.220', 'conf_serport': 'COM1'})
        config.setdefaults('client', {'conf_user': 'administrator', 'conf_cert_chain': 'bob-bundle.crt'})
        config.setdefaults('server', {'conf_serv_cert': 'alice.crt', 'conf_serv_key': 'alice.key'})
        self.myconfig = config;
        self.add_json_panel('IO device connect', config, data=json)

    def get_settings(self):
        self.ioc_role = self.myconfig.get('My Label', 'conf_role')
        self.ioc_transport = self.myconfig.get('My Label', 'conf_transport')
        self.ioc_ip = self.myconfig.get('My Label', 'conf_ip')
        self.ioc_serialport = self.myconfig.get('My Label', 'conf_serport')
        self.ioc_user = self.myconfig.get('client', 'conf_user')
        self.ioc_cert_chain = self.myconfig.get('client', 'conf_cert_chain')
        self.ioc_server_cert = self.myconfig.get('server', 'conf_serv_cert')
        self.ioc_server_key = self.myconfig.get('server', 'conf_serv_key')

    def password_popup(self):
        self.get_settings()
        if self.ioc_role == "SERVER":
            # self.app.connect()
            return

        # create popup layout
        content = BoxLayout(orientation='vertical', spacing='5dp')
        popup_width = min(0.95 * Window.width, dp(500))
        self.popup = popup = Popup(
            title="password", content=content, size_hint=(None, None),
            size=(popup_width, '250dp'))

        # create the textinput used for numeric input
        self.textinput = textinput = TextInput(
            text="", font_size='24sp', multiline=False,
            size_hint_y=None, height='42sp')
        self.textinput = textinput

        # construct the content, widget are used as a spacer
        content.add_widget(Widget())
        content.add_widget(textinput)
        content.add_widget(Widget())

        # 2 buttons are created for accept or cancel the current value
        btnlayout = BoxLayout(size_hint_y=None, height='50dp', spacing='5dp')
        btn = Button(text='connect')
        btn.bind(on_release=self.on_connect)
        btnlayout.add_widget(btn)
        btn = Button(text='cancel')
        btn.bind(on_release=popup.dismiss)
        btnlayout.add_widget(btn)
        content.add_widget(btnlayout)

        # all done, open the popup !
        popup.open()

    def on_connect(self,instance):
        password = self.textinput.text
        # self.app.connect()
        self.popup.dismiss()

    def on_config_change(self, config, section, key, value):
        print(
            "main.py: MySettingsWithNoMenu.on_config_change: "
            "{0}, {1}, {2}, {3}".format(config, section, key, value))

        if section == "My Label":
            if key == 'conf_button':
                if value == 'button_connect':
                    print("BUTTON PRESSED")
                    self.password_popup()

class MainApp(App):
    def build(self):
        self.root = MyConnectDialog()
        # self.root = MyConnectDialog(pos_hint={'top': 1})
        return self.root

if __name__ == '__main__':
    MainApp().run()