from kivy.app import App
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label

from kivy.graphics import Rectangle
from kivy.graphics.texture import Texture
from array import array

from item_heading import HeadingItem
from iocompython import Root,BrickBuffer

import itertools
from math import sin, cos, pi
from random import randrange
from kivy.utils import get_color_from_hex as rgb
from kivy.uix.boxlayout import BoxLayout
# from kivy.clock import Clock

try:
    import numpy as np
except ImportError as e:
    np = None

# rom garden_graph import Graph, SmoothLinePlot, MeshLinePlot, MeshStemPlot, BarPlot, ContourPlot


class VideoItem(GridLayout):
    def __init__(self, **kwargs):
        super(VideoItem, self).__init__(**kwargs)
        self.cols = 1

        self.height = self.minimum_height = 400
        self.size_hint_y = None
        self.bind(height=self.setter('height'))

        g = GridLayout(cols = 2)
        g.height = 60
        g.size_hint_y = None
        title = HeadingItem()
        g.add_widget(title)
        self.my_title = title

        graph_display = self.make_video_display()
        self.add_widget(graph_display)

    def make_video_display(self):
        b = BoxLayout(orientation='vertical')

        # create a 64x64 texture, defaults to rgba / ubyte
        texture = Texture.create(size=(640, 640))

        # create 64x64 rgb tab, and fill with values from 0 to 255
        # we'll have a gradient from black to white
        size = 640 * 640 * 3
        buf = [int(x * 255 / size) for x in range(size)]

        # then, convert the array to a ubyte string
        # buf = b''.join(map(chr, buf))
        arr = array('B', buf)

        # then blit the buffer
        texture.blit_buffer(arr, colorfmt='rgb', bufferfmt='ubyte')

        with self.canvas:
            Rectangle(texture=texture, pos=self.pos, size=(640, 640))

        return b

    def set_device(self, ioc_root, device_path, assembly_data):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.my_title.set_group_label("program", self.device_path, 1)
        # assembly_name = assembly_data.get("name", "no_name")
        exp = assembly_data.get("exp", "exp.brick_").split('.')
        imp = assembly_data.get("imp", "imp.brick_").split('.')
        exp_path = exp[0] + '.' + device_path
        imp_path = imp[0] + '.' + device_path
        prefix = exp[1]
        self.camera_buffer = BrickBuffer(ioc_root, exp_path, imp_path, prefix, timeout=-1)
        self.camera_buffer.set_receive(True);

    def delete(self):
        pass

 
    def update_signal(self):
        self.run()

    def run(self):
        data = self.camera_buffer.get()
        if data != None:
            self.update_plot(data)

    def update_plot(self, data):
        n = 1

class MainApp(App):
    def build(self):
        assembly_data = {}
        self.root = VideoItem()
        self.root.set_device(Root('testwidget'), "gina2.iocafenet", assembly_data)
    
        return self.root

if __name__ == '__main__':
    MainApp().run()

