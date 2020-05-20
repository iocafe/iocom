from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
import json
from time import time
from panel import Panel 

class MemoryBlockPanel(Panel):
    def __init__(self, **kwargs):
        super(MemoryBlockPanel, self).__init__(**kwargs)
        self.cols = 1

    def add_mblk_to_page(self, ioc_root, device_path):
        mblk_path = "*." + device_path
        self.clear_widgets()
        self.add_heading("memory - ", device_path, 1)

        json_text = ioc_root.print('memory_blocks', mblk_path, 'data')
        self.process_json(json_text)
        self.ioc_root = ioc_root;
        self.my_device_path = device_path
        self.my_update_time = time()

    def append_text_line(self, text):
        font_size = 15
        my_label = Label(text = text, markup = True, halign="left", font_name="RobotoMono-Regular.ttf", font_size=font_size)
        my_label.bind(size=my_label.setter('text_size')) 
        self.add_widget(my_label)
        my_label.size_hint_y = None
        my_label.height = 1.5 * my_label.font_size 
        my_label.padding = [8, 0]


    def process_json(self, json_text):
        data = json.loads(json_text)

        mblks = data.get("mblk", None)
        if mblks == None:
            print("'mblk' not found")
            return
            
        for mblk in mblks:
            self.process_mblk(mblk)

    def process_mblk(self, data):
        mblk_name = data.get("mblk_name", "no_name")
        mblk_id = data.get("mblk_id", 0)
        sz = data.get("size", 0)
        flags = data.get("flags", "none")

        self.add_heading(mblk_name, None, 2)
        self.append_text_line('id=' + str(mblk_id) + ', size=' + str(sz) + ", flags=" + flags)

        data = data.get("data", None)
        if data == None:
            return

        n = len(data)
        n_cols = 16
        n_rows = (n + n_cols - 1) // n_cols

        for y in range(n_rows):
            my_text = str(y * n_cols).zfill(6) + ': '
            nn = n - y * n_cols
            if nn > n_cols:
                nn = n_cols
            for x in range(nn):
                my_text += str(data[y * n_cols + x]).zfill(3) + ' '
            self.append_text_line(my_text)

    def delete(self):
        pass

    def run(self):
        if time() > self.my_update_time + 2:
            self.add_mblk_to_page(self.ioc_root, self.my_device_path)



