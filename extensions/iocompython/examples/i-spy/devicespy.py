from kivy.config import ConfigParser
from iocompython import Root, MemoryBlock, Connection, EndPoint, Signal, json2bin, bin2json


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
        data = info.read()
        json = bin2json(data)
        print (json)
        
        # config = ConfigParser()
        self.setdefaults('My Label', {'conf_role': 'CLIENT', 'conf_transport': 'TLS', 'conf_ip': '192.168.1.220', 'conf_serport': 'COM1', 'conf_authentication': 'CLIENT'})
        self.setdefaults('client', {'conf_user': 'administrator', 'conf_cert_chain': 'bob-bundle.crt'})
        self.setdefaults('server', {'conf_serv_cert': 'alice.crt', 'conf_serv_key': 'alice.key'})
        # config.adddefaultsection('client')
        # config.setdefaults('client', {'conf_cert_chain': 'bob-bundle.crt'})
        settings.add_json_panel(dev_path, self, data=json2)


