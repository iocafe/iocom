from kivy.config import ConfigParser
from iocompython import Root, MemoryBlock, Connection, EndPoint, Signal, json2bin, bin2json
import json


# This JSON defines entries we want to appear in our App configuration screen
json2 = '''
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


    

class DeviceSpy(ConfigParser):
    #def __init__(self, *args, **kwargs):
    #    self.image_nr = -1

    def setup_my_panel(self, app, settings, dev_path):
        self.app = app
        info = MemoryBlock(app.ioc_root, mblk_name='info.' + dev_path)
        json_bin = info.read();        
        json_text = bin2json(json_bin)
        self.process_json(json_text)

        for group_name in self.sign_values:
            self.setdefaults(group_name, self.sign_values[group_name])

        json_str = json.dumps(self.sign_display)
        settings.add_json_panel(dev_path, self, data=json_str)

    def process_json(self, json_text):
        self.sign_display = []
        self.sign_values = {}

        data = json.loads(json_text)

        mblks = data.get("mblk", None)
        if mblks == None:
            print("'mblk' not found")
            return
            
        for mblk in mblks:
            self.process_mblk(mblk)


    def process_mblk(self, data):
        name = data.get("name", "no_name")
        title = data.get("title", "no_title")
        groups = data.get("groups", None)
        if groups == None:
            return;

        for group in groups:
            self.process_group(group)
        
    def process_group(self, data):
        group_name = data.get("name", "no_name")
        title = data.get("title", "no_title")
        signals = data.get("signals", None)
        if signals == None:
            return;

        for signal in signals:
            self.process_signal(signal, group_name)

    def process_signal(self, data, group_name):
        name = data.get("name", "no_name")
        item = {"type": "string", "title": "server key", "desc": "Server's private key (seacret)", "section": "inputs", "key": "conf_serv_key"}

        if len(self.sign_display) == 0:
            self.sign_display.append(item);

        if len(self.sign_values) == 0:
            self.sign_values[group_name] = {'conf_serv_cert': 'alice.crt', 'conf_serv_key': 'alice.key'}

