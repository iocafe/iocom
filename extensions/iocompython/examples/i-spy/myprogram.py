import json
from kivy.app import App
from kivy.config import ConfigParser
from kivy.uix.filechooser import FileChooserListView
from kivy.uix.filechooser import FileChooserIconView
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button

from iocompython import Root,bin2json, json2bin
from mysettings import MySettingsGroup, MyButton


class MyFileChooser(FileChooserIconView):
    def __init__(self, **kwargs):
        super(MyFileChooser, self).__init__(**kwargs)

    pass

class MyProgram(GridLayout):
# class MyProgram(MySettingsDisplay):
    def __init__(self, **kwargs):
        super(MyProgram, self).__init__(**kwargs)
        self.cols = 1

        g = GridLayout(cols = 2)
        g.height = 60
        g.size_hint_y = None
        title = MySettingsGroup()
        g.add_widget(title)
        self.my_title = title

        b = MyButton()
        b.setup_button("program flash", self)
        g.add_widget(b)

        self.add_widget(g)

        self.my_file_chooser  = MyFileChooser(path='/coderoot', size_hint=(1, 1), dirselect=True)
        self.add_widget(self.my_file_chooser)


    def set_device(self, ioc_root, device_path):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.my_title.set_group_label("program", self.device_path, 1)

    # Write the flash program to micro-controller
    def my_button_pressed(self, i):
        print("here")

        path = self.my_file_chooser.selection[0]
        with open(path, mode='rb') as file: # b is important -> binary
            file_content = file.read()
            rval = self.ioc_root.setconf(self.device_path, file_content, select=1)
            print(rval)

    def delete(self):
        pass

    def run(self):
        pass

class MyProgramButton(Button):
    def __init__(self, **kwargs):
        super(MyProgramButton, self).__init__(**kwargs)
        b = Button()
        # b.size_hint = (0.35, 1)
        self.add_widget(b)
        self.my_button = b

    def setup_button(self, text, signal_me):
        self.my_button.text = text
        if signal_me != None:
            self.my_button.bind(on_release = signal_me.my_button_pressed)

class MainApp(App):
    def build(self):
        self.root = MyProgram()
        self.root.set_device(Root('testwidget'), "gina2.iocafenet")
        return self.root

if __name__ == '__main__':
    MainApp().run()