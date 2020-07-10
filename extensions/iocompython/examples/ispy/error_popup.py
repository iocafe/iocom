from kivy.uix.popup import Popup
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.button import Button

class MyErrorPopup(Popup):
    def __init__(self, **kwargs):
        super(MyErrorPopup, self).__init__(**kwargs)
        self.size_hint=(None, None)
        self.size=(600, 150)

        grid = GridLayout()
        grid.cols = 1

        label = Label(text='?')
        self.mylabel = label
        grid.add_widget(label)

        b = Button(text='close')
        grid.add_widget(b)
        self.content = grid
        b.bind(on_release = self.dismiss)

    def error_message(self, message):
        self.title='error'
        self.mylabel.text = message
        self.open()

    def success_message(self, message):
        self.title='success'
        self.mylabel.text = message
        self.open()        