from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
import json
from time import time

class MyMemoryBlockDialog(GridLayout):
    def __init__(self, **kwargs):
        super(MyMemoryBlockDialog, self).__init__(**kwargs)
        self.cols = 1
        self.padding = [8, 6]
        self.size_hint_y = None
        self.bind(minimum_height=self.setter('height'))
        self.bind(height=self.setter('height'))

    def add_mblk_to_page(self, ioc_root, mblk_path):
        self.clear_widgets()
        json_text = ioc_root.print('memory_blocks', mblk_path, 'data')
        self.process_json(json_text)
        self.ioc_root = ioc_root;
        self.mblk_path = mblk_path;
        self.my_update_time = time()

    def append_text_line(self, text, header_line=False):
        if header_line:
            font_size = 20
            my_label = Label(text = '[b]' + text + '[/b]', markup = True, halign="left", font_size=font_size)
        else:            
            font_size = 15
            my_label = Label(text = text, markup = True, halign="left", font_name="RobotoMono-Regular.ttf", font_size=font_size)
        my_label.bind(size=my_label.setter('text_size')) 
        self.add_widget(my_label)
        my_label.size_hint_y = None
        my_label.height = 1.5 * my_label.font_size 

    def process_json(self, json_text):
        data = json.loads(json_text)

        mblks = data.get("mblk", None)
        if mblks == None:
            print("'mblk' not found")
            return
            
        for mblk in mblks:
            self.process_mblk(mblk)

    def process_mblk(self, data):
        dev_name = data.get("dev_name", "no_name")
        dev_nr = data.get("dev_nr", 0)
        mblk_name = data.get("mblk_name", "no_name")
        mblk_id = data.get("mblk_id", 0)
        net_name = data.get("net_name", "no_name")
        sz = data.get("size", 0)
        flags = data.get("flags", "none")

        self.append_text_line(mblk_name + '.' + dev_name + str(dev_nr) + '.' + net_name, True)
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
            self.add_mblk_to_page(self.ioc_root, self.mblk_path)



