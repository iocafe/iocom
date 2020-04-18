from kivy.uix.gridlayout import GridLayout
from kivy.uix.widget import Widget

from item_iosignal import IoSignalItem
from item_notification import NotificationItem
from item_configuration import ConfigurationItem
from item_user_account import UserAccountItem
from item_button import ButtonItem
from item_heading import HeadingItem
from item_linear_cam import LinearCameraItem

from iocompython import Signal


class Panel(GridLayout):
    def __init__(self, **kwargs):
        super(Panel, self).__init__(**kwargs)
        self.cols = 2
        self.run_list = []
        self.height = self.minimum_height = 400
        self.size_hint_y = None
        self.bind(minimum_height=self.setter('height'))
        self.my_nro_widgets = 0

    def delete(self):
        for s in self.run_list:
            s.delete()
        self.run_list = []

    def run(self):
        for s in self.run_list:
            s.update_signal()

    def add_io_signal(self, ioc_root, signal_name, signal_addr, signal_type, n, mblk_name, mblk_flags, device_path):
        s = IoSignalItem()
        s.setup_signal(ioc_root, signal_name, signal_addr, signal_type, n, mblk_name, mblk_flags, device_path)
        self.add_my_widget(s)
        self.run_list.append(s)

    def add_notification(self, ioc_root, groupdict, prefix, nr, mblk_name, device_path, flags):
        n = NotificationItem()
        n.setup_notification(ioc_root, groupdict, prefix, nr, mblk_name, device_path, flags)
        self.add_my_widget(n)
        self.run_list.append(n)
        return n

    def add_configuration_item(self, ioc_root, setting_name, dict, value_d, value, description):
        s = ConfigurationItem()
        s.setup_setting(ioc_root, setting_name, dict, value_d, value, description)
        self.add_my_widget(s)

    def add_user_account_item(self, ioc_root, groupdict, groupname, group, item, flags):
        u = UserAccountItem()
        u.setup(ioc_root, groupdict, groupname, group, item, flags)
        self.add_my_widget(u)
        return u

    def add_button(self, text, signal_me):
        b = ButtonItem()
        b.setup_button(text, signal_me)
        self.add_my_widget(b)
        return b

    def add_assembly(self, ioc_root, assembly_data, device_path):
        # assembly_type= assembly_data.get("type", "no_type")
        # if assembly_type is linear camera

        cam = LinearCameraItem()
        cam.set_device(ioc_root, device_path, assembly_data)
        self.add_my_widget(cam)
        return cam

    def add_heading(self, label1, label2, level):
        widgets_on_row = self.my_nro_widgets % self.cols
        if widgets_on_row > 0:
            for i in range(widgets_on_row, self.cols):
                self.add_my_widget(Widget())
        g = HeadingItem()
        g.set_group_label(label1, label2, level)
        self.add_my_widget(g)
        for i in range(1, self.cols):
            spacer = HeadingItem()
            spacer.my_level = level;
            self.add_my_widget(spacer)
        return g

    def add_my_widget(self, w):
        self.add_widget(w)
        self.my_nro_widgets += 1
        
