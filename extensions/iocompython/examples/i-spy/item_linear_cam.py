import json
import os
import re
from kivy.app import App
from kivy.config import ConfigParser
from kivy.uix.filechooser import FileChooserListView
from kivy.uix.filechooser import FileChooserIconView
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.textinput import TextInput
from kivy.uix.widget import Widget
from kivy.core.window import Window
from kivy.uix.popup import Popup
from kivy.uix.label import Label
from kivy.metrics import dp

from item_heading import HeadingItem
from item_button import ButtonItem
from item import make_my_text_input
from item import Item
from iocompython import Root,bin2json, json2bin


import itertools
from math import sin, cos, pi
from random import randrange
from kivy.utils import get_color_from_hex as rgb
from kivy.uix.boxlayout import BoxLayout
from kivy.clock import Clock

try:
    import numpy as np
except ImportError as e:
    np = None

from garden_graph import Graph, SmoothLinePlot, MeshLinePlot, MeshStemPlot, BarPlot, ContourPlot

class MyFileChooserDummy(FileChooserIconView):
    def __init__(self, **kwargs):
        super(MyFileChooserDummy, self).__init__(**kwargs)

#    def on_submit(self, selected, touch=None):
#        print("HERE")

    pass

class LinearCameraItem(GridLayout):
    def __init__(self, **kwargs):
        super(LinearCameraItem, self).__init__(**kwargs)
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

        bg = GridLayout(cols = 3)
        bg.size_hint_y = None
        bg.bind(height=self.setter('height'))
        # bg.padding = [8, 6]

        b = Button(text='1. program')
        b.bind(on_release = self.my_select_block_dialog)
        bg.add_widget(b)
        self.my_block_button = b
        self.my_set_select()

        b = Button(text='read')
        b.bind(on_release = self.my_read_block_dialog)
        bg.add_widget(b)

        b = Button(text='write')
        b.bind(on_release = self.my_write_block_dialog)
        bg.add_widget(b)

        g.add_widget(bg)
        # self.add_widget(g)

        g = GridLayout(cols = 2)
        g.height = 40
        g.size_hint_y = None
        g.padding = [8, 6]

        self.current_path = '/coderoot'

        self.my_path = TextInput(text=self.current_path,  multiline=False, write_tab=False)
        g.add_widget(self.my_path)
        self.my_fname = TextInput(text='',  multiline=False, write_tab=False)
        g.add_widget(self.my_fname)
        # self.add_widget(g)

        graph_display = self.make_graph_display()
        self.add_widget(graph_display)

        self.my_file_chooser = MyFileChooserDummy(path=self.current_path, size_hint=(1, 1), dirselect=True)
        # self.add_widget(self.my_file_chooser)

        self.my_file_chooser.bind(selection=lambda *x: self.my_select(x[1:]))
        self.my_file_chooser.bind(path=lambda *x: self.my_set_path(x[1:]))

# XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

    def make_graph_display(self):
        b = BoxLayout(orientation='vertical')
        # example of a custom theme
        colors = itertools.cycle([
            rgb('7dac9f'), rgb('dc7062'), rgb('66a8d4'), rgb('e5b060')])
        graph_theme = {
            'label_options': {
                'color': rgb('B0B0B0'),  # color of tick labels and titles
                'bold': False},
            'background_color': rgb('000000'), # rgb('f8f8f2'),  # back ground color of canvas
            'tick_color': rgb('B0B0B0'),  # ticks and grid
            'border_color': rgb('808080')}  # border drawn around each graph

        graph = Graph(
            xlabel='Cheese',
            ylabel='Apples',
            x_ticks_minor=5,
            x_ticks_major=25,
            y_ticks_major=1,
            y_grid_label=True,
            x_grid_label=True,
            padding=5,
            xlog=False,
            ylog=False,
            x_grid=True,
            y_grid=True,
            xmin=-50,
            xmax=50,
            ymin=-1,
            ymax=1,
            **graph_theme)

        plot = SmoothLinePlot(color=next(colors))
        plot.points = [(x / 10., sin(x / 50.)) for x in range(-500, 501)]
        # for efficiency, the x range matches xmin, xmax
        graph.add_plot(plot)

        plot = MeshLinePlot(color=next(colors))
        plot.points = [(x / 10., cos(x / 50.)) for x in range(-500, 501)]
        graph.add_plot(plot)
        self.plot = plot  # this is the moving graph, so keep a reference

        plot = MeshStemPlot(color=next(colors))
        graph.add_plot(plot)
        plot.points = [(x, x / 50.) for x in range(-50, 51)]

        plot = BarPlot(color=next(colors), bar_spacing=.72)
        graph.add_plot(plot)
        plot.bind_to_graph(graph)
        plot.points = [(x, .1 + randrange(10) / 10.) for x in range(-50, 1)]

        Clock.schedule_interval(self.update_points, 1 / 60.)

        graph2 = Graph(
            xlabel='Position (m)',
            ylabel='Time (s)',
            x_ticks_minor=0,
            x_ticks_major=1,
            y_ticks_major=10,
            y_grid_label=True,
            x_grid_label=True,
            padding=5,
            xlog=False,
            ylog=False,
            xmin=0,
            ymin=0,
            **graph_theme)
        b.add_widget(graph)

        if np is not None:
            (xbounds, ybounds, data) = self.make_contour_data()
            # This is required to fit the graph to the data extents
            graph2.xmin, graph2.xmax = xbounds
            graph2.ymin, graph2.ymax = ybounds

            plot = ContourPlot()
            plot.data = data
            plot.xrange = xbounds
            plot.yrange = ybounds
            plot.color = [1, 0.7, 0.2, 1]
            graph2.add_plot(plot)

            b.add_widget(graph2)
            self.contourplot = plot

            Clock.schedule_interval(self.update_contour, 1 / 60.)

        return b

    def make_contour_data(self, ts=0):
        omega = 2 * pi / 30
        k = (2 * pi) / 2.0

        ts = sin(ts * 2) + 1.5  # emperically determined 'pretty' values
        npoints = 100
        data = np.ones((npoints, npoints))

        position = [ii * 0.1 for ii in range(npoints)]
        time = [(ii % 100) * 0.6 for ii in range(npoints)]

        for ii, t in enumerate(time):
            for jj, x in enumerate(position):
                data[ii, jj] = sin(k * x + omega * t) + sin(-k * x + omega * t) / ts
        return ((0, max(position)), (0, max(time)), data)

    def update_points(self, *args):
        self.plot.points = [(x / 10., cos(Clock.get_time() + x / 50.)) for x in range(-500, 501)]

    def update_contour(self, *args):
        _, _, self.contourplot.data[:] = self.make_contour_data(Clock.get_time())
        # this does not trigger an update, because we replace the
        # values of the arry and do not change the object.
        # However, we cannot do "...data = make_contour_data()" as
        # kivy will try to check for the identity of the new and
        # old values.  In numpy, 'nd1 == nd2' leads to an error
        # (you have to use np.all).  Ideally, property should be patched
        # for this.
        self.contourplot.ask_draw()

# XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    def my_set_select(self):
        extension_list = {"1.":".elf", "4.":".key", "6.":".crt", "7.":".crt", "8.":".crt"}

        self.my_select_nr = int(re.search(r'\d+', self.my_block_button.text).group())
        l = self.my_block_button.text.split()
        ext = extension_list.get(l[0], ".dat")
        self.my_ext = ext
        self.my_default_fname = l[1] + '-' + str(self.my_select_nr) + ext

    def my_select(self, path):
        # Only one selection
        if len(path) < 1:
            return;
        path = str(path[0][0])
        if len(path) > len(self.current_path) + 1:
            self.my_fname.text = path[len(self.current_path) + 1:] 
        else:
            self.my_fname.text = ''

    def my_set_path(self, path):
        if len(path) < 1:
            return;
        path = str(path[0])
        self.current_path = path
        self.my_path.text = self.current_path
        self.my_fname.text = ''

    def set_device(self, ioc_root, device_path, assembly_name, assembly_params):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.my_title.set_group_label("program", self.device_path, 1)

    def delete(self):
        pass

    def run(self):
        pass

    def my_read_block_dialog(self, instance):
        grid = GridLayout()
        grid.cols = 1
        grid.spacing = [6, 6]
        grid.padding = [6, 6]

        my_path = os.path.join(self.my_path.text, self.my_fname.text)

        if os.path.isdir(my_path):
            my_path = os.path.join(my_path, self.my_default_fname)

        if os.path.isfile(my_path):
            warn = Label(text = "FILE ALREADY EXISTS, IF YOU SELECT DOWNLOAD\nTHE FILE WILL BE OVERWRITTEN", markup = True, halign="center")
            grid.add_widget(warn)

        pathinput = make_my_text_input(my_path)
        grid.add_widget(pathinput)
        self.pathinput = pathinput

        self.popup = popup = Popup(
            title='confirm persistent block download', content=grid)

        bg = GridLayout(cols = 2)
        bg.spacing = [6, 6]

        b = Button(text='close')
        b.height = 60
        b.size_hint_y = None
        b.bind(on_release = popup.dismiss)
        bg.add_widget(b)

        b = Button(text='download')
        b.height = 60
        b.size_hint_y = None
        b.bind(on_release = self.read_selected)
        bg.add_widget(b)

        grid.add_widget(bg)

        # all done, open the popup !
        popup.open()

    def read_selected(self, instance):
        # self.my_block_button.text = instance.text
        file_content = self.ioc_root.getconf(self.device_path, select=self.my_select_nr)
        if file_content == None:
            return

        # If this is key or certificate (text content), remove terminating '\0' character, if any.
        # We should not have it in PC text files
        if self.my_ext == '.key' or self.my_ext == '.crt':
            l = len(file_content)
            if file_content[l-1] == 0:
                file_content = file_content[:l-1]

        with open(self.pathinput.text, mode='wb') as file: # b is important -> binary
            file.write(file_content)
         
        self.popup.dismiss()
        self.my_file_chooser._update_files()

    def my_write_block_dialog(self, instance):
        grid = GridLayout()
        grid.cols = 1
        grid.spacing = [6, 6]
        grid.padding = [6, 6]

        my_path = os.path.join(self.my_path.text, self.my_fname.text)

        if os.path.isdir(my_path):
            my_path = os.path.join(my_path, self.my_default_fname)

        pathinput = make_my_text_input(my_path)
        grid.add_widget(pathinput)
        self.pathinput = pathinput

        self.popup = popup = Popup(
            title='confirm file upload', content=grid)

        bg = GridLayout(cols = 2)
        bg.spacing = [6, 6]

        b = Button(text='close')
        b.height = 60
        b.size_hint_y = None
        b.bind(on_release = popup.dismiss)
        bg.add_widget(b)

        b = Button(text='upload')
        b.height = 60
        b.size_hint_y = None
        b.bind(on_release = self.write_selected)
        bg.add_widget(b)

        grid.add_widget(bg)

        # all done, open the popup !
        popup.open()

    def write_selected(self, instance):
        with open(self.pathinput.text, mode='rb') as file: # b is important -> binary
            file_content = file.read()

            # If this is key or certificate (text content), append terminating '\0' character, if missing.
            # We should not have it in PC text files but need it on micro-controller
            if self.my_ext == '.key' or self.my_ext == '.crt':
                l = len(file_content)
                if file_content[l-1] != 0:
                    file_content = bytearray(file_content)
                    file_content.append(0)
                    file_content = bytes(file_content)

            rval = self.ioc_root.setconf(self.device_path, file_content, select=self.my_select_nr)
        
        self.popup.dismiss()

    def my_select_block_dialog(self, instance):
        button_list = ["1. program", "2. config", "3. defaults", "5. server key", "6. server cert", "7. root cert", "8. cert chain", "9. publish chain", "10. wifi select", "12. cust", "21. accounts"]

        grid = GridLayout()
        grid.cols = 2;
        grid.spacing = [6, 6]
        grid.padding = [6, 6]

        for button_text in button_list:
            b = Button(text=button_text)
            b.height = 60
            b.size_hint_y = None
            b.bind(on_release = self.block_selected)
            grid.add_widget(b)

        self.popup = popup = Popup(
            title='select persistent block', content=grid)

        # all done, open the popup !
        popup.open()

    def block_selected(self, instance):
        self.my_block_button.text = instance.text
        self.my_set_select()
        self.popup.dismiss()

class MainApp(App):
    def build(self):
        self.root = LinearCameraItem()
        self.root.set_device(Root('testwidget'), "gina2.iocafenet", "kamera", "parametrit")
    
        return self.root

if __name__ == '__main__':
    MainApp().run()

