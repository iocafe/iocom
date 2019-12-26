from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.clock import Clock 

from kivy.uix.scrollview import ScrollView
from kivy.core.window import Window

from myactionbar import MyActionBar 
from myconnectdialog import MyConnectDialog
from mydevice import MyDevice

from iocompython import Root, MemoryBlock, Connection, EndPoint, Signal, json2bin

class MainApp(App):
    def __init__(self, **kwargs):
        super(MainApp, self).__init__(**kwargs)
        self.ioc_root = None
        self.my_view = None
        self.ioc_devices = {}

    def build(self):
        self.title = 'i-spy'
        self.root = BoxLayout(orientation='vertical')
        self.my_widget_home = self.root
        
        action_bar = MyActionBar()
        action_bar.bind (on_button_press=self.button_press)
        self.my_widget_home.add_widget(action_bar)

        connect_dlg = MyConnectDialog()
        connect_dlg.bind(on_connect=self.connect)
        self.set_displayed_page(connect_dlg, False)

        return self.root

    def set_displayed_page(self, w, make_scroll_view):
        if self.my_view != None:
            self.my_widget_home.remove_widget(self.my_view)
            self.my_view.delete()
            self.my_view = None
    
        if make_scroll_view:
            scroll_view = ScrollView()
            self.my_widget_home = scroll_view
            self.root.add_widget(scroll_view)

        else:
            self.my_widget_home = self.root

        self.my_view = w;           
        self.my_widget_home.add_widget(w)

    def connect(self, source_object, *args):
        if self.ioc_root != None:
            self.disconnect()

        self.ioc_password = args[0]
        self.ioc_params = args[1]
        transport_flag = self.ioc_params['transport'].lower();

        if self.ioc_params['role'] == "CLIENT":
            self.ioc_root = Root('ispy', device_nr=10000, network_name='iocafenet', security='certchainfile=' + self.ioc_params['cert_chain'])
            self.ioc_root.queue_events()
            self.ioc_connection = Connection(self.ioc_root, self.ioc_params['ip'], transport_flag + ',down,dynamic')

        else:
            self.ioc_root = Root('ispy', device_nr=10000, network_name='iocafenet', security='certfile=' + self.ioc_params['serv_cert'] + ',keyfile=' + self.ioc_params['serv_key'])
            self.ioc_root.queue_events()
            self.ioc_epoint = EndPoint(self.ioc_root, flags= transport_flag + ',dynamic')

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
                    d = MyDevice()
                    self.ioc_devices[dev_path] = d
                    d.setup_device(self.ioc_root, self.ioc_params, dev_path)
                    w = d.create_signal_display()
                    self.set_displayed_page(w, True)
            
            # Device disconnected
            if event == 'device_disconnected':
                a = self.ioc_devices.get(dev_path, None)
                if a != None:
                    del self.ioc_devices[dev_path]
            

    def mytimer_tick(self, interval): 
        self.timer_ms  += .1
        self.check_iocom_events()
        for d in self.ioc_devices:
            self.ioc_devices[d].run()
        if self.my_view:
            self.my_view.run()            
 
    def start_mytimer(self): 
        self.timer_ms = 0;
        Clock.schedule_interval(self.mytimer_tick, .1) 
  
    def stop_mytimer(self): 
        Clock.unschedule(self.mytimer_tick) 

    def button_press(self, source_object, *args):
        button = args[0]
        if button=='close':
            self.disconnect()
            self.stop()        

if __name__ == '__main__':
    MainApp().run()