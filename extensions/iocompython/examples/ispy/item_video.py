from kivy.app import App
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
# from kivy.uix.button import Button

from kivy.graphics import Rectangle
from kivy.graphics import Color
from kivy.graphics.texture import Texture
from array import array

from item_heading import HeadingItem
from iocompython import Root,BrickBuffer

from kivy.utils import get_color_from_hex as rgb
from kivy.uix.boxlayout import BoxLayout

import math

class VideoItem(GridLayout):
    def __init__(self, assembly_type, **kwargs):
        self.texture = None
        super(VideoItem, self).__init__(**kwargs)
        self.assembly_type = assembly_type;
        self.cols = 1

        self.height = self.minimum_height = 480
        self.size_hint_y = None
        self.bind(height=self.setter('height'))

        self.make_video_display()

    def make_video_display(self):
        sz = 100
        hsz = sz/2 - 0.5
        arr = array('B', [0] * sz * sz * 3)
        for y in range(sz):
            dy = y - hsz
            dy2 = dy * dy
            for x in range(sz):
                dx = x - hsz
                dx2 = dx * dx
                d = math.sqrt(dx2 + dy2 + 0.1) / hsz
                v = math.cos(52.1 *d) / (1.0 + d * 3)
                if v < 0.0:
                    v = 0.0
                # v2 = math.cos(0.63 *d)
                i = y * 300 + x * 3;
                # arr[i] = (int)(100 * (dx2 * dy2) / (hsz * hsz * hsz * hsz))
                arr[i] = 0
                # (int)((v2 + 1.0) * 66.9)
                arr[i+1] = 0
                arr[i+2] = (int)((v + 0.0) * 254.0 + 100 * (dx2 * dy2) / (hsz * hsz * hsz * hsz)) 


        # then, convert the array to a ubyte string
        # buf = b''.join(map(chr, buf))
        # buf = [int(x * 255 / size) for x in range(size)]
        # arr = array('B', buf)

        # create a 64x64 texture, defaults to rgba / ubyte
        texture = Texture.create(size=(sz, sz))
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

        # self.canvas.before.clear()
        # with self.canvas.before:
        self.canvas.clear()
        with self.canvas:
            Color(1., 1., 1., 1.)
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
        if self.texture.size[0] != data[2] or self.texture.size[1] !=  data[3]:
            texture = Texture.create(size=(data[2], data[3]))
            self.texture = texture
            self.texture.blit_buffer(arr, colorfmt='rgb', bufferfmt='ubyte')
            self.my_redraw_state_bits()
        else:
            self.texture.blit_buffer(arr, colorfmt='rgb', bufferfmt='ubyte')
            

class MainApp(App):
    def build(self):
        assembly_data = {}
        self.root = VideoItem("cam_flat")
        self.root.set_device(Root('testwidget'), "gina2.iocafenet", assembly_data)
    
        return self.root

if __name__ == '__main__':
    MainApp().run()


'''
# Python program showing  
# Graphical representation of  
# cos() function  
import math 
import numpy as np 
import matplotlib.pyplot as plt  
  
in_array = np.linspace(-(2 * np.pi), 2 * np.pi, 20) 
  
out_array = [] 
  
for i in range(len(in_array)): 
    out_array.append(math.cos(in_array[i])) 
    i += 1
  
   
print("in_array : ", in_array)  
print("\nout_array : ", out_array)  
  
# red for numpy.sin()  
plt.plot(in_array, out_array, color = 'red', marker = "o")  
plt.title("math.cos()")  
plt.xlabel("X")  
plt.ylabel("Y")  
plt.show()  
'''