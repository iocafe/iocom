from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.checkbox import CheckBox
from kivy.graphics import Color, Rectangle, Line
from kivy.core.window import Window
from kivy.uix.popup import Popup
from kivy.uix.textinput import TextInput
from kivy.uix.button import Button
from kivy.metrics import dp
from kivy.uix.widget import Widget

from iocompython import Signal

def make_my_text_input(text, small = False):
    if small:
        return TextInput(text=text, font_size='16sp', multiline=False, write_tab=False, 
            size_hint_y=None, height='32sp')
    else:
        return TextInput(text=text, font_size='24sp', multiline=False, write_tab=False, 
            size_hint_y=None, height='42sp')

class Item(GridLayout):
    def __init__(self, **kwargs):
        self.my_state_bits = 0
        self.my_up = True
        self.my_down = True
        super(Item, self).__init__(**kwargs)
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

            if self.my_state_bits & 12 == 12:
                Color(1.0, 0, 0, 1)
            elif self.my_state_bits & 2 == 0: 
                Color(0.5, 0.5, 0.5, 1)
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
            text=self.my_text.text, font_size='24sp', multiline=False, write_tab=False,
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

