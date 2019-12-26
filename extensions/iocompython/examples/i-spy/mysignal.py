from kivy.app import App
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.checkbox import CheckBox
from kivy.graphics import Color, Rectangle, Line

from kivy.uix.widget import Widget

from iocompython import Signal


class MySignal(GridLayout):
    def __init__(self, **kwargs):
        self.my_state_bits = 0
        self.my_up = self.my_down = False
        super(MySignal, self).__init__(**kwargs)
        self.cols = 2
        self.padding = [8, 6]
        self.orientation='horizontal'

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

    def delete(self):
        self.signal.delete()
        self.signal = None

    def setup_signal(self, ioc_root, signal_name, signal_addr, signal_type, n,  mblk_name, mblk_flags, dev_path):
        flaglist = mblk_flags.split(',')
        self.my_up = "up" in flaglist
        self.my_down = "down" in flaglist

        if signal_type == "boolean" and n <= 1:
            b = CheckBox()
            b.size_hint = (0.35, 1)
            b.bind(on_press = self.on_checkbox_modified)
            self.add_widget(b)
            self.my_checkbox = b
            self.my_text = None

        else:
            t = Label(text = '[b][size=16]test[color=3333ff]value[/color][/size][/b]', markup = True, halign="center", valign="center")
            t.size_hint = (0.35, 1)
            t.bind(size=t.setter('text_size')) 
            self.add_widget(t)
            self.my_text = t
            self.my_checkbox = None

        description = signal_type 
        if n > 1:
            description += "[" + str(n) + "]"
        description += " at '" + mblk_name + "' address " + str(signal_addr) 

        self.my_label.text = '[size=16]' + signal_name + '[/size]'
        self.my_description.text = '[size=14][color=909090]' + description + '[/color][/size]'

        self.signal = Signal(ioc_root, signal_name + "." + mblk_name + "." + dev_path)

    def on_checkbox_modified(self, i):
        if self.my_up and not self.my_down:
            self.update_signal()

        else:            
            self.signal.set(self.my_checkbox.active)

    def update_signal(self):
        v = self.signal.get()
        new_state_bits = int(v[0]);
        if new_state_bits != self.my_state_bits:
            self.my_state_bits = new_state_bits
            self.my_redraw_status_bits(None)

        if self.my_checkbox != None:
            checked = False
            try:
                if int(v[1]) != 0:
                    checked = True
            except:
                print("mysignal.py: Unable to get check box state")        

            if self.my_checkbox.active != checked:
                self.my_checkbox.active = checked

        if self.my_text != None:
            self.my_text.text = str(v[1])

    def on_size(self, *args):
        self.my_redraw_status_bits(args)

    def my_redraw_status_bits(self, *args):
        self.canvas.before.clear()
        with self.canvas.before:
            Color(0.8, 0.8, 0.8, 0.25)
            mysz = self.size.copy()
            mysz[1] = 1
            Rectangle(pos=self.pos, size=mysz)            

            if self.my_up and not self.my_down:
                if self.my_state_bits & 2 == 0: # 2
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


class MySignalGroup(GridLayout):
    def __init__(self, **kwargs):
        super(MySignalGroup, self).__init__(**kwargs)
        self.cols = 2
        self.padding = [8, 6]
        self.orientation='horizontal'
        self.size_hint_y = None
        self.height = 50 

    def set_group_label(self, group_label, mblk_name):
        my_text = '[b][size=20]' + group_label + ' [color=3333ff]' + mblk_name + '[/color][/size][/b]'

        l = Label(text = my_text, markup = True, halign="left")
        self.my_label = l
        l.bind(size=l.setter('text_size')) 
        self.add_widget(l)

    def on_size(self, *args):
        self.canvas.before.clear()
        with self.canvas.before:
            Color(0.8, 0.8, 0.8, 0.40)
            mysz = self.size.copy()
            mysz[1] = 1
            Rectangle(pos=self.pos, size=mysz)            

class MySignalDisplay(GridLayout):
    def __init__(self, **kwargs):
        super(MySignalDisplay, self).__init__(**kwargs)
        self.cols = 2
        self.signals = []
        self.height = self.minimum_height = 400
        self.size_hint_y = None
        self.bind(minimum_height=self.setter('height'))
        self.my_nro_widgets = 0

    def delete(self):
        for s in self.signals:
            s.delete()
        self.signals = []

    def run(self):
        for s in self.signals:
            s.update_signal()

    def new_signal(self, ioc_root, signal_name, signal_addr, signal_type, n,  mblk_name, mblk_flags, dev_path):
        s = MySignal() 
        s.setup_signal(ioc_root, signal_name, signal_addr, signal_type, n,  mblk_name, mblk_flags, dev_path)
        self.add_widget(s)
        self.my_nro_widgets += 1
        self.signals.append(s)

    def new_signal_group(self, group_label, mblk_name):
        widgets_on_row = self.my_nro_widgets % self.cols
        if widgets_on_row > 0:
            for i in range(widgets_on_row, self.cols):
                self.add_widget(Widget())
                self.my_nro_widgets += 1

        g = MySignalGroup()
        g.set_group_label(group_label, mblk_name)
        self.add_widget(g)
        self.my_nro_widgets += 1

        for i in range(1, self.cols):
            self.add_widget(MySignalGroup())
            self.my_nro_widgets += 1

