from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.widget import Widget
from kivy.graphics import Color, Rectangle, Line
from iconbutton import IconButton
from iocompython import Signal

class NotificationItem(GridLayout):
    def __init__(self, **kwargs):
        self.my_state_bits = 0
        self.control_buttons = None
        super(NotificationItem, self).__init__(**kwargs)
        self.cols = 3
        self.padding = [8, 6]

        self.size_hint_y = None
        self.height = 60 

        l = Label(text = '', markup = True, halign="left")
        l.bind(size=l.setter('text_size')) 
        self.my_label = l

        d = Label(text = '', markup = True, halign="left")
        d.bind(size=d.setter('text_size')) 
        self.my_description = d

        lb = GridLayout()
        lb.cols = 1
        lb.size_hint = (0.65, 1) 

        lb.add_widget(l)
        lb.add_widget(d)
        self.add_widget(lb)

    def on_size(self, *args):
        self.my_redraw_state_bits(args)

    def on_pos(self, *args):
        self.my_redraw_state_bits(args)

    def my_redraw_state_bits(self, *args):
        self.canvas.before.clear()
        with self.canvas.before:
            if self.control_buttons != None:
                Color(0.8, 0.8, 0.8, 0.25)
                mysz = self.size.copy()
                mysz[1] = 1
                Rectangle(pos=self.pos, size=mysz)            

    def delete(self):
        self.text_signal.delete()
        self.name_signal.delete()
        self.password_signal.delete()
        self.privileges_signal.delete()
        self.ip_signal.delete()
        self.count_signal.delete()
        self.text_signal = None
        self.name_signal = None
        self.password_signal = None
        self.privileges_signal = None
        self.ip_signal = None
        self.count_signal = None

    def setup_notification(self, ioc_root, groupdict, prefix, nr, mblk_name, device_path, flags):
        self.my_groupdict = groupdict
        t = Button(text = '', markup = True, halign="center", valign="center")
        t.size_hint = (0.35, 1)
        t.bind(size=t.setter('text_size')) 
        t.background_color = [0 , 0, 0, 0]
        self.add_widget(t)
        self.my_text = t
        self.my_flags = flags

        self.register_event_type('on_remake_page')

        self.text_signal = Signal(ioc_root, prefix + str(nr) + "_text." + mblk_name + "." + device_path)
        self.name_signal = Signal(ioc_root, prefix + str(nr) + "_name." + mblk_name + "." + device_path)
        self.password_signal = Signal(ioc_root, prefix + str(nr) + "_password." + mblk_name + "." + device_path)
        self.privileges_signal = Signal(ioc_root, prefix + str(nr) + "_privileges." + mblk_name + "." + device_path)
        self.ip_signal = Signal(ioc_root, prefix + str(nr) + "_ip." + mblk_name + "." + device_path)
        self.count_signal = Signal(ioc_root, prefix + str(nr) + "_count." + mblk_name + "." + device_path)

    def show_control_buttons(self):
        if self.control_buttons != None:
            return

        lb = GridLayout()
        lb.size_hint = (0.65, 1) 
        lb.add_widget(Widget())

        # Make buttons
        ncols = 1
        flaglist = self.my_flags.split(',')
        for button_name in flaglist:
            b = IconButton()
            b.set_image(button_name, None, None) # groupdict, groupname)
            b.my_button_action = button_name
            b.bind(on_release = self.my_user_button_pressed)
            lb.add_widget(b)
            ncols += 1

        lb.cols = ncols
        self.add_widget(lb)
        self.control_buttons = lb

    def hide_control_buttons(self):
        if self.control_buttons != None:
            self.remove_widget(self.control_buttons)
            self.control_buttons = None

    def my_user_button_pressed(self, instance):
        action = instance.my_button_action
        p = self.parent
        if action == 'delete':
            pass
            # self.my_group.remove(self.my_item)
            # p.remove_widget(self)

        if action == 'blacklist':
            pass
            item = {}
            item['ip'] = 'kaviaaria'
            item['user'] = 'ja heti'

            self.my_groupdict['blacklist'].append(item);
            self.dispatch('on_remake_page', action)

        if action == 'accept':
            pass
            # self.my_groupdict['accounts'].append(self.my_item);
            # self.dispatch('on_remake_page', action)

    def on_remake_page(self, *args):
        pass
        print("user button press dispatched")

    def update_signal(self):
        try:
            name = self.name_signal.get(check_tbuf=True)
        except:
            name = [0, "?"]

        if (name[0] & 2) == 0:            
            self.my_label.text = ''
            self.my_text.text = ''
            self.my_description.text = ''
            self.hide_control_buttons()
            self.height = 0
            return

        try:
            text = str(self.text_signal.get0())
        except:
            text = '?'

        try:
            password = str(self.password_signal.get0())
        except:
            password = '?'

        try:
            privileges = str(self.privileges_signal.get0())
        except:
            privileges = '?'

        try:
            ip = str(self.ip_signal.get0())
        except:
            ip = '?'

        try:
            count = str(self.count_signal.get0())
        except:
            count = '?'

        self.my_label.text = str(name[1])
        self.my_text.text = text


        description = str(ip)
        if privileges != '':
            if description != "":
                description += " "
            description += '[color=9090FF]' + str(privileges) + '[/color]'
        if password != '':
            if description != "":
                description += " "
            description += '([color=FF9090]' + str(password) + '[/color])'

        self.my_description.text = '[color=909090][size=14]' + description + ' count=' + count + '[/size][/color]'

        self.show_control_buttons()
        self.height = 60 

#        new_state_bits = int(v[0])
#        if new_state_bits != self.my_state_bits:
#            self.my_state_bits = new_state_bits
#            self.my_redraw_state_bits(None)

