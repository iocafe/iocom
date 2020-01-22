from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.graphics import Color, Rectangle, Line
from iocompython import Signal

class NotificationItem(GridLayout):
    def __init__(self, **kwargs):
        self.my_state_bits = 0
        super(NotificationItem, self).__init__(**kwargs)
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

            ''' if self.my_state_bits == -1: 
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
            '''

    def delete(self):
        self.signal.delete()
        self.signal = None

    def setup_notification(self, ioc_root, signal_name, mblk_name, device_path, flags):

        description = "jeppo"

        t = Button(text = '', markup = True, halign="center", valign="center")
        t.size_hint = (0.35, 1)
        t.bind(size=t.setter('text_size')) 
        t.background_color = [0 , 0, 0, 0]
        self.add_widget(t)
        self.my_text = t

        self.my_label.text = '[size=16]' + "PEPEPEP"  + '[/size]'
        self.my_description.text = '[size=14][color=909090]' + description + '[/color][/size]'

        self.signal = Signal(ioc_root, signal_name + "." + mblk_name + "." + device_path)

    def update_signal(self):
        try:
            v = self.signal.get(check_tbuf=True)
        except:
            print("mysettings.py: update_signal failed")
            self.my_text.text = '?'
            self.my_state_bits = 6
            return

        new_state_bits = int(v[0])
        if new_state_bits != self.my_state_bits:
            self.my_state_bits = new_state_bits
            self.my_redraw_state_bits(None)

        if self.my_text != None:
            self.my_text.text = str(v[1])
