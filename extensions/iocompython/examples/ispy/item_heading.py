from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.graphics import Color, Rectangle

from iocompython import Signal

class HeadingItem(GridLayout):
    def __init__(self, **kwargs):
        self.my_level = 0
        super(HeadingItem, self).__init__(**kwargs)
        self.cols = 2
        self.padding = [8, 6]
        # self.orientation='horizontal'
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

