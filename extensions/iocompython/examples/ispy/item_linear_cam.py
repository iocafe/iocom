from kivy.app import App
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label

from item_heading import HeadingItem
from iocompython import Root,BrickBuffer

import itertools
from math import sin, cos, pi
from random import randrange
from kivy.utils import get_color_from_hex as rgb
from kivy.uix.boxlayout import BoxLayout

try:
    import numpy as np
except ImportError as e:
    np = None

from garden_graph import Graph, SmoothLinePlot, MeshLinePlot, MeshStemPlot, BarPlot, ContourPlot


class LinearCameraItem(GridLayout):
    def __init__(self, assembly_type, **kwargs):
        super(LinearCameraItem, self).__init__(**kwargs)
        self.assembly_type = assembly_type;
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

        graph_display = self.make_graph_display()
        self.add_widget(graph_display)

    def make_graph_display(self):
        b = BoxLayout(orientation='vertical')
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
            x_ticks_minor=25,
            x_ticks_major=100,
            y_ticks_major=10,
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

        self.graph = graph

        # plot = SmoothLinePlot(color=next(colors))
        # plot.points = [(x / 10., sin(x / 50.)) for x in range(-500, 501)]
        # for efficiency, the x range matches xmin, xmax
        #graph.add_plot(plot)

        plot = MeshLinePlot(color=next(colors))
        plot.points = [(x / 10., cos(x / 50.)) for x in range(-500, 501)]
        graph.add_plot(plot)
        self.plot = plot  # this is the moving graph, so keep a reference

        # plot = MeshStemPlot(color=next(colors))
        # graph.add_plot(plot)
        # plot.points = [(x, x / 50.) for x in range(-50, 51)]

        # plot = BarPlot(color=next(colors), bar_spacing=.72)
        # graph.add_plot(plot)
        # plot.bind_to_graph(graph)
        # plot.points = [(x, .1 + randrange(10) / 10.) for x in range(-50, 1)]

        # Clock.schedule_interval(self.update_points, 1 / 60.)
        b.add_widget(graph)

        """
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
        """            

        return b

    '''
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
    '''

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
        d = data[0]
        n = len(d)

        self.plot.points = [(1.1*x, 1.1*d[x]) for x in range(0, n)]

        self.graph.xmin = 0
        self.graph.xmax = n
        self.graph.ymin = 120 # min(d)
        self.graph.ymax = 255 # max(d)

class MainApp(App):
    def build(self):
        assembly_data = {}
        self.root = LinearCameraItem("lcam_flat")
        self.root.set_device(Root('testwidget'), "gina2.iocafenet", assembly_data)
    
        return self.root

if __name__ == '__main__':
    MainApp().run()

