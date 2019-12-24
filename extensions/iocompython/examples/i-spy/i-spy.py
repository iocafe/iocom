from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.clock import Clock 

from myactionbar import MyActionBar 
from myconnectdialog import MyConnectDialog

from iocompython import Root, MemoryBlock, Connection, EndPoint, Signal, json2bin

class MainApp(App):
    def __init__(self, **kwargs):
        super(MainApp, self).__init__(**kwargs)
        self.ioc_root = None
        self.connect_dlg = None

    def build(self):
        self.title = 'i-spy'
        self.root = BoxLayout(orientation='vertical')
        action_bar = MyActionBar()
        self.root.add_widget(action_bar)

        connect_dlg = MyConnectDialog()
        connect_dlg.bind(on_connect=self.connect)
        self.root.add_widget(connect_dlg)
        self.connect_dlg = connect_dlg

        # self.root.remove_widget(connect_dlg)
        return self.root

    def connect(self, source_object, *args):
        if self.ioc_root != None:
            self.disconnect()

        self.ioc_password = args[0]
        self.ioc_params = args[1]
        transport_flag = self.ioc_params['transport'].lower();

        if self.ioc_params['role'] == "CLIENT":
            self.ioc_root = Root('ispy', device_nr=10000, network_name='iocafenet', security='certchainfile=' + self.ioc_params['cert_chain'])
            self.ioc_root.queue_events()
            self.ioc_connection = Connection(self.ioc_root, self.ioc_params['ip'], transport_flag + ',downward,dynamic')

        else:
            self.ioc_root = Root('ispy', device_nr=10000, network_name='iocafenet', security='certfile=' + self.ioc_params['serv_cert'] + ',keyfile=' + self.ioc_params['serv_key'])
            self.ioc_root.queue_events()
            self.ioc_epoint = EndPoint(self.ioc_root, flags= transport_flag + ',dynamic')

        if self.connect_dlg != None:
            self.root.remove_widget(self.connect_dlg)
            self.connect_dlg = None

        self.start_mytimer() 

    def disconnect(self):
        if self.ioc_root != None:
            self.stop_mytimer() 
            self.ioc_root.delete()
            self.ioc_root = None

    def check_iocom_events(self):
        e = self.ioc_root.wait_com_event(0)
        if e != None:
            print(e)

            event = e[0]
            # mblk_name = e[3]
            device_name = e[2]
            network_name = e[1]
            dev_path = device_name + '.' + network_name;

            '''
            # New network, means a new game board
            if event == 'new_network':
                ioc_devices[network_name] = asteroidapp.start(root, network_name)

            # Close the game board
            if event == 'network_disconnected':
                a = ioc_devices.get(network_name, None)
                if a != None:
                    a[1].put('exit ' + network_name)
                    del ioc_devices[network_name]

            # Switch 'imp' and 'exp' memory blocks to manual synchronization
            if event == 'new_mblk':
                mblk_path = mblk_name + '.' + device_name + '.' + network_name
                if mblk_name == 'imp' or mblk_name == 'exp':
                    root.set_mblk_param(mblk_path, "auto", 0)
            '''

            # Device connected
            if event == 'new_device':
                a = self.ioc_devices.get(dev_path, None)
                if a == None:
                    print("new device " + dev_path)
                    # d = DeviceSpy()
                    # self.ioc_devices[dev_path] = d
                    # d.setup_my_panel(self, self.mysettings, dev_path)
            
            # Device disconnected
            if event == 'device_disconnected':
                a = self.ioc_devices.get(dev_path, None)
                if a != None:
                    del self.ioc_devices[dev_path]
            

    def increment_mytimer(self, interval): 
        self.timer_ms  += .1
        self.check_iocom_events()
        # for dev_path in self.ioc_devices:
        #    self.ioc_devices[dev_path].run()
  
    def start_mytimer(self): 
        self.timer_ms = 0;
        Clock.schedule_interval(self.increment_mytimer, .1) 
  
    def stop_mytimer(self): 
        Clock.unschedule(self.increment_mytimer) 


if __name__ == '__main__':
    MainApp().run()