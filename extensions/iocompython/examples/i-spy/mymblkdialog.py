from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from mytabledata import MyTableData
from mytable import MyTable
import json

class MyMemoryBlockDialog(GridLayout):
    def __init__(self, **kwargs):
        super(MyMemoryBlockDialog, self).__init__(**kwargs)
        self.cols = 1
        self.height = self.minimum_height = 400
        self.size_hint_y = None
        self.bind(minimum_height=self.setter('height'))

        w = Label(text = 'memory block...')
        w.size_hint_y = None
        w.height = 60 
        self.add_widget(w)

    def add_mblk_to_page(self, ioc_root, mblk_path):
        json_text = ioc_root.print('memory_blocks', mblk_path, 'data')
        print(json_text)
        self.process_json(json_text)

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

        title = []
        title.append(mblk_name + '.' + dev_name + str(dev_nr) + '.' + net_name) 
        title.append('id=' + str(mblk_id) + ', size=' + str(sz) + ", flags=" + flags)

        table = MyTable()
        self.add_widget(table)

        table_data = MyTableData()
        table_data.set_title(title)
        table.set_table_data(table_data)

        data = data.get("data", None)
        if data == None:
            return

        # for d in data:

    def delete(self):
        pass

    def run(self):
        pass



