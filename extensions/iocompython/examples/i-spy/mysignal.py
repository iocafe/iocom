from kivy.app import App
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
# from kivy.uix.button import Button
from kivy.uix.checkbox import CheckBox

from kivy.graphics import Color, Rectangle

# from kivy.uix.actionbar import ActionBar, ActionView, ActionPrevious, ActionOverflow, ActionGroup, ActionButton
# , ActionSeparator, ActionCheck, ActionItem, ActionLabel

class MySignal(GridLayout):
    def __init__(self, **kwargs):
        super(MySignal, self).__init__(**kwargs)
        self.cols = 2
        self.padding = [8, 6]
        self.orientation='horizontal'

        self.size_hint_y = None
        self.height = 60 

        l = Label(text = '[b][size=16]Hello[color=3333ff]World[/color][/size][/b]', markup = True, halign="left")
        l.bind(size=l.setter('text_size')) 
        self.my_label = l

        d = Label(text = '[size=14][color=909090]Kuvaus ja paljon asjksjaks asj asjkasjkas kas asjk[/color][/size]', markup = True, halign="left")
        d.bind(size=d.setter('text_size')) 
        self.my_desctiprion = d

        lb = GridLayout()
        lb.cols = 1
        lb.size_hint = (0.65, 1)
        # lb.size_hint = (0.65, None)
        # lb.height = 50

        self.my_label_box = lb
        lb.add_widget(l)
        lb.add_widget(d)

        b = CheckBox(pos_hint={'right': 1})
        self.my_button = b
        b.size_hint = (0.35, 1)
        # b.size_hint = (0.35, None)
        # b.height = 50

        self.add_widget(lb)
        self.add_widget(b)

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
        self.cols = 1

        sig1 = MySignal() # pos_hint={'top': 1})
        self.add_widget(sig1)

        sig2 = MySignal() # pos_hint={'bottom': 1})
        self.add_widget(sig2)

    def delete(self):
        pass

class MainApp(App):
    def build(self):
        self.root = MySignalDisplay()
        return self.root

if __name__ == '__main__':
    MainApp().run()