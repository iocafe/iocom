from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.checkbox import CheckBox
from kivy.graphics import Color, Rectangle, Line
from kivy.uix.widget import Widget
from kivy.uix.boxlayout import BoxLayout
from kivy.core.window import Window
from kivy.uix.popup import Popup
from kivy.uix.textinput import TextInput
from kivy.uix.button import Button
from kivy.metrics import dp

from myiconbutton import MyIconButton
from iocompython import Signal

def none_to_empty_str(x):
    if x == None:
        return ""
    return x;                    

def make_my_text_input(text):
    return TextInput(text=text, font_size='24sp', multiline=False,
        size_hint_y=None, height='42sp')

class MyVariable(GridLayout):
    def __init__(self, **kwargs):
        self.my_state_bits = 0
        self.my_up = True
        self.my_down = True
        super(MyVariable, self).__init__(**kwargs)
        self.cols = 2
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

    def setup_variable(self, ioc_root, label_text, description, use_checkbox):
        self.my_label_text = label_text

        if use_checkbox:
            b = CheckBox()
            b.size_hint = (0.35, 1)
            if self.my_down:
                b.bind(on_release = self.on_checkbox_modified)
            self.add_widget(b)
            self.my_checkbox = b
            self.my_text = None

        else:
            t = Button(text = '', markup = True, halign="center", valign="center")
            t.size_hint = (0.35, 1)
            t.bind(size=t.setter('text_size')) 
            t.background_color = [0 , 0, 0, 0]
            t.background_normal =''
            if self.my_down:
                t.bind(on_release = self.my_create_popup)
            self.add_widget(t)
            self.my_text = t
            self.my_checkbox = None

        self.my_label.text = '[size=16]' + label_text + '[/size]'
        self.my_description.text = '[size=14][color=909090]' + description + '[/color][/size]'

    def on_size(self, *args):
        self.my_redraw_state_bits(args)

    def on_pos(self, *args):
        self.my_redraw_state_bits(args)

    def my_redraw_state_bits(self, *args):
        self.canvas.before.clear()
        with self.canvas.before:
            Color(0.8, 0.8, 0.8, 0.25)
            mysz = self.size.copy()
            mysz[1] = 1
            Rectangle(pos=self.pos, size=mysz)            

            if self.my_state_bits == -1: 
                return;

            if self.my_state_bits & 2 == 0: 
                Color(0.5, 0.5, 0.5, 1)
            elif self.my_state_bits & 12 == 12:
                Color(1.0, 0, 0, 1)
            elif self.my_state_bits & 8 == 8:
                Color(1.0, 1.0, 0, 1)
            elif self.my_state_bits & 4 == 4:
                Color(1.0, 0.65, 0, 1)
            else:
                Color(0, 1, 0, 1)
            Line(circle=(self.pos[0] + 0.9 * self.size[0], self.pos[1] + 12, 6))

    def my_create_popup(self, instance):
        # create popup layout
        content = BoxLayout(orientation='vertical', spacing='5dp')
        popup_width = min(0.95 * Window.width, dp(500))
        self.popup = popup = Popup(
            title=self.my_label_text, content=content, size_hint=(None, None),
            size=(popup_width, '250dp'))

        # create the textinput used for numeric input
        textinput = TextInput(
            text=self.my_text.text, font_size='24sp', multiline=False,
            size_hint_y=None, height='42sp')
        # textinput.bind(on_text_validate=self._validate)
        self.textinput = textinput
        # self.popup.bind(on_open=self.my_open_callback)

        # construct the content, widget are used as a spacer
        content.add_widget(Widget())
        content.add_widget(textinput)
        content.add_widget(Widget())

        # 2 buttons are created for accept or cancel the current value
        btnlayout = BoxLayout(size_hint_y=None, height='50dp', spacing='5dp')
        btn = Button(text='ok')
        btn.bind(on_release=self.my_user_input)
        btnlayout.add_widget(btn)
        btn = Button(text='cancel')
        btn.bind(on_release=popup.dismiss)
        btnlayout.add_widget(btn)
        content.add_widget(btnlayout)

        # all done, open the popup !
        popup.open()
        textinput.focus = True

class MyButton(MyVariable):
    def __init__(self, **kwargs):
        super(MyButton, self).__init__(**kwargs)
        b = Button()
        # b.size_hint = (0.35, 1)
        self.add_widget(b)
        self.my_button = b

    def setup_button(self, text, signal_me):
        self.my_button.text = text
        if signal_me != None:
            self.my_button.bind(on_release = signal_me.my_button_pressed)

class MySignal(MyVariable):
    def __init__(self, **kwargs):
        super(MySignal, self).__init__(**kwargs)

    def delete(self):
        self.signal.delete()
        self.signal = None

    def setup_signal(self, ioc_root, signal_name, signal_addr, signal_type, n,  mblk_name, mblk_flags, device_path):
        flaglist = mblk_flags.split(',')
        self.my_up = "up" in flaglist
        self.my_down = "down" in flaglist

        description = signal_type 
        if n > 1:
            description += "[" + str(n) + "]"
        description += " at '" + mblk_name + "' address " + str(signal_addr) 

        self.setup_variable(ioc_root, signal_name, description, signal_type == "boolean" and n <= 1)
        self.signal = Signal(ioc_root, signal_name + "." + mblk_name + "." + device_path)

    def on_checkbox_modified(self, i):
        if self.my_up and not self.my_down:
            self.update_signal()

        else:            
            self.signal.set(self.my_checkbox.active)

    def update_signal(self):
        v = self.signal.get(check_tbuf=self.my_up)
        new_state_bits = int(v[0])
        if new_state_bits != self.my_state_bits:
            self.my_state_bits = new_state_bits
            self.my_redraw_state_bits(None)

        if self.my_checkbox != None:
            checked = False
            try:
                checked = int(v[1]) != 0
            except:
                print("mysignal.py: Unable to get check box state")        

            if self.my_checkbox.active != checked:
                self.my_checkbox.active = checked

        if self.my_text != None:
            self.my_text.text = str(v[1])

    def my_user_input(self, instance):
        try:
            v = self.textinput.text
            self.popup.dismiss()
            self.signal.set(v)
        except:
            print("Conversion failed")

class MySetting(MyVariable):
    def __init__(self, **kwargs):
        super(MySetting, self).__init__(**kwargs)

    def delete(self):
        pass

    def setup_setting(self, ioc_root, setting_name, dict, value_d, value, description):
        self.setup_variable(ioc_root, setting_name, description, False)
        self.my_dict = dict

        if value == None:
            self.set_value(value_d, 0)

        else:            
            self.set_value(value, 2) # 2 = state bit "connected" 

    def set_value(self, value, state_bits):
        if state_bits != self.my_state_bits:
            self.my_state_bits = state_bits
            self.my_redraw_state_bits(None)

        if self.my_checkbox != None:
            checked = False
            try:
                checked = int(value) != 0
            except:
                print("Unable to get check box state")        

            if self.my_checkbox.active != checked:
                self.my_checkbox.active = checked

        if self.my_text != None:
            self.my_text.text = str(value)

        self.my_dict[self.my_label_text] = value

    def on_checkbox_modified(self, i):
        self.my_dict[self.my_label_text] = self.my_checkbox.active

    def update_signal(self):
        pass

    def my_user_input(self, instance):
        try:
            v = self.textinput.text
            self.popup.dismiss()
            self.my_text.text = v
            self.my_dict[self.my_label_text] = v
        except:
            print("Conversion failed")

class MyUser(MyVariable):
    def __init__(self, **kwargs):
        super(MyUser, self).__init__(**kwargs)
        self.my_state_bits = -1

    def delete(self):
        pass

    def setup_user(self, ioc_root, groupdict, group, item, flags):
        self.my_group = group
        self.my_groupdict = groupdict
        self.my_item = item
        self.set_value()

        lb = GridLayout()
        lb.size_hint = (0.65, 1) 

        # Make buttons
        self.register_event_type('on_remake_page')
        lb.add_widget(Widget())

        ncols = 1
        flaglist = flags.split(',')
        for button_name in flaglist:
            b = MyIconButton()
            b.set_image(button_name)
            b.my_button_action = button_name
            
            b.bind(on_release = self.my_user_button_pressed)
            lb.add_widget(b)
            ncols += 1

        lb.cols = ncols
        self.add_widget(lb)

    def set_value(self):
        item = self.my_item

        ip = item.get("ip", "")
        text = item.get("user", "")
        if text == "":
            text = ip
            ip = ""

        self.my_label.text = '[size=16]' + text + '[/size]'

        description = ip
        priviliges = item.get("priviliges", None)
        if priviliges != None:
            if description != "":
                description += " "
            description += '[color=9090FF]' + priviliges + '[/color]'
        password = item.get("password", None)            
        if password != None:
            if description != "":
                description += " "
            description += '([color=FF9090]' + password + '[/color])'

        self.my_description.text = '[size=14][color=909090]' + description + '[/color][/size]'

    def on_remake_page(self, *args):
        pass
        print("user button press dispatched")

    def my_user_button_pressed(self, instance):
        action = instance.my_button_action
        if action == 'edit':
            self.my_edit_user_popup()
            return

        p = self.parent
        if action == 'delete' or action == 'dismiss' or action == 'blacklist' or action == 'accept':
            self.my_group.remove(self.my_item)
            p.remove_widget(self)

        if action == 'blacklist':
            self.my_groupdict['blacklist'].append(self.my_item);
            self.dispatch('on_remake_page', action)

        if action == 'accept':
            self.my_groupdict['valid'].append(self.my_item);
            self.dispatch('on_remake_page', action)

            # p.add_widget(self)

    def my_user_edit_ok_button_pressed(self, instance):
        self.my_set_account_attr("user", self.user_name_input.text)
        self.my_set_account_attr("password", self.password_input.text)
        self.my_set_account_attr("priviliges", self.priviliges_input.text)
        self.my_set_account_attr("ip", self.ip_input.text)
        self.set_value()
        self.popup.dismiss()

    def my_set_account_attr(self, attr, value):
        if value == "":
            if self.my_item.get(attr, None) != None:
                del self.my_item[attr]
        else:
            self.my_item[attr] = value

    def my_edit_user_popup(self):
        item = self.my_item
        text = item.get("user", "")
        if text == "":
            text = item.get("ip", "")

        content = BoxLayout(orientation='vertical', spacing='5dp')
        popup_width = min(0.95 * Window.width, dp(500))
        self.popup = popup = Popup(
            title=text, content=content, size_hint=(None, None),
            size=(popup_width, '250dp'))

        # create the text inputs
        user_name_input = make_my_text_input(text)
        self.user_name_input = user_name_input
        password_input = make_my_text_input(item.get("password", ""))
        self.password_input = password_input
        priviliges_input = make_my_text_input(item.get("priviliges", ""))
        self.priviliges_input = priviliges_input
        ip_input = make_my_text_input(item.get("ip", ""))
        self.ip_input = ip_input

        # construct the content, empty widget are used as a spacer
        content.add_widget(Widget())
        content.add_widget(user_name_input)
        content.add_widget(password_input)
        content.add_widget(priviliges_input)
        content.add_widget(ip_input)
        content.add_widget(Widget())

        # 2 buttons are created for accept or cancel the current value
        btnlayout = BoxLayout(size_hint_y=None, height='50dp', spacing='5dp')
        btn = Button(text='ok')
        btn.bind(on_release=self.my_user_edit_ok_button_pressed)
        btnlayout.add_widget(btn)
        btn = Button(text='cancel')
        btn.bind(on_release=popup.dismiss)
        btnlayout.add_widget(btn)
        content.add_widget(btnlayout)

        # all done, open the popup !
        popup.open()
        user_name_input.focus = True

class MySettingsGroup(GridLayout):
    def __init__(self, **kwargs):
        self.my_level = 0
        super(MySettingsGroup, self).__init__(**kwargs)
        self.cols = 2
        self.padding = [8, 6]
        self.orientation='horizontal'
        self.size_hint_y = None
        self.height = 50 

    def set_group_label(self, label1, label2, level):
        self.my_level = level
        if level == 1:
            my_text = '[b][size=20][color=ffffff]' + label1
            if label2 != None:
                my_text += ' ' + label2
        else:
            my_text = '[b][size=18][color=77AAff]' + label1  
            if label2 != None:
                my_text += '[/color] [color=3333ff]' + label2

        my_text += '[/color][/size][/b]'

        l = Label(text = my_text, markup = True, halign="left")
        self.my_label = l
        l.bind(size=l.setter('text_size')) 
        self.add_widget(l)

    def on_size(self, *args):
        self.my_draw_background(args)

    def on_pos(self, *args):
        self.my_draw_background(args)

    def my_draw_background(self, *args):
        self.canvas.before.clear()
        if self.my_level == 2:
            with self.canvas.before:
                Color(0.8, 0.8, 0.8, 0.40)
                mysz = self.size.copy()
                mysz[1] = 1
                Rectangle(pos=self.pos, size=mysz)            

class MySettingsDisplay(GridLayout):
    def __init__(self, **kwargs):
        super(MySettingsDisplay, self).__init__(**kwargs)
        self.cols = 2
        self.my_variables = []
        self.height = self.minimum_height = 400
        self.size_hint_y = None
        self.bind(minimum_height=self.setter('height'))
        self.my_nro_widgets = 0

    def delete(self):
        for s in self.my_variables:
            s.delete()
        self.my_variables = []

    def run(self):
        for s in self.my_variables:
            s.update_signal()

    def new_signal(self, ioc_root, signal_name, signal_addr, signal_type, n, mblk_name, mblk_flags, device_path):
        s = MySignal() 
        s.setup_signal(ioc_root, signal_name, signal_addr, signal_type, n, mblk_name, mblk_flags, device_path)
        self.my_add_widget(s)
        self.my_variables.append(s)

    def new_setting(self, ioc_root, setting_name, dict, value_d, value, description):
        s = MySetting() 
        s.setup_setting(ioc_root, setting_name, dict, value_d, value, description)
        self.my_add_widget(s)
        self.my_variables.append(s)

    def new_user(self, ioc_root, groupdict, group, item, flags):
        u = MyUser() 
        u.setup_user(ioc_root, groupdict, group, item, flags)
        self.my_add_widget(u)
        self.my_variables.append(u)
        return u

    def new_button(self, text, signal_me):
        b = MyButton()
        b.setup_button(text, signal_me)
        self.my_add_widget(b)

    def new_settings_group(self, label1, label2, level):
        widgets_on_row = self.my_nro_widgets % self.cols
        if widgets_on_row > 0:
            for i in range(widgets_on_row, self.cols):
                self.my_add_widget(Widget())

        g = MySettingsGroup()

        g.set_group_label(label1, label2, level)
        self.my_add_widget(g)

        for i in range(1, self.cols):
            g = MySettingsGroup()
            g.my_level = level;
            self.my_add_widget(g)

    def my_add_widget(self, w):
        self.add_widget(w)
        self.my_nro_widgets += 1
        