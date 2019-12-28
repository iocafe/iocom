from kivy.app import App
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.scrollview import ScrollView
from kivy.uix.widget import Widget
from kivy.graphics import Color, Rectangle
from random import random


class MyTable(GridLayout):
    def __init__(self, **kwargs):
        # self.orientation='vertical'
        super(MyTable, self).__init__(**kwargs)
        self.cols = 1
        # self.height = self.minimum_height = 400
        # self.size_hint_y = None
        # self.bind(minimum_height=self.setter('height'))

        # w = Label(text = 'waiting for connection...')
        # w.size_hint_y = None
        # w.height = 60 
        # self.add_widget(w)

        self.title = MyTitle()
        self.add_widget(self.title)

        self.content = MyContent()
        self.add_widget(self.content)

    def delete(self):
        pass

    def run(self):
        pass

class MyTitle(GridLayout):
    def __init__(self, **kwargs):
        super(MyTitle, self).__init__(**kwargs)
        self.size_hint_y = None
        self.height = 60 

class MyContent(GridLayout):
    def __init__(self, **kwargs):
        super(MyContent, self).__init__(**kwargs)
        self.cols = 2

        self.column_hdr_corner = MyColumnHeaderCorner()
        self.add_widget(self.column_hdr_corner)

        self.column_hdr_panel = MyColumnHeaderPanel()
        self.add_widget(self.column_hdr_panel)

        self.row_hdr_panel = MyRowHeaderPanel()
        self.add_widget(self.row_hdr_panel)

        self.data_panel = MyDataPanel()
        self.add_widget(self.data_panel)

        self.insert_row_hdr_panel = MyInsertRowHeaderPanel()
        self.add_widget(self.insert_row_hdr_panel)

        self.insert_data_panel = MyInsertDataPanel()
        self.add_widget(self.insert_data_panel)

        self.summary_row_hdr_panel = MySummaryRowHeaderPanel()
        self.add_widget(self.summary_row_hdr_panel)

        self.summary_data_panel = MySummaryDataPanel()
        self.add_widget(self.summary_data_panel)

class MyPanel(ScrollView):
    def __init__(self, **kwargs):
        super(MyPanel, self).__init__(**kwargs)
        
        w = MyBaseWidget()
        self.add_widget(w)
        self.my_widget = w

    def set_my_height(self, h):
        self.my_widget.size_hint_y = None
        self.my_widget.height = h 

        self.size_hint_y = None
        self.height = h 

    def set_my_width(self, w):
        self.my_widget.size_hint_x = None
        self.my_widget.width = w 

        self.size_hint_x = None
        self.width = w 

class MyBaseWidget(Widget):
    def __init__(self, **kwargs):
        self.my_color = [random(), random(), random()]
        super(MyBaseWidget, self).__init__(**kwargs)
        
        # self.size_hint_x = None
        # self.size_hint_y = None
        # self.height = 100 
        # self.width = 400
        # self.bind(minimum_height=self.setter('height'))


        self.bind(pos = self.my_paint)
        self.bind(size = self.my_paint)

    def my_paint(self, *args):
        with self.canvas.before:
            #if self.bg:
            #    Color(0.2, 0.2, 0.2, mode = 'rgb')
            # else:
            #    Color(0.1, 0.1, 0.1, mode = 'rgb')

            Color(rgb=self.my_color)
            Rectangle(pos = self.pos, size = self.size)            

class MyColumnHeaderCorner(MyPanel):
    def __init__(self, **kwargs):
        super(MyColumnHeaderCorner, self).__init__(**kwargs)
        self.set_my_height(100)
        self.set_my_width(100)

class MyColumnHeaderPanel(MyPanel):
    def __init__(self, **kwargs):
        super(MyColumnHeaderPanel, self).__init__(**kwargs)
        self.set_my_height(100)

class MyRowHeaderPanel(MyPanel):
    def __init__(self, **kwargs):
        super(MyRowHeaderPanel, self).__init__(**kwargs)
        self.set_my_width(100)

class MyDataPanel(MyPanel):
    def __init__(self, **kwargs):
        super(MyDataPanel, self).__init__(**kwargs)

class MyInsertRowHeaderPanel(MyPanel):
    def __init__(self, **kwargs):
        super(MyInsertRowHeaderPanel, self).__init__(**kwargs)
        self.set_my_height(80)
        self.set_my_width(100)

class MyInsertDataPanel(MyPanel):
    def __init__(self, **kwargs):
        super(MyInsertDataPanel, self).__init__(**kwargs)
        self.set_my_height(80)

class MySummaryRowHeaderPanel(MyPanel):
    def __init__(self, **kwargs):
        super(MySummaryRowHeaderPanel, self).__init__(**kwargs)
        self.set_my_height(0)
        self.set_my_width(100)

class MySummaryDataPanel(MyPanel):
    def __init__(self, **kwargs):
        super(MySummaryDataPanel, self).__init__(**kwargs)
        self.set_my_height(0)

    

#class MyTableData:
#    calculate layout(resolution, options)
#    get number of: column header rows, data rows, summary rows 
#    get number of: row header columns, data columns 
#    get row height (panel) for: column header, data, summary and insert row
#    get column width for: each column

#    get header, summary or data value as string (panel, x, y)
#    set row header or data value as string (panel, x, y)

#    insert row
#    delete row
#    update row



class MainApp(App):
    def __init__(self, **kwargs):
        super(MainApp, self).__init__(**kwargs)

    def build(self):
        self.title = 'test'
        self.root = MyTable()
        return self.root


if __name__ == '__main__':
    MainApp().run()