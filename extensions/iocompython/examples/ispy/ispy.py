from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.clock import Clock

from kivy.uix.scrollview import ScrollView
from kivy.core.window import Window

from myactionbar import MyActionBar
from myconnectdialog import MyConnectDialog
from panel_wait import WaitPanel
from panel_memory_blocks import MemoryBlockPanel
from panel_configuration import ConfigurationPanel
from panel_program import ProgramPanel
from iodevice import IoDevice

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

        ioc_user = args[0]
        ioc_password = args[1]
        self.ioc_params = args[2]
        transport_flag = self.ioc_params['transport'].lower()

        if transport_flag == 'serial':
            parameters = self.ioc_params['serport']
        else:
            parameters = self.ioc_params['ip']

        if self.ioc_params['role'] == "CLIENT":
            if transport_flag == 'tls':
                self.ioc_root = Root('ispy', device_nr=9000, network_name='cafenet', security='certchainfile=' + self.ioc_params['cert_chain'])
                self.ioc_root.queue_events()
                self.ioc_connection = Connection(self.ioc_root, parameters=parameters, flags=transport_flag + ',down,dynamic,bidirectional', user=ioc_user, password=ioc_password)
            else:
                self.ioc_root = Root('ispy', device_nr=9000, network_name='cafenet')
                self.ioc_root.queue_events()
                self.ioc_connection = Connection(self.ioc_root, parameters=parameters, flags=transport_flag + ',down,dynamic,bidirectional', user=ioc_user)

        else:
            if transport_flag == 'tls':
                self.ioc_root = Root('ispy', device_nr=9000, network_name='cafenet', security='certfile=' + self.ioc_params['serv_cert'] + ',keyfile=' + self.ioc_params['serv_key'])
                self.ioc_root.queue_events()
                self.ioc_epoint = EndPoint(self.ioc_root, flags=transport_flag + ',dynamic')
            else:
                self.ioc_root = Root('ispy', device_nr=9000, network_name='cafenet')
                self.ioc_root.queue_events()

                if transport_flag == 'serial':
                    self.ioc_epoint = EndPoint(self.ioc_root, parameters=parameters, flags=transport_flag + ',dynamic')
                else:
                    self.ioc_epoint = EndPoint(self.ioc_root, flags=transport_flag + ',dynamic')

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
            device_path = device_name + '.' + network_name

            # Device connected
            if event == 'new_device':
                a = self.ioc_devices.get(device_path, None)
                if a == None:
                    print("new device " + device_path)
                    d = IoDevice()
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
        self.ioc_root.receive("*")
        self.check_iocom_events()
        for d in self.ioc_devices:
            self.ioc_devices[d].run()
        if self.my_view:
            self.my_view.run()
        self.ioc_root.send("*")

    def start_mytimer(self):
        Clock.schedule_interval(self.mytimer_tick, 1.0 / 30)

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
            return

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
            self.my_view = connect_dlg
            self.root.add_widget(connect_dlg)
            return

        my_scroll_view = ScrollView()
        self.my_widget_home.add_widget(my_scroll_view)
        self.my_scroll_panel = my_scroll_view

        if page_name == 'wait':
            dlg = WaitPanel()
            self.my_view = dlg
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
            dlg = ConfigurationPanel()
            dlg.set_device(self.ioc_root, device_path)

        elif page_name == 'program':
            dlg = ProgramPanel()
            dlg.set_device(self.ioc_root, device_path)

        elif page_name == 'memory':
            dlg = MemoryBlockPanel()
            dlg.add_mblk_to_page(self.ioc_root, device_path)

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
