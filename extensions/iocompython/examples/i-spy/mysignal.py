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
        lb.size_hint = (0.65, None)
        lb.height = 50

        self.my_label_box = lb
        lb.add_widget(l)
        lb.add_widget(d)

        b = CheckBox(pos_hint={'right': 1})
        self.my_button = b
        b.size_hint = (0.35, None)
        b.height = 50

        self.add_widget(lb)
        self.add_widget(b)

    def on_size(self, *args):
        self.canvas.before.clear()
        with self.canvas.before:
            Color(0.8, 0.8, 0.8, 0.25)
            mypos = self.pos.copy()
            mysz = self.size.copy()
            myline_w = 1
            #mypos[1] = mypos[1] + mysz[1] - myline_w
            #mysz[1] = myline_w
            # mypos[1] = mypos[1] + mysz[1] - myline_w
            mysz[1] = myline_w
            Rectangle(pos=mypos, size=mysz)            

    # def on_button_press(self, *args):
    #    print("button press dispatched")

    # def my_close_pressed(self, instance):
    #    self.dispatch('on_button_press', 'close')


class MySignalSeparator(Label):
    def __init__(self, **kwargs):
        super(MySignalSeparator, self).__init__(**kwargs)

        # self.register_event_type('on_button_press')

        # l = Label(text = 'XXX, markup = True, halign="left")
        # l.halign = 'left'
        self.text = "Ukekek"
        self.size_hint_y = None
        self.size_hint_x = None
        self.height = 5
        self.width  = 500
        self.color =[1, 0, 0, 1]
        self.background_color =[1, 0, 0, 1]
        # l.bind(size=l.setter('text_size')) 

class MainApp(App):
    def build(self):
        self.root = GridLayout()
        self.root.cols = 1

        sig1 = MySignal() # pos_hint={'top': 1})
        self.root.add_widget(sig1)

        # sep = MySignalSeparator()
        # self.root.add_widget(sep)


        sig2 = MySignal() # pos_hint={'bottom': 1})
        self.root.add_widget(sig2)


        return self.root

if __name__ == '__main__':
    MainApp().run()