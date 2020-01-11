from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.clock import Clock 

from kivy.uix.scrollview import ScrollView
from kivy.core.window import Window

from myactionbar import MyActionBar 
from myconnectdialog import MyConnectDialog
from mywaitdialog import MyWaitDialog
from mymblkdialog import MyMemoryBlockDialog
from mydevice import MyDevice
from myconfig import MyConfig
from myprogram import MyProgram

from iocompython import Root, MemoryBlock, Connection, EndPoint, Signal, json2bin

class MainApp(App):
    def __init__(self, **kwargs):
        super(MainApp, self).__init__(**kwargs)
        self.ioc_root = None
        self.my_action_bar = None
        self.my_scroll_panel = None
        self.my_view = None
        self.ioc_devices = {}
        self.ioc_selected_device = None
        self.ioc_selected_page = 'signals'

    def connect(self, source_object, *args):
        if self.ioc_root != None:
            self.disconnect()

        self.ioc_password = args[0]
        self.ioc_params = args[1]
        transport_flag = self.ioc_params['transport'].lower();

        if self.ioc_params['role'] == "CLIENT":
            # self.ioc_root = Root('ispy', device_nr=10000, network_name='iocafenet', security='certchainfile=' + self.ioc_params['cert_chain'])
            self.ioc_root = Root('ispy', device_nr=1, network_name='iocafenet', security='certchainfile=' + self.ioc_params['cert_chain'])
            self.ioc_root.queue_events()
            self.ioc_connection = Connection(self.ioc_root, self.ioc_params['ip'], transport_flag + ',down,dynamic,bidirectional')

        else:
            self.ioc_root = Root('ispy', device_nr=10000, network_name='iocafenet', security='certfile=' + self.ioc_params['serv_cert'] + ',keyfile=' + self.ioc_params['serv_key'])
            # self.ioc_root = Root('ispy', device_nr=1, network_name='iocafenet', security='certfile=' + self.ioc_params['serv_cert'] + ',keyfile=' + self.ioc_params['serv_key'])
            self.ioc_root.queue_events()
            self.ioc_epoint = EndPoint(self.ioc_root, flags= transport_flag + ',dynamic')

        self.set_displayed_page(None, 'wait')

        self.start_mytimer() 

    def disconnect(self):
        if self.ioc_root != None:
            self.stop_mytimer()

            for device_path in self.ioc_devices:
                self.my_action_bar.remove_my_device(device_path)
            self.ioc_devices = {}
            self.set_displayed_page(None, None)

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
            device_path = device_name + '.' + network_name;

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
                a = self.ioc_devices.get(device_path, None)
                if a == None:
                    print("new device " + device_path)
                    d = MyDevice()
                    self.ioc_devices[device_path] = d
                    d.set_device(self.ioc_root, self.ioc_params, device_path)

                    self.set_displayed_page(self.ioc_selected_device, None)
                    self.my_action_bar.add_my_device(device_path)
            
            # Device disconnected
            if event == 'device_disconnected':
                a = self.ioc_devices.get(device_path, None)
                if a != None:
                    del self.ioc_devices[device_path]
                    self.my_action_bar.remove_my_device(device_path)
                    self.set_displayed_page(None, None)

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

    # kivy gui build
    def build(self):
        self.title = 'i-spy'
        self.root = BoxLayout(orientation='vertical')
        self.my_widget_home = self.root
        
        action_bar = MyActionBar()
        action_bar.bind (on_button_press=self.button_press)
        self.my_action_bar = action_bar
        self.my_widget_home.add_widget(action_bar)

        self.set_displayed_page(None, 'connect')
        return self.root

    # display page in kivy gui
    def set_displayed_page(self, device_path, page_name):
        if len(self.ioc_devices) == 0:
            if self.ioc_root == None:
                page_name = 'connect'
            else:
                page_name = 'wait'
            self.ioc_selected_device = None

        if page_name == None:
            page_name = self.ioc_selected_page

        if self.ioc_selected_device != None:
            if self.ioc_selected_device not in self.ioc_devices:
                self.ioc_selected_device = None

        if self.ioc_selected_device == device_path and device_path != None and self.ioc_selected_page == page_name:
            return;

        if self.my_scroll_panel != None:
            self.my_widget_home.remove_widget(self.my_scroll_panel)
            self.my_scroll_panel = None
            if self.my_view != None:
                self.my_view.delete()
                self.my_view = None

        if self.my_view != None:
            self.my_widget_home.remove_widget(self.my_view)
            self.my_view.delete()
            self.my_view = None

        if page_name == 'connect':
            connect_dlg = MyConnectDialog()
            connect_dlg.bind(on_connect=self.connect)
            self.my_view = connect_dlg;           
            self.root.add_widget(connect_dlg)
            return

        my_scroll_view = ScrollView()
        self.my_widget_home.add_widget(my_scroll_view)
        self.my_scroll_panel = my_scroll_view

        if page_name == 'wait':
            dlg = MyWaitDialog()
            self.my_view = dlg;           
            my_scroll_view.add_widget(dlg)
            return

        if device_path != None:
            if device_path not in self.ioc_devices:
                device_path = None

        if device_path == None:
            device_path = next(iter(self.ioc_devices))

        d = self.ioc_devices[device_path]
        if page_name == 'signals' or page_name == None:
            page_name = 'signals'
            dlg = d.create_signal_display()

        elif page_name == 'configure':
            dlg = MyConfig()
            dlg.set_device(self.ioc_root, device_path)

        elif page_name == 'program':
            dlg = MyProgram()
            dlg.set_device(self.ioc_root, device_path)

        elif page_name == 'memory':
            dlg = MyMemoryBlockDialog()
            dlg.add_mblk_to_page(self.ioc_root, device_path);

        else:
            dlg = None
            print("Unknown page name " + page_name)

        self.ioc_selected_device = device_path
        self.ioc_selected_page = page_name
        if dlg != None:
            self.my_view = dlg
            my_scroll_view.add_widget(dlg)

    # kivy gui action bar button handling
    def button_press(self, source_object, *args):
        button_text = args[0]
        if button_text == 'close':
            self.disconnect()
            self.stop()        

        if button_text == 'disconnect':
            self.disconnect()
            self.set_displayed_page(None, 'connect')

        elif button_text == 'signals' or button_text == 'memory' or button_text == 'configure' or button_text == 'program':
            self.set_displayed_page(self.ioc_selected_device, button_text)

        elif button_text in self.ioc_devices:
            self.set_displayed_page(button_text, None)

if __name__ == '__main__':
    MainApp().run()
