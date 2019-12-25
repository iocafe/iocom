from kivy.app import App
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.checkbox import CheckBox
from kivy.graphics import Color, Rectangle

from iocompython import Signal


class MySignal(GridLayout):
    def __init__(self, **kwargs):
        super(MySignal, self).__init__(**kwargs)
        self.cols = 2
        self.padding = [8, 6]
        self.orientation='horizontal'

        self.size_hint_y = None
        self.height = 60 

        l = Label(text = '[b][size=16]test[color=3333ff]label[/color][/size][/b]', markup = True, halign="left")
        l.bind(size=l.setter('text_size')) 
        self.my_label = l

        d = Label(text = '[size=14][color=909090]test descriotion[/color][/size]', markup = True, halign="left")
        d.bind(size=d.setter('text_size')) 
        self.my_description = d

        lb = GridLayout()
        lb.cols = 1
        lb.size_hint = (0.65, 1)

        self.my_label_box = lb
        lb.add_widget(l)
        lb.add_widget(d)

        b = CheckBox(pos_hint={'right': 1})
        self.my_button = b
        b.size_hint = (0.35, 1)

        self.add_widget(lb)
        self.add_widget(b)

    def setup_signal(self, ioc_root, signal_name, signal_addr, signal_type, n,  mblk_name, dev_path):
        description = signal_type 
        if n > 1:
            description += "[" + str(n) + "]"
        description += " at '" + mblk_name + "' address " + str(signal_addr) 

        self.my_label.text = '[size=16][b]' + signal_name + '[/b][/size]'
        self.my_description.text = '[size=14][color=909090]' + description + '[/color][/size]'

        self.signal = Signal(ioc_root, signal_name + "." + mblk_name + "." + dev_path)

    def update_signal(self):
        self.my_description.text = '[size=14][color=2020FF]' + str(self.signal.get()) + '[/color][/size]'

    def on_size(self, *args):
        self.canvas.before.clear()
        with self.canvas.before:
            Color(0.8, 0.8, 0.8, 0.25)
            mysz = self.size.copy()
            mysz[1] = 1
            Rectangle(pos=self.pos, size=mysz)            


class MySignalDisplay(GridLayout):
    def __init__(self, **kwargs):
        super(MySignalDisplay, self).__init__(**kwargs)
        self.cols = 2
        self.signals = []

    def delete(self):
        for s in self.signals:
            s.delete()
        self.signals = []

    def run(self):
        for s in self.signals:
            s.update_signal()

    def new_signal(self, ioc_root, signal_name, signal_addr, signal_type, n,  mblk_name, dev_path):
        s = MySignal() 
        s.setup_signal(ioc_root, signal_name, signal_addr, signal_type, n,  mblk_name, dev_path)
        self.add_widget(s)
        self.signals.append(s)


class MainApp(App):
    def build(self):
        self.root = GridLayout()
        self.root.cols = 1
        
        sig1 = MySignal() # pos_hint={'top': 1})
        self.root.add_widget(sig1)

        sig2 = MySignal()
        self.root.add_widget(sig2)

        return self.root

if __name__ == '__main__':
    MainApp().run()