from kivy.app import App
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
# from kivy.uix.button import Button

from kivy.graphics import Rectangle
from kivy.graphics.texture import Texture
from array import array

from item_heading import HeadingItem
from iocompython import Root,BrickBuffer

from kivy.utils import get_color_from_hex as rgb
from kivy.uix.boxlayout import BoxLayout


class VideoItem(GridLayout):
    def __init__(self, **kwargs):
        self.texture = None
        super(VideoItem, self).__init__(**kwargs)
        self.cols = 1

        self.height = self.minimum_height = 480
        self.size_hint_y = None
        self.bind(height=self.setter('height'))

        self.make_video_display()

    def make_video_display(self):
        # create 64x64 rgb tab, and fill with values from 0 to 255
        # we'll have a gradient from black to white
        size = 640 * 480 * 3
        buf = [int(x * 255 / size) for x in range(size)]

        # then, convert the array to a ubyte string
        # buf = b''.join(map(chr, buf))
        arr = array('B', buf)

        # create a 64x64 texture, defaults to rgba / ubyte
        texture = Texture.create(size=(640, 480))
        self.texture = texture
        # then blit the buffer
        texture.blit_buffer(arr, colorfmt='rgb', bufferfmt='ubyte')

    def on_size(self, *args):
        self.my_redraw_state_bits(args)

    def on_pos(self, *args):
        self.my_redraw_state_bits(args)

    def my_redraw_state_bits(self, *args):
        if self.texture == None:
            return;

        # self.canvas.clear()
        # with self.canvas:
        self.canvas.before.clear()
        with self.canvas.before:
            Rectangle(texture=self.texture, pos=self.pos, size=(640, 480))

    def set_device(self, ioc_root, device_path, assembly_data):
        self.ioc_root = ioc_root
        self.device_path = device_path
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
        arr = array('B', data[0])
        self.texture.blit_buffer(arr, colorfmt='rgb', bufferfmt='ubyte')

class MainApp(App):
    def build(self):
        assembly_data = {}
        self.root = VideoItem()
        self.root.set_device(Root('testwidget'), "gina2.iocafenet", assembly_data)
    
        return self.root

if __name__ == '__main__':
    MainApp().run()

