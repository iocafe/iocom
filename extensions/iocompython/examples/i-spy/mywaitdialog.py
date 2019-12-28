from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label

class MyWaitDialog(GridLayout):
    def __init__(self, **kwargs):
        super(MyWaitDialog, self).__init__(**kwargs)
        self.cols = 1
        self.height = self.minimum_height = 400
        self.size_hint_y = None
        self.bind(minimum_height=self.setter('height'))

        w = Label(text = 'waiting for connection...')
        w.size_hint_y = None
        w.height = 60 
        self.add_widget(w)

    def delete(self):
        pass

    def run(self):
        pass



